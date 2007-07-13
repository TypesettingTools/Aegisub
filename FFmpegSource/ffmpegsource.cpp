#include <windows.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>

extern "C" {
#include <ffmpeg\avformat.h>
#include <ffmpeg\avcodec.h>
#include <ffmpeg\swscale.h>
}

#include "MatroskaParser.h"
#include "avisynth.h"
#include "stdiostream.c"
#include "matroskacodecs.c"

class FFBase : public IClip {
protected:
	VideoInfo VI;
public:
	bool __stdcall GetParity(int n) { return false; }
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { }
	void __stdcall SetCacheHints(int cachehints, int frame_range) { }
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return NULL; }
	const VideoInfo& __stdcall GetVideoInfo() { return VI; }

	FFBase() {
		memset(&VI, 0, sizeof(VI));
	}
};

class FFVideoBase : public FFBase {
private:
    SwsContext *SWS;
	int ConvertToFormat;
	int ConvertFromFormat;
protected:
	struct FrameInfo {
		int64_t DTS;
		bool KeyFrame;

		FrameInfo(int64_t _DTS, bool _KeyFrame) : DTS(_DTS), KeyFrame(_KeyFrame) {};
	};

	std::vector<FrameInfo> FrameToDTS;

	int FindClosestKeyFrame(int Frame) {
		for (unsigned int i = Frame; i > 0; i--)
			if (FrameToDTS[i].KeyFrame)
				return i;
		return 0;
	}

	int FrameFromDTS(int64_t DTS) {
		for (unsigned int i = 0; i < FrameToDTS.size(); i++)
			if (FrameToDTS[i].DTS == DTS)
				return i;
		return -1;
	}

	int ClosestFrameFromDTS(int64_t DTS) {
		int Frame = 0;
		int64_t BestDiff = 0xFFFFFFFFFFFFFF;
		for (unsigned int i = 0; i < FrameToDTS.size(); i++) {
			int64_t CurrentDiff = FFABS(FrameToDTS[i].DTS - DTS);
			if (CurrentDiff < BestDiff) {
				BestDiff = CurrentDiff;
				Frame = i;
			}
		}
		return Frame;
	}

	bool LoadFrameInfoFromFile(FILE *CacheFile) {
		if (ftell(CacheFile)) {
			rewind(CacheFile);

			fscanf(CacheFile, "%d\n", &VI.num_frames);

			for (int i = 0; i < VI.num_frames; i++) {
				int64_t DTSTemp;
				int KFTemp;
				fscanf(CacheFile, "%lld %d\n", &DTSTemp, &KFTemp);
				FrameToDTS.push_back(FrameInfo(DTSTemp, KFTemp != 0));
			}

			return true;
		}

		return false;
	}

	void SetOutputFormat(int CurrentFormat, IScriptEnvironment *Env) {
		int Loss;
		int BestFormat = avcodec_find_best_pix_fmt((1 << PIX_FMT_YUV420P) | (1 << PIX_FMT_YUYV422) | (1 << PIX_FMT_RGB32) | (1 << PIX_FMT_BGR24), CurrentFormat, 1 /* Required to prevent pointless RGB32 => RGB24 conversion */, &Loss);

		switch (BestFormat) {
			case PIX_FMT_YUV420P: VI.pixel_type = VideoInfo::CS_I420; break;
			case PIX_FMT_YUYV422: VI.pixel_type = VideoInfo::CS_YUY2; break;
			case PIX_FMT_RGB32: VI.pixel_type = VideoInfo::CS_BGR32; break;
			case PIX_FMT_BGR24: VI.pixel_type = VideoInfo::CS_BGR24; break;
			default:
				Env->ThrowError("FFmpegSource: No suitable output format found");
		}

		if (BestFormat != CurrentFormat) {
			ConvertFromFormat = CurrentFormat;
			ConvertToFormat = BestFormat;
			SWS = sws_getContext(VI.width, VI.height, ConvertFromFormat, VI.width, VI.height, ConvertToFormat, SWS_BICUBIC, NULL, NULL, NULL);
		}
	}

