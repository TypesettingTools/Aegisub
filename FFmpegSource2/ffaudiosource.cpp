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

#include "ffaudiosource.h"
#include <errno.h>

#ifdef __UNIX__
#define _snprintf snprintf
#endif

AudioBase::AudioBase() {
	DecodingBuffer = new uint8_t[AVCODEC_MAX_AUDIO_FRAME_SIZE * 10];
};

AudioBase::~AudioBase() {
	delete[] DecodingBuffer;
};

size_t AudioBase::FindClosestAudioKeyFrame(int64_t Sample) {
	for (size_t i = 0; i < Frames.size(); i++) {
		if (Frames[i].SampleStart == Sample && Frames[i].KeyFrame)
			return i;
		else if (Frames[i].SampleStart > Sample && Frames[i].KeyFrame)
			return i  - 1;
	}
	return Frames.size() - 1;
}

void FFAudioSource::Free(bool CloseCodec) {
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_close_input_file(FormatContext);
}

FFAudioSource::FFAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	FormatContext = NULL;
	AVCodec *Codec = NULL;
	AudioTrack = Track;
	Frames = (*TrackIndices)[AudioTrack];

	if (Frames.size() == 0) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Audio track contains no frames");
		throw ErrorMsg;
	}

	if (av_open_input_file(&FormatContext, SourceFile, NULL, 0, NULL) != 0) {
		_snprintf(ErrorMsg, MsgSize, "Couldn't open '%s'", SourceFile);
		throw ErrorMsg;
	}
	
	if (av_find_stream_info(FormatContext) < 0) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Couldn't find stream information");
		throw ErrorMsg;
	}

	CodecContext = FormatContext->streams[AudioTrack]->codec;

	Codec = avcodec_find_decoder(CodecContext->codec_id);
	if (Codec == NULL) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Audio codec not found");
		throw ErrorMsg;
	}

	if (avcodec_open(CodecContext, Codec) < 0) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Could not open audio codec");
		throw ErrorMsg;
	}

	// Always try to decode a frame to make sure all required parameters are known
	uint8_t DummyBuf[512];
	if (GetAudio(DummyBuf, 0, 1, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	AP.BitsPerSample = av_get_bits_per_sample_format(CodecContext->sample_fmt);
	AP.Channels = CodecContext->channels;;
	AP.Float = AudioFMTIsFloat(CodecContext->sample_fmt);
	AP.SampleRate = CodecContext->sample_rate;
	AP.NumSamples = (Frames.back()).SampleStart;

	if (AP.SampleRate <= 0 || AP.BitsPerSample <= 0) {
		Free(true);
		_snprintf(ErrorMsg, MsgSize, "Codec returned zero size audio");
		throw ErrorMsg;
	}
}

int FFAudioSource::DecodeNextAudioBlock(uint8_t *Buf, int64_t *Count, char *ErrorMsg, unsigned MsgSize) {
	const size_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	int Ret = -1;
	*Count = 0;
	AVPacket Packet;

	while (av_read_frame(FormatContext, &Packet) >= 0) {
        if (Packet.stream_index == AudioTrack) {
			uint8_t *Data = Packet.data;
			int Size = Packet.size;

			while (Size > 0) {
				int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * 10;
				Ret = avcodec_decode_audio2(CodecContext, (int16_t *)Buf, &TempOutputBufSize, Data, Size);

				if (Ret < 0) {// throw error or something?
					av_free_packet(&Packet);
					goto Done;
				}

				if (Ret > 0) {
					Size -= Ret;
					Data += Ret;
					Buf += TempOutputBufSize;
					if (SizeConst)
						*Count += TempOutputBufSize / SizeConst;
				}
			}

			av_free_packet(&Packet);
			goto Done;
        }

		av_free_packet(&Packet);
	}

Done:
	return Ret;
}

int FFAudioSource::GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) {
	const size_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	size_t CurrentAudioBlock = FFMAX((int64_t)FindClosestAudioKeyFrame(Start) - 50, (int64_t)0);
	memset(Buf, 0, SizeConst * Count);
	AVPacket Packet;

	avcodec_flush_buffers(CodecContext);
	av_seek_frame(FormatContext, AudioTrack, Frames[CurrentAudioBlock].DTS, AVSEEK_FLAG_BACKWARD);

	// Establish where we actually are
	// Trigger on packet dts difference since groups can otherwise be indistinguishable
	int64_t LastDTS = - 1;
	while (av_read_frame(FormatContext, &Packet) >= 0) {
        if (Packet.stream_index == AudioTrack) {
			if (LastDTS < 0) {
				LastDTS = Packet.dts;
			} else if (LastDTS != Packet.dts) {
				for (size_t i = 0; i < Frames.size(); i++)
					if (Frames[i].DTS == Packet.dts) {
						// The current match was consumed
						CurrentAudioBlock = i + 1;
						break;
					}

				av_free_packet(&Packet);
				break;
			}
		}

		av_free_packet(&Packet);
	}

	uint8_t *DstBuf = (uint8_t *)Buf;
	int64_t RemainingSamples = Count;
	int64_t DecodeCount;

	do {
		int64_t DecodeStart = Frames[CurrentAudioBlock].SampleStart;
		int Ret = DecodeNextAudioBlock(DecodingBuffer, &DecodeCount, ErrorMsg, MsgSize);
		if (Ret < 0) {
			// FIXME
			//Env->ThrowError("Bleh, bad audio decoding");
		}
		CurrentAudioBlock++;

		int64_t OffsetBytes = SizeConst * FFMAX(0, Start - DecodeStart);
		int64_t CopyBytes = FFMAX(0, SizeConst * FFMIN(RemainingSamples, DecodeCount - FFMAX(0, Start - DecodeStart)));

		memcpy(DstBuf, DecodingBuffer + OffsetBytes, CopyBytes);
		DstBuf += CopyBytes;

		if (SizeConst)
			RemainingSamples -= CopyBytes / SizeConst;

	} while (RemainingSamples > 0 && CurrentAudioBlock < Frames.size());

	return 0;
}

