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
#include <sstream>
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

class FFMS_AudioSource {
friend class FFSourceResources<FFMS_AudioSource>;
protected:
	TAudioCache AudioCache;
	int64_t CurrentSample;
	AlignedBuffer<uint8_t> DecodingBuffer;
	FFMS_Track Frames;
	AVCodecContext *CodecContext;
	int AudioTrack;
	FFMS_AudioProperties AP;

	virtual void Free(bool CloseCodec) = 0;
public:
	FFMS_AudioSource(const char *SourceFile, FFMS_Index *Index, int Track);
	virtual ~FFMS_AudioSource();
	FFMS_Track *GetTrack() { return &Frames; }
	const FFMS_AudioProperties& GetAudioProperties() { return AP; }
	virtual void GetAudio(void *Buf, int64_t Start, int64_t Count) = 0;
	void GetAudioCheck(int64_t Start, int64_t Count);
};

class FFLAVFAudio : public FFMS_AudioSource {
private:
	AVFormatContext *FormatContext;
	FFSourceResources<FFMS_AudioSource> Res;

	void DecodeNextAudioBlock(int64_t *Count);
	void Free(bool CloseCodec);
public:
	FFLAVFAudio(const char *SourceFile, int Track, FFMS_Index *Index);
	void GetAudio(void *Buf, int64_t Start, int64_t Count);
};

class FFMatroskaAudio : public FFMS_AudioSource {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
    TrackCompressionContext *TCC;
	char ErrorMessage[256];
	FFSourceResources<FFMS_AudioSource> Res;
	size_t PacketNumber;

	void DecodeNextAudioBlock(int64_t *Count);
	void Free(bool CloseCodec);
public:
	FFMatroskaAudio(const char *SourceFile, int Track, FFMS_Index *Index);
	void GetAudio(void *Buf, int64_t Start, int64_t Count);
};

#ifdef HAALISOURCE

class FFHaaliAudio : public FFMS_AudioSource {
private:
	CComPtr<IMMContainer> pMMC;
	std::vector<uint8_t> CodecPrivate;
	FFSourceResources<FFMS_AudioSource> Res;

	void DecodeNextAudioBlock(int64_t *AFirstStartTime, int64_t *Count);
	void Free(bool CloseCodec);
public:
	FFHaaliAudio(const char *SourceFile, int Track, FFMS_Index *Index, enum FFMS_Sources SourceMode);
	void GetAudio(void *Buf, int64_t Start, int64_t Count);
};

#endif // HAALISOURCE

#endif
