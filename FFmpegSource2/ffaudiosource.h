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

#ifndef FFAUDIOSOURCE_H
#define FFAUDIOSOURCE_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <vector>
#include "indexing.h"
#include "utils.h"
#include "ffms.h"

class AudioBase {
protected:
	uint8_t *DecodingBuffer;
	FrameInfoVector Frames;
	AVCodecContext *CodecContext;
	AudioProperties AP;

	size_t FindClosestAudioKeyFrame(int64_t Sample);
public:
	AudioBase();
	~AudioBase();

	const AudioProperties& GetAudioProperties() { return AP; }
	virtual int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) = 0;
};

/*
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
*/

class MatroskaAudioSource : public AudioBase {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
    CompressedStream *CS;
	char ErrorMessage[256];

	int DecodeNextAudioBlock(uint8_t *Buf, int64_t *Count, uint64_t FilePos, unsigned int FrameSize, char *ErrorMsg, unsigned MsgSize);
	int GetTrackIndex(int &Index, char *ErrorMsg, unsigned MsgSize);
	void Free(bool CloseAudio);
public:
	MatroskaAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);
	~MatroskaAudioSource();

	int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
};

#endif
