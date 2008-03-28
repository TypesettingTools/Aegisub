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

#include "ffmpegsource.h"

int FFMatroskaAudioSource::GetTrackIndex(int Index, unsigned char ATrackType, IScriptEnvironment *Env) {
	if (Index == -1)
		for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++)
			if (mkv_GetTrackInfo(MF, i)->Type == ATrackType) {
				Index = i;
				break;
			}

	if (Index == -1)
		Env->ThrowError("FFmpegSource: No %s track found", (ATrackType & TT_VIDEO) ? "video" : "audio");
	if (Index <= -2)
		return -2;

	if (Index >= (int)mkv_GetNumTracks(MF))
		Env->ThrowError("FFmpegSource: Invalid %s track number", (ATrackType & TT_VIDEO) ? "video" : "audio");

	TrackInfo *TI = mkv_GetTrackInfo(MF, Index);

	if (TI->Type != ATrackType)
		Env->ThrowError("FFmpegSource: Selected track is not %s", (ATrackType & TT_VIDEO) ? "video" : "audio");

	return Index;
}

FFMatroskaAudioSource::FFMatroskaAudioSource(const char *ASource, int AAudioTrack, const char *AAudioCache, IScriptEnvironment *Env) {
	int AudioTrack;
	AudioCodecContext = NULL;
	AVCodec *AudioCodec = NULL;
	TrackInfo *VideoTI = NULL;
	BufferSize = 0;
	Buffer = NULL;
	AudioCS = NULL;

	memset(&ST,0,sizeof(ST));
	ST.base.read = (int (__cdecl *)(InputStream *,ulonglong,void *,int))StdIoRead;
	ST.base.scan = (longlong (__cdecl *)(InputStream *,ulonglong,unsigned int))StdIoScan;
	ST.base.getcachesize = (unsigned int (__cdecl *)(InputStream *))StdIoGetCacheSize;
	ST.base.geterror = (const char *(__cdecl *)(InputStream *))StdIoGetLastError;
	ST.base.memalloc = (void *(__cdecl *)(InputStream *,size_t))StdIoMalloc;
	ST.base.memrealloc = (void *(__cdecl *)(InputStream *,void *,size_t))StdIoRealloc;
	ST.base.memfree = (void (__cdecl *)(InputStream *,void *)) StdIoFree;
	ST.base.progress = (int (__cdecl *)(InputStream *,ulonglong,ulonglong))StdIoProgress;

	ST.fp = fopen(ASource, "rb");
	if (ST.fp == NULL)
		Env->ThrowError("FFmpegSource: Can't open '%s': %s", ASource, strerror(errno));

	setvbuf(ST.fp, NULL, _IOFBF, CACHESIZE);

	MF = mkv_OpenEx(&ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
	if (MF == NULL) {
		fclose(ST.fp);
		Env->ThrowError("FFmpegSource: Can't parse Matroska file: %s", ErrorMessage);
	}

	AudioTrack = GetTrackIndex(AAudioTrack, TT_AUDIO, Env);
	mkv_SetTrackMask(MF, ~(1 << AudioTrack));

	TrackInfo *AudioTI = mkv_GetTrackInfo(MF, AudioTrack);

	if (AudioTI->CompEnabled) {
		AudioCS = cs_Create(MF, AudioTrack, ErrorMessage, sizeof(ErrorMessage));
		if (AudioCS == NULL)
			Env->ThrowError("FFmpegSource: Can't create decompressor: %s", ErrorMessage);
	}

	AudioCodecContext = avcodec_alloc_context();
	AudioCodecContext->extradata = (uint8_t *)AudioTI->CodecPrivate;
	AudioCodecContext->extradata_size = AudioTI->CodecPrivateSize;

	AudioCodec = avcodec_find_decoder(MatroskaToFFCodecID(AudioTI));
	if (AudioCodec == NULL)
		Env->ThrowError("FFmpegSource: Audio codec not found");

	if (avcodec_open(AudioCodecContext, AudioCodec) < 0)
		Env->ThrowError("FFmpegSource: Could not open audio codec");

	// Fix for ac3 and other codecs where decoding a block of audio is required to get information about it
	if (AudioCodecContext->channels == 0 || AudioCodecContext->sample_rate == 0) {
		uint64_t StartTime, EndTime, FilePos;
		unsigned int Track, FrameFlags, FrameSize;
		mkv_ReadFrame(MF, 0, &Track, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags);

		int Size = ReadFrame(FilePos, FrameSize, AudioCS, Env);
		uint8_t *Data = Buffer;

		while (Size > 0) {
			int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
			int Ret = avcodec_decode_audio2(AudioCodecContext, (int16_t *)DecodingBuffer, &TempOutputBufSize, Data, Size);
			if (Ret < 0)
				Env->ThrowError("FFmpegSource: Audio decoding error");

			Size -= Ret;
			Data += Ret;
		}

		mkv_Seek(MF, 0, MKVF_SEEK_TO_PREV_KEYFRAME);
		avcodec_flush_buffers(AudioCodecContext);
	}

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

	//load audio cache
	bool ACacheIsValid = LoadSampleInfoFromFile(AAudioCache, ASource, AudioTrack);

	// Needs to be indexed?
	if (!ACacheIsValid) {
		uint64_t StartTime, EndTime, FilePos;
		unsigned int Track, FrameFlags, FrameSize;

		while (mkv_ReadFrame(MF, 0, &Track, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
			SI.push_back(SampleInfo(VI.num_audio_samples, FilePos, FrameSize, (FrameFlags & FRAME_KF) != 0));

			if (AudioCodecContext->frame_size > 0) {
				VI.num_audio_samples += AudioCodecContext->frame_size;
			} else {
				int Size = ReadFrame(FilePos, FrameSize, AudioCS, Env);
				uint8_t *Data = Buffer;

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

		mkv_Seek(MF, 0, MKVF_SEEK_TO_PREV_KEYFRAME);
		avcodec_flush_buffers(AudioCodecContext);

		if (!SaveSampleInfoToFile(AAudioCache, ASource, AudioTrack))
			Env->ThrowError("FFmpegSource: Failed to save audio cache index");
	}

	if (VI.num_audio_samples == 0)
		Env->ThrowError("FFmpegSource: Audio track contains no samples");
}

FFMatroskaAudioSource::~FFMatroskaAudioSource() {
	free(Buffer);
	mkv_Close(MF);
	fclose(ST.fp);
	if (AudioCodecContext)
		avcodec_close(AudioCodecContext);
	av_free(AudioCodecContext);
}

int FFMatroskaAudioSource::ReadFrame(uint64_t AFilePos, unsigned int AFrameSize, CompressedStream *ACS, IScriptEnvironment *Env) {
	if (ACS) {
		char CSBuffer[4096];

		unsigned int DecompressedFrameSize = 0;

		cs_NextFrame(ACS, AFilePos, AFrameSize);

		for (;;) {
			int ReadBytes = cs_ReadData(ACS, CSBuffer, sizeof(CSBuffer));
			if (ReadBytes < 0)
				Env->ThrowError("FFmpegSource: Error decompressing data: %s", cs_GetLastError(ACS));
			if (ReadBytes == 0) {
				return DecompressedFrameSize;
			}

			if (BufferSize < DecompressedFrameSize + ReadBytes) {
				BufferSize = AFrameSize;
				Buffer = (uint8_t *)realloc(Buffer, BufferSize);
				if (Buffer == NULL) 
					Env->ThrowError("FFmpegSource: Out of memory");
			}

			memcpy(Buffer + DecompressedFrameSize, CSBuffer, ReadBytes);
			DecompressedFrameSize += ReadBytes;
		}
	} else {
		if (_fseeki64(ST.fp, AFilePos, SEEK_SET))
			Env->ThrowError("FFmpegSource: fseek(): %s", strerror(errno));

		if (BufferSize < AFrameSize) {
			BufferSize = AFrameSize;
			Buffer = (uint8_t *)realloc(Buffer, BufferSize);
			if (Buffer == NULL) 
				Env->ThrowError("FFmpegSource: Out of memory");
		}

		size_t ReadBytes = fread(Buffer, 1, AFrameSize, ST.fp);
		if (ReadBytes != AFrameSize) {
			if (ReadBytes == 0) {
				if (feof(ST.fp))
					Env->ThrowError("FFmpegSource: Unexpected EOF while reading frame");
				else
					Env->ThrowError("FFmpegSource: Error reading frame: %s", strerror(errno));
			} else
				Env->ThrowError("FFmpegSource: Short read while reading frame");
			Env->ThrowError("FFmpegSource: Unknown read error");
		}

		return AFrameSize;
	}

	return 0;
}

void __stdcall FFMatroskaAudioSource::GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env) {
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

	} while (RemainingSamples > 0 && CurrentAudioBlock < SI.size());
}

int FFMatroskaAudioSource::DecodeNextAudioBlock(uint8_t *ABuf, int64_t *ACount, uint64_t AFilePos, unsigned int AFrameSize, IScriptEnvironment *Env) {
	int Ret = -1;
	*ACount = 0;

	int FrameSize = ReadFrame(AFilePos, AFrameSize, AudioCS, Env);
	uint8_t *Data = Buffer;
	int Size = FrameSize;

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