	PVideoFrame OutputFrame(AVFrame *Frame, IScriptEnvironment *Env) {
		PVideoFrame Dst = Env->NewVideoFrame(VI);

		if (ConvertToFormat != PIX_FMT_NONE && VI.pixel_type == VideoInfo::CS_I420) {
			uint8_t *DstData[3] = {Dst->GetWritePtr(PLANAR_Y), Dst->GetWritePtr(PLANAR_U), Dst->GetWritePtr(PLANAR_V)};
			int DstStride[3] = {Dst->GetPitch(PLANAR_Y), Dst->GetPitch(PLANAR_U), Dst->GetPitch(PLANAR_V)};
			sws_scale(SWS, Frame->data, Frame->linesize, 0, VI.height, DstData, DstStride);
		} else if (ConvertToFormat != PIX_FMT_NONE) {
			if (VI.IsRGB()) {
				uint8_t *DstData[1] = {Dst->GetWritePtr() + Dst->GetPitch() * (Dst->GetHeight() - 1)};
				int DstStride[1] = {-Dst->GetPitch()};
				sws_scale(SWS, Frame->data, Frame->linesize, 0, VI.height, DstData, DstStride);
			} else {
				uint8_t *DstData[1] = {Dst->GetWritePtr()};
				int DstStride[1] = {Dst->GetPitch()};
				sws_scale(SWS, Frame->data, Frame->linesize, 0, VI.height, DstData, DstStride);
			}
		} else if (VI.pixel_type == VideoInfo::CS_I420) {
			Env->BitBlt(Dst->GetWritePtr(PLANAR_Y), Dst->GetPitch(PLANAR_Y), Frame->data[0], Frame->linesize[0], Dst->GetRowSize(PLANAR_Y), Dst->GetHeight(PLANAR_Y)); 
			Env->BitBlt(Dst->GetWritePtr(PLANAR_U), Dst->GetPitch(PLANAR_U), Frame->data[1], Frame->linesize[1], Dst->GetRowSize(PLANAR_U), Dst->GetHeight(PLANAR_U)); 
			Env->BitBlt(Dst->GetWritePtr(PLANAR_V), Dst->GetPitch(PLANAR_V), Frame->data[2], Frame->linesize[2], Dst->GetRowSize(PLANAR_V), Dst->GetHeight(PLANAR_V)); 
		} else {
			if (VI.IsRGB())
				Env->BitBlt(Dst->GetWritePtr() + Dst->GetPitch() * (Dst->GetHeight() - 1), -Dst->GetPitch(), Frame->data[0], Frame->linesize[0], Dst->GetRowSize(), Dst->GetHeight()); 
			else
				Env->BitBlt(Dst->GetWritePtr(), Dst->GetPitch(), Frame->data[0], Frame->linesize[0], Dst->GetRowSize(), Dst->GetHeight()); 
		}

		return Dst;
	}

public:
	FFVideoBase() {
		SWS = NULL;
		ConvertToFormat = PIX_FMT_NONE;
		ConvertFromFormat = PIX_FMT_NONE;
	}
};

class FFAudioBase : public FFBase {
protected:
	struct SampleInfo {
		int64_t SampleStart;
		int64_t DTS;
		int64_t FilePos;
		bool KeyFrame;
		SampleInfo(int64_t _SampleStart, int64_t _DTS, int64_t _FilePos, bool _KeyFrame) {
			DTS = _DTS;
			SampleStart = _SampleStart;
			FilePos = _FilePos;
			KeyFrame = _KeyFrame;
		}
	};

	std::vector<SampleInfo> SI;

	int FindClosestAudioKeyFrame(int64_t Sample) {
		for (unsigned int i = SI.size() - 1; i > 0; i--)
			if (SI[i].SampleStart <= Sample && SI[i].KeyFrame)
				return i;
		return 0;
	}

	int SampleFromDTS(int64_t DTS) {
		for (unsigned int i = 0; i < SI.size(); i++)
			if (SI[i].DTS == DTS)
				return i;
		return -1;
	}

	int ClosestSampleFromDTS(int64_t DTS) {
		int Index = 0;
		int64_t BestDiff = 0xFFFFFFFFFFFFFF;
		for (unsigned int i = 0; i < SI.size(); i++) {
			int64_t CurrentDiff = FFABS(SI[i].DTS - DTS);
			if (CurrentDiff < BestDiff) {
				BestDiff = CurrentDiff;
				Index = i;
			}
		}
		return Index;
	}
};

class FFMatroskaBase {
private:
	StdIoStream	ST; 
    unsigned int BufferSize;
    CompressedStream *CS;
protected:
	AVCodecContext CodecContext;
	AVCodec *Codec;
	MatroskaFile *MF;
	int Track;
	char ErrorMessage[256];
    uint8_t *Buffer;

	FFMatroskaBase(const char *Source, int _Track, unsigned char TrackType, IScriptEnvironment* Env) {
		Track = _Track;
		BufferSize = 0;
		Buffer = NULL;
		CS = NULL;

		memset(&ST,0,sizeof(ST));
		ST.base.read = (int (__cdecl *)(InputStream *,ulonglong,void *,int))StdIoRead;
		ST.base.scan = (longlong (__cdecl *)(InputStream *,ulonglong,unsigned int))StdIoScan;
		ST.base.getcachesize = (unsigned int (__cdecl *)(InputStream *))StdIoGetCacheSize;
		ST.base.geterror = (const char *(__cdecl *)(InputStream *))StdIoGetLastError;
		ST.base.memalloc = (void *(__cdecl *)(InputStream *,size_t))StdIoMalloc;
		ST.base.memrealloc = (void *(__cdecl *)(InputStream *,void *,size_t))StdIoRealloc;
		ST.base.memfree = (void (__cdecl *)(InputStream *,void *)) StdIoFree;
		ST.base.progress = (int (__cdecl *)(InputStream *,ulonglong,ulonglong))StdIoProgress;

		ST.fp = fopen(Source, "rb");
		if (ST.fp == NULL)
			Env->ThrowError("FFmpegSource: Can't open '%s': %s", Source, strerror(errno));

		setvbuf(ST.fp, NULL, _IOFBF, CACHESIZE);

		MF = mkv_OpenEx(&ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
		if (MF == NULL) {
			fclose(ST.fp);
			Env->ThrowError("FFmpegSource: Can't parse Matroska file: %s", ErrorMessage);
		}

		if (Track < 0)
			for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++)
				if (mkv_GetTrackInfo(MF, i)->Type == TrackType) {
					Track = i;
					break;
				}

		if (Track < 0)
			Env->ThrowError("FFmpegSource: No suitable track found");

		if ((unsigned int)Track >= mkv_GetNumTracks(MF))
			Env->ThrowError("FFmpegSource: Invalid track number");

		TrackInfo *TI = mkv_GetTrackInfo(MF, Track);

		if (TI->Type != TrackType)
			Env->ThrowError("FFmpegSource: Selected track is not of the right type");

		mkv_SetTrackMask(MF, ~(1 << Track));

		if (TI->CompEnabled) {
			CS = cs_Create(MF, Track, ErrorMessage, sizeof(ErrorMessage));
			if (CS == NULL)
				Env->ThrowError("FFmpegSource: Can't create decompressor: %s", ErrorMessage);
		}

		avcodec_get_context_defaults(&CodecContext);
		CodecContext.extradata = (uint8_t *)TI->CodecPrivate;
		CodecContext.extradata_size = TI->CodecPrivateSize;

		Codec = avcodec_find_decoder(MatroskaToFFCodecID(TI));
		if (Codec == NULL)
			Env->ThrowError("FFmpegSource: Codec not found");

		if (avcodec_open(&CodecContext, Codec) < 0)
			Env->ThrowError("FFmpegSource: Could not open codec");
	}

