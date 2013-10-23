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

#include "config.h"

#include "mkv_wrap.h"

#include "ass_file.h"
#include "ass_parser.h"
#include "ass_time.h"
#include "compat.h"
#include "dialog_progress.h"
#include "MatroskaParser.h"

#include <libaegisub/fs.h>
#include <libaegisub/scoped_ptr.h>

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/irange.hpp>
#include <boost/tokenizer.hpp>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <iterator>

#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.

class MkvStdIO : public InputStream {
public:
	MkvStdIO(agi::fs::path const& filename);
	~MkvStdIO() { if (fp) fclose(fp); }

	FILE *fp;
	int error;
};

#define CACHESIZE     1024

#ifdef __VISUALC__
#define std_fseek _fseeki64
#define std_ftell _ftelli64
#else
#define std_fseek fseeko
#define std_ftell ftello
#endif

static void read_subtitles(agi::ProgressSink *ps, MatroskaFile *file, MkvStdIO *input, bool srt, double totalTime, AssParser *parser) {
	std::vector<std::pair<int, std::string>> subList;
	std::string readBuf;

	// Load blocks
	ulonglong startTime, endTime, filePos;
	unsigned int rt, frameSize, frameFlags;

	while (mkv_ReadFrame(file, 0, &rt, &startTime, &endTime, &filePos, &frameSize, &frameFlags) == 0) {
		if (ps->IsCancelled()) return;
		if (frameSize == 0) continue;

		readBuf.resize(frameSize);
		std_fseek(input->fp, filePos, SEEK_SET);
		fread(&readBuf[0], 1, frameSize, input->fp);

		// Get start and end times
		longlong timecodeScaleLow = 1000000;
		AssTime subStart = startTime / timecodeScaleLow;
		AssTime subEnd = endTime / timecodeScaleLow;

		// Process SSA/ASS
		if (!srt) {
			std::vector<boost::iterator_range<std::string::iterator>> chunks;
			boost::split(chunks, readBuf, boost::is_any_of(","));

			subList.emplace_back(
				boost::lexical_cast<int>(chunks[0]),
				str(boost::format("Dialogue: %d,%s,%s,%s")
					% boost::lexical_cast<int>(chunks[1])
					% subStart.GetAssFormated()
					% subEnd.GetAssFormated()
					% boost::make_iterator_range(begin(chunks[2]), readBuf.end())));
		}
		// Process SRT
		else {
			readBuf = str(boost::format("Dialogue: 0,%s,%s,Default,,0,0,0,,%s") % subStart.GetAssFormated() % subEnd.GetAssFormated() % readBuf);
			boost::replace_all(readBuf, "\r\n", "\\N");
			boost::replace_all(readBuf, "\r", "\\N");
			boost::replace_all(readBuf, "\n", "\\N");

			subList.emplace_back(subList.size(), readBuf);
		}

		ps->SetProgress(startTime / timecodeScaleLow, totalTime);
	}

	// Insert into file
	sort(begin(subList), end(subList));
	for (auto order_value_pair : subList)
		parser->AddLine(order_value_pair.second);
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
			tracksNames.emplace_back(str(boost::format("%d (%s %s)") % track % CodecID % trackInfo->Language));
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
	else {
		target->LoadDefault(false);
		parser.AddLine("[Events]");
	}

	// Read timecode scale
	auto segInfo = mkv_GetFileInfo(file);
	longlong timecodeScale = mkv_TruncFloat(trackInfo->TimecodeScale) * segInfo->TimecodeScale;

	// Progress bar
	auto totalTime = double(segInfo->Duration) / timecodeScale;
	DialogProgress progress(nullptr, _("Parsing Matroska"), _("Reading subtitles from Matroska file."));
	progress.Run([&](agi::ProgressSink *ps) { read_subtitles(ps, file, &input, srt, totalTime, &parser); });
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

int StdIoRead(InputStream *_st, ulonglong pos, void *buffer, int count) {
	auto *st = static_cast<MkvStdIO*>(_st);
	if (std_fseek(st->fp, pos, SEEK_SET)) {
		st->error = errno;
		return -1;
	}

	auto rd = fread(buffer, 1, count, st->fp);
	if (rd == 0) {
		if (feof(st->fp))
			return 0;
		st->error = errno;
		return -1;
	}
	return rd;
}

/// @brief scan for a signature sig(big-endian) starting at file position pos
/// @return position of the first byte of signature or -1 if error/not found
longlong StdIoScan(InputStream *st, ulonglong start, unsigned signature) {
	FILE *fp = static_cast<MkvStdIO*>(st)->fp;

	if (std_fseek(fp, start, SEEK_SET))
		return -1;

	int c;
	unsigned cmp = 0;
	while ((c = getc(fp)) != EOF) {
		cmp = ((cmp << 8) | c) & 0xffffffff;
		if (cmp == signature)
			return std_ftell(fp) - 4;
	}

	return -1;
}

longlong StdIoGetFileSize(InputStream *st) {
	auto fp = static_cast<MkvStdIO*>(st)->fp;
	auto cpos = std_ftell(fp);
	std_fseek(fp, 0, SEEK_END);
	auto epos = std_ftell(fp);
	std_fseek(fp, cpos, SEEK_SET);
	return epos;
}

MkvStdIO::MkvStdIO(agi::fs::path const& filename)
: error(0)
{
	read = StdIoRead;
	scan = StdIoScan;
	getcachesize = [](InputStream *) -> unsigned int { return CACHESIZE; };
	geterror = [](InputStream *st) -> const char * { return strerror(((MkvStdIO *)st)->error); };
	memalloc = [](InputStream *, size_t size) { return malloc(size); };
	memrealloc = [](InputStream *, void *mem, size_t size) { return realloc(mem, size); };
	memfree = [](InputStream *, void *mem) { free(mem); };
	progress = [](InputStream *, ulonglong, ulonglong) { return 1; };
	getfilesize = StdIoGetFileSize;

#ifdef __VISUALC__
	fp = _wfopen(filename.c_str(), L"rb");
#else
	fp = fopen(filename.c_str(), "rb");
#endif
	if (!fp)
		throw agi::fs::FileNotFound(filename);

	setvbuf(fp, nullptr, _IOFBF, CACHESIZE);
}
