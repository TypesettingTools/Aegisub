// Copyright (c) 2008-2009, Karl Blomster <thefluff@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file ffms_video.h
/// @brief FFmpegSource Video support.
/// @ingroup fmms video

#include "config.h"

#ifdef WITH_FFMPEGSOURCE

#ifndef AGI_PRE
#ifdef __WINDOWS__
#include <objbase.h>
#endif

#include <map>

//#include <wx/choicdlg.h>
//#include <wx/msgdlg.h>
//#include <wx/utils.h>
#endif

#include "ffms_video.h"
#include "libmedia/exception.h"
#include "libaegisub/util.h"

//#include "aegisub_endian.h"
//#include "compat.h"
//#include "main.h"
//#include "utils.h"
//#include "video_context.h"
//#include "video_provider_ffmpegsource.h"

namespace agi {
	namespace ffms {

/// @brief Constructor
/// @param filename The filename to open
Video::Video(std::string filename)
: VideoSource(NULL)
, VideoInfo(NULL)
, Width(-1)
, Height(-1)
, FrameNumber(-1)
, COMInited(false)
{
#ifdef WIN32
	HRESULT res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(res)) 
		COMInited = true;
	else if (res != RPC_E_CHANGED_MODE)
		throw VideoOpenError("COM initialization failure");
#endif
	// initialize ffmpegsource
	// FIXME: CPU detection?
#if FFMS_VERSION >= ((2 << 24) | (14 << 16) | (0 << 8) | 0)
	FFMS_Init(0, 1);
#else
	FFMS_Init(0);
#endif

	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;

//	SetLogLevel();

	// and here we go
	try {
		LoadVideo(filename);
	}
	catch (std::string const& err) {
		Close();
		throw VideoOpenError(err);
	}
	catch (...) {
		Close();
		throw;
	}
}


/// @brief Destructor 
Video::~Video() {
	Close();
}


/// @brief Opens video 
/// @param filename The filename to open
void Video::LoadVideo(std::string filename) {

	FFMS_Indexer *Indexer = FFMS_CreateIndexer(filename.c_str(), &ErrInfo);
	if (Indexer == NULL) {
		throw agi::FileNotFoundError(ErrInfo.Buffer);
	}

	std::map<int,std::string> TrackList = GetTracksOfType(Indexer, FFMS_TYPE_VIDEO);
	if (TrackList.size() <= 0)
		throw VideoNotSupported("no video tracks found");

	// initialize the track number to an invalid value so we can detect later on
	// whether the user actually had to choose a track or not
	int TrackNumber = -1;
	if (TrackList.size() > 1) {
		TrackNumber = AskForTrackSelection(TrackList, FFMS_TYPE_VIDEO);
		// if it's still -1 here, user pressed cancel
		if (TrackNumber == -1)
			throw agi::UserCancelException("video loading cancelled by user");
	}

	// generate a name for the cache file
	std::string CacheName = GetCacheFilename(filename);

	// try to read index
	FFMS_Index *Index = NULL;
	Index = FFMS_ReadIndex(CacheName.c_str(), &ErrInfo);
	bool IndexIsValid = false;
	if (Index != NULL) {
		if (FFMS_IndexBelongsToFile(Index, filename.c_str(), &ErrInfo)) {
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
//		int TrackMask = OPT_GET("Provider/FFmpegSource/Index All Tracks")->GetBool() ? FFMS_TRACKMASK_ALL : FFMS_TRACKMASK_NONE;
		int TrackMask = 1 ? FFMS_TRACKMASK_ALL : FFMS_TRACKMASK_NONE;
		try {
			// ignore audio decoding errors here, we don't care right now
			Index = DoIndexing(Indexer, CacheName, TrackMask, FFMS_IEH_IGNORE);
		}
		catch (std::string err) {
			throw VideoOpenError(err);
		}
	}

	// update access time of index file so it won't get cleaned away
//XXX:	wxFileName(CacheName).Touch();

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
			throw VideoNotSupported(std::string("Couldn't find any video tracks: ") + ErrInfo.Buffer);
		}
	}

