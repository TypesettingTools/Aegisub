// Copyright (c) 2008-2009, Karl Blomster
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

/// @file ffmpegsource_common.cpp
/// @brief Shared code for ffms video and audio providers
/// @ingroup video_input audio_input ffms
///

#ifdef WITH_FFMS2
#include "ffmpegsource_common.h"

#include "compat.h"
#include "format.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/background_runner.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/string.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/crc.hpp>
#include <wx/intl.h>
#include <wx/choicdlg.h>

FFmpegSourceProvider::FFmpegSourceProvider(agi::BackgroundRunner *br)
: br(br)
{
	FFMS_Init(0, 0);
}

/// @brief Does indexing of a source file
/// @param Indexer		A pointer to the indexer object representing the file to be indexed
/// @param CacheName    The filename of the output index file
/// @param Trackmask    A binary mask of the track numbers to index
FFMS_Index *FFmpegSourceProvider::DoIndexing(FFMS_Indexer *Indexer,
	                                         std::filesystem::path const& CacheName,
	                                         TrackSelection Track,
	                                         FFMS_IndexErrorHandling IndexEH) {
	char FFMSErrMsg[1024];
	FFMS_ErrorInfo ErrInfo;
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;

	// index all audio tracks
	FFMS_Index *Index;
	br->Run([&](agi::ProgressSink *ps) {
		ps->SetTitle(from_wx(_("Indexing")));
		ps->SetMessage(from_wx(_("Reading timecodes and frame/sample data")));
		TIndexCallback callback = [](int64_t Current, int64_t Total, void *Private) -> int {
			auto ps = static_cast<agi::ProgressSink *>(Private);
			ps->SetProgress(Current, Total);
			return ps->IsCancelled();
		};
		if (Track == TrackSelection::All)
			FFMS_TrackTypeIndexSettings(Indexer, FFMS_TYPE_AUDIO, 1, 0);
		else if (Track != TrackSelection::None)
			FFMS_TrackIndexSettings(Indexer, static_cast<int>(Track), 1, 0);
		FFMS_TrackTypeIndexSettings(Indexer, FFMS_TYPE_VIDEO, 1, 0);
		FFMS_SetProgressCallback(Indexer, callback, ps);
		Index = FFMS_DoIndexing2(Indexer, IndexEH, &ErrInfo);
	});

	if (Index == nullptr)
		throw agi::EnvironmentError(std::string("Failed to index: ") + ErrInfo.Buffer);

	// write index to disk for later use
	FFMS_WriteIndex(CacheName.string().c_str(), Index, &ErrInfo);

	return Index;
}

/// @brief Finds all tracks of the given type and return their track numbers and respective codec names
/// @param Indexer	The indexer object representing the source file
/// @param Type		The track type to look for
/// @return			Returns a std::map with the track numbers as keys and the codec names as values.
std::map<int, std::string> FFmpegSourceProvider::GetTracksOfType(FFMS_Indexer *Indexer, FFMS_TrackType Type) {
	std::map<int,std::string> TrackList;
	int NumTracks = FFMS_GetNumTracksI(Indexer);

	for (int i=0; i<NumTracks; i++) {
		if (FFMS_GetTrackTypeI(Indexer, i) == Type) {
			if (auto CodecName = FFMS_GetCodecNameI(Indexer, i))
				TrackList[i] = CodecName;
		}
	}
	return TrackList;
}

FFmpegSourceProvider::TrackSelection
FFmpegSourceProvider::AskForTrackSelection(const std::map<int, std::string> &TrackList,
                                           FFMS_TrackType Type) {
	std::vector<int> TrackNumbers;
	wxArrayString Choices;

	for (auto const& track : TrackList) {
		Choices.Add(agi::wxformat(_("Track %02d: %s"), track.first, track.second));
		TrackNumbers.push_back(track.first);
	}

	int Choice = wxGetSingleChoiceIndex(
		Type == FFMS_TYPE_VIDEO ? _("Multiple video tracks detected, please choose the one you wish to load:") : _("Multiple audio tracks detected, please choose the one you wish to load:"),
		Type == FFMS_TYPE_VIDEO ? _("Choose video track") : _("Choose audio track"),
		Choices);

	if (Choice < 0)
		return TrackSelection::None;
	return static_cast<TrackSelection>(TrackNumbers[Choice]);
}

/// @brief Set ffms2 log level according to setting in config.dat
void FFmpegSourceProvider::SetLogLevel() {
	auto LogLevel = OPT_GET("Provider/FFmpegSource/Log Level")->GetString();
	boost::to_lower(LogLevel);

	if (LogLevel == "panic")
		FFMS_SetLogLevel(FFMS_LOG_PANIC);
	else if (LogLevel == "fatal")
		FFMS_SetLogLevel(FFMS_LOG_FATAL);
	else if (LogLevel == "error")
		FFMS_SetLogLevel(FFMS_LOG_ERROR);
	else if (LogLevel == "warning")
		FFMS_SetLogLevel(FFMS_LOG_WARNING);
	else if (LogLevel == "info")
		FFMS_SetLogLevel(FFMS_LOG_INFO);
	else if (LogLevel == "verbose")
		FFMS_SetLogLevel(FFMS_LOG_VERBOSE);
	else if (LogLevel == "debug")
		FFMS_SetLogLevel(FFMS_LOG_DEBUG);
	else
		FFMS_SetLogLevel(FFMS_LOG_QUIET);
}

FFMS_IndexErrorHandling FFmpegSourceProvider::GetErrorHandlingMode() {
	auto Mode = OPT_GET("Provider/Audio/FFmpegSource/Decode Error Handling")->GetString();
	boost::to_lower(Mode);

	if (Mode == "ignore")
		return FFMS_IEH_IGNORE;
	if (Mode == "clear")
		return FFMS_IEH_CLEAR_TRACK;
	if (Mode == "stop")
		return FFMS_IEH_STOP_TRACK;
	if (Mode == "abort")
		return FFMS_IEH_ABORT;
	return FFMS_IEH_STOP_TRACK; // questionable default?
}

/// @brief	Generates an unique name for the ffms2 index file and prepares the cache folder if it doesn't exist
/// @param filename	The name of the source file
/// @return			Returns the generated filename.
std::filesystem::path FFmpegSourceProvider::GetCacheFilename(std::filesystem::path const& filename) {
	// Get the size of the file to be hashed
	uintmax_t len = agi::fs::Size(filename);

	// Get the hash of the filename
	boost::crc_32_type hash;
	hash.process_bytes(filename.string().c_str(), filename.string().size());

	// Generate the filename
	auto modified_time = std::chrono::duration_cast<std::chrono::seconds>(agi::fs::ModifiedTime(filename).time_since_epoch()).count();
	auto result = config::path->Decode(agi::Str("?local/ffms2cache/", std::to_string(hash.checksum()), "_", std::to_string(len), "_", std::to_string(modified_time), ".ffindex"));

	// Ensure that folder exists
	agi::fs::CreateDirectory(result.parent_path());

	return result;
}

void FFmpegSourceProvider::CleanCache() {
	::CleanCache(config::path->Decode("?local/ffms2cache/"),
		"*.ffindex",
		OPT_GET("Provider/FFmpegSource/Cache/Size")->GetInt(),
		OPT_GET("Provider/FFmpegSource/Cache/Files")->GetInt());
}

#endif // WITH_FFMS2
