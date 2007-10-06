//  Copyright (c) 2007 Fredrik Mellbin
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

#define FLAC_CACHE

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>

#ifdef FLAC_CACHE
#include <stream_decoder.h>
#include <stream_encoder.h>
#endif // FLAC_CACHE

extern "C" {
#include <ffmpeg\avformat.h>
#include <ffmpeg\avcodec.h>
#include <ffmpeg\swscale.h>
#include <postproc\postprocess.h>

#include "stdiostream.h"
}

#include "MatroskaParser.h"
#include "avisynth.h"

enum AudioCacheFormat {acNone, acRaw, acFLAC};

int GetPPCPUFlags(IScriptEnvironment *Env);
int GetSWSCPUFlags(IScriptEnvironment *Env);
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

#ifdef FLAC_CACHE
	FLAC__StreamDecoder *FLACAudioCache;
	FLAC__int32 *FLACBuffer;
#endif // FLAC_CACHE



	struct FrameInfo {
		int64_t DTS;
		bool KeyFrame;
		FrameInfo(int64_t ADTS, bool AKeyFrame) : DTS(ADTS), KeyFrame(AKeyFrame) {};
	};

	std::vector<FrameInfo> FrameToDTS;

	int FindClosestKeyFrame(int AFrame);
	int FrameFromDTS(int64_t ADTS);
	int ClosestFrameFromDTS(int64_t ADTS);
	bool LoadFrameInfoFromFile(const char *AVideoCacheFile, const char *ASource, int AVideoTrack);
	bool SaveFrameInfoToFile(const char *AVideoCacheFile, const char *ASource, int AVideoTrack);
	bool SaveTimecodesToFile(const char *ATimecodeFile, int64_t ScaleD, int64_t ScaleN);

	bool OpenAudioCache(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, IScriptEnvironment *Env);
#ifdef FLAC_CACHE
	FLAC__StreamEncoder *FFBase::NewFLACCacheWriter(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, int ACompression, IScriptEnvironment *Env);
	void FFBase::CloseFLACCacheWriter(FLAC__StreamEncoder *AFSE);
#endif // FLAC_CACHE
	FILE *FFBase::NewRawCacheWriter(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, IScriptEnvironment *Env);
	void FFBase::CloseRawCacheWriter(FILE *ARawCache);

	void InitPP(int AWidth, int AHeight, const char *APPString, int AQuality, int APixelFormat, IScriptEnvironment *Env);
	void SetOutputFormat(int ACurrentFormat, IScriptEnvironment *Env);
	PVideoFrame OutputFrame(AVFrame *AFrame, IScriptEnvironment *Env);
public:
	// FLAC decoder variables, have to be public
	FILE *FCFile;
	__int64 FCCount;
	void *FCBuffer;
	bool FCError;

	FFBase();
	~FFBase();

	bool __stdcall GetParity(int n) { return false; }
	void __stdcall SetCacheHints(int cachehints, int frame_range) { }
	const VideoInfo& __stdcall GetVideoInfo() { return VI; }
	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment* Env);
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
	FFmpegSource(const char *ASource, int AVideoTrack, int AAudioTrack, const char *ATimecodes, bool AVCache, const char *AVideoCache, const char *AAudioCache, int AACCompression, const char *APPString, int AQuality, int ASeekMode, IScriptEnvironment* Env);
	~FFmpegSource();
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
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
	FFMatroskaSource(const char *ASource, int AVideoTrack, int AAudioTrack, const char *ATimecodes, bool AVCache, const char *AVideoCache, const char *AAudioCache, int AACCompression, const char *APPString, int AQuality, IScriptEnvironment* Env);
	~FFMatroskaSource();
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
};

#endif