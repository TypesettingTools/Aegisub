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
#include "MatroskaParser.h"
#include "options.h"

#include <libaegisub/ass/time.h>
#include <libaegisub/file_mapping.h>
#include <libaegisub/format.h>
#include <libaegisub/scoped_ptr.h>

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/irange.hpp>
#include <boost/tokenizer.hpp>
#include <iterator>

#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.

struct MkvStdIO final : InputStream {
	agi::read_file_mapping file;
	std::string error;

	static int Read(InputStream *st, uint64_t pos, void *buffer, int count) {
		auto *self = static_cast<MkvStdIO*>(st);
		if (pos >= self->file.size())
			return 0;

		auto remaining = self->file.size() - pos;
		if (remaining < INT_MAX)
			count = std::min(static_cast<int>(remaining), count);

		try {
			memcpy(buffer, self->file.read(pos, count), count);
		}
		catch (agi::Exception const& e) {
			self->error = e.GetMessage();
			return -1;
		}

		return count;
	}

	static int64_t Scan(InputStream *st, uint64_t start, unsigned signature) {
		auto *self = static_cast<MkvStdIO*>(st);
		try {
			unsigned cmp = 0;
			for (auto i : boost::irange(start, self->file.size())) {
				int c = *self->file.read(i, 1);
				cmp = ((cmp << 8) | c) & 0xffffffff;
				if (cmp == signature)
					return i - 4;
			}
		}
		catch (agi::Exception const& e) {
			self->error = e.GetMessage();
		}

		return -1;
	}

	static int64_t Size(InputStream *st) {
		return static_cast<MkvStdIO*>(st)->file.size();
	}

	MkvStdIO(agi::fs::path const& filename) : file(filename) {
		read = &MkvStdIO::Read;
		scan = &MkvStdIO::Scan;
		getcachesize = [](InputStream *) -> unsigned int { return 16 * 1024 * 1024; };
		geterror = [](InputStream *st) -> const char * { return ((MkvStdIO *)st)->error.c_str(); };
		memalloc = [](InputStream *, size_t size) { return malloc(size); };
		memrealloc = [](InputStream *, void *mem, size_t size) { return realloc(mem, size); };
		memfree = [](InputStream *, void *mem) { free(mem); };
		progress = [](InputStream *, uint64_t, uint64_t) { return 1; };
		getfilesize = &MkvStdIO::Size;
	}
};

static bool read_subtitles(agi::ProgressSink *ps, MatroskaFile *file, MkvStdIO *input, bool srt, double totalTime, AssParser *parser, CompressedStream *cs) {
	std::vector<std::pair<int, std::string>> subList;

	// Load blocks
	uint64_t startTime, endTime, filePos;
	unsigned int rt, frameSize, frameFlags;

	std::vector<char> uncompBuf(cs ? 256 : 0);

	while (mkv_ReadFrame(file, 0, &rt, &startTime, &endTime, &filePos, &frameSize, &frameFlags) == 0) {
		if (ps->IsCancelled()) return true;
		if (frameSize == 0) continue;

		std::string_view readBuf;

		if (cs) {
			cs_NextFrame(cs, filePos, frameSize);
			int bytesRead = 0;

			int res;
			do {
				res = cs_ReadData(cs, &uncompBuf[bytesRead], uncompBuf.size() - bytesRead);
				if (res == -1) {
					const char *err = cs_GetLastError(cs);
					if (!err) err = "Unknown error";
					ps->Log("Failed to decompress subtitles: " + std::string(err));
					return false;
				}

				bytesRead += res;

				if (bytesRead >= uncompBuf.size())
					uncompBuf.resize(2 * uncompBuf.size());
			} while (res != 0);

			readBuf = std::string_view(&uncompBuf[0], bytesRead);
		} else {
			readBuf = std::string_view(input->file.read(filePos, frameSize), frameSize);
		}

		// Get start and end times
		int64_t timecodeScaleLow = 1000000;
		agi::Time subStart = startTime / timecodeScaleLow;
		agi::Time subEnd = endTime / timecodeScaleLow;

		// Process SSA/ASS
		if (!srt) {
			auto first = readBuf.find(',');
			if (first == readBuf.npos) continue;
			auto second = readBuf.find(',', first + 1);
			if (second == readBuf.npos) continue;

			subList.emplace_back(
				boost::lexical_cast<int>(readBuf.substr(0, first)),
				agi::format("Dialogue: %d,%s,%s,%s"
					, boost::lexical_cast<int>(readBuf.substr(first + 1, second - (first + 1)))
					, subStart.GetAssFormatted()
					, subEnd.GetAssFormatted()
					, readBuf.substr(second + 1)));
		}
		// Process SRT
		else {
			auto line = agi::format("Dialogue: 0,%s,%s,Default,,0,0,0,,%s"
				, subStart.GetAssFormatted()
				, subEnd.GetAssFormatted()
				, readBuf);
			boost::replace_all(line, "\r\n", "\\N");
			boost::replace_all(line, "\r", "\\N");
			boost::replace_all(line, "\n", "\\N");

			subList.emplace_back(subList.size(), std::move(line));
		}

		ps->SetProgress(startTime / timecodeScaleLow, totalTime);
	}

	// Insert into file
	sort(begin(subList), end(subList));
	for (auto order_value_pair : subList)
		parser->AddLine(order_value_pair.second);
	return true;
}

