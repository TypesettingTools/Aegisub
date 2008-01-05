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

#include "ffmpegsource.h"

int FFmpegAudioSource::GetTrackIndex(int Index, CodecType ATrackType, IScriptEnvironment *Env) {
	if (Index == -1)
		for (unsigned int i = 0; i < FormatContext->nb_streams; i++)
			if (FormatContext->streams[i]->codec->codec_type == ATrackType) {
				Index = i;
				break;
			}

	if (Index == -1)
		Env->ThrowError("FFmpegSource: No %s track found", (ATrackType == CODEC_TYPE_VIDEO) ? "video" : "audio");
	if (Index <= -2)
		return -2;

	if (Index >= (int)FormatContext->nb_streams)
		Env->ThrowError("FFmpegSource: Invalid %s track number", (ATrackType ==  CODEC_TYPE_VIDEO) ? "video" : "audio");

	if (FormatContext->streams[Index]->codec->codec_type != ATrackType)
		Env->ThrowError("FFmpegSource: Selected track is not %s", (ATrackType == CODEC_TYPE_VIDEO) ? "video" : "audio");

	return Index;
}

bool FFmpegAudioSource::LoadSampleInfoFromFile(const char *AAudioCacheFile, const char *ADemuxedAudioFile, const char *ASource, int AAudioTrack) {
	if (!FFAudioBase::LoadSampleInfoFromFile(AAudioCacheFile, ASource, AAudioTrack))
		return false;

	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffasd%dcache", ASource, AAudioTrack);
	if (!strcmp(ADemuxedAudioFile, ""))
		ADemuxedAudioFile = DefaultCacheFilename;

	RawCache = fopen(ADemuxedAudioFile, "rb");
	if (!RawCache)
		return false;

	return true;
}

FFmpegAudioSource::FFmpegAudioSource(const char *ASource, int AAudioTrack, const char *AAudioCache, const char *ADemuxedAudioFile, IScriptEnvironment *Env) {
	BufferSize = 0;
	Buffer = NULL;
	RawCache = NULL;
	FormatContext = NULL;
	AudioCodecContext = NULL;
	AVCodec *AudioCodec = NULL;

	if (av_open_input_file(&FormatContext, ASource, NULL, 0, NULL) != 0)
		Env->ThrowError("FFmpegSource: Couldn't open '%s'", ASource);
	
	if (av_find_stream_info(FormatContext) < 0)
		Env->ThrowError("FFmpegSource: Couldn't find stream information");

	AudioTrack = GetTrackIndex(AAudioTrack, CODEC_TYPE_AUDIO, Env);

	AudioCodecContext = FormatContext->streams[AudioTrack]->codec;

	AudioCodec = avcodec_find_decoder(AudioCodecContext->codec_id);
	if (AudioCodec == NULL)
		Env->ThrowError("FFmpegSource: Audio codec not found");

	if (avcodec_open(AudioCodecContext, AudioCodec) < 0)
		Env->ThrowError("FFmpegSource: Could not open audio codec");

	VI.nchannels = AudioCodecContext->channels;
	VI.audio_samples_per_second = AudioCodecContext->sample_rate;

	switch (AudioCodecContext->sample_fmt) {
		case SAMPLE_FMT_U8: VI.sample_type = SAMPLE_INT8; break;
		case SAMPLE_FMT_S16: VI.sample_type = SAMPLE_INT16; break;
		case SAMPLE_FMT_S24: VI.sample_type = SAMPLE_INT24; break;
		case SAMPLE_FMT_S32: VI.sample_type = SAMPLE_INT32; break;
		case SAMPLE_FMT_FLT: VI.sample_type = SAMPLE_FLOAT; break;
		default:
			Env->ThrowError("FFmpegSource: Unsupported/unknown sample format");
	}

	//load cache
	bool ACacheIsValid = LoadSampleInfoFromFile(AAudioCache, ADemuxedAudioFile, ASource, AudioTrack);

	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffasd%dcache", ASource, AudioTrack);
	if (!strcmp(ADemuxedAudioFile, ""))
		ADemuxedAudioFile = DefaultCacheFilename;
	if (!RawCache)
		RawCache = fopen(ADemuxedAudioFile, "wb+");

	// Needs to be indexed?
	if (!ACacheIsValid) {
		AVPacket Packet;

		while (av_read_frame(FormatContext, &Packet) >= 0) {
			if (Packet.stream_index == AudioTrack) {
				SI.push_back(SampleInfo(VI.num_audio_samples, _ftelli64(RawCache), Packet.size, (Packet.flags & PKT_FLAG_KEY) ? 1 : 0));
				fwrite(Packet.data, 1, Packet.size, RawCache);

				if (AudioCodecContext->frame_size > 0) {
					VI.num_audio_samples += AudioCodecContext->frame_size;
				} else {
					int Size = Packet.size;
					uint8_t *Data = Packet.data;

					while (Size > 0) {
						int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
						int Ret = avcodec_decode_audio2(AudioCodecContext, (int16_t *)DecodingBuffer, &TempOutputBufSize, Data, Size);
						if (Ret < 0)
							Env->ThrowError("FFmpegSource: Audio decoding error");

						if (Ret > 0) {
							int DecodedSamples = (int)VI.AudioSamplesFromBytes(TempOutputBufSize);
							Size -= Ret;
							Data += Ret;
							VI.num_audio_samples += DecodedSamples;
						}
					}
				}
			}

			av_free_packet(&Packet);
		}

		av_seek_frame(FormatContext, AudioTrack, 0, AVSEEK_FLAG_BACKWARD);
		avcodec_flush_buffers(AudioCodecContext);

		if (!SaveSampleInfoToFile(AAudioCache, ASource, AudioTrack))
			Env->ThrowError("FFmpegSource: Failed to save audio cache index");
	}

	if (VI.num_audio_samples == 0)
		Env->ThrowError("FFmpegSource: Audio track contains no samples");
}

