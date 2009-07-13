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

#include "ffaudiosource.h"

void FFMatroskaAudio::Free(bool CloseCodec) {
	if (CS)
		cs_Destroy(CS);
	if (MC.ST.fp) {
		mkv_Close(MF);
		fclose(MC.ST.fp);
	}
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_freep(&CodecContext);
}
	
FFMatroskaAudio::FFMatroskaAudio(const char *SourceFile, int Track,
								 FFIndex *Index, char *ErrorMsg, unsigned MsgSize)
								 : FFAudio(SourceFile, Index, ErrorMsg, MsgSize) {
	CodecContext = NULL;
	AVCodec *Codec = NULL;
	TrackInfo *TI = NULL;
	CS = NULL;
	Frames = (*Index)[Track];

	if (Frames.size() == 0) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Audio track contains no frames, was it indexed properly?");
		throw ErrorMsg;
	}

	MC.ST.fp = ffms_fopen(SourceFile, "rb");
	if (MC.ST.fp == NULL) {
		snprintf(ErrorMsg, MsgSize, "Can't open '%s': %s", SourceFile, strerror(errno));
		throw ErrorMsg;
	}

	setvbuf(MC.ST.fp, NULL, _IOFBF, CACHESIZE);

	MF = mkv_OpenEx(&MC.ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
	if (MF == NULL) {
		fclose(MC.ST.fp);
		snprintf(ErrorMsg, MsgSize, "Can't parse Matroska file: %s", ErrorMessage);
		throw ErrorMsg;
	}

	mkv_SetTrackMask(MF, ~(1 << Track));
	TI = mkv_GetTrackInfo(MF, Track);

	if (TI->CompEnabled) {
		CS = cs_Create(MF, Track, ErrorMessage, sizeof(ErrorMessage));
		if (CS == NULL) {
			Free(false);
			snprintf(ErrorMsg, MsgSize, "Can't create decompressor: %s", ErrorMessage);
			throw ErrorMsg;
		}
	}

	CodecContext = avcodec_alloc_context();

	Codec = avcodec_find_decoder(MatroskaToFFCodecID(TI->CodecID, TI->CodecPrivate));
	if (Codec == NULL) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Video codec not found");
		throw ErrorMsg;
	}

	InitializeCodecContextFromMatroskaTrackInfo(TI, CodecContext);

	if (avcodec_open(CodecContext, Codec) < 0) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Could not open video codec");
		throw ErrorMsg;
	}

	// Always try to decode a frame to make sure all required parameters are known
	int64_t Dummy;
	if (DecodeNextAudioBlock(&Dummy, 0, ErrorMsg, MsgSize) < 0) {
		Free(true);
		throw ErrorMsg;
	}
	avcodec_flush_buffers(CodecContext);

	FillAP(AP, CodecContext, Frames);

	if (AP.SampleRate <= 0 || AP.BitsPerSample <= 0) {
		Free(true);
		snprintf(ErrorMsg, MsgSize, "Codec returned zero size audio");
		throw ErrorMsg;
	}

	AudioCache.Initialize((AP.Channels * AP.BitsPerSample) / 8, 50);
}

FFMatroskaAudio::~FFMatroskaAudio() {
	Free(true);
}

int FFMatroskaAudio::GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) {
	const int64_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	memset(Buf, 0, static_cast<size_t>(SizeConst * Count));

	int PreDecBlocks = 0;
	uint8_t *DstBuf = static_cast<uint8_t *>(Buf);

	// Fill with everything in the cache
	int64_t CacheEnd = AudioCache.FillRequest(Start, Count, DstBuf);
	// Was everything in the cache?
	if (CacheEnd == Start + Count)
		return 0;

	int CurrentAudioBlock;
	// Is seeking required to decode the requested samples?
//	if (!(CurrentSample >= Start && CurrentSample <= CacheEnd)) {
	if (CurrentSample != CacheEnd) {
		PreDecBlocks = 15;
		CurrentAudioBlock = FFMAX((int64_t)Frames.FindClosestAudioKeyFrame(CacheEnd) - PreDecBlocks, (int64_t)0);
		avcodec_flush_buffers(CodecContext);
	} else {
		CurrentAudioBlock = Frames.FindClosestAudioKeyFrame(CurrentSample);
	}

	int64_t DecodeCount;

	do {
		int Ret = DecodeNextAudioBlock(&DecodeCount, CurrentAudioBlock, ErrorMsg, MsgSize);
		if (Ret < 0) {
			// FIXME
			//Env->ThrowError("Bleh, bad audio decoding");
		}

		// Cache the block if enough blocks before it have been decoded to avoid garbage
		if (PreDecBlocks == 0) {
			AudioCache.CacheBlock(Frames[CurrentAudioBlock].SampleStart, DecodeCount, &DecodingBuffer[0]);
			CacheEnd = AudioCache.FillRequest(CacheEnd, Start + Count - CacheEnd, DstBuf + (CacheEnd - Start) * SizeConst);
		} else {
			PreDecBlocks--;
		}

		CurrentAudioBlock++;
		if (CurrentAudioBlock < static_cast<int>(Frames.size()))
			CurrentSample = Frames[CurrentAudioBlock].SampleStart;
	} while (Start + Count - CacheEnd > 0 && CurrentAudioBlock < static_cast<int>(Frames.size()));

	return 0;
}

int FFMatroskaAudio::DecodeNextAudioBlock(int64_t *Count, int AudioBlock, char *ErrorMsg, unsigned MsgSize) {
	const size_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	int Ret = -1;
	*Count = 0;
	uint8_t *Buf = &DecodingBuffer[0];
	AVPacket TempPacket;
	InitNullPacket(&TempPacket);

	unsigned int FrameSize = Frames[AudioBlock].FrameSize;
	if (ReadFrame(Frames[AudioBlock].FilePos, FrameSize, CS, MC, ErrorMsg, MsgSize))
		return 1;
	TempPacket.data = MC.Buffer;
	TempPacket.size = FrameSize;
	if (Frames[AudioBlock].KeyFrame)
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

Done:
	return 0;
}
