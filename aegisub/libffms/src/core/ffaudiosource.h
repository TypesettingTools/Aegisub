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

#ifndef FFAUDIOSOURCE_H
#define FFAUDIOSOURCE_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <vector>
#include <list>
#include <memory>
#include "indexing.h"
#include "utils.h"
#include "ffms.h"

#ifdef HAALISOURCE
#	define WIN32_LEAN_AND_MEAN
#	define _WIN32_DCOM
#	include <windows.h>
#	include <tchar.h>
#	include <atlbase.h>
#	include <dshow.h>
#	include <initguid.h>
#	include "CoParser.h"
#	include "guids.h"
#endif

class TAudioBlock {
public:
	int64_t Start;
	int64_t Samples;
	uint8_t *Data;

	TAudioBlock(int64_t Start, int64_t Samples, uint8_t *SrcData, size_t SrcBytes);
	~TAudioBlock();
};

class TAudioCache : private std::list<TAudioBlock *> {
private:
	int MaxCacheBlocks;
	int BytesPerSample;
	static bool AudioBlockComp(TAudioBlock *A, TAudioBlock *B);
public:
	TAudioCache();
	~TAudioCache();
	void Initialize(int BytesPerSample, int MaxCacheBlocks);
	void CacheBlock(int64_t Start, int64_t Samples, uint8_t *SrcData);
	int64_t FillRequest(int64_t Start, int64_t Samples, uint8_t *Dst);
};

class FFAudio {
protected:
	TAudioCache AudioCache;
	int64_t CurrentSample;
	std::vector<uint8_t> DecodingBuffer;
	FFTrack Frames;
	AVCodecContext *CodecContext;
	int AudioTrack;
	FFAudioProperties AP;
public:
	FFAudio(const char *SourceFile, FFIndex *Index, char *ErrorMsg, unsigned MsgSize);
	virtual ~FFAudio();
	FFTrack *GetFFTrack() { return &Frames; }
	const FFAudioProperties& GetFFAudioProperties() { return AP; }
	virtual int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) = 0;
};

class FFLAVFAudio : public FFAudio {
private:
	AVFormatContext *FormatContext;

	int DecodeNextAudioBlock(int64_t *Count, char *ErrorMsg, unsigned MsgSize);
	void Free(bool CloseCodec);
public:
	FFLAVFAudio(const char *SourceFile, int Track, FFIndex *Index, char *ErrorMsg, unsigned MsgSize);
	~FFLAVFAudio();
	int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
};

class FFMatroskaAudio : public FFAudio {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
    CompressedStream *CS;
	char ErrorMessage[256];

	int DecodeNextAudioBlock(int64_t *Count, int AudioBlock, char *ErrorMsg, unsigned MsgSize);
	void Free(bool CloseCodec);
public:
	FFMatroskaAudio(const char *SourceFile, int Track, FFIndex *Index, char *ErrorMsg, unsigned MsgSize);
	~FFMatroskaAudio();
	int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
};

#ifdef HAALISOURCE

class FFHaaliAudio : public FFAudio {
private:
	CComPtr<IMMContainer> pMMC;
	std::vector<uint8_t> CodecPrivate;

	void Free(bool CloseCodec);
	int DecodeNextAudioBlock(int64_t *AFirstStartTime, int64_t *Count, char *ErrorMsg, unsigned MsgSize);
public:
	FFHaaliAudio(const char *SourceFile, int Track, FFIndex *Index, int SourceMode, char *ErrorMsg, unsigned MsgSize);
	~FFHaaliAudio();
	int GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize);
};

#endif // HAALISOURCE

#endif