void MatroskaWrapper::GetSubtitles(agi::fs::path const& filename, AssFile *target) {
	MkvStdIO input(filename);
	char err[2048];
	agi::scoped_holder<MatroskaFile*, decltype(&mkv_Close)> file(mkv_Open(&input, err, sizeof(err)), mkv_Close);
	if (!file) throw MatroskaException(err);

	// Get info
	unsigned tracks = mkv_GetNumTracks(file);
	std::vector<unsigned> tracksFound;
	std::vector<std::string> tracksNames;

	// Find tracks
	for (auto track : boost::irange(0u, tracks)) {
		auto trackInfo = mkv_GetTrackInfo(file, track);
		if (trackInfo->Type != 0x11) continue;

		// Known subtitle format
		std::string CodecID(trackInfo->CodecID);
		if (CodecID == "S_TEXT/SSA" || CodecID == "S_TEXT/ASS" || CodecID == "S_TEXT/UTF8") {
			tracksFound.push_back(track);
			tracksNames.emplace_back(agi::format("%d (%s %s)", track, CodecID, trackInfo->Language));
			if (trackInfo->Name) {
				tracksNames.back() += ": ";
				tracksNames.back() += trackInfo->Name;
			}
		}
	}

	// No tracks found
	if (tracksFound.empty())
		throw MatroskaException("File has no recognised subtitle tracks.");

	unsigned trackToRead;
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
	mkv_SetTrackMask(file, ~(1 << trackToRead));
	auto trackInfo = mkv_GetTrackInfo(file, trackToRead);
	std::string CodecID(trackInfo->CodecID);
	bool srt = CodecID == "S_TEXT/UTF8";
	bool ssa = CodecID == "S_TEXT/SSA";

	AssParser parser(target, !ssa);

	// Read private data if it's ASS/SSA
	if (!srt) {
		// Read raw data
		std::string priv((const char *)trackInfo->CodecPrivate, trackInfo->CodecPrivateSize);

		// Load into file
		boost::char_separator<char> sep("\r\n");
		for (auto const& cur : boost::tokenizer<boost::char_separator<char>>(priv, sep))
			parser.AddLine(cur);
	}
	// Load default if it's SRT
	else
		target->LoadDefault(false, OPT_GET("Subtitle Format/SRT/Default Style Catalog")->GetString());

	parser.AddLine("[Events]");

	agi::scoped_holder<CompressedStream *, decltype(&cs_Destroy)> cs(nullptr, cs_Destroy);
	if (trackInfo->CompEnabled) {
		cs = cs_Create(file, trackToRead, err, sizeof(err));
		if (!cs)
			throw MatroskaException(err);
	}

	// Read timecode scale
	auto segInfo = mkv_GetFileInfo(file);
	int64_t timecodeScale = mkv_TruncFloat(trackInfo->TimecodeScale) * segInfo->TimecodeScale;

	// Progress bar
	auto totalTime = double(segInfo->Duration) / timecodeScale;
	DialogProgress progress(nullptr, _("Parsing Matroska"), _("Reading subtitles from Matroska file."));
	bool result;
	progress.Run([&](agi::ProgressSink *ps) { result = read_subtitles(ps, file, &input, srt, totalTime, &parser, cs); });

	if (!result)
		throw MatroskaException("Failed to read subtitles");
}

bool MatroskaWrapper::HasSubtitles(agi::fs::path const& filename) {
	char err[2048];
	try {
		MkvStdIO input(filename);
		agi::scoped_holder<MatroskaFile*, decltype(&mkv_Close)> file(mkv_Open(&input, err, sizeof(err)), mkv_Close);
		if (!file) return false;

		// Find tracks
		auto tracks = mkv_GetNumTracks(file);
		for (auto track : boost::irange(0u, tracks)) {
			auto trackInfo = mkv_GetTrackInfo(file, track);

			if (trackInfo->Type == 0x11) {
				std::string CodecID(trackInfo->CodecID);
				if (CodecID == "S_TEXT/SSA" || CodecID == "S_TEXT/ASS" || CodecID == "S_TEXT/UTF8")
					return true;
			}
		}
	}
	catch (...) {
		// We don't care about why we couldn't read subtitles here
	}

	return false;
}
