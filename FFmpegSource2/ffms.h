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

enum TrackType {
	FFMS_TYPE_VIDEO = 0,
    FFMS_TYPE_AUDIO = 1,
};

// PixelFormat declarations from avutil.h so external libraries don't necessarily have to include and ffmpeg headers
enum FFMS_PixelFormat {
    FFMS_PIX_FMT_NONE= -1,
    FFMS_PIX_FMT_YUV420P,   ///< Planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    FFMS_PIX_FMT_YUYV422,   ///< Packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    FFMS_PIX_FMT_RGB24,     ///< Packed RGB 8:8:8, 24bpp, RGBRGB...
    FFMS_PIX_FMT_BGR24,     ///< Packed RGB 8:8:8, 24bpp, BGRBGR...
    FFMS_PIX_FMT_YUV422P,   ///< Planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    FFMS_PIX_FMT_YUV444P,   ///< Planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    FFMS_PIX_FMT_RGB32,     ///< Packed RGB 8:8:8, 32bpp, (msb)8A 8R 8G 8B(lsb), in cpu endianness
    FFMS_PIX_FMT_YUV410P,   ///< Planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    FFMS_PIX_FMT_YUV411P,   ///< Planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    FFMS_PIX_FMT_RGB565,    ///< Packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), in cpu endianness
    FFMS_PIX_FMT_RGB555,    ///< Packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), in cpu endianness most significant bit to 0
    FFMS_PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
    FFMS_PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black
    FFMS_PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white
    FFMS_PIX_FMT_PAL8,      ///< 8 bit with PIX_FMT_RGB32 palette
    FFMS_PIX_FMT_YUVJ420P,  ///< Planar YUV 4:2:0, 12bpp, full scale (jpeg)
    FFMS_PIX_FMT_YUVJ422P,  ///< Planar YUV 4:2:2, 16bpp, full scale (jpeg)
    FFMS_PIX_FMT_YUVJ444P,  ///< Planar YUV 4:4:4, 24bpp, full scale (jpeg)
    FFMS_PIX_FMT_XVMC_MPEG2_MC,///< XVideo Motion Acceleration via common packet passing(xvmc_render.h)
    FFMS_PIX_FMT_XVMC_MPEG2_IDCT,
    FFMS_PIX_FMT_UYVY422,   ///< Packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    FFMS_PIX_FMT_UYYVYY411, ///< Packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    FFMS_PIX_FMT_BGR32,     ///< Packed RGB 8:8:8, 32bpp, (msb)8A 8B 8G 8R(lsb), in cpu endianness
    FFMS_PIX_FMT_BGR565,    ///< Packed RGB 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), in cpu endianness
    FFMS_PIX_FMT_BGR555,    ///< Packed RGB 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), in cpu endianness most significant bit to 1
    FFMS_PIX_FMT_BGR8,      ///< Packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    FFMS_PIX_FMT_BGR4,      ///< Packed RGB 1:2:1,  4bpp, (msb)1B 2G 1R(lsb)
    FFMS_PIX_FMT_BGR4_BYTE, ///< Packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    FFMS_PIX_FMT_RGB8,      ///< Packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    FFMS_PIX_FMT_RGB4,      ///< Packed RGB 1:2:1,  4bpp, (msb)2R 3G 3B(lsb)
    FFMS_PIX_FMT_RGB4_BYTE, ///< Packed RGB 1:2:1,  8bpp, (msb)2R 3G 3B(lsb)
    FFMS_PIX_FMT_NV12,      ///< Planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 for UV
    FFMS_PIX_FMT_NV21,      ///< as above, but U and V bytes are swapped

    FFMS_PIX_FMT_RGB32_1,   ///< Packed RGB 8:8:8, 32bpp, (msb)8R 8G 8B 8A(lsb), in cpu endianness
    FFMS_PIX_FMT_BGR32_1,   ///< Packed RGB 8:8:8, 32bpp, (msb)8B 8G 8R 8A(lsb), in cpu endianness

    FFMS_PIX_FMT_GRAY16BE,  ///<        Y        , 16bpp, big-endian
    FFMS_PIX_FMT_GRAY16LE,  ///<        Y        , 16bpp, little-endian
    FFMS_PIX_FMT_YUV440P,   ///< Planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    FFMS_PIX_FMT_YUVJ440P,  ///< Planar YUV 4:4:0 full scale (jpeg)
    FFMS_PIX_FMT_YUVA420P,  ///< Planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
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
	int Num;
	int Den;
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
	int PixelFormat;
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
