//  Copyright (c) 2007-2009 Fredrik Mellbin
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
#include <libavutil/pixfmt.h>

#ifdef __cplusplus
#	define EXTERN_C extern "C"
#else
#	define EXTERN_C
#endif

#ifdef _WIN32
#	define FFMS_CC __stdcall
#	ifdef FFMS_EXPORTS
#		define FFMS_API(ret) EXTERN_C __declspec(dllexport) ret FFMS_CC
#	else
#		define FFMS_API(ret) EXTERN_C __declspec(dllimport) ret FFMS_CC
#	endif
#else
#	define FFMS_CC
#	define FFMS_API(ret) EXTERN_C ret FFMS_CC
#endif

class VideoBase;
class AudioBase;
class FrameIndex;
class FrameInfoVector;

typedef int (FFMS_CC *IndexCallback)(int State, int64_t Current, int64_t Total, void *Private);

enum FFMS_SeekMode {
	FFMS_SEEK_LINEAR_NO_RW  = -1,
	FFMS_SEEK_LINEAR = 0,
	FFMS_SEEK_NORMAL = 1,
	FFMS_SEEK_UNSAFE = 2,
	FFMS_SEEK_AGGRESSIVE = 3,
};

enum FFMS_TrackType {
	FFMS_TYPE_VIDEO = 0,
    FFMS_TYPE_AUDIO = 1,
};

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
	int64_t Num;
	int64_t Den;
};

class FrameInfo {
public:
	int64_t DTS;
	int64_t SampleStart;
	int64_t FilePos;
	unsigned int FrameSize;
	bool KeyFrame;
	FrameInfo(int64_t DTS, bool KeyFrame);
	FrameInfo(int64_t DTS, int64_t SampleStart, bool KeyFrame);
	FrameInfo(int64_t SampleStart, int64_t FilePos, unsigned int FrameSize, bool KeyFrame);
};

struct VideoProperties {
	int Width;
	int Height;
	int FPSDenominator;
	int FPSNumerator;
	int NumFrames;
	PixelFormat VPixelFormat;
	int SARNum;
	int SARDen;
	int CropTop;
	int CropBottom;
	int CropLeft;
	int CropRight;
	double FirstTime;
	double LastTime;
};

struct AudioProperties {
	int SampleRate;
	int Channels;
	int BitsPerSample;
	bool Float;
	int64_t NumSamples;
};

// Most functions return 0 on success
// Functions without error message output can be assumed to never fail
FFMS_API(void) FFMS_Init();
FFMS_API(void) FFMS_NoLog();
FFMS_API(VideoBase *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize);
FFMS_API(AudioBase *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_DestroyVideoSource(VideoBase *VB);
FFMS_API(void) FFMS_DestroyAudioSource(AudioBase *AB);
FFMS_API(const VideoProperties *) FFMS_GetVideoProperties(VideoBase *VB);
FFMS_API(const AudioProperties *) FFMS_GetAudioProperties(AudioBase *AB);
FFMS_API(const AVFrameLite *) FFMS_GetFrame(VideoBase *VB, int n, char *ErrorMsg, unsigned MsgSize);
FFMS_API(const AVFrameLite *) FFMS_GetFrameByTime(VideoBase *VB, double Time, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetAudio(AudioBase *AB, void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_SetOutputFormat(VideoBase *VB, int TargetFormat, int Width, int Height, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_ResetOutputFormat(VideoBase *VB);
FFMS_API(void) FFMS_DestroyFrameIndex(FrameIndex *FI);
FFMS_API(int) FFMS_GetFirstTrackOfType(FrameIndex *TrackIndices, int TrackType, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetNumTracks(FrameIndex *TrackIndices);
FFMS_API(int) FFMS_GetTrackType(FrameInfoVector *FIV);
FFMS_API(int) FFMS_GetNumFrames(FrameInfoVector *FIV);
FFMS_API(const FrameInfo *) FFMS_GetFrameInfo(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FrameInfoVector *) FFMS_GetTITrackIndex(FrameIndex *TrackIndices, int Track, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FrameInfoVector *) FFMS_GetVSTrackIndex(VideoBase *VB);
FFMS_API(FrameInfoVector *) FFMS_GetASTrackIndex(AudioBase *AB);
FFMS_API(int) FFMS_FindClosestKeyFrame(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_FrameFromDTS(FrameInfoVector *FIV, int64_t DTS);
FFMS_API(int) FFMS_ClosestFrameFromDTS(FrameInfoVector *FIV, int64_t DTS);
FFMS_API(const TrackTimeBase *) FFMS_GetTimeBase(FrameInfoVector *FIV);
FFMS_API(int) FFMS_WriteTimecodes(FrameInfoVector *FIV, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FrameIndex *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, IndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FrameIndex *) FFMS_ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);

#endif