	~FFMatroskaBase() {
		free(Buffer);
		mkv_Close(MF);
		fclose(ST.fp);
		avcodec_close(&CodecContext);
	}

	unsigned int ReadNextFrame(int64_t *FirstStartTime, IScriptEnvironment *Env) {
		unsigned int TrackNumber, FrameFlags, FrameSize;
		uint64_t EndTime, FilePos, StartTime;
		*FirstStartTime = -1;

		while (mkv_ReadFrame(MF, 0, &TrackNumber, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
			if (*FirstStartTime < 0)
				*FirstStartTime = StartTime;
			if (CS) {
				char CSBuffer[4096];

				int DecompressedFrameSize = 0;

				cs_NextFrame(CS, FilePos, FrameSize);

				for (;;) {
					int ReadBytes = cs_ReadData(CS, CSBuffer, sizeof(CSBuffer));
					if (ReadBytes < 0)
						Env->ThrowError("FFmpegSource: Error decompressing data: %s", cs_GetLastError(CS));
					if (ReadBytes == 0)
						return DecompressedFrameSize;

					if (BufferSize < DecompressedFrameSize + ReadBytes) {
						BufferSize = FrameSize;
						Buffer = (uint8_t *)realloc(Buffer, BufferSize);
						if (Buffer == NULL) 
							Env->ThrowError("FFmpegSource: Out of memory");
					}

					memcpy(Buffer + DecompressedFrameSize, CSBuffer, ReadBytes);
					DecompressedFrameSize += ReadBytes;
				}
			} else {
				if (fseek(ST.fp, FilePos, SEEK_SET))
					Env->ThrowError("FFmpegSource: fseek(): %s", strerror(errno));

				if (BufferSize < FrameSize) {
					BufferSize = FrameSize;
					Buffer = (uint8_t *)realloc(Buffer, BufferSize);
					if (Buffer == NULL) 
						Env->ThrowError("FFmpegSource: Out of memory");
				}

				size_t ReadBytes = fread(Buffer, 1, FrameSize, ST.fp);
				if (ReadBytes != FrameSize) {
					if (ReadBytes == 0) {
						if (feof(ST.fp))
							Env->ThrowError("FFmpegSource: Unexpected EOF while reading frame");
						else
							Env->ThrowError("FFmpegSource: Error reading frame: %s", strerror(errno));
					} else
						Env->ThrowError("FFmpegSource: Short read while reading frame");
					return 0;
				}

				return FrameSize;
			}
		}

		return 0;
	}
};


class FFMKASource : public FFAudioBase, private FFMatroskaBase {
private:
	int	CurrentSample;
	int DecodeNextAudioBlock(uint8_t *Buf, int64_t *FirstStartTime, int64_t *Count, IScriptEnvironment* Env);
public:
	FFMKASource(const char *Source, int _Track, IScriptEnvironment* Env) : FFMatroskaBase(Source, _Track, TT_AUDIO, Env) {
		CurrentSample = 0;
		TrackInfo *TI = mkv_GetTrackInfo(MF, Track);

		switch (CodecContext.sample_fmt) {
			case SAMPLE_FMT_U8: VI.sample_type = SAMPLE_INT8; break;
			case SAMPLE_FMT_S16: VI.sample_type = SAMPLE_INT16; break;
			case SAMPLE_FMT_S24: VI.sample_type = SAMPLE_INT24; break;
			case SAMPLE_FMT_S32: VI.sample_type = SAMPLE_INT32; break;
			case SAMPLE_FMT_FLT: VI.sample_type = SAMPLE_FLOAT; break;
			default:
				Env->ThrowError("FFmpegSource: Unsupported/unknown sample format");
		}

		VI.nchannels = TI->AV.Audio.Channels;
		VI.audio_samples_per_second = mkv_TruncFloat(TI->AV.Audio.SamplingFreq);

		unsigned TrackNumber, FrameSize, FrameFlags;
		ulonglong StartTime, EndTime, FilePos;

		while (mkv_ReadFrame(MF, 0, &TrackNumber, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
			if (!(FrameFlags & FRAME_UNKNOWN_START))
				SI.push_back(SampleInfo(VI.num_audio_samples, StartTime, FilePos, (FrameFlags & FRAME_KF) != 0));
			if (true) {
				switch (CodecContext.codec_id) {
					case CODEC_ID_MP3:
					case CODEC_ID_MP2:
						VI.num_audio_samples += 1152; break;
					default:
						Env->ThrowError("Only mp2 and mp3 is currently supported in matroska");
				}
			} else {
			}
		}

		mkv_Seek(MF, SI[0].DTS, MKVF_SEEK_TO_PREV_KEYFRAME);
		avcodec_flush_buffers(&CodecContext);
	}

	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment* Env);
};

int FFMKASource::DecodeNextAudioBlock(uint8_t *Buf, int64_t *FirstStartTime, int64_t *Count, IScriptEnvironment* Env) {
	int Ret = -1;
	int FrameSize;
	int64_t TempStartTime = -1;
	*FirstStartTime = -1;
	*Count = 0;

	while (FrameSize = ReadNextFrame(&TempStartTime, Env)) {
		if (*FirstStartTime < 0)
			*FirstStartTime = TempStartTime;
			uint8_t *Data = (uint8_t *)Buffer;
			int Size = FrameSize;

			while (Size > 0) {
				int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
				Ret = avcodec_decode_audio2(&CodecContext, (int16_t *)Buf, &TempOutputBufSize, Data, Size);
				if (Ret < 0)
					goto Done;
				Size -= Ret;
				Data += Ret;
				Buf += TempOutputBufSize;
				*Count += VI.AudioSamplesFromBytes(TempOutputBufSize);

				if (Size <= 0)
					goto Done;
			}
	}

Done:
	return Ret;
}

void __stdcall FFMKASource::GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment* Env) {
	bool HasSeeked = false;
	int ClosestKF = FindClosestAudioKeyFrame(Start);

	memset(Buf, 0, VI.BytesFromAudioSamples(Count));

	if (Start < CurrentSample || SI[ClosestKF].SampleStart > CurrentSample) {
		mkv_Seek(MF, SI[max(ClosestKF - 10, 0)].DTS, MKVF_SEEK_TO_PREV_KEYFRAME);
		avcodec_flush_buffers(&CodecContext);
		HasSeeked = true;
	}

	uint8_t *DstBuf = (uint8_t *)Buf;
	int64_t RemainingSamples = Count;
	int64_t DecodeCount;

	do {
		int64_t StartTime;
		uint8_t DecodingBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
		int64_t DecodeStart = CurrentSample;
		int Ret = DecodeNextAudioBlock(DecodingBuffer, &StartTime, &DecodeCount, Env);

		if (HasSeeked) {
			HasSeeked = false;

			int CurrentSampleIndex = SampleFromDTS(StartTime);
			if (StartTime < 0 || CurrentSampleIndex < 0 || (CurrentSample = DecodeStart = SI[max(CurrentSampleIndex, 0)].SampleStart) < 0)
				Env->ThrowError("FFmpegSource: Sample accurate seeking is not possible in this file");
		}

		int OffsetBytes = VI.BytesFromAudioSamples(max(0, Start - DecodeStart));
		int CopyBytes = max(0, VI.BytesFromAudioSamples(min(RemainingSamples, DecodeCount - max(0, Start - DecodeStart))));

		memcpy(DstBuf, DecodingBuffer + OffsetBytes, CopyBytes);
		DstBuf += CopyBytes;

		CurrentSample += DecodeCount;
		RemainingSamples -= VI.AudioSamplesFromBytes(CopyBytes);

	} while (RemainingSamples > 0 && DecodeCount > 0);
}

