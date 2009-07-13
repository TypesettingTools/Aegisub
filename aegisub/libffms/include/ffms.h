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

enum FFMS_CPUFeatures {
	FFMS_CPU_CAPS_MMX		= 0x01,
	FFMS_CPU_CAPS_MMX2		= 0x02,
	FFMS_CPU_CAPS_3DNOW		= 0x04,
	FFMS_CPU_CAPS_ALTIVEC	= 0x08,
	FFMS_CPU_CAPS_BFIN		= 0x10
};

enum FFMS_SeekMode {
	FFMS_SEEK_LINEAR_NO_RW	= -1,
	FFMS_SEEK_LINEAR		= 0,
	FFMS_SEEK_NORMAL		= 1,
	FFMS_SEEK_UNSAFE		= 2,
	FFMS_SEEK_AGGRESSIVE	= 3
};

enum FFMS_TrackType {
	FFMS_TYPE_UNKNOWN = -1,
	FFMS_TYPE_VIDEO,
    FFMS_TYPE_AUDIO,
    FFMS_TYPE_DATA,
    FFMS_TYPE_SUBTITLE,
    FFMS_TYPE_ATTACHMENT
};

enum FFMS_SampleFormat {
    FFMS_FMT_U8 = 0,
    FFMS_FMT_S16,
    FFMS_FMT_S32,
    FFMS_FMT_FLT,
    FFMS_FMT_DBL
};

enum FFMS_AudioChannel {
	FFMS_CH_FRONT_LEFT				= 0x00000001,
	FFMS_CH_FRONT_RIGHT				= 0x00000002,
	FFMS_CH_FRONT_CENTER			= 0x00000004,
	FFMS_CH_LOW_FREQUENCY			= 0x00000008,
	FFMS_CH_BACK_LEFT				= 0x00000010,
	FFMS_CH_BACK_RIGHT				= 0x00000020,
	FFMS_CH_FRONT_LEFT_OF_CENTER	= 0x00000040,
	FFMS_CH_FRONT_RIGHT_OF_CENTER	= 0x00000080,
	FFMS_CH_BACK_CENTER				= 0x00000100,
	FFMS_CH_SIDE_LEFT				= 0x00000200,
	FFMS_CH_SIDE_RIGHT				= 0x00000400,
	FFMS_CH_TOP_CENTER				= 0x00000800,
	FFMS_CH_TOP_FRONT_LEFT			= 0x00001000,
	FFMS_CH_TOP_FRONT_CENTER		= 0x00002000,
	FFMS_CH_TOP_FRONT_RIGHT			= 0x00004000,
	FFMS_CH_TOP_BACK_LEFT			= 0x00008000,
	FFMS_CH_TOP_BACK_CENTER			= 0x00010000,
	FFMS_CH_TOP_BACK_RIGHT			= 0x00020000,
	FFMS_CH_STEREO_LEFT				= 0x20000000,
	FFMS_CH_STEREO_RIGHT			= 0x40000000
};

enum FFMS_Resizers {
	FFMS_RESIZER_FAST_BILINEAR	= 0x01,
	FFMS_RESIZER_BILINEAR		= 0x02,
	FFMS_RESIZER_BICUBIC		= 0x04,
	FFMS_RESIZER_X				= 0x08,
	FFMS_RESIZER_POINT			= 0x10,
	FFMS_RESIZER_AREA			= 0x20,
	FFMS_RESIZER_BICUBLIN		= 0x40,
	FFMS_RESIZER_GAUSS			= 0x80,
	FFMS_RESIZER_SINC			= 0x100,
	FFMS_RESIZER_LANCZOS		= 0x200,
	FFMS_RESIZER_SPLINE			= 0x400
};

struct FFAVFrame {
	uint8_t *Data[4];
	int Linesize[4];
	int EncodedWidth;
	int EncodedHeight;
	int EncodedPixelFormat;
	int ScaledWidth;
	int ScaledHeight;
	int ConvertedPixelFormat;
	int KeyFrame;
	int RepeatPict;
	int InterlacedFrame;
	int TopFieldFirst;
	char PictType;
};

