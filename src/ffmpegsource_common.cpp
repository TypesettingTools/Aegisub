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

#include "config.h"

#ifdef WITH_FFMS2
#include "ffmpegsource_common.h"

#include "compat.h"
#include "dialog_progress.h"
#include "frame_main.h"
#include "main.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/log.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
#include <inttypes.h>

#include <wx/config.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.

#ifdef _WIN32
#include <objbase.h>

static void deinit_com(bool) {
	CoUninitialize();
}
#else
static void deinit_com(bool) { }
#endif

FFmpegSourceProvider::FFmpegSourceProvider()
: COMInited(false, deinit_com)
{
#ifdef _WIN32
	HRESULT res = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(res))
		COMInited = true;
	else if (res != RPC_E_CHANGED_MODE)
		throw "COM initialization failure";
#endif

	// initialize ffmpegsource
	FFMS_Init(0, 1);
}

/// @brief Does indexing of a source file
/// @param Indexer		A pointer to the indexer object representing the file to be indexed
/// @param CacheName    The filename of the output index file
/// @param Trackmask    A binary mask of the track numbers to index
/// @param IgnoreDecodeErrors	True if audio decoding errors will be tolerated, false otherwise
/// @return				Returns the index object on success, nullptr otherwise
///
FFMS_Index *FFmpegSourceProvider::DoIndexing(FFMS_Indexer *Indexer, agi::fs::path const& CacheName, int Trackmask, FFMS_IndexErrorHandling IndexEH) {
	char FFMSErrMsg[1024];
	FFMS_ErrorInfo ErrInfo;
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;
	std::string MsgString;

	// set up progress dialog callback
	DialogProgress Progress(wxGetApp().frame, _("Indexing"), _("Reading timecodes and frame/sample data"));

	// index all audio tracks
	FFMS_Index *Index;
	Progress.Run([&](agi::ProgressSink *ps) {
		struct progress {
			agi::ProgressSink *ps;
			int calls;
		};
		progress state = { ps, 0 };
		TIndexCallback callback = [](int64_t Current, int64_t Total, void *Private) -> int {
			auto state = static_cast<progress *>(Private);
			if (++state->calls % 10 == 0)
				state->ps->SetProgress(Current, Total);
			return state->ps->IsCancelled();
		};
		Index = FFMS_DoIndexing(Indexer, Trackmask, FFMS_TRACKMASK_NONE,
			nullptr, nullptr, IndexEH, callback, &state, &ErrInfo);
	});

	if (Index == nullptr) {
		MsgString += "Failed to index: ";
		MsgString += ErrInfo.Buffer;
		throw MsgString;
	}

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
			const char *CodecName = FFMS_GetCodecNameI(Indexer, i);
			if (CodecName)
				TrackList.insert(std::pair<int,std::string>(i, CodecName));
		}
	}
	return TrackList;
}

/// @brief Ask user for which track he wants to load
/// @param TrackList	A std::map with the track numbers as keys and codec names as values
/// @param Type			The track type to ask about
/// @return				Returns the track number chosen (an integer >= 0) on success, or a negative integer if the user cancelled.
int FFmpegSourceProvider::AskForTrackSelection(const std::map<int, std::string> &TrackList, FFMS_TrackType Type) {
	std::vector<int> TrackNumbers;
	wxArrayString Choices;

	for (auto const& track : TrackList) {
		Choices.Add(wxString::Format(_("Track %02d: %s"), track.first, to_wx(track.second)));
		TrackNumbers.push_back(track.first);
	}

	int Choice = wxGetSingleChoiceIndex(
		Type == FFMS_TYPE_VIDEO ? _("Multiple video tracks detected, please choose the one you wish to load:") : _("Multiple audio tracks detected, please choose the one you wish to load:"),
		Type == FFMS_TYPE_VIDEO ? _("Choose video track") : _("Choose audio track"),
		Choices);

	if (Choice < 0)
		return Choice;
	else
		return TrackNumbers[Choice];
}

/// @brief Set ffms2 log level according to setting in config.dat
void FFmpegSourceProvider::SetLogLevel() {
	std::string LogLevel = OPT_GET("Provider/FFmpegSource/Log Level")->GetString();

	if (boost::iequals(LogLevel, "panic"))
		FFMS_SetLogLevel(FFMS_LOG_PANIC);
	else if (boost::iequals(LogLevel, "fatal"))
		FFMS_SetLogLevel(FFMS_LOG_FATAL);
	else if (boost::iequals(LogLevel, "error"))
		FFMS_SetLogLevel(FFMS_LOG_ERROR);
	else if (boost::iequals(LogLevel, "warning"))
		FFMS_SetLogLevel(FFMS_LOG_WARNING);
	else if (boost::iequals(LogLevel, "info"))
		FFMS_SetLogLevel(FFMS_LOG_INFO);
	else if (boost::iequals(LogLevel, "verbose"))
		FFMS_SetLogLevel(FFMS_LOG_VERBOSE);
	else if (boost::iequals(LogLevel, "debug"))
		FFMS_SetLogLevel(FFMS_LOG_DEBUG);
	else
		FFMS_SetLogLevel(FFMS_LOG_QUIET);
}

FFMS_IndexErrorHandling FFmpegSourceProvider::GetErrorHandlingMode() {
	std::string Mode = OPT_GET("Provider/Audio/FFmpegSource/Decode Error Handling")->GetString();

	if (boost::iequals(Mode, "ignore"))
		return FFMS_IEH_IGNORE;
	else if (boost::iequals(Mode, "clear"))
		return FFMS_IEH_CLEAR_TRACK;
	else if (boost::iequals(Mode, "stop"))
		return FFMS_IEH_STOP_TRACK;
	else if (boost::iequals(Mode, "abort"))
		return FFMS_IEH_ABORT;
	else
		return FFMS_IEH_STOP_TRACK; // questionable default?
}

/// @brief	Generates an unique name for the ffms2 index file and prepares the cache folder if it doesn't exist
/// @param filename	The name of the source file
/// @return			Returns the generated filename.
agi::fs::path FFmpegSourceProvider::GetCacheFilename(agi::fs::path const& filename) {
	// Get the size of the file to be hashed
	uintmax_t len = agi::fs::Size(filename);

	// Get the hash of the filename
	boost::crc_32_type hash;
	hash.process_bytes(filename.string().c_str(), filename.string().size());

	// Generate the filename
	auto result = config::path->Decode("?local/ffms2cache/" + std::to_string(hash.checksum()) + "_" + std::to_string(len) + "_" + std::to_string(agi::fs::ModifiedTime(filename)) + ".ffindex");

	// Ensure that folder exists
	agi::fs::CreateDirectory(result.parent_path());

	return result;
}

/// @brief		Starts the cache cleaner thread
void FFmpegSourceProvider::CleanCache() {
	::CleanCache(config::path->Decode("?local/ffms2cache/"),
		"*.ffindex",
		OPT_GET("Provider/FFmpegSource/Cache/Size")->GetInt(),
		OPT_GET("Provider/FFmpegSource/Cache/Files")->GetInt());
}

#endif // WITH_FFMS2
