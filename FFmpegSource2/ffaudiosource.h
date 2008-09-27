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

class FFAudioSource : public AudioBase {
private:
	AVFormatContext *FormatContext;
	int AudioTrack;

	int DecodeNextAudioBlock(uint8_t *Buf, int64_t *Count, char *ErrorMsg, unsigned MsgSize);
	void Free(bool CloseCodec);
public:
	FFAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);
	~FFAudioSource();

	int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
};

class MatroskaAudioSource : public AudioBase {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
    CompressedStream *CS;
	char ErrorMessage[256];

	int DecodeNextAudioBlock(uint8_t *Buf, int64_t *Count, uint64_t FilePos, unsigned int FrameSize, char *ErrorMsg, unsigned MsgSize);
	void Free(bool CloseCodec);
public:
	MatroskaAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);
	~MatroskaAudioSource();

	int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
};

#endif
