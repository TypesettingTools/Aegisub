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

#ifndef FFMS_H
#define FFMS_H

#include <stdint.h>

#define FFMS_EXPORTS

#ifdef __cplusplus
#  define EXTERN_C extern "C"
#else
#  define EXTERN_C
#endif

#ifdef FFMS_EXPORTS
#  define FFMS_API(ret) EXTERN_C __declspec(dllexport) ret __stdcall
#else
#  define FFMS_API(ret) EXTERN_C __declspec(dllimport) ret __stdcall
#endif

class VideoBase;
class FrameIndex;
class FrameInfoVector;

// This is a subset of the original AVFrame only containing the most used parts.
// Even if it might seem like a good idea to cast it back to a full AVFrame to
// access a few more values you really shouldn't do that. Only the values present
// in AVFrameLite are actually updated when postprocessing is used.

struct AVFrameLite {
    uint8_t *Data[4];
    int Linesize[4];
    uint8_t *Base[4];
    int KeyFrame;
    int PictType;
};

struct TrackTimeBase {
	int Num;
	int Den;
};

struct FrameInfo {
	int64_t DTS;
	bool KeyFrame;
	FrameInfo(int64_t DTS, bool KeyFrame);
};

struct VideoProperties {
public:
	int Width;
	int Height;
	int FPSDenominator;
	int FPSNumerator;
	int NumFrames;
	int PixelFormat;
	int SARNum;
	int SARDen;
	int CropTop;
	int CropBottom;
	int CropLeft;
	int CropRight;
};

FFMS_API(void) FFMS_Init();
FFMS_API(VideoBase *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_DestroyVideoSource(VideoBase *VB);
FFMS_API(int) FFMS_GetVSTrack(VideoBase *VB);
FFMS_API(const VideoProperties *) FFMS_GetVideoProperties(VideoBase *VB);
FFMS_API(const AVFrameLite *) FFMS_GetFrame(VideoBase *VB, int n, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FrameIndex *) FFMS_CreateFrameIndex();
FFMS_API(void) FFMS_DestroyFrameIndex(FrameIndex *FI);
FFMS_API(int) FFMS_GetNumTracks(FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);
FFMS_API(const FrameInfo *) FFMS_GetFrameInfo(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FrameInfoVector *) FFMS_GetTITrackIndex(FrameIndex *TrackIndices, int Track, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FrameInfoVector *) FFMS_GetVSTrackIndex(VideoBase *VB, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_FindClosestKeyFrame(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_FrameFromDTS(FrameInfoVector *FIV, int64_t DTS, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_ClosestFrameFromDTS(FrameInfoVector *FIV, int64_t DTS, char *ErrorMsg, unsigned MsgSize);
FFMS_API(const TrackTimeBase *) FFMS_GetTimeBase(FrameInfoVector *FIV, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_WriteTimecodes(FrameInfoVector *FIV, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_MakeIndex(const char *SourceFile, FrameIndex *TrackIndices, int AudioTrackMask, const char *AudioFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_ReadIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);

#endif
