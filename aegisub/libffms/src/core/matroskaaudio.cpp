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

#include "audiosource.h"

void FFMatroskaAudio::Free(bool CloseCodec) {
	if (TCC)
		delete TCC;
	if (MC.ST.fp) {
		mkv_Close(MF);
		fclose(MC.ST.fp);
	}
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_freep(&CodecContext);
}
	
FFMatroskaAudio::FFMatroskaAudio(const char *SourceFile, int Track, FFMS_Index *Index)
								 : Res(FFSourceResources<FFMS_AudioSource>(this)), FFMS_AudioSource(SourceFile, Index, Track) {
	CodecContext = NULL;
	AVCodec *Codec = NULL;
	TrackInfo *TI = NULL;
	TCC = NULL;
	PacketNumber = 0;
	Frames = (*Index)[Track];

	MC.ST.fp = ffms_fopen(SourceFile, "rb");
	if (MC.ST.fp == NULL) {
		std::ostringstream buf;
		buf << "Can't open '" << SourceFile << "': " << strerror(errno);
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	setvbuf(MC.ST.fp, NULL, _IOFBF, CACHESIZE);

	MF = mkv_OpenEx(&MC.ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
	if (MF == NULL) {
		fclose(MC.ST.fp);
		std::ostringstream buf;
		buf << "Can't parse Matroska file: " << ErrorMessage;
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}


	TI = mkv_GetTrackInfo(MF, Track);

	if (TI->CompEnabled)
		TCC = new TrackCompressionContext(MF, TI, Track);

	CodecContext = avcodec_alloc_context();

	Codec = avcodec_find_decoder(MatroskaToFFCodecID(TI->CodecID, TI->CodecPrivate, 0, TI->AV.Audio.BitDepth));
	if (Codec == NULL)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Audio codec not found");

	InitializeCodecContextFromMatroskaTrackInfo(TI, CodecContext);

	if (avcodec_open(CodecContext, Codec) < 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Could not open audio codec");

	Res.CloseCodec(true);

	// Always try to decode a frame to make sure all required parameters are known
	int64_t Dummy;
	DecodeNextAudioBlock(&Dummy);

	avcodec_flush_buffers(CodecContext);

	FillAP(AP, CodecContext, Frames);

	if (AP.SampleRate <= 0 || AP.BitsPerSample <= 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Codec returned zero size audio");

	AudioCache.Initialize((AP.Channels * AP.BitsPerSample) / 8, 50);
}

void FFMatroskaAudio::GetAudio(void *Buf, int64_t Start, int64_t Count) {
	GetAudioCheck(Start, Count);

	const int64_t SizeConst = (av_get_bits_per_sample_fmt(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	memset(Buf, 0, static_cast<size_t>(SizeConst * Count));

	unsigned int PreDecBlocks = 0;
	uint8_t *DstBuf = static_cast<uint8_t *>(Buf);

	// Fill with everything in the cache
	int64_t CacheEnd = AudioCache.FillRequest(Start, Count, DstBuf);
	// Was everything in the cache?
	if (CacheEnd == Start + Count)
		return;

	if (CurrentSample != CacheEnd) {
		PreDecBlocks = 15;
		PacketNumber = FFMAX((int64_t)Frames.FindClosestAudioKeyFrame(CacheEnd) - PreDecBlocks, (int64_t)0);
		
		if (PacketNumber <= PreDecBlocks) {
			PacketNumber = 0;
			PreDecBlocks = 0;
		}

		avcodec_flush_buffers(CodecContext);
	} 

	int64_t DecodeCount;

	do {
		const TFrameInfo &FI = Frames[Frames[PacketNumber].OriginalPos];
		DecodeNextAudioBlock(&DecodeCount);

		// Cache the block if enough blocks before it have been decoded to avoid garbage
		if (PreDecBlocks == 0) {
			AudioCache.CacheBlock(FI.SampleStart, DecodeCount, &DecodingBuffer[0]);
			CacheEnd = AudioCache.FillRequest(CacheEnd, Start + Count - CacheEnd, DstBuf + (CacheEnd - Start) * SizeConst);
		} else {
			PreDecBlocks--;
		}

	} while (Start + Count - CacheEnd > 0 && PacketNumber < Frames.size());
}

void FFMatroskaAudio::DecodeNextAudioBlock(int64_t *Count) {
	const size_t SizeConst = (av_get_bits_per_sample_fmt(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	int Ret = -1;
	*Count = 0;
	uint8_t *Buf = &DecodingBuffer[0];
	AVPacket TempPacket;
	InitNullPacket(TempPacket);

	const TFrameInfo &FI = Frames[Frames[PacketNumber].OriginalPos];
	unsigned int FrameSize = FI.FrameSize;
	CurrentSample = FI.SampleStart + FI.SampleCount;
	PacketNumber++;

	ReadFrame(FI.FilePos, FrameSize, TCC, MC);
	TempPacket.data = MC.Buffer;
	TempPacket.size = (TCC && TCC->CompressionMethod == COMP_PREPEND) ? FrameSize + TCC->CompressedPrivateDataSize : FrameSize;
	if (FI.KeyFrame)
		TempPacket.flags = AV_PKT_FLAG_KEY;
	else
		TempPacket.flags = 0;

	while (TempPacket.size > 0) {
		int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		Ret = avcodec_decode_audio3(CodecContext, (int16_t *)Buf, &TempOutputBufSize, &TempPacket);

		if (Ret < 0) // throw error or something?
			goto Done;

		if (Ret > 0) {
			TempPacket.size -= Ret;
			TempPacket.data += Ret;
			Buf += TempOutputBufSize;
			if (SizeConst)
				*Count += TempOutputBufSize / SizeConst;
		}
	}

Done:;
}
