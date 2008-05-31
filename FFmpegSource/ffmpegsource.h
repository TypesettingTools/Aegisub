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

#ifndef FFMPEGSOURCE_H
#define FFMPEGSOURCE_H

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>

#include "stdiostream.h"
}

#include "MatroskaParser.h"
#include "avisynth.h"

#define strcmpi _strcmpi

enum AudioCacheFormat {acNone, acRaw};

struct FrameInfo {
	int64_t DTS;
	bool KeyFrame;
	FrameInfo(int64_t ADTS, bool AKeyFrame) : DTS(ADTS), KeyFrame(AKeyFrame) {};
};

typedef std::vector<FrameInfo> FrameInfoVector;

struct SampleInfo {
	int64_t SampleStart;
	int64_t FilePos;
	unsigned int FrameSize;
	bool KeyFrame;
	SampleInfo(int64_t ASampleStart, int64_t AFilePos, unsigned int AFrameSize, bool AKeyFrame) {
		SampleStart = ASampleStart;
		FilePos = AFilePos;
		FrameSize = AFrameSize;
		KeyFrame = AKeyFrame;
	}
};

typedef std::vector<SampleInfo> SampleInfoVector;

int GetPPCPUFlags(IScriptEnvironment *Env);
int GetSWSCPUFlags(IScriptEnvironment *Env);
int CSNameToPIXFMT(const char * ACSName, int ADefault);
int ResizerNameToSWSResizer(const char *AResizerName);
int GetNumberOfLogicalCPUs();
CodecID MatroskaToFFCodecID(TrackInfo *TI);

class FFPP : public GenericVideoFilter {
private:
	pp_context_t *PPContext;
	pp_mode_t *PPMode;
	SwsContext *SWSTo422P;
	SwsContext *SWSFrom422P;
	AVPicture InputPicture;
	AVPicture OutputPicture;
public:
	FFPP(PClip AChild, const char *APPString, int AQuality, IScriptEnvironment *Env);
	~FFPP();
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
};

class SWScale : public GenericVideoFilter {
private:
	SwsContext *Context;
	int OrigWidth;
	int OrigHeight;
	bool FlipOutput;
public:
	SWScale(PClip AChild, int AResizeToWidth, int AResizeToHeight, const char *AResizer, const char *AConvertToFormat, IScriptEnvironment *Env);
	~SWScale();
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *Env);
};

class FFBase : public IClip{
private:
	pp_context_t *PPContext;
	pp_mode_t *PPMode;
    SwsContext *SWS;
	int ConvertToFormat;
	AVPicture PPPicture;
protected:
	VideoInfo VI;
	AVFrame *DecodeFrame;
	AudioCacheFormat AudioCacheType;
	FILE *RawAudioCache;
	PVideoFrame LastFrame;
	int LastFrameNum;
	uint8_t *DecodingBuffer;

	FrameInfoVector Frames;

	int FindClosestKeyFrame(int AFrame);
	int FrameFromDTS(int64_t ADTS);
	int ClosestFrameFromDTS(int64_t ADTS);
	bool LoadFrameInfoFromFile(const char *AVideoCacheFile, const char *ASource, int AVideoTrack);
	bool SaveFrameInfoToFile(const char *AVideoCacheFile, const char *ASource, int AVideoTrack);
	bool SaveTimecodesToFile(const char *ATimecodeFile, int64_t ScaleD, int64_t ScaleN);

	bool OpenAudioCache(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, IScriptEnvironment *Env);
	FILE *FFBase::NewRawCacheWriter(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, IScriptEnvironment *Env);
	void FFBase::CloseRawCacheWriter(FILE *ARawCache);

	void InitPP(int AWidth, int AHeight, const char *APPString, int AQuality, int APixelFormat, IScriptEnvironment *Env);
	void SetOutputFormat(int ACurrentFormat, IScriptEnvironment *Env);
	PVideoFrame OutputFrame(AVFrame *AFrame, IScriptEnvironment *Env);
public:

	FFBase();
	~FFBase();

	bool __stdcall GetParity(int n) { return false; }
	void __stdcall SetCacheHints(int cachehints, int frame_range) { }
	const VideoInfo& __stdcall GetVideoInfo() { return VI; }
	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env);
};

class FFmpegSource : public FFBase {
private:
	AVFormatContext *FormatContext;
	AVCodecContext *VideoCodecContext;