struct FFTrackTimeBase {
	int64_t Num;
	int64_t Den;
};

#define FFMS_FRAMEINFO_COMMON int64_t DTS; int RepeatPict; bool KeyFrame;

struct FFFrameInfo {
	FFMS_FRAMEINFO_COMMON
};

struct FFVideoProperties {
	int Width;
	int Height;
	int FPSDenominator;
	int FPSNumerator;
	int RFFDenominator;
	int RFFNumerator;
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

struct FFAudioProperties {
	int SampleFormat;
	int SampleRate;
	int BitsPerSample;
	int Channels;
	int64_t ChannelLayout;
	int64_t NumSamples;
	double FirstTime;
	double LastTime;
};

typedef int (FFMS_CC *TIndexCallback)(int64_t Current, int64_t Total, void *ICPrivate);
typedef int (FFMS_CC *TAudioNameCallback)(const char *SourceFile, int Track, const FFAudioProperties *AP, char *FileName, int FNSize, void *Private);

// Most functions return 0 on success
// Functions without error message output can be assumed to never fail in a graceful way
FFMS_API(void) FFMS_Init(int CPUFeatures);
FFMS_API(int) FFMS_GetLogLevel();
FFMS_API(void) FFMS_SetLogLevel(int Level);
FFMS_API(FFVideo *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FFIndex *Index, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FFAudio *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FFIndex *Index, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_DestroyVideoSource(FFVideo *V);
FFMS_API(void) FFMS_DestroyAudioSource(FFAudio *A);
FFMS_API(const FFVideoProperties *) FFMS_GetVideoProperties(FFVideo *V);
FFMS_API(const FFAudioProperties *) FFMS_GetAudioProperties(FFAudio *A);
FFMS_API(const FFAVFrame *) FFMS_GetFrame(FFVideo *V, int n, char *ErrorMsg, unsigned MsgSize);
FFMS_API(const FFAVFrame *) FFMS_GetFrameByTime(FFVideo *V, double Time, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetAudio(FFAudio *A, void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_SetOutputFormatV(FFVideo *V, int64_t TargetFormats, int Width, int Height, int Resizer, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_ResetOutputFormatV(FFVideo *V);
FFMS_API(void) FFMS_DestroyIndex(FFIndex *Index);
FFMS_API(int) FFMS_GetFirstTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetFirstIndexedTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetNumTracks(FFIndex *Index);
FFMS_API(int) FFMS_GetNumTracksI(FFIndexer *Indexer);
FFMS_API(int) FFMS_GetTrackType(FFTrack *T);
FFMS_API(int) FFMS_GetTrackTypeI(FFIndexer *Indexer, int Track);
FFMS_API(const char *) FFMS_GetCodecNameI(FFIndexer *Indexer, int Track);
FFMS_API(int) FFMS_GetNumFrames(FFTrack *T);
FFMS_API(const FFFrameInfo *) FFMS_GetFrameInfo(FFTrack *T, int Frame);
FFMS_API(FFTrack *) FFMS_GetTrackFromIndex(FFIndex *Index, int Track);
FFMS_API(FFTrack *) FFMS_GetTrackFromVideo(FFVideo *V);
FFMS_API(FFTrack *) FFMS_GetTrackFromAudio(FFAudio *A);
FFMS_API(const FFTrackTimeBase *) FFMS_GetTimeBase(FFTrack *T);
FFMS_API(int) FFMS_WriteTimecodes(FFTrack *T, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FFIndex *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_DefaultAudioFilename(const char *SourceFile, int Track, const FFAudioProperties *AP, char *FileName, int FNSize, void *Private);
FFMS_API(FFIndexer *) FFMS_CreateIndexer(const char *SourceFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(FFIndex *) FFMS_DoIndexing(FFIndexer *Indexer, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize);
FFMS_API(void) FFMS_CancelIndexing(FFIndexer *Indexer);
FFMS_API(FFIndex *) FFMS_ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_IndexBelongsToFile(FFIndex *Index, const char *SourceFile, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FFIndex *Index, char *ErrorMsg, unsigned MsgSize);
FFMS_API(int) FFMS_GetPixFmt(const char *Name);

#endif
