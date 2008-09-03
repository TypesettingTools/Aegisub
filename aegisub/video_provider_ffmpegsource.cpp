// Copyright (c) 2008, Karl Blomster
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

#ifdef WITH_FFMPEGSOURCE

#ifdef WIN32
#define EMULATE_INTTYPES
#endif /* WIN32 */

///////////
// Headers
#include <wx/wxprec.h>
#include "video_provider_ffmpegsource.h"
#include <ffms.h>
#include "vfr.h"
#include "video_context.h"
// #include "options.h" // for later use


///////////////
// Constructor
FFmpegSourceVideoProvider::FFmpegSourceVideoProvider(Aegisub::String filename, double fps) {
	// initialize ffmpegsource
	FFMS_Init();

	// clean up variables
	VideoSource = NULL;
	SWSContext = NULL;
	BufferRGB = NULL;
	KeyFramesLoaded = false;
	FrameAllocated = false;
	FrameNumber = -1;
	MessageSize = sizeof(FFMSErrorMessage);

	// and here we go
	LoadVideo(filename, fps);
}

///////////////
// Destructor
FFmpegSourceVideoProvider::~FFmpegSourceVideoProvider() {
	Close();
}

///////////////
// Open video
void FFmpegSourceVideoProvider::LoadVideo(Aegisub::String filename, double fps) {
	// make sure we don't have anything messy lying around
	Close();

	wxString FileNameWX(filename.c_str(), wxConvUTF8);

	// generate a name for the cache file
	wxString CacheName(filename.c_str());
	CacheName.append(_T(".ffindex"));

	// initialize index object
	FrameIndex *Index = FFMS_CreateFrameIndex();
	// try to read index (these index functions all return 0 on success)
	if (FFMS_ReadIndex(CacheName.char_str(), Index, FFMSErrorMessage, MessageSize)) {
		// reading failed, create index
		if (FFMS_MakeIndex(FileNameWX.char_str(), Index, 0, NULL, NULL, FFMSErrorMessage, MessageSize)) {
			ErrorMsg.Printf(_T("FFmpegSource video provider: %s"), FFMSErrorMessage);
			throw ErrorMsg;
		}
		// write it to disk
		if (FFMS_WriteIndex(CacheName.char_str(), Index, FFMSErrorMessage, MessageSize)) {
			ErrorMsg.Printf(_T("FFmpegSource video provider: %s"), FFMSErrorMessage);
			throw ErrorMsg;
		}
	}

	// TODO: make this user-configurable
	int Threads = 1;
	if (Threads < 1)
		throw _T("FFmpegSource video provider: invalid decoding thread count");

	// TODO: tie this to the option "ffmpeg allow unsafe seeking"
	int SeekMode = 1;

	// finally create the actual video source
	VideoSource = FFMS_CreateVideoSource(FileNameWX.char_str(), -1, Index, "", Threads, SeekMode, FFMSErrorMessage, MessageSize);
	if (VideoSource == NULL) {
		ErrorMsg.Printf(_T("FFmpegSource video provider: %s"), FFMSErrorMessage);
		throw ErrorMsg;
	}

	// load video properties
	VideoInfo = FFMS_GetVideoProperties(VideoSource);

	// get frame info data
	// disabled until Myrsloik answers some questions
#if 0
	FrameInfoVector FrameData = FFMS_GetTITrackIndex(Index, -1, FFMSErrorMessage, MessageSize);
	if (FrameData == NULL) {
		ErrorMsg.Printf(_T("FFmpegSource video provider: %s"), FFMSErrorMessage);
		throw ErrorMsg;
	}
	FrameInfo CurFrameData;

	// build list of keyframes
	for (int CurFrameNum = 0; CurFrameNum < VideoInfo->NumFrames; CurFrameNum++) {
		CurFrameData = FFMS_GetFrameInfo(FrameData, CurFrameNum, FFMSErrorMessage, MessageSize);
		if (CurFrameData == NULL) {
			ErrorMsg.Printf(_T("FFmpegSource video provider: %s"), FFMSErrorMessage);
			throw ErrorMsg;
		}
		if (CurFrameData.KeyFrame)
			KeyFramesList.Add(CurFrameNum);
	}
	KeyFramesLoaded = true;
#endif

	// TODO: VFR handling

	// we don't need this anymore
	FFMS_DestroyFrameIndex(Index);

	FrameNumber = 0;
}

///////////////
// Close video
void FFmpegSourceVideoProvider::Close() {
	if (SWSContext)
		sws_freeContext(SWSContext);
	SWSContext = NULL;
	if (VideoSource)
		FFMS_DestroyVideoSource(VideoSource);
	VideoSource = NULL;
	if (FrameAllocated)
		avpicture_free(&FrameRGB);
	FrameAllocated = false;
	if (BufferRGB)
		delete BufferRGB;

	KeyFramesLoaded = false;
	KeyFramesList.clear();
	FrameNumber = -1;
}

///////////////
// Get frame
const AegiVideoFrame FFmpegSourceVideoProvider::GetFrame(int n, int FormatType) {
	const AVFrameLite *SrcFrame = FFMS_GetFrame(VideoSource, n, FFMSErrorMessage, MessageSize);
	if (SrcFrame == NULL) {
		ErrorMsg.Printf(_T("FFmpegSource video provider: %s"), FFMSErrorMessage);
		throw ErrorMsg;
	}

	// set position
	FrameNumber = n;

	AVPicture *SrcPicture = reinterpret_cast<AVPicture *>(const_cast<AVFrameLite *>(SrcFrame));

	// prepare stuff for conversion to RGB32
	int w = VideoInfo->Width;
	int h = VideoInfo->Height;
	PixelFormat DstFormat;

	switch (FormatType) {
		case FORMAT_RGB32: DstFormat = PIX_FMT_RGB32; break;
		case FORMAT_RGB24: DstFormat = PIX_FMT_RGB24; break;
		// TODO: add YV12?
		default: throw _T("FFmpegSource video provider: upstream provider requested unknown or unsupported pixel format");
	}

	if (!SWSContext) {
		if (avpicture_alloc(&FrameRGB, DstFormat, w, h) != 0)
			throw _T("FFmpegSource video provider: could not allocate output picture buffer");
		FrameAllocated = true;
		unsigned int DstSize = avpicture_get_size(DstFormat,w,h);
		BufferRGB = new uint8_t[DstSize];

		// initialize swscaler context
		SWSContext = sws_getContext(w, h, VideoInfo->PixelFormat, w, h, DstFormat, SWS_BICUBIC, NULL, NULL, NULL);
		if (SWSContext == NULL)
			throw _T("FFmpegSource video provider: failed to initialize SWScale colorspace conversion");
	}
	avpicture_fill(&FrameRGB, BufferRGB, DstFormat, w, h);

	// this is what we'll return eventually
	AegiVideoFrame &DstFrame = CurFrame;

	// set some properties
	DstFrame.w = w;
	DstFrame.h = h;
	// only rgb supported for now
	DstFrame.flipped = false;
	DstFrame.invertChannels = true;
	DstFrame.format = (VideoFrameFormat)FormatType;

	// allocate destination frame
	for (int i=0;i<4;i++) DstFrame.pitch[i] = FrameRGB.linesize[i];
	DstFrame.Allocate();

	// let swscale do the conversion to RGB and write directly to the output frame
	sws_scale(SWSContext, SrcPicture->data, SrcPicture->linesize, 0, h, DstFrame.data, FrameRGB.linesize);

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