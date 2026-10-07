// Copyright (c) 2004-2006, Rodrigo Braz Monteiro, Mike Matsnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file mkv_wrap.cpp
/// @brief High-level interface for obtaining various data from Matroska files
/// @ingroup video_input
///

#include "mkv_wrap.h"

#include "ass_file.h"
#include "ass_parser.h"
#include "compat.h"
#include "dialog_progress.h"
#include "matroska.h"
#include "options.h"
#include "subtitle_format_srt.h"

#include <libaegisub/ass/time.h>
#include <libaegisub/format.h>

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <exception>
#include <functional>
#include <limits>
#include <optional>

#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.

namespace {
namespace mkv = agi::matroska;

/// Limit on the total size of the subtitle data read from a file
constexpr size_t max_total_subtitle_bytes = 64 * 1024 * 1024;

agi::Time to_ass_time(std::optional<mkv::Timestamp> const& time) {
	if (!time) return 0;
	if (time->nanoseconds < 0)
		throw MatroskaException("Negative Matroska subtitle timestamp");
	auto milliseconds = time->nanoseconds / 1000000;
	if (milliseconds > std::numeric_limits<int>::max())
		throw MatroskaException("Matroska subtitle timestamp is out of range");
	return static_cast<int>(milliseconds);
}

void read_subtitles(agi::ProgressSink *ps, mkv::Demuxer& demuxer, bool srt, int64_t total_time, AssParser *parser) {
	std::vector<std::pair<int, std::string>> subList;
	size_t total_bytes = 0;
	SrtTagParser srtParser;

	while (auto packet = demuxer.ReadPacket()) {
		if (ps->IsCancelled()) return;
		if (packet->data.empty()) continue;

		if (packet->data.size() > max_total_subtitle_bytes - total_bytes)
			throw MatroskaException(agi::format("Matroska subtitle data exceeds the %d MiB limit", max_total_subtitle_bytes / 1024 / 1024));
		total_bytes += packet->data.size();

		agi::Time subStart = to_ass_time(packet->start);
		agi::Time subEnd = packet->end ? to_ass_time(packet->end) : subStart;
		std::string_view readBuf(reinterpret_cast<const char *>(packet->data.data()), packet->data.size());

		// Process SSA/ASS
		if (!srt) {
			auto first = readBuf.find(',');
			if (first == readBuf.npos) continue;
			auto second = readBuf.find(',', first + 1);
			if (second == readBuf.npos) continue;

			try {
				subList.emplace_back(
					boost::lexical_cast<int>(readBuf.substr(0, first)),
					agi::format("Dialogue: %d,%s,%s,%s"
						, boost::lexical_cast<int>(readBuf.substr(first + 1, second - (first + 1)))
						, subStart.GetAssFormatted()
						, subEnd.GetAssFormatted()
						, readBuf.substr(second + 1)));
			}
			catch (boost::bad_lexical_cast const&) {
				throw MatroskaException("Malformed ASS packet in Matroska subtitle track");
			}
		}
		// Process SRT
		else {
			auto line = agi::format("Dialogue: 0,%s,%s,Default,,0,0,0,,%s"
				, subStart.GetAssFormatted()
				, subEnd.GetAssFormatted()
				, srtParser.ToAss(std::string(readBuf)));
			boost::replace_all(line, "\r\n", "\\N");
			boost::replace_all(line, "\r", "\\N");
			boost::replace_all(line, "\n", "\\N");

			subList.emplace_back(subList.size(), std::move(line));
		}

		if (total_time > 0)
			ps->SetProgress(subStart, total_time);
	}

	// Insert into file
	sort(begin(subList), end(subList));
	for (auto const& order_value_pair : subList)
		parser->AddLine(order_value_pair.second);
}

/// Run a task in a progress dialog, rethrowing any error it fails with
/// rather than just logging it to the dialog
void run_with_progress(wxString const& message, agi::ProgressSink *&active_sink, std::function<void(agi::ProgressSink *)> task) {
	DialogProgress progress(nullptr, _("Parsing Matroska"), message);
	std::exception_ptr failure;
	progress.Run([&](agi::ProgressSink *ps) {
		active_sink = ps;
		try {
			task(ps);
		}
		catch (...) {
			failure = std::current_exception();
		}
		// The sink is destroyed when Run returns
		active_sink = nullptr;
	});
	if (failure)
		std::rethrow_exception(failure);
}
}

