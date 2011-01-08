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

class FFMS_AudioSource {
	struct AudioBlock {
		int64_t Age;
		int64_t Start;
		int64_t Samples;
		std::vector<uint8_t> Data;

		AudioBlock(int64_t Start, int64_t Samples, uint8_t *SrcData, size_t SrcBytes)
			: Start(Start)
			, Samples(Samples)
			, Data(SrcData, SrcData + SrcBytes)
		{
			static int64_t Now = 0;
			Age = Now++;
		}
	};
	typedef std::list<AudioBlock>::iterator CacheIterator;

	// delay in samples to apply to the audio
	int64_t Delay;
	// cache of decoded audio blocks
	std::list<AudioBlock> Cache;
	// max size of the cache in blocks
	size_t MaxCacheBlocks;
	// pointer to last element of the cache which should never be deleted
	CacheIterator CacheNoDelete;
	// bytes per sample * number of channels
	size_t BytesPerSample;
	// Number of samples stored in the decoding buffer
	size_t Decoded;

	// Insert a block into the cache
	void CacheBlock(CacheIterator &pos, int64_t Start, size_t Samples, uint8_t *SrcData);

	// Called after seeking
	virtual void Seek() { };
	// Read the next packet from the file
	virtual bool ReadPacket(AVPacket *) = 0;
	virtual void FreePacket(AVPacket *) { }
protected:
	// First sample which is stored in the decoding buffer
	int64_t CurrentSample;
	// Next packet to be read
	size_t PacketNumber;
	// Current audio frame
	TFrameInfo *CurrentFrame;
	// Track which this corresponds to
	int TrackNumber;
	// Number of packets which the demuxer requires to know where it is
	// If -1, seeking is assumed to be impossible
	int SeekOffset;

	// Buffer which audio is decoded into
	AlignedBuffer<uint8_t> DecodingBuffer;
	FFMS_Track Frames;
	FFCodecContext CodecContext;
	FFMS_AudioProperties AP;

	void DecodeNextBlock();
	// Initialization which has to be done after the codec is opened
	void Init(FFMS_Index &Index, int DelayMode);

	FFMS_AudioSource(const char *SourceFile, FFMS_Index &Index, int Track);

public:
	virtual ~FFMS_AudioSource() { };
	FFMS_Track *GetTrack() { return &Frames; }
	const FFMS_AudioProperties& GetAudioProperties() const { return AP; }
	void GetAudio(void *Buf, int64_t Start, int64_t Count);
};

class FFLAVFAudio : public FFMS_AudioSource {
	AVFormatContext *FormatContext;

	bool ReadPacket(AVPacket *);
	void FreePacket(AVPacket *);
	void Seek();

public:
	FFLAVFAudio(const char *SourceFile, int Track, FFMS_Index &Index, int DelayMode);
	~FFLAVFAudio();
};

class FFMatroskaAudio : public FFMS_AudioSource {
	MatroskaFile *MF;
	MatroskaReaderContext MC;
	TrackInfo *TI;
	std::auto_ptr<TrackCompressionContext> TCC;
	char ErrorMessage[256];

	bool ReadPacket(AVPacket *);

public:
	FFMatroskaAudio(const char *SourceFile, int Track, FFMS_Index &Index, int DelayMode);
	~FFMatroskaAudio();
};

#ifdef HAALISOURCE

class FFHaaliAudio : public FFMS_AudioSource {
	CComPtr<IMMContainer> pMMC;
	CComPtr<IMMFrame> pMMF;

	bool ReadPacket(AVPacket *);
	void Seek();

public:
	FFHaaliAudio(const char *SourceFile, int Track, FFMS_Index &Index, enum FFMS_Sources SourceMode, int DelayMode);
};

#endif // HAALISOURCE

size_t GetSeekablePacketNumber(FFMS_Track const& Frames, size_t PacketNumber);

#endif
