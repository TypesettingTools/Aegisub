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

#ifdef WITH_FFMPEGSOURCE

#ifndef AGI_PRE
#include <inttypes.h>
#include <map>

//#include <wx/dir.h>
//#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.
#endif

#include <libaegisub/log.h>

#include "ffms_common.h"

//#include "compat.h"
//#include "ffmpegsource_common.h"
//#include "frame_main.h"
//#include "main.h"
//#include "md5.h"
//#include "standard_paths.h"

namespace media {
	namespace ffms {

/// @brief Callback function that updates the indexing progress dialog
/// @param Current	The current file positition in bytes
/// @param Total	The total file size in bytes
/// @param Private	A pointer to the progress dialog box to update 
/// @return			Returns non-0 if indexing is cancelled, 0 otherwise.
///
int FFMS_CC FFmpegSourceProvider::UpdateIndexingProgress(int64_t Current, int64_t Total, void *Private) {
	IndexingProgressDialog *Progress = (IndexingProgressDialog *)Private;

	if (Progress->IndexingCanceled)
		return 1;

	// no one cares about a little bit of a rounding error here anyway
//	Progress->ProgressDialog->SetProgress(((int64_t)1000*Current)/Total, 1000);

	return 0;
}



/// @brief Does indexing of a source file
/// @param Indexer		A pointer to the indexer object representing the file to be indexed
/// @param CacheName    The filename of the output index file
/// @param Trackmask    A binary mask of the track numbers to index
/// @param IgnoreDecodeErrors	True if audio decoding errors will be tolerated, false otherwise
/// @return				Returns the index object on success, NULL otherwise
///
FFMS_Index *FFmpegSourceProvider::DoIndexing(FFMS_Indexer *Indexer, const std::string &CacheName, int Trackmask, FFMS_IndexErrorHandling IndexEH) {
	char FFMSErrMsg[1024];
	FFMS_ErrorInfo ErrInfo;
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;
	std::string MsgString;
/*
	// set up progress dialog callback
	IndexingProgressDialog Progress;
	Progress.IndexingCanceled = false;
	Progress.ProgressDialog = new DialogProgress(AegisubApp::Get()->frame,
		_("Indexing"), &Progress.IndexingCanceled,
		_("Reading timecodes and frame/sample data"), 0, 1);
	Progress.ProgressDialog->Show();
	Progress.ProgressDialog->SetProgress(0,1);

	// index all audio tracks
	FFMS_Index *Index = FFMS_DoIndexing(Indexer, Trackmask, FFMS_TRACKMASK_NONE, NULL, NULL, IndexEH,
		FFmpegSourceProvider::UpdateIndexingProgress, &Progress, &ErrInfo);
	Progress.ProgressDialog->Destroy();
	if (Progress.IndexingCanceled) {
		throw agi::UserCancelException("indexing cancelled by user");
	}
	if (Index == NULL) {
		MsgString.append("Failed to index: ").append(ErrInfo.Buffer);
		throw MsgString;
	}

	// write index to disk for later use
	// ignore write errors for now
	FFMS_WriteIndex(CacheName.c_str(), Index, &ErrInfo);
//	if (FFMS_WriteIndex(CacheName.char_str(), Index, FFMSErrMsg, MsgSize)) {
//		wxString temp(FFMSErrMsg, wxConvUTF8);
//		MsgString << _T("Failed to write index: ") << temp;
//		throw MsgString;
//	}
*/
FFMS_Index *Index;
	return Index;
}

/// @brief Finds all tracks of the given type and return their track numbers and respective codec names 
/// @param Indexer	The indexer object representing the source file
/// @param Type		The track type to look for
/// @return			Returns a std::map with the track numbers as keys and the codec names as values.
std::map<int,std::string> FFmpegSourceProvider::GetTracksOfType(FFMS_Indexer *Indexer, FFMS_TrackType Type) {
	std::map<int,std::string> TrackList;
	int NumTracks = FFMS_GetNumTracksI(Indexer);

	for (int i=0; i<NumTracks; i++) {
		if (FFMS_GetTrackTypeI(Indexer, i) == Type) {
			std::string CodecName(FFMS_GetCodecNameI(Indexer, i));
			TrackList.insert(std::pair<int,std::string>(i, CodecName));
		}
	}
	return TrackList;
}

/// @brief Ask user for which track he wants to load
/// @param TrackList	A std::map with the track numbers as keys and codec names as values
/// @param Type			The track type to ask about
/// @return				Returns the track number chosen (an integer >= 0) on success, or a negative integer if the user cancelled.
int FFmpegSourceProvider::AskForTrackSelection(const std::map<int,std::string> &TrackList, FFMS_TrackType Type) {
/*
	std::vector<int> TrackNumbers;
	wxArrayString Choices;
	std::string TypeName = "";
	if (Type == FFMS_TYPE_VIDEO)
		TypeName = _("video");
	else if (Type == FFMS_TYPE_AUDIO)
		TypeName = _("audio");

	for (std::map<int,std::string>::const_iterator i = TrackList.begin(); i != TrackList.end(); i++) {
		istream str;
//XXX		Choices.Add(ng::Format(_("Track %02d: %s"), i->first, i->second.c_str()));
		TrackNumbers.push_back(i->first);
	}

	int Choice = wxGetSingleChoiceIndex(wxString::Format(_("Multiple %s tracks detected, please choose the one you wish to load:"), TypeName.c_str()),
		wxString::Format(_("Choose %s track"), TypeName.c_str()), Choices);

	if (Choice < 0)
		return Choice;
	else
		return TrackNumbers[Choice];
*/
}


/// @brief Set ffms2 log level according to setting in config.dat
void FFmpegSourceProvider::SetLogLevel() {
/*
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
	else */
		FFMS_SetLogLevel(FFMS_LOG_QUIET);
}


FFMS_IndexErrorHandling FFmpegSourceProvider::GetErrorHandlingMode() {
//	std::string Mode = lagi_wxString(OPT_GET("Provider/Audio/FFmpegSource/Decode Error Handling")->GetString());
std::string mode = "ignore";

	if (mode == "ignore")
		return FFMS_IEH_IGNORE;
	else if (mode == "clear")
		return FFMS_IEH_CLEAR_TRACK;
	else if (mode == "stop")
		return FFMS_IEH_STOP_TRACK;
	else if (mode == "abort")
		return FFMS_IEH_ABORT;
	else
		return FFMS_IEH_STOP_TRACK; // questionable default?
}

#include <inttypes.h>
/// @brief	Generates an unique name for the ffms2 index file and prepares the cache folder if it doesn't exist 
/// @param filename	The name of the source file
/// @return			Returns the generated filename.
std::string FFmpegSourceProvider::GetCacheFilename(const std::string& _filename)
{
/*
	// Get the size of the file to be hashed
	wxFileOffset len = 0;
	{
		wxFile file(_filename,wxFile::read);
		if (file.IsOpened()) len = file.Length();
	}

	wxFileName filename(_filename);

	// Generate string to be hashed
//	std::string toHash = wxString::Format("%s %" PRId64 " %" PRId64, filename.GetFullName(), len, (int64_t)filename.GetModificationTime().GetTicks());
std::string toHash = "XXX";

	// Get the MD5 digest of the string
	const wchar_t *tmp = toHash.wc_str();
	md5_state_t state;
	md5_byte_t digest[16];
	md5_init(&state);
	md5_append(&state,(md5_byte_t*)tmp,toHash.Length()*sizeof(wchar_t));
	md5_finish(&state,digest);

	// Generate the filename
	unsigned int *md5 = (unsigned int*) digest;
	wxString result = wxString::Format("?user/ffms2cache/%08X%08X%08X%08X.ffindex",md5[0],md5[1],md5[2],md5[3]);
	result = StandardPaths::DecodePath(result);

	// Ensure that folder exists
	wxFileName fn(result);
	wxString dir = fn.GetPath();
	if (!wxFileName::DirExists(dir)) {
		wxFileName::Mkdir(dir);
	}

	wxFileName dirfn(dir);
	return dirfn.GetShortPath() + "/" + fn.GetFullName();
*/
return "XXX";
}

#endif // WITH_FFMPEGSOURCE


	} // namespace ffms
} // namespace media