class FFMKVSource : public FFVideoBase, private FFMatroskaBase {
private:
	AVFrame *Frame; 
	int	CurrentFrame;

	int DecodeNextFrame(AVFrame *Frame, int64_t *FirstStartTime, IScriptEnvironment* Env);
public:
	FFMKVSource(const char *Source, int _Track, FILE *Timecodes, bool Cache, FILE *CacheFile, IScriptEnvironment* Env) : FFMatroskaBase(Source, _Track, TT_VIDEO, Env) {
		Frame = NULL;
		CurrentFrame = 0;
		TrackInfo *TI = mkv_GetTrackInfo(MF, Track);

		VI.image_type = VideoInfo::IT_TFF;
		VI.width = TI->AV.Video.PixelWidth;
		VI.height = TI->AV.Video.PixelHeight;
		VI.fps_denominator = 1;
		VI.fps_numerator = 30;

		SetOutputFormat(CodecContext.pix_fmt, Env);	

		// Needs to be indexed?
		if (!(Cache && LoadFrameInfoFromFile(CacheFile))) {
			unsigned int TrackNumber, FrameSize, FrameFlags;
			uint64_t StartTime, EndTime, FilePos;

			while (mkv_ReadFrame(MF, 0, &TrackNumber, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
				FrameToDTS.push_back(FrameInfo(StartTime, (FrameFlags & FRAME_KF) != 0));
				VI.num_frames++;
			}

			if (VI.num_frames == 0)
				Env->ThrowError("FFmpegSource: Video track contains no frames");

			mkv_Seek(MF, FrameToDTS[0].DTS, MKVF_SEEK_TO_PREV_KEYFRAME);

			if (Cache) {
				fprintf(CacheFile, "%d\n", VI.num_frames);
				for (int i = 0; i < VI.num_frames; i++)
					fprintf(CacheFile, "%lld %d\n", FrameToDTS[i].DTS, (int)(FrameToDTS[i].KeyFrame ? 1 : 0));			
			}
		}

		if (Timecodes)
			for (int i = 0; i < VI.num_frames; i++)
				fprintf(Timecodes, "%f\n", (FrameToDTS[i].DTS * mkv_TruncFloat(TI->TimecodeScale)) / (double)(1000000));

		Frame = avcodec_alloc_frame();
	}

	~FFMKVSource() {
		av_free(Frame);
	}

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
};

int FFMKVSource::DecodeNextFrame(AVFrame *Frame, int64_t *FirstStartTime, IScriptEnvironment* Env) {
	int FrameFinished = 0;
	int Ret = -1;
	int FrameSize;
	int64_t TempStartTime = -1;

	while (FrameSize = ReadNextFrame(&TempStartTime, Env)) {
		if (*FirstStartTime < 0)
			*FirstStartTime = TempStartTime;
		Ret = avcodec_decode_video(&CodecContext, Frame, &FrameFinished, Buffer, FrameSize);

		if (FrameFinished)
			goto Done;
	}

	// Flush the last frame
	if (CurrentFrame == VI.num_frames - 1)
		Ret = avcodec_decode_video(&CodecContext, Frame, &FrameFinished, NULL, 0);

Done:
	return Ret;
}

PVideoFrame __stdcall FFMKVSource::GetFrame(int n, IScriptEnvironment* Env) {
	bool HasSeeked = false;

	if (n < CurrentFrame || FindClosestKeyFrame(n) > CurrentFrame) {
		mkv_Seek(MF, FrameToDTS[n].DTS, MKVF_SEEK_TO_PREV_KEYFRAME);
		avcodec_flush_buffers(&CodecContext);
		HasSeeked = true;
	}

	do {
		int64_t StartTime;
		int Ret = DecodeNextFrame(Frame, &StartTime, Env);

		if (HasSeeked) {
			HasSeeked = false;

			if (StartTime < 0 || (CurrentFrame = FrameFromDTS(StartTime)) < 0)
				Env->ThrowError("FFmpegSource: Frame accurate seeking is not possible in this file");
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	return OutputFrame(Frame, Env);
}

class FFAVFormatBase {
protected:
	AVFormatContext *FormatContext;
	AVCodecContext *CodecContext;
	AVCodec *Codec;
	int Track;
public:
	FFAVFormatBase(const char *Source, int _Track, CodecType TrackType, IScriptEnvironment* Env) {
		FormatContext = NULL;
		Codec = NULL;
		Track = _Track;

		if (av_open_input_file(&FormatContext, Source, NULL, 0, NULL) != 0)
			Env->ThrowError("FFmpegSource: Couldn't open \"%s\"", Source);

		if (av_find_stream_info(FormatContext) < 0)
			Env->ThrowError("FFmpegSource: Couldn't find stream information");

		if (Track < 0)
			for(unsigned int i = 0; i < FormatContext->nb_streams; i++)
				if(FormatContext->streams[i]->codec->codec_type == TrackType) {
					Track = i;
					break;
				}

		if(Track < -1)
			Env->ThrowError("FFmpegSource: No suitable track found");

		if (Track >= (int)FormatContext->nb_streams)
			Env->ThrowError("FFmpegSource: Invalid track number");

		if (FormatContext->streams[Track]->codec->codec_type != TrackType)
			Env->ThrowError("FFmpegSource: Selected track is not of the right type");

		CodecContext = FormatContext->streams[Track]->codec;

		Codec = avcodec_find_decoder(CodecContext->codec_id);
		if(Codec == NULL)
			Env->ThrowError("FFmpegSource: Codec not found");

		if(avcodec_open(CodecContext, Codec) < 0)
			Env->ThrowError("FFmpegSource: Could not open codec");
	}

	~FFAVFormatBase() {
		avcodec_close(CodecContext);
		av_close_input_file(FormatContext);
	}
};

class FFAudioSource : public FFAudioBase, private FFAVFormatBase {
private:
	int	CurrentSample;

	int DecodeNextAudioBlock(uint8_t *Buf, int64_t *DTS, int64_t *Count, IScriptEnvironment* Env);
public:
	FFAudioSource(const char *Source, int _Track, IScriptEnvironment* Env) : FFAVFormatBase(Source, _Track, CODEC_TYPE_AUDIO, Env) {
		CurrentSample = 0;

		switch (CodecContext->sample_fmt) {
			case SAMPLE_FMT_U8: VI.sample_type = SAMPLE_INT8; break;
			case SAMPLE_FMT_S16: VI.sample_type = SAMPLE_INT16; break;
			case SAMPLE_FMT_S24: VI.sample_type = SAMPLE_INT24; break;
			case SAMPLE_FMT_S32: VI.sample_type = SAMPLE_INT32; break;
			case SAMPLE_FMT_FLT: VI.sample_type = SAMPLE_FLOAT; break;
			default:
				Env->ThrowError("FFmpegSource: Unsupported/unknown sample format");
		}

		VI.nchannels = CodecContext->channels;
		VI.audio_samples_per_second = CodecContext->sample_rate;

		if (CodecContext->frame_size == 0)
				Env->ThrowError("Variable frame sizes currently unsupported");
		AVPacket Packet;
		while (av_read_frame(FormatContext, &Packet) >= 0) {
			if (Packet.stream_index == Track) {
				SI.push_back(SampleInfo(VI.num_audio_samples, Packet.dts, Packet.pos, (Packet.flags & PKT_FLAG_KEY) != 0));
				VI.num_audio_samples += CodecContext->frame_size;
			}
			av_free_packet(&Packet);
		}

		if (VI.num_audio_samples == 0)
			Env->ThrowError("FFmpegSource: Audio track contains no samples");

		av_seek_frame(FormatContext, Track, SI[0].DTS, AVSEEK_FLAG_BACKWARD);
		avcodec_flush_buffers(CodecContext);
	}

	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment* Env);
};

int FFAudioSource::DecodeNextAudioBlock(uint8_t *Buf, int64_t *DTS, int64_t *Count, IScriptEnvironment* Env) {
	AVPacket Packet;
	int Ret = -1;
	*Count = 0;
	*DTS = -1;

	while (av_read_frame(FormatContext, &Packet) >= 0) {
        if (Packet.stream_index == Track) {
			if (*DTS < 0)
				*DTS = Packet.dts;

			uint8_t *Data = (uint8_t *)Packet.data;
			int Size = Packet.size;

			while (Size > 0) {
				int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
				Ret = avcodec_decode_audio2(CodecContext, (int16_t *)Buf, &TempOutputBufSize, Data, Size);
				if (Ret < 0) {
					av_free_packet(&Packet);
					goto Done;
				}

				Size -= Ret;
				Data += Ret;
				Buf += TempOutputBufSize;
				*Count += VI.AudioSamplesFromBytes(TempOutputBufSize);

				if (Size <= 0) {
					av_free_packet(&Packet);
					goto Done;
				}
			}
        }

		av_free_packet(&Packet);
	}

Done:
	return Ret;
}

void __stdcall FFAudioSource::GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment* Env) {
	bool HasSeeked = false;
	int ClosestKF = FindClosestAudioKeyFrame(Start);

	memset(Buf, 0, VI.BytesFromAudioSamples(Count));

	if (Start < CurrentSample || SI[ClosestKF].SampleStart > CurrentSample) {
		av_seek_frame(FormatContext, Track, SI[max(ClosestKF - 10, 0)].DTS, AVSEEK_FLAG_BACKWARD);
		avcodec_flush_buffers(CodecContext);
		HasSeeked = true;
	}

	uint8_t *DstBuf = (uint8_t *)Buf;
	int64_t RemainingSamples = Count;
	int64_t DecodeCount;

	do {
		int64_t StartTime;
		uint8_t DecodingBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
		int64_t DecodeStart = CurrentSample;
		int Ret = DecodeNextAudioBlock(DecodingBuffer, &StartTime, &DecodeCount, Env);

		if (HasSeeked) {
			HasSeeked = false;

			if (StartTime < 0 || (CurrentSample = DecodeStart = SI[max(ClosestSampleFromDTS(StartTime) - 0, 0)].SampleStart) < 0)
				Env->ThrowError("FFmpegSource: Sample accurate seeking is not possible in this file");
		}

		int OffsetBytes = VI.BytesFromAudioSamples(max(0, Start - DecodeStart));
		int CopyBytes = max(0, VI.BytesFromAudioSamples(min(RemainingSamples, DecodeCount - max(0, Start - DecodeStart))));

		memcpy(DstBuf, DecodingBuffer + OffsetBytes, CopyBytes);
		DstBuf += CopyBytes;

		CurrentSample += DecodeCount;
		RemainingSamples -= VI.AudioSamplesFromBytes(CopyBytes);

	} while (RemainingSamples > 0 && DecodeCount > 0);
}

class FFVideoSource : public FFVideoBase, private FFAVFormatBase {
private:
	AVFrame *Frame;
	int	CurrentFrame;
	int SeekMode;

