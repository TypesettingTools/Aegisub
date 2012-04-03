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
//
// $Id$

/// @file ffmpegsource_common.cpp
/// @brief Shared code for ffms video and audio providers
/// @ingroup video_input audio_input ffms
///

#include "config.h"

#ifdef WITH_FFMS2

#ifndef AGI_PRE
#include <inttypes.h>
#include <map>

#include <wx/dir.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.
#endif

#include <libaegisub/log.h>

#include "compat.h"
#include "dialog_progress.h"
#include "ffmpegsource_common.h"
#include "frame_main.h"
#include "main.h"
#include "md5.h"
#include "standard_paths.h"
#include "utils.h"

#ifdef WIN32
static void deinit_com(bool) {
	CoUninitialize();
}
#else
static void deinit_com(bool) { }
#endif

FFmpegSourceProvider::FFmpegSourceProvider()
: COMInited(false, deinit_com)
{
#ifdef WIN32
	HRESULT res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(res))
		COMInited = true;
	else if (res != RPC_E_CHANGED_MODE)
		throw "COM initialization failure";
#endif

	// initialize ffmpegsource
	FFMS_Init(0, 1);
}

/// @brief Callback function that updates the indexing progress dialog
/// @param Current	The current file positition in bytes
/// @param Total	The total file size in bytes
/// @param Private	A pointer to the progress sink to update
/// @return			Returns non-0 if indexing is cancelled, 0 otherwise.
///
static int FFMS_CC UpdateIndexingProgress(int64_t Current, int64_t Total, void *Private) {
	agi::ProgressSink *ps = static_cast<agi::ProgressSink*>(Private);
	ps->SetProgress(Current, Total);
	return ps->IsCancelled();
}

/// A wrapper around FFMS_DoIndexing to make the signature void -> void
static void DoIndexingWrapper(FFMS_Index **Ret, FFMS_Indexer *Indexer, int IndexMask, int ErrorHandling, void *ICPrivate, FFMS_ErrorInfo *ErrorInfo) {
	*Ret = FFMS_DoIndexing(Indexer, IndexMask, FFMS_TRACKMASK_NONE, NULL, NULL, ErrorHandling,
		UpdateIndexingProgress, ICPrivate, ErrorInfo);
}

/// @brief Does indexing of a source file
/// @param Indexer		A pointer to the indexer object representing the file to be indexed
/// @param CacheName    The filename of the output index file
/// @param Trackmask    A binary mask of the track numbers to index
/// @param IgnoreDecodeErrors	True if audio decoding errors will be tolerated, false otherwise
/// @return				Returns the index object on success, NULL otherwise
///
FFMS_Index *FFmpegSourceProvider::DoIndexing(FFMS_Indexer *Indexer, const wxString &CacheName, int Trackmask, FFMS_IndexErrorHandling IndexEH) {
	char FFMSErrMsg[1024];
	FFMS_ErrorInfo ErrInfo;
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;
	wxString MsgString;

	// set up progress dialog callback
	DialogProgress Progress(wxGetApp().frame, _("Indexing"), _("Reading timecodes and frame/sample data"));

	// index all audio tracks
	FFMS_Index *Index;
	Progress.Run(bind(DoIndexingWrapper, &Index, Indexer, Trackmask, IndexEH, std::tr1::placeholders::_1, &ErrInfo));

	if (Index == NULL) {
		MsgString.Append("Failed to index: ").Append(wxString(ErrInfo.Buffer, wxConvUTF8));
		throw MsgString;
	}

	// write index to disk for later use
	// ignore write errors for now
	FFMS_WriteIndex(CacheName.utf8_str(), Index, &ErrInfo);
	/*if (FFMS_WriteIndex(CacheName.char_str(), Index, FFMSErrMsg, MsgSize)) {
		wxString temp(FFMSErrMsg, wxConvUTF8);
		MsgString << "Failed to write index: " << temp;
		throw MsgString;
	} */

	return Index;
}

/// @brief Finds all tracks of the given type and return their track numbers and respective codec names
/// @param Indexer	The indexer object representing the source file
/// @param Type		The track type to look for
/// @return			Returns a std::map with the track numbers as keys and the codec names as values.
std::map<int,wxString> FFmpegSourceProvider::GetTracksOfType(FFMS_Indexer *Indexer, FFMS_TrackType Type) {
	std::map<int,wxString> TrackList;
	int NumTracks = FFMS_GetNumTracksI(Indexer);

	for (int i=0; i<NumTracks; i++) {
		if (FFMS_GetTrackTypeI(Indexer, i) == Type) {
			wxString CodecName(FFMS_GetCodecNameI(Indexer, i), wxConvUTF8);
			TrackList.insert(std::pair<int,wxString>(i, CodecName));
		}
	}
	return TrackList;
}