	// set thread count
//	int Threads = OPT_GET("Provider/Video/FFmpegSource/Decoding Threads")->GetInt();
	int Threads = 1;
	if (Threads < 1)
		throw VideoOpenError("invalid decoding thread count");

	// set seekmode
	// TODO: give this its own option?
	int SeekMode;
//	if (OPT_GET("Provider/Video/FFmpegSource/Unsafe Seeking")->GetBool())
//		SeekMode = FFMS_SEEK_UNSAFE;
//	else
		SeekMode = FFMS_SEEK_NORMAL;

	VideoSource = FFMS_CreateVideoSource(filename.c_str(), TrackNumber, Index, Threads, SeekMode, &ErrInfo);
	FFMS_DestroyIndex(Index);
	Index = NULL;
	if (VideoSource == NULL) {
		throw VideoOpenError(std::string("Failed to open video track: ") + ErrInfo.Buffer);
	}

	// load video properties
	VideoInfo = FFMS_GetVideoProperties(VideoSource);

	const FFMS_Frame *TempFrame = FFMS_GetFrame(VideoSource, 0, &ErrInfo);
	if (TempFrame == NULL) {
		throw VideoOpenError(std::string("Failed to decode first frame: ") + ErrInfo.Buffer);
	}
	Width	= TempFrame->EncodedWidth;
	Height	= TempFrame->EncodedHeight;

	if (FFMS_SetOutputFormatV(VideoSource, 1LL << FFMS_GetPixFmt("bgra"), Width, Height, FFMS_RESIZER_BICUBIC, &ErrInfo)) {
		throw VideoOpenError(std::string("Failed to set output format: ") + ErrInfo.Buffer);
	}

	// get frame info data
	FFMS_Track *FrameData = FFMS_GetTrackFromVideo(VideoSource);
	if (FrameData == NULL)
		throw VideoOpenError("failed to get frame data");
	const FFMS_TrackTimeBase *TimeBase = FFMS_GetTimeBase(FrameData);
	if (TimeBase == NULL)
		throw VideoOpenError("failed to get track time base");

	const FFMS_FrameInfo *CurFrameData;

	// build list of keyframes and timecodes
	std::vector<int> TimecodesVector;
	for (int CurFrameNum = 0; CurFrameNum < VideoInfo->NumFrames; CurFrameNum++) {
		CurFrameData = FFMS_GetFrameInfo(FrameData, CurFrameNum);
		if (CurFrameData == NULL) {
//XXX		throw VideoOpenError(STD_STR(wxString::Format(L"Couldn't get info about frame %d", CurFrameNum)));
		throw VideoOpenError("Couldn't get info about frame %d");
		}

		// keyframe?
		if (CurFrameData->KeyFrame)
			KeyFramesList.push_back(CurFrameNum);

		// calculate timestamp and add to timecodes vector
		int Timestamp = (int)((CurFrameData->PTS * TimeBase->Num) / TimeBase->Den);
		TimecodesVector.push_back(Timestamp);
	}
	Timecodes = agi::vfr::Framerate(TimecodesVector);

	FrameNumber = 0;
}

/// @brief Close video 
///
void Video::Close() {
	if (VideoSource) FFMS_DestroyVideoSource(VideoSource);
#ifdef WIN32
	if (COMInited)
		CoUninitialize();
#endif
}

/// @brief Get frame 
/// @param _n 
/// @return 
///
const media::AegiVideoFrame Video::GetFrame(int n) {
	FrameNumber = util::mid(0, n, GetFrameCount() - 1);

	// decode frame
	const FFMS_Frame *SrcFrame = FFMS_GetFrame(VideoSource, FrameNumber, &ErrInfo);
	if (SrcFrame == NULL) {
		throw VideoDecodeError(std::string("Failed to retrieve frame:") +  ErrInfo.Buffer);
	}

	CurFrame.SetTo(SrcFrame->Data[0], Width, Height, SrcFrame->Linesize[0]);
	return CurFrame;
}
#endif /* WITH_FFMPEGSOURCE */

	} // namespace ffms
} // namespace agi