void MatroskaWrapper::GetSubtitles(agi::fs::path const& filename, AssFile *target) {
	// The demuxer polls for cancellation from whichever progress dialog is
	// currently running it
	agi::ProgressSink *active_sink = nullptr;
	auto cancelled = [&] { return active_sink && active_sink->IsCancelled(); };

	std::optional<mkv::Demuxer> demuxer;
	run_with_progress(_("Reading Matroska track information."), active_sink, [&](agi::ProgressSink *) {
		demuxer.emplace(mkv::OpenFile(filename), cancelled);
	});

	// Find tracks
	std::vector<mkv::SubtitleTrack const *> tracksFound;
	std::vector<std::string> tracksNames;
	for (auto const& track : demuxer->SubtitleTracks()) {
		if (track.codec == mkv::SubtitleCodec::unsupported) continue;
		tracksFound.push_back(&track);
		tracksNames.emplace_back(agi::format("%d (%s %s)", track.id.value, track.codec_id, track.language));
		if (!track.name.empty()) {
			tracksNames.back() += ": ";
			tracksNames.back() += track.name;
		}
	}

	// No tracks found
	if (tracksFound.empty())
		throw MatroskaException("File has no recognised subtitle tracks.");

	mkv::SubtitleTrack const *trackToRead;
	// Only one track found
	if (tracksFound.size() == 1)
		trackToRead = tracksFound[0];
	// Pick a track
	else {
		int choice = wxGetSingleChoiceIndex(_("Choose which track to read:"), _("Multiple subtitle tracks found"), to_wx(tracksNames));
		if (choice == -1)
			throw agi::UserCancelException("canceled");

		trackToRead = tracksFound[choice];
	}

	// Picked track
	demuxer->SelectTrack(trackToRead->id);
	bool srt = trackToRead->codec == mkv::SubtitleCodec::srt;
	bool ssa = trackToRead->codec == mkv::SubtitleCodec::ssa;

	// Parse into a temporary file so a failure partway through leaves the target untouched
	AssFile imported;
	AssParser parser(&imported, !ssa);

	// Read private data if it's ASS/SSA
	if (!srt) {
		// Read raw data
		std::string priv(trackToRead->codec_private.begin(), trackToRead->codec_private.end());

		// Load into file
		boost::char_separator<char> sep("\r\n");
		for (auto const& cur : boost::tokenizer<boost::char_separator<char>>(priv, sep))
			parser.AddLine(cur);
	}
	// Load default if it's SRT
	else
		imported.LoadDefault(false, OPT_GET("Subtitle Format/SRT/Default Style Catalog")->GetString());

	parser.AddLine("[Events]");

	auto duration = demuxer->Duration();
	int64_t totalTime = duration ? duration->nanoseconds / 1000000 : 0;
	run_with_progress(_("Reading subtitles from Matroska file."), active_sink, [&](agi::ProgressSink *ps) {
		read_subtitles(ps, *demuxer, srt, totalTime, &parser);
	});

	target->swap(imported);
}

bool MatroskaWrapper::HasSubtitles(agi::fs::path const& filename) {
	try {
		mkv::Demuxer demuxer(mkv::OpenFile(filename));
		auto const& tracks = demuxer.SubtitleTracks();
		return std::any_of(tracks.begin(), tracks.end(), [](mkv::SubtitleTrack const& track) {
			return track.codec != mkv::SubtitleCodec::unsupported;
		});
	}
	catch (...) {
		// We don't care about why we couldn't read subtitles here
	}

	return false;
}
