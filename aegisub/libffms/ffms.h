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

#ifdef __cplusplus
#	define EXTERN_C extern "C"
#	define FFMS_CLASS_TYPE class
#else
#	define EXTERN_C
#	define FFMS_CLASS_TYPE struct
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

FFMS_CLASS_TYPE FFVideo;
FFMS_CLASS_TYPE FFAudio;
FFMS_CLASS_TYPE FFIndexer;
FFMS_CLASS_TYPE FFIndex;
FFMS_CLASS_TYPE FFTrack;

enum FFMS_SeekMode {
	FFMS_SEEK_LINEAR_NO_RW  = -1,
	FFMS_SEEK_LINEAR = 0,
	FFMS_SEEK_NORMAL = 1,
	FFMS_SEEK_UNSAFE = 2,
	FFMS_SEEK_AGGRESSIVE = 3
};

enum FFMS_TrackType {
	FFMS_TYPE_UNKNOWN = -1,
	FFMS_TYPE_VIDEO,
    FFMS_TYPE_AUDIO,
    FFMS_TYPE_DATA,
    FFMS_TYPE_SUBTITLE,
    FFMS_TYPE_ATTACHMENT
};

// This is a subset of the original AVFrame only containing the most used parts.
// Even if it might seem like a good idea to cast it back to a full AVFrame to
// access a few more values you really shouldn't do that. Only the values present
// in TAVFrameLite are actually updated when postprocessing is used.

struct TAVFrameLite {
    uint8_t *Data[4];
    int Linesize[4];
    uint8_t *Base[4];
    int KeyFrame;
    int PictType;
};

struct TTrackTimeBase {
	int64_t Num;
	int64_t Den;
};

#define FFMS_FRAMEINFO_COMMON int64_t DTS; bool KeyFrame;

struct FFFrameInfo {
	FFMS_FRAMEINFO_COMMON
};

struct TVideoProperties {
	int Width;
	int Height;
	int FPSDenominator;
	int FPSNumerator;
	int NumFrames;
	int VPixelFormat;
	int SARNum;
	int SARDen;
	int CropTop;
	int CropBottom;
	int CropLeft;
	int CropRight;
	double FirstTime;
	double LastTime;
};

struct TAudioProperties {
	int SampleRate;
	int Channels;
	int BitsPerSample;
	bool Float;
	int64_t NumSamples;
	double FirstTime;
	double LastTime;
};

typedef int (FFMS_CC *TIndexCallback)(int64_t Current, int64_t Total, void *ICPrivate);
typedef int (FFMS_CC *TAudioNameCallback)(const char *SourceFile, int Track, const TAudioProperties *AP, char *FileName, void *Private);

// Most functions return 0 on success
// Functions without error message output can be assumed to never fail
FFMS_API(void) FFMS_Init();
FFMS_API(int) FFMS_GetLogLevel();
FFMS_API(void) FFMS_SetLogLevel(int Level);
FFMS_API(FFVideo *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FFIndex *Index, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FFAudio *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FFIndex *Index, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_DestroyVideoSource(FFVideo *V);
FFMS_API(void) FFMS_DestroyAudioSource(FFAudio *A);
FFMS_API(const TVideoProperties *) FFMS_GetVideoProperties(FFVideo *V);
FFMS_API(const TAudioProperties *) FFMS_GetAudioProperties(FFAudio *A);
FFMS_API(const TAVFrameLite *) FFMS_GetFrame(FFVideo *V, int n, char *ErrorMsg, unsigned MsgSize);
FFMS_API(const TAVFrameLite *) FFMS_GetFrameByTime(FFVideo *V, double Time, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetAudio(FFAudio *A, void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_SetOutputFormat(FFVideo *V, int64_t TargetFormats, int Width, int Height, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_ResetOutputFormat(FFVideo *V);
FFMS_API(void) FFMS_DestroyIndex(FFIndex *Index);
FFMS_API(int) FFMS_GetFirstTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetFirstIndexedTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetNumTracks(FFIndex *Index);
FFMS_API(int) FFMS_GetNumTracksI(FFIndexer *Indexer);
FFMS_API(FFMS_TrackType) FFMS_GetTrackType(FFTrack *T);
FFMS_API(FFMS_TrackType) FFMS_GetTrackTypeI(FFIndexer *Indexer, int Track);
FFMS_API(const char *) FFMS_GetCodecNameI(FFIndexer *Indexer, int Track);
FFMS_API(int) FFMS_GetNumFrames(FFTrack *T);
FFMS_API(const FFFrameInfo *) FFMS_GetFrameInfo(FFTrack *T, int Frame);
FFMS_API(FFTrack *) FFMS_GetTrackFromIndex(FFIndex *Index, int Track);
FFMS_API(FFTrack *) FFMS_GetTrackFromVideo(FFVideo *V);
FFMS_API(FFTrack *) FFMS_GetTrackFromAudio(FFAudio *A);
FFMS_API(const TTrackTimeBase *) FFMS_GetTimeBase(FFTrack *T);
FFMS_API(int) FFMS_WriteTimecodes(FFTrack *T, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FFIndex *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_DefaultAudioFilename(const char *SourceFile, int Track, const TAudioProperties *AP, char *FileName, void *Private);
FFMS_API(FFIndexer *) FFMS_CreateIndexer(const char *SourceFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FFIndex *) FFMS_DoIndexing(FFIndexer *Indexer, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_CancelIndexing(FFIndexer *Indexer);
FFMS_API(FFIndex *) FFMS_ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FFIndex *Index, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetPixFmt(const char *Name);

#endif
