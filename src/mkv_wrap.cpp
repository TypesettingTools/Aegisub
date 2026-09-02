// Copyright (c) 2004-2006, Rodrigo Braz Monteiro, Mike Matsnev
// Copyright (c) 2026 Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any purpose
// with or without fee is hereby granted.

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
#include <limits>
#include <optional>
#include <wx/choicdlg.h>

namespace {
agi::Time to_ass_time(std::optional<agi::matroska::Timestamp> const& value) {
	if (!value) return agi::Time{};
	if (value->nanoseconds < 0) throw MatroskaException("Negative Matroska subtitle timestamp");
	auto milliseconds = value->nanoseconds / 1000000;
	if (milliseconds > std::numeric_limits<int>::max()) throw MatroskaException("Matroska subtitle timestamp is out of range");
	return agi::Time(static_cast<int>(milliseconds));
}

void read_subtitles(agi::ProgressSink *ps, agi::matroska::Demuxer& demuxer,
	agi::matroska::SubtitleTrack const& track, double total_time, AssParser *parser) {
	std::vector<std::pair<int, std::string>> subtitles;
	SrtTagParser srt_parser;
	for (;;) {
		auto packet = demuxer.ReadPacket();
		if (!packet) break;
		if (packet->data.empty()) continue;
		auto start = to_ass_time(packet->start);
		auto end = packet->end ? to_ass_time(packet->end) : start;
		std::string_view data(reinterpret_cast<char const *>(packet->data.data()), packet->data.size());
		if (track.codec != agi::matroska::SubtitleCodec::srt) {
			auto first = data.find(',');
			auto second = first == data.npos ? data.npos : data.find(',', first + 1);
			if (second == data.npos) continue;
			subtitles.emplace_back(boost::lexical_cast<int>(data.substr(0, first)),
				agi::format("Dialogue: %d,%s,%s,%s", boost::lexical_cast<int>(data.substr(first + 1, second - first - 1)),
					start.GetAssFormatted(), end.GetAssFormatted(), data.substr(second + 1)));
		}
		else {
			auto line = agi::format("Dialogue: 0,%s,%s,Default,,0,0,0,,%s", start.GetAssFormatted(), end.GetAssFormatted(), srt_parser.ToAss(std::string(data)));
			boost::replace_all(line, "\r\n", "\\N"); boost::replace_all(line, "\r", "\\N"); boost::replace_all(line, "\n", "\\N");
			subtitles.emplace_back(subtitles.size(), std::move(line));
		}
		ps->SetProgress(static_cast<int>(start), total_time);
	}
	std::sort(subtitles.begin(), subtitles.end());
	for (auto const& item : subtitles) parser->AddLine(item.second);
}
}

void MatroskaWrapper::GetSubtitles(agi::fs::path const& filename, AssFile *target) {
	DialogProgress metadata_progress(nullptr, _("Parsing Matroska"), _("Reading Matroska track information."));
	std::optional<agi::matroska::Demuxer> demuxer;
	metadata_progress.Run([&](agi::ProgressSink *ps) {
		demuxer.emplace(agi::matroska::OpenFile(filename), [ps] { return ps->IsCancelled(); });
	});
	std::vector<agi::matroska::SubtitleTrack const *> tracks;
	std::vector<std::string> names;
	for (auto const& track : demuxer->SubtitleTracks()) if (track.codec != agi::matroska::SubtitleCodec::unsupported) {
		tracks.push_back(&track);
		names.emplace_back(agi::format("%d (%s %s)%s%s", track.id.value, track.codec_id, track.language, track.name.empty() ? "" : ": ", track.name));
	}
	if (tracks.empty()) throw MatroskaException("File has no recognised subtitle tracks.");
	size_t choice = 0;
	if (tracks.size() > 1) {
		int selected = wxGetSingleChoiceIndex(_("Choose which track to read:"), _("Multiple subtitle tracks found"), to_wx(names));
		if (selected < 0) throw agi::UserCancelException("cancelled");
		choice = static_cast<size_t>(selected);
	}
	auto const& track = *tracks[choice];
	demuxer->SelectTrack(track.id);
	bool srt = track.codec == agi::matroska::SubtitleCodec::srt;
	bool ssa = track.codec == agi::matroska::SubtitleCodec::ssa;
	AssFile imported;
	AssParser parser(&imported, !ssa);
	if (!srt) {
		std::string private_data(track.codec_private.begin(), track.codec_private.end());
		boost::char_separator<char> separator("\r\n");
		for (auto const& line : boost::tokenizer<boost::char_separator<char>>(private_data, separator)) parser.AddLine(line);
	}
	else imported.LoadDefault(false, OPT_GET("Subtitle Format/SRT/Default Style Catalog")->GetString());
	parser.AddLine("[Events]");
	double duration = demuxer->Duration() ? demuxer->Duration()->nanoseconds / 1000000.0 : 0.0;
	DialogProgress progress(nullptr, _("Parsing Matroska"), _("Reading subtitles from Matroska file."));
	std::exception_ptr failure;
	progress.Run([&](agi::ProgressSink *ps) {
		try {
			read_subtitles(ps, *demuxer, track, duration, &parser);
		}
		catch (...) { failure = std::current_exception(); }
	});
	if (failure) std::rethrow_exception(failure);
	target->swap(imported);
}

bool MatroskaWrapper::HasSubtitles(agi::fs::path const& filename) {
	// Compatibility: project loading expects a boolean probe. Corrupt/unreadable files
	// therefore return false here; explicit imports preserve their structured error.
	try {
		agi::matroska::Demuxer demuxer(agi::matroska::OpenFile(filename));
		return std::any_of(demuxer.SubtitleTracks().begin(), demuxer.SubtitleTracks().end(), [](auto const& track) { return track.codec != agi::matroska::SubtitleCodec::unsupported; });
	}
	catch (...) { return false; }
}