	int DecodeNextFrame(AVFrame *Frame, int64_t *DTS);
public:
	FFVideoSource(const char *Source, int _Track, int _SeekMode, FILE *Timecodes, bool Cache, FILE *CacheFile, IScriptEnvironment* Env) : FFAVFormatBase(Source, _Track, CODEC_TYPE_VIDEO, Env) {
		CurrentFrame = 0;
		Frame = NULL;
		SeekMode = _SeekMode;

		VI.image_type = VideoInfo::IT_TFF;
		VI.width = CodecContext->width;
		VI.height = CodecContext->height;
		VI.fps_denominator = FormatContext->streams[Track]->time_base.num;
		VI.fps_numerator = FormatContext->streams[Track]->time_base.den;
		VI.num_frames = (int)FormatContext->streams[Track]->duration;

		// sanity check framerate
		if (VI.fps_denominator > VI.fps_numerator || VI.fps_denominator <= 0 || VI.fps_numerator <= 0) {
			VI.fps_denominator = 1;
			VI.fps_numerator = 30;
		}
		
		SetOutputFormat(CodecContext->pix_fmt, Env);

		// Needs to be indexed?
		if (!(Cache && LoadFrameInfoFromFile(CacheFile))) {
			AVPacket Packet;
			VI.num_frames = 0;
			while (av_read_frame(FormatContext, &Packet) >= 0) {
				if (Packet.stream_index == Track) {
					FrameToDTS.push_back(FrameInfo(Packet.dts, (Packet.flags & PKT_FLAG_KEY) != 0));
					VI.num_frames++;
				}
				av_free_packet(&Packet);
			}

			if (VI.num_frames == 0)
				Env->ThrowError("FFmpegSource: Video track contains no frames");

			av_seek_frame(FormatContext, Track, FrameToDTS[0].DTS, AVSEEK_FLAG_BACKWARD);

			if (Cache) {
				fprintf(CacheFile, "%d\n", VI.num_frames);
				for (int i = 0; i < VI.num_frames; i++)
					fprintf(CacheFile, "%lld %d\n", FrameToDTS[i].DTS, (int)(FrameToDTS[i].KeyFrame ? 1 : 0));			
			}
		}

		if (Timecodes)
			for (int i = 0; i < VI.num_frames; i++)
				fprintf(Timecodes, "%f\n", (FrameToDTS[i].DTS * FormatContext->streams[Track]->time_base.num * 1000) / (double)(FormatContext->streams[Track]->time_base.den));

		Frame = avcodec_alloc_frame();
	}

