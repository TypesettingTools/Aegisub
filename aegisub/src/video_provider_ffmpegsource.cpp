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

/// @file video_provider_ffmpegsource.cpp
/// @brief FFmpegSource2-based video provider
/// @ingroup video_input ffms
///

#include "config.h"

#ifdef WITH_FFMPEGSOURCE

///////////
// Headers
#ifndef AGI_PRE
#ifdef __WINDOWS__
#include <objbase.h>
#endif

#include <map>

#include <wx/choicdlg.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>
#endif

#include "aegisub_endian.h"
#include "include/aegisub/aegisub.h"
#include "options.h"
#include "video_context.h"
#include "video_provider_ffmpegsource.h"



/// @brief Constructor 
/// @param filename The filename to open
FFmpegSourceVideoProvider::FFmpegSourceVideoProvider(wxString filename) {
	COMInited = false;
#ifdef WIN32
	HRESULT res;
	res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(res)) 
		COMInited = true;
	else if (res != RPC_E_CHANGED_MODE)
		throw _T("FFmpegSource video provider: COM initialization failure");
#endif
	// initialize ffmpegsource
	// FIXME: CPU detection?
	FFMS_Init(0);

	// clean up variables
	VideoSource = NULL;
	KeyFramesLoaded = false;
	FrameNumber = -1;
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;
	ErrorMsg = _T("FFmpegSource video provider: ");

	SetLogLevel();

	// and here we go
	try {
		LoadVideo(filename);
	} catch (...) {
		Close();
		throw;
	}
}


/// @brief Destructor 
FFmpegSourceVideoProvider::~FFmpegSourceVideoProvider() {
	Close();
#ifdef WIN32
	if (COMInited)
		CoUninitialize();
#endif
}


/// @brief Opens video 
/// @param filename The filename to open
void FFmpegSourceVideoProvider::LoadVideo(wxString filename) {
	// make sure we don't have anything messy lying around
	Close();

	wxString FileNameShort = wxFileName(filename).GetShortPath(); 

	FFMS_Indexer *Indexer = FFMS_CreateIndexer(FileNameShort.utf8_str(), &ErrInfo);
	if (Indexer == NULL) {
		// error messages that can possibly contain a filename use this method instead of
		// wxString::Format because they may contain utf8 characters
		ErrorMsg.Append(_T("Failed to create indexer: ")).Append(wxString(ErrInfo.Buffer, wxConvUTF8));
		throw ErrorMsg;
	}

	std::map<int,wxString> TrackList = GetTracksOfType(Indexer, FFMS_TYPE_VIDEO);
	if (TrackList.size() <= 0)
		throw _T("FFmpegSource video provider: no video tracks found");

	// initialize the track number to an invalid value so we can detect later on
	// whether the user actually had to choose a track or not
	int TrackNumber = -1;
	if (TrackList.size() > 1) {
		TrackNumber = AskForTrackSelection(TrackList, FFMS_TYPE_VIDEO);
		// if it's still -1 here, user pressed cancel
		if (TrackNumber == -1)
			throw _T("FFmpegSource video provider: video loading cancelled by user");
	}

	// generate a name for the cache file
	wxString CacheName = GetCacheFilename(filename);

	// try to read index
	FFMS_Index *Index = NULL;
	Index = FFMS_ReadIndex(CacheName.utf8_str(), &ErrInfo);
	bool IndexIsValid = false;
	if (Index != NULL) {
		if (FFMS_IndexBelongsToFile(Index, FileNameShort.utf8_str(), &ErrInfo)) {
			FFMS_DestroyIndex(Index);
			Index = NULL;
		}
		else
			IndexIsValid = true;
	}

	// time to examine the index and check if the track we want is indexed
	// technically this isn't really needed since all video tracks should always be indexed,
	// but a bit of sanity checking never hurt anyone
	if (IndexIsValid && TrackNumber >= 0) {
		FFMS_Track *TempTrackData = FFMS_GetTrackFromIndex(Index, TrackNumber);
		if (FFMS_GetNumFrames(TempTrackData) <= 0) {
			IndexIsValid = false;
			FFMS_DestroyIndex(Index);
			Index = NULL;
		}
	}
	
	// moment of truth
	if (!IndexIsValid) {
		int TrackMask = Options.AsBool(_T("FFmpegSource always index all tracks")) ? FFMS_TRACKMASK_ALL : FFMS_TRACKMASK_NONE;
		try {
			// ignore audio decoding errors here, we don't care right now
			Index = DoIndexing(Indexer, CacheName, TrackMask, true);
		} catch (wxString temp) {
			ErrorMsg.Append(temp);
			throw ErrorMsg;
		} catch (...) {
			throw;
		}
	}
	
	// update access time of index file so it won't get cleaned away
	wxFileName(CacheName).Touch();

	// we have now read the index and may proceed with cleaning the index cache
	if (!CleanCache()) {
		//do something?
	}

	// track number still not set?
	if (TrackNumber < 0) {
		// just grab the first track
		TrackNumber = FFMS_GetFirstIndexedTrackOfType(Index, FFMS_TYPE_VIDEO, &ErrInfo);
		if (TrackNumber < 0) {
			FFMS_DestroyIndex(Index);
			Index = NULL;
			ErrorMsg.Append(wxString::Format(_T("Couldn't find any video tracks: %s"), ErrInfo.Buffer));
			throw ErrorMsg;	
		}
	}

	// set thread count
	int Threads = Options.AsInt(_T("FFmpegSource decoding threads"));
	if (Threads < 1)
		throw _T("FFmpegSource video provider: invalid decoding thread count");

	// set seekmode
	// TODO: give this its own option?
	int SeekMode;
	if (Options.AsBool(_T("FFmpeg allow unsafe seeking")))
		SeekMode = FFMS_SEEK_UNSAFE;
	else 
		SeekMode = FFMS_SEEK_NORMAL;

	VideoSource = FFMS_CreateVideoSource(FileNameShort.utf8_str(), TrackNumber, Index, Threads, SeekMode, &ErrInfo);
	FFMS_DestroyIndex(Index);
	Index = NULL;
	if (VideoSource == NULL) {
		ErrorMsg.Append(wxString::Format(_T("Failed to open video track: %s"), ErrInfo.Buffer));
		throw ErrorMsg;
	}

	// load video properties
	VideoInfo = FFMS_GetVideoProperties(VideoSource);

	const FFMS_Frame *TempFrame = FFMS_GetFrame(VideoSource, 0, &ErrInfo);
	if (TempFrame == NULL) {
		ErrorMsg.Append(wxString::Format(_T("Failed to decode first frame: %s"), ErrInfo.Buffer));
		throw ErrorMsg;
	}
	Width	= TempFrame->EncodedWidth;
	Height	= TempFrame->EncodedHeight;

	if (FFMS_SetOutputFormatV(VideoSource, 1 << FFMS_GetPixFmt("bgra"), Width, Height, FFMS_RESIZER_BICUBIC, &ErrInfo)) {
		ErrorMsg.Append(wxString::Format(_T("Failed to set output format: %s"), ErrInfo.Buffer));
		throw ErrorMsg;
	}

	// get frame info data
	FFMS_Track *FrameData = FFMS_GetTrackFromVideo(VideoSource);
	if (FrameData == NULL)
		throw _T("FFmpegSource video provider: failed to get frame data");
	const FFMS_TrackTimeBase *TimeBase = FFMS_GetTimeBase(FrameData);
	if (TimeBase == NULL)
		throw _T("FFmpegSource video provider: failed to get track time base");

	const FFMS_FrameInfo *CurFrameData;

	// build list of keyframes and timecodes
	for (int CurFrameNum = 0; CurFrameNum < VideoInfo->NumFrames; CurFrameNum++) {
		CurFrameData = FFMS_GetFrameInfo(FrameData, CurFrameNum);
		if (CurFrameData == NULL) {
			ErrorMsg.Append(wxString::Format(_T("Couldn't get info about frame %d"), CurFrameNum));
			throw ErrorMsg;
		}

		// keyframe?
		if (CurFrameData->KeyFrame)
			KeyFramesList.Add(CurFrameNum);

		// calculate timestamp and add to timecodes vector
		int Timestamp = (int)((CurFrameData->PTS * TimeBase->Num) / TimeBase->Den);
		TimecodesVector.push_back(Timestamp);
	}
	KeyFramesLoaded = true;

	// override already loaded timecodes?
	Timecodes.SetVFR(TimecodesVector);
	int OverrideTC = wxYES;
	if (VFR_Output.IsLoaded()) {
		OverrideTC = wxMessageBox(_("You already have timecodes loaded. Would you like to replace them with timecodes from the video file?"), _("Replace timecodes?"), wxYES_NO | wxICON_QUESTION);
		if (OverrideTC == wxYES) {
			VFR_Input.SetVFR(TimecodesVector);
			VFR_Output.SetVFR(TimecodesVector);
		}
	} else { // no timecodes loaded, go ahead and apply
		VFR_Input.SetVFR(TimecodesVector);
		VFR_Output.SetVFR(TimecodesVector);
	}

	FrameNumber = 0;
}