FFAudioSource::~FFAudioSource() {
	Free(true);
}

void MatroskaAudioSource::Free(bool CloseCodec) {
	if (CS)
		cs_Destroy(CS);
	if (MC.ST.fp) {
		mkv_Close(MF);
		fclose(MC.ST.fp);
	}
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_free(CodecContext);
}
	
MatroskaAudioSource::MatroskaAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	CodecContext = NULL;
	AVCodec *Codec = NULL;
	TrackInfo *TI = NULL;
	CS = NULL;
	Frames = (*TrackIndices)[Track];

	if (Frames.size() == 0) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Audio track contains no frames");
		throw ErrorMsg;
	}

	MC.ST.fp = fopen(SourceFile, "rb");
	if (MC.ST.fp == NULL) {
		_snprintf(ErrorMsg, MsgSize, "Can't open '%s': %s", SourceFile, strerror(errno));
		throw ErrorMsg;
	}

	setvbuf(MC.ST.fp, NULL, _IOFBF, CACHESIZE);

	MF = mkv_OpenEx(&MC.ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
	if (MF == NULL) {
		fclose(MC.ST.fp);
		_snprintf(ErrorMsg, MsgSize, "Can't parse Matroska file: %s", ErrorMessage);
		throw ErrorMsg;
	}

	mkv_SetTrackMask(MF, ~(1 << Track));
	TI = mkv_GetTrackInfo(MF, Track);

	if (TI->CompEnabled) {
		CS = cs_Create(MF, Track, ErrorMessage, sizeof(ErrorMessage));
		if (CS == NULL) {
			Free(false);
			_snprintf(ErrorMsg, MsgSize, "Can't create decompressor: %s", ErrorMessage);
			throw ErrorMsg;
		}
	}

	CodecContext = avcodec_alloc_context();
	CodecContext->extradata = (uint8_t *)TI->CodecPrivate;
	CodecContext->extradata_size = TI->CodecPrivateSize;

	Codec = avcodec_find_decoder(MatroskaToFFCodecID(TI->CodecID, TI->CodecPrivate));
	if (Codec == NULL) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Video codec not found");
		throw ErrorMsg;
	}

	if (avcodec_open(CodecContext, Codec) < 0) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Could not open video codec");
		throw ErrorMsg;
	}

	// Always try to decode a frame to make sure all required parameters are known
	uint8_t DummyBuf[512];
	if (GetAudio(DummyBuf, 0, 1, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	AP.BitsPerSample = av_get_bits_per_sample_format(CodecContext->sample_fmt);
	AP.Channels = CodecContext->channels;;
	AP.Float = AudioFMTIsFloat(CodecContext->sample_fmt);
	AP.SampleRate = CodecContext->sample_rate;
	AP.NumSamples = (Frames.back()).SampleStart;

	if (AP.SampleRate <= 0 || AP.BitsPerSample <= 0) {
		Free(true);
		_snprintf(ErrorMsg, MsgSize, "Codec returned zero size audio");
		throw ErrorMsg;
	}
}

MatroskaAudioSource::~MatroskaAudioSource() {
	Free(true);
}

int MatroskaAudioSource::GetAudio(void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) {
	const size_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	size_t CurrentAudioBlock = FFMAX((int64_t)FindClosestAudioKeyFrame(Start) - 10, (int64_t)0);
	avcodec_flush_buffers(CodecContext);

	memset(Buf, 0, SizeConst * Count);

	uint8_t *DstBuf = (uint8_t *)Buf;
	int64_t RemainingSamples = Count;
	int64_t DecodeCount;

	do {
		int64_t DecodeStart = Frames[CurrentAudioBlock].SampleStart;
		int Ret = DecodeNextAudioBlock(DecodingBuffer, &DecodeCount, Frames[CurrentAudioBlock].FilePos, Frames[CurrentAudioBlock].FrameSize, ErrorMsg, MsgSize);
		if (Ret < 0) {
			// FIXME
			//Env->ThrowError("Bleh, bad audio decoding");
		}
		CurrentAudioBlock++;

		int64_t OffsetBytes = SizeConst * FFMAX(0, Start - DecodeStart);
		int64_t CopyBytes = FFMAX(0, SizeConst * FFMIN(RemainingSamples, DecodeCount - FFMAX(0, Start - DecodeStart)));

		memcpy(DstBuf, DecodingBuffer + OffsetBytes, CopyBytes);
		DstBuf += CopyBytes;

		if (SizeConst)
			RemainingSamples -= CopyBytes / SizeConst;

	} while (RemainingSamples > 0 && CurrentAudioBlock < Frames.size());

	return 0;
}

int MatroskaAudioSource::DecodeNextAudioBlock(uint8_t *Buf, int64_t *Count, uint64_t FilePos, unsigned int FrameSize, char *ErrorMsg, unsigned MsgSize) {
	const size_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	int Ret = -1;
	*Count = 0;

	// FIXME check return
	ReadFrame(FilePos, FrameSize, CS, MC, ErrorMsg, MsgSize);
	int Size = FrameSize;
	uint8_t *Data = MC.Buffer;

	while (Size > 0) {
		int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		Ret = avcodec_decode_audio2(CodecContext, (int16_t *)Buf, &TempOutputBufSize, Data, Size);

		if (Ret < 0) // throw error or something?
			goto Done;

		if (Ret > 0) {
			Size -= Ret;
			Data += Ret;
			Buf += TempOutputBufSize;
			if (SizeConst)
				*Count += TempOutputBufSize / SizeConst;
		}
	}

Done:
	return Ret;
}