int FFmpegAudioSource::DecodeNextAudioBlock(uint8_t *ABuf, int64_t *ACount, uint64_t AFilePos, unsigned int AFrameSize, IScriptEnvironment *Env) {
	int Ret = -1;
	*ACount = 0;

	_fseeki64(RawCache, AFilePos, SEEK_SET);

	if (AFrameSize > BufferSize) {
		Buffer = (uint8_t *)realloc(Buffer, AFrameSize);
		BufferSize = AFrameSize;
	}

	fread(Buffer, 1, AFrameSize, RawCache);

	uint8_t *Data = Buffer;
	int Size = AFrameSize;

	while (Size > 0) {
		int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		Ret = avcodec_decode_audio2(AudioCodecContext, (int16_t *)ABuf, &TempOutputBufSize, Data, Size);

		if (Ret < 0) // throw error or something?
			goto Done;

		if (Ret > 0) {
			Size -= Ret;
			Data += Ret;
			ABuf += TempOutputBufSize;
			*ACount += VI.AudioSamplesFromBytes(TempOutputBufSize);
		}
	}

Done:
	return Ret;
}

void FFmpegAudioSource::GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env) {
	size_t CurrentAudioBlock = FFMAX((int64_t)FindClosestAudioKeyFrame(Start) - 10, (int64_t)0);
	avcodec_flush_buffers(AudioCodecContext);

	memset(Buf, 0, VI.BytesFromAudioSamples(Count));

	uint8_t *DstBuf = (uint8_t *)Buf;
	int64_t RemainingSamples = Count;
	int64_t DecodeCount;

	do {
		int64_t DecodeStart = SI[CurrentAudioBlock].SampleStart;
		int Ret = DecodeNextAudioBlock(DecodingBuffer, &DecodeCount, SI[CurrentAudioBlock].FilePos, SI[CurrentAudioBlock].FrameSize, Env);
		if (Ret < 0)
			Env->ThrowError("Bleh, bad audio decoding");
		CurrentAudioBlock++;

		int64_t OffsetBytes = VI.BytesFromAudioSamples(FFMAX(0, Start - DecodeStart));
		int64_t CopyBytes = FFMAX(0, VI.BytesFromAudioSamples(FFMIN(RemainingSamples, DecodeCount - FFMAX(0, Start - DecodeStart))));

		memcpy(DstBuf, DecodingBuffer + OffsetBytes, CopyBytes);
		DstBuf += CopyBytes;

		RemainingSamples -= VI.AudioSamplesFromBytes(CopyBytes);
	} while (RemainingSamples > 0 && DecodeCount > 0);
}

FFmpegAudioSource::~FFmpegAudioSource() {
	if (RawCache)
		fclose(RawCache);
	if (AudioCodecContext)
		avcodec_close(AudioCodecContext);
	av_close_input_file(FormatContext);
}
