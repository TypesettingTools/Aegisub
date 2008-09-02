//  Copyright (c) 2007-2008 Fredrik Mellbin
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "ffms.h"
#include "ffvideosource.h"
#include "indexing.h"

FFMS_API(void) FFMS_Init() {
	static bool InitDone = false;
	if (!InitDone)
		av_register_all();
	InitDone = true;
}

FFMS_API(VideoBase *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize) {
	switch (TrackIndices->Decoder) {
		case 0: return new FFVideoSource(SourceFile, Track, TrackIndices, PP, Threads, SeekMode, ErrorMsg, MsgSize);
		case 1: return new MatroskaVideoSource(SourceFile, Track, TrackIndices, PP, Threads, ErrorMsg, MsgSize);
		default: return NULL;
	}
}

FFMS_API(void) FFMS_DestroyVideoSource(VideoBase *VB) {
	delete VB;
}

FFMS_API(int) FFMS_GetVSTrack(VideoBase *VB) {
	return VB->GetTrack();
}

FFMS_API(const VideoProperties *) FFMS_GetVideoProperties(VideoBase *VB) {
	return &VB->GetVideoProperties();
}

FFMS_API(const AVFrameLite *) FFMS_GetFrame(VideoBase *VB, int n, char *ErrorMsg, unsigned MsgSize) {
	return (AVFrameLite *)VB->GetFrame(n, ErrorMsg, MsgSize);
}

FFMS_API(FrameIndex *) FFMS_CreateFrameIndex() {
	return new FrameIndex();
}

FFMS_API(void) FFMS_DestroyFrameIndex(FrameIndex *FI) {
	delete FI;
}

FFMS_API(int) FFMS_GetNumTracks(FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	return TrackIndices->size();
}

FFMS_API(int) FFMS_GetNumFrames(FrameInfoVector *FIV, char *ErrorMsg, unsigned MsgSize) {
	return FIV->size();
}

FFMS_API(const FrameInfo *) FFMS_GetFrameInfo(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize) {
	if (Frame < 0 || Frame >= FIV->size()) {
		_snprintf(ErrorMsg, MsgSize, "Invalid frame specified");
		return NULL;
	} else {
		return &(*FIV)[Frame];
	}	
}

FFMS_API(FrameInfoVector *) FFMS_GetTITrackIndex(FrameIndex *TrackIndices, int Track, char *ErrorMsg, unsigned MsgSize) {
	if (Track < 0 || Track >= TrackIndices->size()) {
		_snprintf(ErrorMsg, MsgSize, "Invalid track specified");
		return NULL;
	} else {
		return &(*TrackIndices)[Track];
	}	
}

FFMS_API(FrameInfoVector *) FFMS_GetVSTrackIndex(VideoBase *VB, char *ErrorMsg, unsigned MsgSize) {
	return VB->GetFrameInfoVector();
}

FFMS_API(int) FFMS_FindClosestKeyFrame(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize) {
	if (Frame < 0 || Frame >= FIV->size()) {
		_snprintf(ErrorMsg, MsgSize, "Out of range frame specified");
		return -1;
	} else {
		return FIV->FindClosestKeyFrame(Frame);
	}
}

FFMS_API(int) FFMS_FrameFromDTS(FrameInfoVector *FIV, int64_t DTS, char *ErrorMsg, unsigned MsgSize) {
	return FIV->FrameFromDTS(DTS);
}

FFMS_API(int) FFMS_ClosestFrameFromDTS(FrameInfoVector *FIV, int64_t DTS, char *ErrorMsg, unsigned MsgSize) {
	return FIV->ClosestFrameFromDTS(DTS);
}

FFMS_API(const TrackTimeBase *) FFMS_GetTimeBase(FrameInfoVector *FIV, char *ErrorMsg, unsigned MsgSize) {
	return &FIV->TB;
}

FFMS_API(int) FFMS_WriteTimecodes(FrameInfoVector *FIV, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize) {
	return FIV->WriteTimecodes(TimecodeFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_MakeIndex(const char *SourceFile, FrameIndex *TrackIndices, int AudioTrackMask, const char *AudioFile, IndexProgress *IP, char *ErrorMsg, unsigned MsgSize) {
	return MakeIndex(SourceFile, TrackIndices, AudioTrackMask, AudioFile, IP, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_ReadIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	return ReadIndex(IndexFile, TrackIndices, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	return WriteIndex(IndexFile, TrackIndices, ErrorMsg, MsgSize);
}