/// @brief Ask user for which track he wants to load
/// @param TrackList	A std::map with the track numbers as keys and codec names as values
/// @param Type			The track type to ask about
/// @return				Returns the track number chosen (an integer >= 0) on success, or a negative integer if the user cancelled.
int FFmpegSourceProvider::AskForTrackSelection(const std::map<int,wxString> &TrackList, FFMS_TrackType Type) {
	std::vector<int> TrackNumbers;
	wxArrayString Choices;

	for (std::map<int,wxString>::const_iterator i = TrackList.begin(); i != TrackList.end(); i++) {
		Choices.Add(wxString::Format(_("Track %02d: %s"), i->first, i->second));
		TrackNumbers.push_back(i->first);
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
	wxString LogLevel = lagi_wxString(OPT_GET("Provider/FFmpegSource/Log Level")->GetString());

	if (!LogLevel.CmpNoCase("panic"))
		FFMS_SetLogLevel(FFMS_LOG_PANIC);
	else if (!LogLevel.CmpNoCase("fatal"))
		FFMS_SetLogLevel(FFMS_LOG_FATAL);
	else if (!LogLevel.CmpNoCase("error"))
		FFMS_SetLogLevel(FFMS_LOG_ERROR);
	else if (!LogLevel.CmpNoCase("warning"))
		FFMS_SetLogLevel(FFMS_LOG_WARNING);
	else if (!LogLevel.CmpNoCase("info"))
		FFMS_SetLogLevel(FFMS_LOG_INFO);
	else if (!LogLevel.CmpNoCase("verbose"))
		FFMS_SetLogLevel(FFMS_LOG_VERBOSE);
	else if (!LogLevel.CmpNoCase("debug"))
		FFMS_SetLogLevel(FFMS_LOG_DEBUG);
	else
		FFMS_SetLogLevel(FFMS_LOG_QUIET);
}


FFMS_IndexErrorHandling FFmpegSourceProvider::GetErrorHandlingMode() {
	wxString Mode = lagi_wxString(OPT_GET("Provider/Audio/FFmpegSource/Decode Error Handling")->GetString());

	if (!Mode.CmpNoCase("ignore"))
		return FFMS_IEH_IGNORE;
	else if (!Mode.CmpNoCase("clear"))
		return FFMS_IEH_CLEAR_TRACK;
	else if (!Mode.CmpNoCase("stop"))
		return FFMS_IEH_STOP_TRACK;
	else if (!Mode.CmpNoCase("abort"))
		return FFMS_IEH_ABORT;
	else
		return FFMS_IEH_STOP_TRACK; // questionable default?
}

#include <inttypes.h>
/// @brief	Generates an unique name for the ffms2 index file and prepares the cache folder if it doesn't exist
/// @param filename	The name of the source file
/// @return			Returns the generated filename.
wxString FFmpegSourceProvider::GetCacheFilename(const wxString& _filename)
{
	// Get the size of the file to be hashed
	wxFileOffset len = 0;
	{
		wxFile file(_filename,wxFile::read);
		if (file.IsOpened()) len = file.Length();
	}

	wxFileName filename(_filename);

	// Generate string to be hashed
	wxString toHash = wxString::Format("%s %" PRId64 " %" PRId64, filename.GetFullName(), len, (int64_t)filename.GetModificationTime().GetTicks());


	// Get the MD5 digest of the string
	const wchar_t *tmp = toHash.wc_str();
	md5_state_t state;
	md5_byte_t digest[16];
	md5_init(&state);
	md5_append(&state,(md5_byte_t*)tmp,toHash.Length()*sizeof(wchar_t));
	md5_finish(&state,digest);

	// Generate the filename
	unsigned int *md5 = (unsigned int*) digest;
	wxString result = wxString::Format("?local/ffms2cache/%08X%08X%08X%08X.ffindex",md5[0],md5[1],md5[2],md5[3]);
	result = StandardPaths::DecodePath(result);

	// Ensure that folder exists
	wxFileName fn(result);
	wxString dir = fn.GetPath();
	if (!wxFileName::DirExists(dir)) {
		wxFileName::Mkdir(dir);
	}

	wxFileName dirfn(dir);
	return dirfn.GetShortPath() + "/" + fn.GetFullName();
}

/// @brief		Starts the cache cleaner thread
void FFmpegSourceProvider::CleanCache() {
	::CleanCache(StandardPaths::DecodePath("?local/ffms2cache/"),
		"*.ffindex",
		OPT_GET("Provider/FFmpegSource/Cache/Size")->GetInt(),
		OPT_GET("Provider/FFmpegSource/Cache/Files")->GetInt());
}

#endif // WITH_FFMS2
