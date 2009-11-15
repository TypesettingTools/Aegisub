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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#include "config.h"

#ifdef WITH_FFMPEGSOURCE

///////////
// Headers
#include <wx/utils.h>
#include <wx/choicdlg.h>
#include <map>
#include "include/aegisub/aegisub.h"
#include "video_provider_ffmpegsource.h"
#include "video_context.h"
#include "options.h"
#include "aegisub_endian.h"
#ifdef WIN32
#include <objbase.h>
#endif


///////////////
// Constructor
FFmpegSourceVideoProvider::FFmpegSourceVideoProvider(Aegisub::String filename, double fps) {
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
	DstFormat = FFMS_GetPixFmt("none");
	LastDstFormat = FFMS_GetPixFmt("none");
	KeyFramesLoaded = false;
	FrameNumber = -1;
	MsgSize = sizeof(FFMSErrMsg);
	ErrorMsg = _T("FFmpegSource video provider: ");

	// and here we go
	try {
		LoadVideo(filename, fps);
	} catch (...) {
		Close();
		throw;
	}
}

///////////////
// Destructor
FFmpegSourceVideoProvider::~FFmpegSourceVideoProvider() {
	Close();
#ifdef WIN32
	if (COMInited)
		CoUninitialize();
#endif
}

///////////////
// Open video
void FFmpegSourceVideoProvider::LoadVideo(Aegisub::String filename, double fps) {
	// make sure we don't have anything messy lying around
	Close();

	wxString FileNameWX = wxFileName(wxString(filename.c_str(), wxConvFile)).GetShortPath(); 

	FFIndexer *Indexer = FFMS_CreateIndexer(FileNameWX.mb_str(wxConvUTF8), FFMSErrMsg, MsgSize);
	if (Indexer == NULL) {
		// error messages that can possibly contain a filename use this method instead of
		// wxString::Format because they may contain utf8 characters
		ErrorMsg.Append(_T("Failed to create indexer: ")).Append(wxString(FFMSErrMsg, wxConvUTF8));
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
	wxString CacheName = GetCacheFilename(filename.c_str());

	// try to read index
	FFIndex *Index = NULL;
	Index = FFMS_ReadIndex(CacheName.mb_str(wxConvUTF8), FFMSErrMsg, MsgSize);
	bool IndexIsValid = false;
	if (Index != NULL) {
		if (FFMS_IndexBelongsToFile(Index, FileNameWX.mb_str(wxConvUTF8), FFMSErrMsg, MsgSize)) {
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
		FFTrack *TempTrackData = FFMS_GetTrackFromIndex(Index, TrackNumber);
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
			try {
				// ignore audio decoding errors here, we don't care right now
				Index = DoIndexing(Indexer, CacheName, TrackMask, true);
			} catch (...) {
				// something borked, try if it works without audio
				Index = DoIndexing(Indexer, CacheName, FFMS_TRACKMASK_NONE, true);
			}
		} catch (wxString temp) {
			ErrorMsg.Append(temp);
			throw ErrorMsg;
		} catch (...) {
			throw;
		}
	}
	
	// update access time of index file so it won't get cleaned away
	if (!wxFileName(CacheName).Touch()) {
		// warn user?
		// FIND OUT WHY IT'S POPPING UP ERROR MESSAGES HERE
	}

	// we have now read the index and may proceed with cleaning the index cache
	if (!CleanCache()) {
		//do something?
	}

	// track number still not set?
	if (TrackNumber < 0) {
		// just grab the first track
		TrackNumber = FFMS_GetFirstIndexedTrackOfType(Index, FFMS_TYPE_VIDEO, FFMSErrMsg, MsgSize);
		if (TrackNumber < 0) {
			FFMS_DestroyIndex(Index);
			Index = NULL;
			ErrorMsg.Append(wxString::Format(_T("Couldn't find any video tracks: %s"), FFMSErrMsg));
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

	VideoSource = FFMS_CreateVideoSource(FileNameWX.mb_str(wxConvUTF8), TrackNumber, Index, "", Threads, SeekMode, FFMSErrMsg, MsgSize);
	FFMS_DestroyIndex(Index);
	Index = NULL;
	if (VideoSource == NULL) {
		ErrorMsg.Append(wxString::Format(_T("Failed to open video track: %s"), FFMSErrMsg));
		throw ErrorMsg;
	}

	// load video properties
	VideoInfo = FFMS_GetVideoProperties(VideoSource);

	// get frame info data
	FFTrack *FrameData = FFMS_GetTrackFromVideo(VideoSource);
	if (FrameData == NULL)
		throw _T("FFmpegSource video provider: failed to get frame data");
	const FFTrackTimeBase *TimeBase = FFMS_GetTimeBase(FrameData);
	if (TimeBase == NULL)
		throw _T("FFmpegSource video provider: failed to get track time base");

	const FFFrameInfo *CurFrameData;

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
		int Timestamp = (int)((CurFrameData->DTS * TimeBase->Num) / TimeBase->Den);
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

///////////////
// Close video
void FFmpegSourceVideoProvider::Close() {
	FFMS_DestroyVideoSource(VideoSource);
	VideoSource = NULL;

	DstFormat = FFMS_GetPixFmt("none");
	LastDstFormat = FFMS_GetPixFmt("none");
	KeyFramesLoaded = false;
	KeyFramesList.clear();
	TimecodesVector.clear();
	FrameNumber = -1;
	CurFrame.Clear();
}


///////////////
// Get frame
const AegiVideoFrame FFmpegSourceVideoProvider::GetFrame(int _n, int FormatType) {
	// don't try to seek to insane places
	int n = _n;
	if (n < 0)
		n = 0;
	if (n >= GetFrameCount())
		n = GetFrameCount()-1;
	// set position
	FrameNumber = n;
	
	// these are for convenience
	int w = VideoInfo->Width;
	int h = VideoInfo->Height;

	// this is what we'll return eventually
	AegiVideoFrame &DstFrame = CurFrame;
	
	// choose output format
	if (FormatType & FORMAT_RGB32) {
		DstFormat		= FFMS_GetPixFmt("bgra");
		DstFrame.format	= FORMAT_RGB32;
	} else if (FormatType & FORMAT_RGB24) {
		DstFormat		= FFMS_GetPixFmt("bgr24");
		DstFrame.format	= FORMAT_RGB24;
	} else if (FormatType & FORMAT_YV12) {
		DstFormat		= FFMS_GetPixFmt("yuv420p");
		DstFrame.format	= FORMAT_YV12;
	} else if (FormatType & FORMAT_YUY2) {
		DstFormat		= FFMS_GetPixFmt("yuyv422"); // may or may not work
		DstFrame.format	= FORMAT_YUY2;
	} else 
		throw _T("FFmpegSource video provider: upstream provider requested unknown or unsupported pixel format");

	// requested format was changed since last time we were called, (re)set output format
	if (LastDstFormat != DstFormat) {
		if (FFMS_SetOutputFormatV(VideoSource, 1 << DstFormat, w, h, FFMS_RESIZER_BICUBIC, FFMSErrMsg, MsgSize)) {
			ErrorMsg.Append(wxString::Format(_T("Failed to set output format: %s"), FFMSErrMsg));
			throw ErrorMsg;
		}
		LastDstFormat = DstFormat;
	}

	// decode frame
	const FFAVFrame *SrcFrame = FFMS_GetFrame(VideoSource, n, FFMSErrMsg, MsgSize);
	if (SrcFrame == NULL) {
		ErrorMsg.Append(wxString::Format(_T("Failed to retrieve frame: %s"), FFMSErrMsg));
		throw ErrorMsg;
	}

	// set some properties
	DstFrame.w = w;
	DstFrame.h = h;
	DstFrame.flipped = false;
	if (DstFrame.format == FORMAT_RGB32 || DstFrame.format == FORMAT_RGB24)
		DstFrame.invertChannels = true;
	else
		DstFrame.invertChannels = false;

	// allocate destination
	for (int i = 0; i < 4; i++)
		DstFrame.pitch[i] = SrcFrame->Linesize[i];
	DstFrame.Allocate();

	// copy data to destination
	memcpy(DstFrame.data[0], SrcFrame->Data[0], DstFrame.pitch[0] * DstFrame.h);
	// if we're dealing with YUV formats we need to copy the U and V planes as well
	if (DstFrame.format == FORMAT_YUY2 || DstFrame.format == FORMAT_YV12) {
		// YV12 has half the vertical U/V resolution too because of the subsampling
		int UVHeight = DstFrame.format == FORMAT_YUY2 ? DstFrame.h : DstFrame.h / 2;
		memcpy(DstFrame.data[1], SrcFrame->Data[1], DstFrame.pitch[1] * UVHeight);
		memcpy(DstFrame.data[2], SrcFrame->Data[2], DstFrame.pitch[2] * UVHeight);
	}

	return DstFrame;
}


///////////////
// Utility functions
int FFmpegSourceVideoProvider::GetWidth() {
	return VideoInfo->Width;
}

int FFmpegSourceVideoProvider::GetHeight() {
	return VideoInfo->Height;
}

int FFmpegSourceVideoProvider::GetFrameCount() {
	return VideoInfo->NumFrames;
}

int FFmpegSourceVideoProvider::GetPosition() {
	return FrameNumber;
}

double FFmpegSourceVideoProvider::GetFPS() {
	return double(VideoInfo->FPSNumerator) / double(VideoInfo->FPSDenominator);
}


#endif /* WITH_FFMPEGSOURCE */