	int VideoTrack;
	int	CurrentFrame;
	int SeekMode;

	int GetTrackIndex(int Index, CodecType ATrackType, IScriptEnvironment *Env);
	int DecodeNextFrame(AVFrame *Frame, int64_t *DTS);
public:
	FFmpegSource(const char *ASource, int AVideoTrack, int AAudioTrack, const char *ATimecodes, bool AVCache, const char *AVideoCache, const char *AAudioCache, const char *APPString, int AQuality, int AThreads, int ASeekMode, IScriptEnvironment *Env);
	~FFmpegSource();
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *Env);
};

class FFMatroskaSource : public FFBase {
private:
	StdIoStream	ST; 
    unsigned int BufferSize;
    CompressedStream *VideoCS;
    CompressedStream *AudioCS;
	AVCodecContext *VideoCodecContext;
	MatroskaFile *MF;
	char ErrorMessage[256];
    uint8_t *Buffer;
	int	CurrentFrame;

	int ReadFrame(uint64_t AFilePos, unsigned int AFrameSize, CompressedStream *ACS, IScriptEnvironment *Env);
	int DecodeNextFrame(AVFrame *AFrame, int64_t *AFirstStartTime, IScriptEnvironment* Env);
	int GetTrackIndex(int Index, unsigned char ATrackType, IScriptEnvironment *Env);
public:
	FFMatroskaSource(const char *ASource, int AVideoTrack, int AAudioTrack, const char *ATimecodes, bool AVCache, const char *AVideoCache, const char *AAudioCache, const char *APPString, int AQuality, int AThreads, IScriptEnvironment *Env);
	~FFMatroskaSource();
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *Env);
};

class FFAudioBase : public IClip{
protected:
	VideoInfo VI;
	uint8_t *DecodingBuffer;
	SampleInfoVector SI;

	size_t FindClosestAudioKeyFrame(int64_t Sample);
	bool LoadSampleInfoFromFile(const char *AAudioCacheFile, const char *ASource, int AAudioTrack);
	bool SaveSampleInfoToFile(const char *AAudioCacheFile, const char *ASource, int AAudioTrack);
public:
	FFAudioBase();
	~FFAudioBase();

	bool __stdcall GetParity(int n) { return false; }
	void __stdcall SetCacheHints(int cachehints, int frame_range) { }
	const VideoInfo& __stdcall GetVideoInfo() { return VI; }
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *Env) { return NULL; }
};

class FFmpegAudioSource : public FFAudioBase {
private:
	AVFormatContext *FormatContext;
	AVCodecContext *AudioCodecContext;

	int AudioTrack;
	FILE *RawCache;
    unsigned int BufferSize;
    uint8_t *Buffer;

	bool LoadSampleInfoFromFile(const char *AAudioCacheFile, const char *AAudioCacheFile2, const char *ASource, int AAudioTrack);
	int DecodeNextAudioBlock(uint8_t *ABuf, int64_t *ACount, uint64_t AFilePos, unsigned int AFrameSize, IScriptEnvironment *Env);
	int GetTrackIndex(int Index, CodecType ATrackType, IScriptEnvironment *Env);
public:
	FFmpegAudioSource(const char *ASource, int AAudioTrack, const char *AAudioCache, const char *AAudioCacheFile2, IScriptEnvironment *Env);
	~FFmpegAudioSource();

	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env);
};

class FFMatroskaAudioSource : public FFAudioBase {
private:
	StdIoStream	ST; 
    CompressedStream *AudioCS;
	AVCodecContext *AudioCodecContext;
	MatroskaFile *MF;
	char ErrorMessage[256];
    unsigned int BufferSize;
    uint8_t *Buffer;

	int ReadFrame(uint64_t AFilePos, unsigned int AFrameSize, CompressedStream *ACS, IScriptEnvironment *Env);
	int DecodeNextAudioBlock(uint8_t *ABuf, int64_t *ACount, uint64_t AFilePos, unsigned int AFrameSize, IScriptEnvironment *Env);
	int GetTrackIndex(int Index, unsigned char ATrackType, IScriptEnvironment *Env);
public:
	FFMatroskaAudioSource(const char *ASource, int AAudioTrack, const char *AAudioCache, IScriptEnvironment *Env);
	~FFMatroskaAudioSource();

	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env);
};

#endif