/// @brief Close video 
///
void FFmpegSourceVideoProvider::Close() {
	FFMS_DestroyVideoSource(VideoSource);
	VideoSource = NULL;

	KeyFramesLoaded = false;
	KeyFramesList.clear();
	TimecodesVector.clear();
	FrameNumber = -1;
	CurFrame.Clear();
}



/// @brief Get frame 
/// @param _n 
/// @return 
///
const AegiVideoFrame FFmpegSourceVideoProvider::GetFrame(int _n) {
	// don't try to seek to insane places
	int n = _n;
	if (n < 0)
		n = 0;
	if (n >= GetFrameCount())
		n = GetFrameCount()-1;
	// set position
	FrameNumber = n;

	// decode frame
	const FFMS_Frame *SrcFrame = FFMS_GetFrame(VideoSource, n, &ErrInfo);
	if (SrcFrame == NULL) {
		ErrorMsg.Append(wxString::Format(_T("Failed to retrieve frame: %s"), ErrInfo.Buffer));
		throw ErrorMsg;
	}

	CurFrame.SetTo(SrcFrame->Data, Width, Height, SrcFrame->Linesize, FORMAT_RGB32);
	return CurFrame;
}



/// @brief Utility functions 
/// @return 
///
int FFmpegSourceVideoProvider::GetWidth() {
	return Width;
}


/// @brief DOCME
/// @return 
///
int FFmpegSourceVideoProvider::GetHeight() {
	return Height;
}


/// @brief DOCME
/// @return 
///
int FFmpegSourceVideoProvider::GetFrameCount() {
	return VideoInfo->NumFrames;
}


/// @brief DOCME
/// @return 
///
int FFmpegSourceVideoProvider::GetPosition() {
	return FrameNumber;
}


/// @brief DOCME
///
double FFmpegSourceVideoProvider::GetFPS() {
	return double(VideoInfo->FPSNumerator) / double(VideoInfo->FPSDenominator);
}


#endif /* WITH_FFMPEGSOURCE */