	~FFVideoSource() {
		av_free(Frame);
	}

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
};

int FFVideoSource::DecodeNextFrame(AVFrame *Frame, int64_t *DTS) {
	AVPacket Packet;
	int FrameFinished = 0;
	int Ret = -1;
	*DTS = -1;

	while (av_read_frame(FormatContext, &Packet) >= 0) {
        if (Packet.stream_index == Track) {
			Ret = avcodec_decode_video(CodecContext, Frame, &FrameFinished, Packet.data, Packet.size);

			if (*DTS < 0)
				*DTS = Packet.dts;
        }

        av_free_packet(&Packet);

		if (FrameFinished)
			goto Done;
	}

	// Flush the last frame
	if (CurrentFrame == VI.num_frames - 1)
		Ret = avcodec_decode_video(CodecContext, Frame, &FrameFinished, NULL, 0);

Done:
	return Ret;
}

PVideoFrame __stdcall FFVideoSource::GetFrame(int n, IScriptEnvironment* Env) {
	bool HasSeeked = false;
	int ClosestKF = FindClosestKeyFrame(n);

	if (SeekMode == 0) {
		if (n < CurrentFrame) {
			av_seek_frame(FormatContext, Track, FrameToDTS[0].DTS, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(CodecContext);
			CurrentFrame = 0;
		}
	} else {
		// 10 frames is used as a margin to prevent excessive seeking since the predicted best keyframe isn't always selected by avformat
		if (n < CurrentFrame || ClosestKF > CurrentFrame + 10 || (SeekMode == 3 && n > CurrentFrame + 10)) {
			av_seek_frame(FormatContext, Track, (SeekMode == 3) ? FrameToDTS[n].DTS : FrameToDTS[ClosestKF].DTS, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(CodecContext);
			HasSeeked = true;
		}
	}

	do {
		int64_t DTS;
		int Ret = DecodeNextFrame(Frame, &DTS);

		if (SeekMode == 0 && Frame->key_frame)
			FrameToDTS[CurrentFrame].KeyFrame = true;

		if (HasSeeked) {
			HasSeeked = false;

			// Is the seek destination time known? Does it belong to a frame?
			if (DTS < 0 || (CurrentFrame = FrameFromDTS(DTS)) < 0) {
				switch (SeekMode) {
					case 1:
						Env->ThrowError("FFmpegSource: Frame accurate seeking is not possible in this file");
					case 2:
					case 3:
						CurrentFrame = ClosestFrameFromDTS(DTS);
						break;
					default:
						Env->ThrowError("FFmpegSource: Failed assertion");
				}	
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	return OutputFrame(Frame, Env);
}

class FFAudioRefSource : public FFBase, private FFAVFormatBase {
private:
	FILE *AudioCache;
public:
	FFAudioRefSource(const char *Source, int _Track, const char *CacheFile, IScriptEnvironment* Env) : FFAVFormatBase(Source, _Track, CODEC_TYPE_AUDIO, Env) {
		switch (CodecContext->sample_fmt) {
			case SAMPLE_FMT_U8: VI.sample_type = SAMPLE_INT8; break;
			case SAMPLE_FMT_S16: VI.sample_type = SAMPLE_INT16; break;
			case SAMPLE_FMT_S24: VI.sample_type = SAMPLE_INT24; break;
			case SAMPLE_FMT_S32: VI.sample_type = SAMPLE_INT32; break;
			case SAMPLE_FMT_FLT: VI.sample_type = SAMPLE_FLOAT; break;
			default:
				Env->ThrowError("FFmpegSource: Unsupported/unknown sample format");
		}

		VI.nchannels = CodecContext->channels;
		VI.audio_samples_per_second = CodecContext->sample_rate;

		AudioCache = fopen(CacheFile, "ab+");
		if (AudioCache == NULL)
			Env->ThrowError("FFmpegSource: Failed to open the cache file");
		_fseeki64(AudioCache, 0, SEEK_END);
		int64_t CacheSize = _ftelli64(AudioCache);
		if (CacheSize > 0) {
			VI.num_audio_samples = VI.AudioSamplesFromBytes(CacheSize);
		} else {
			AVPacket Packet;
			uint8_t DecodingBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];

			while (av_read_frame(FormatContext, &Packet) >= 0) {
				if (Packet.stream_index == Track) {
					uint8_t *Data = (uint8_t *)Packet.data;
					int Size = Packet.size;

					while (Size > 0) {
						int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
						int Ret = avcodec_decode_audio2(CodecContext, (int16_t *)DecodingBuffer, &TempOutputBufSize, Data, Size);
						if (Ret < 0) {
							av_free_packet(&Packet);
							Env->ThrowError("FFmpegSource: Audio decoding error");
						}

						Size -= Ret;
						Data += Ret;
						VI.num_audio_samples += VI.AudioSamplesFromBytes(TempOutputBufSize);

						fwrite(DecodingBuffer, 1, TempOutputBufSize, AudioCache); 
					}
				}

				av_free_packet(&Packet);
			}
		}
	}

	~FFAudioRefSource() {
		fclose(AudioCache);
	}

	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment* Env) {
		_fseeki64(AudioCache, VI.BytesFromAudioSamples(Start), SEEK_SET);
		fread(Buf, 1, VI.BytesFromAudioSamples(Count), AudioCache);
	}
};

AVSValue __cdecl CreateFFVideoSource(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
	if (!Args[0].Defined())
    	Env->ThrowError("FFmpegSource: No source specified");

	const char *Source = Args[0].AsString();
	int Track = Args[1].AsInt(-1);
	int SeekMode = Args[2].AsInt(1);
	const char *Timecodes = Args[3].AsString("");
	bool Cache = Args[4].AsBool(true);

	// Generate default cache filename
	char DefaultCacheFilename[1024];
	strcpy(DefaultCacheFilename, Source);
	strcat(DefaultCacheFilename, ".ffcache");
	const char *CacheFilename = Args[5].AsString(DefaultCacheFilename);

	if (SeekMode < 0 || SeekMode > 3)
    	Env->ThrowError("FFmpegSource: Invalid seek mode selected");

	FILE *TCFile = NULL;
	if (strcmp(Timecodes, "")) {
		TCFile = fopen(Timecodes, "w");
		if (!TCFile)
			Env->ThrowError("FFmpegSource: Failed to open timecode output file for writing");
		fprintf(TCFile, "# timecode format v2\n");
	}

	FILE *CacheFile = NULL;
	if (Cache) {
		CacheFile = fopen(CacheFilename, "a+");
		if (!CacheFile)
			Env->ThrowError("FFmpegSource: Failed to open cache file");
		fseek(CacheFile, 0, SEEK_END);
	}

	av_register_all();
	AVFormatContext *FormatContext;

	if (av_open_input_file(&FormatContext, Source, NULL, 0, NULL) != 0)
		Env->ThrowError("FFmpegSource: Couldn't open \"%s\"", Args[0].AsString());
	bool IsMatroska = !strcmp(FormatContext->iformat->name, "matroska");
	av_close_input_file(FormatContext);

	FFBase *Ret;

	if (IsMatroska)
		Ret = new FFMKVSource(Source, Track, TCFile, Cache, CacheFile, Env);
	else
		Ret = new FFVideoSource(Source, Track, SeekMode, TCFile, Cache, CacheFile, Env);

	if (TCFile)
		fclose(TCFile);

	if (CacheFile)
		fclose(CacheFile);

	return Ret;
}

AVSValue __cdecl CreateFFAudioSource(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
	if (!Args[0].Defined())
    	Env->ThrowError("FFmpegSource: No source specified");

	const char *Source = Args[0].AsString();
	int Track = Args[1].AsInt(-1);

	av_register_all();
	AVFormatContext *FormatContext;

	if (av_open_input_file(&FormatContext, Source, NULL, 0, NULL) != 0)
		Env->ThrowError("FFmpegSource: Couldn't open \"%s\"", Source);
	bool IsMatroska = !strcmp(FormatContext->iformat->name, "matroska");
	av_close_input_file(FormatContext);

	if (IsMatroska)
		return new FFMKASource(Source, Track, Env);
	else
		return new FFAudioSource(Source, Track, Env);
}

AVSValue __cdecl CreateFFAudioRefSource(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
	if (!Args[0].Defined())
    	Env->ThrowError("FFmpegSource: No source specified");

	const char *Source = Args[0].AsString();
	int Track = Args[1].AsInt(-1);

	// Generate default cache filename
	char DefaultCacheFilename[1024];
	strcpy(DefaultCacheFilename, Source);
	strcat(DefaultCacheFilename, ".ffracache");
	const char *CacheFilename = Args[2].AsString(DefaultCacheFilename);

	av_register_all();
	
	return new FFAudioRefSource(Source, Track, CacheFilename, Env);
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* Env) {
    Env->AddFunction("FFVideoSource", "[source]s[track]i[seekmode]i[timecodes]s[cache]b[cachefile]s", CreateFFVideoSource, 0);
    Env->AddFunction("FFAudioSource", "[source]s[track]i", CreateFFAudioSource, 0);
    Env->AddFunction("FFAudioRefSource", "[source]s[track]i[cachefile]s", CreateFFAudioRefSource, 0);
    return "FFmpegSource";
};

