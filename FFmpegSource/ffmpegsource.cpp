#include <windows.h>
#include <stdio.h>
#include <map>
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
private:
    SwsContext *SWS;
	int ConvertToFormat;
	int ConvertFromFormat;
protected:
	VideoInfo VI;

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
	virtual bool __stdcall GetParity(int n) { return 0; }
	virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { }
	virtual void __stdcall SetCacheHints(int cachehints, int frame_range) { }
	virtual const VideoInfo& __stdcall GetVideoInfo() { return VI; }

	FFBase() {
		SWS = NULL;
		ConvertToFormat = PIX_FMT_NONE;
		ConvertFromFormat = PIX_FMT_NONE;
		memset(&VI, 0, sizeof(VI));
	}
};

class FFMKVSource : public FFBase {
private:
	AVCodecContext CodecContext;
	AVCodec *Codec;
	AVFrame *Frame; 
	int	CurrentFrame;

	StdIoStream	ST; 
	MatroskaFile *MF;

	char ErrorMessage[256];
    unsigned int BufferSize;
    uint8_t *Buffer;
    CompressedStream *CS;

	unsigned int ReadNextFrame(int64_t *FirstStartTime, IScriptEnvironment *Env);
	int DecodeNextFrame(AVFrame *Frame, int64_t *FirstStartTime, IScriptEnvironment* Env);
public:
	FFMKVSource(const char *Source, int Track, FILE *Timecodes, bool Cache, FILE *CacheFile, IScriptEnvironment* Env) {
		BufferSize = 0;
		Buffer = NULL;
		Frame = NULL;
		CS = NULL;
		CurrentFrame = 0;

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
				if (mkv_GetTrackInfo(MF, i)->Type == TT_VIDEO) {
					Track = i;
					break;
				}

		if (Track < 0)
			Env->ThrowError("FFmpegSource: No video track found");

		if ((unsigned int)Track >= mkv_GetNumTracks(MF))
			Env->ThrowError("FFmpegSource: Invalid track number: %d", Track);

		TrackInfo *TI = mkv_GetTrackInfo(MF, Track);

		if (TI->Type != TT_VIDEO)
			Env->ThrowError("FFmpegSource: Selected track is not video");

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
		free(Buffer);
		mkv_Close(MF);
		fclose(ST.fp);
		av_free(Frame);
		avcodec_close(&CodecContext);
	}

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
};

unsigned int FFMKVSource::ReadNextFrame(int64_t *FirstStartTime, IScriptEnvironment *Env) {
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
			break;
	}

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
				Env->ThrowError("FFmpegSource: Frame accurate seeking not possible in this file");
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	return OutputFrame(Frame, Env);
}

class FFmpegSource : public FFBase {
private:
	AVFormatContext *FormatContext;
	AVCodecContext *CodecContext;
	AVCodec *Codec;
	AVFrame *Frame;
	int Track;
	int	CurrentFrame;
	int SeekMode;

	int DecodeNextFrame(AVFrame *Frame, int64_t *DTS);
public:
	FFmpegSource(const char *Source, int _Track, int _SeekMode, FILE *Timecodes, bool Cache, FILE *CacheFile, IScriptEnvironment* Env) : Track(_Track), SeekMode(_SeekMode) {
		CurrentFrame = 0;
		Frame = NULL;

		if (av_open_input_file(&FormatContext, Source, NULL, 0, NULL) != 0)
			Env->ThrowError("FFmpegSource: Couldn't open \"%s\"", Source);

		if (av_find_stream_info(FormatContext) < 0)
			Env->ThrowError("FFmpegSource: Couldn't find stream information");

		if (Track >= (int)FormatContext->nb_streams)
			Env->ThrowError("FFmpegSource: Invalid track number");

		if (Track < 0)
			for(unsigned int i = 0; i < FormatContext->nb_streams; i++)
				if(FormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
					Track = i;
					break;
				}

		if(Track < -1)
			Env->ThrowError("FFmpegSource: Couldn't find a video stream");

		if (FormatContext->streams[Track]->codec->codec_type != CODEC_TYPE_VIDEO)
			Env->ThrowError("FFmpegSource: Selected stream doesn't contain video");

		CodecContext = FormatContext->streams[Track]->codec;

		Codec = avcodec_find_decoder(CodecContext->codec_id);
		if(Codec == NULL)
			Env->ThrowError("FFmpegSource: Codec not found");

		if(avcodec_open(CodecContext, Codec) < 0)
			Env->ThrowError("FFmpegSource: Could not open codec");

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

			av_seek_frame(FormatContext, Track, 0, AVSEEK_FLAG_BACKWARD);

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

	~FFmpegSource() {
		av_free(Frame);
		avcodec_close(CodecContext);
		av_close_input_file(FormatContext);
	}

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
};

int FFmpegSource::DecodeNextFrame(AVFrame *Frame, int64_t *DTS) {
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
			break;
	}

	return Ret;
}

PVideoFrame __stdcall FFmpegSource::GetFrame(int n, IScriptEnvironment* Env) {
	bool HasSeeked = false;

	if (SeekMode == 0) {
		if (n < CurrentFrame) {
			av_seek_frame(FormatContext, Track, 0, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(CodecContext);
			CurrentFrame = 0;
		}
	} else {
		if (n < CurrentFrame || FindClosestKeyFrame(n) > CurrentFrame || (SeekMode == 3 && n > CurrentFrame + 10)) {
			av_seek_frame(FormatContext, Track, (SeekMode == 3) ? FrameToDTS[n].DTS : FrameToDTS[FindClosestKeyFrame(n)].DTS, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(CodecContext);
			HasSeeked = true;
		}
	}

	do {
		int64_t DTS;
		int Ret = DecodeNextFrame(Frame, &DTS);

		if (HasSeeked) {
			HasSeeked = false;

			// Is the seek destination time known? Does it belong to a frame?
			if (DTS < 0 || (CurrentFrame = FrameFromDTS(DTS)) < 0) {
				switch (SeekMode) {
					case 1:
						Env->ThrowError("FFmpegSource: Frame accurate seeking not possible in this file");
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

AVSValue __cdecl CreateFFmpegSource(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
	if (!Args[0].Defined())
    	Env->ThrowError("FFmpegSource: No source specified");

	// Generate default cache filename
	char DefaultCacheFilename[MAX_PATH];
	strcpy(DefaultCacheFilename, Args[0].AsString());
	strcat(DefaultCacheFilename, ".ffcache");

	const char *Source = Args[0].AsString();
	int Track = Args[1].AsInt(-1);
	int SeekMode = Args[2].AsInt(1);
	const char *Timecodes = Args[3].AsString("");
	bool Cache = Args[4].AsBool(true);
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
		Ret = new FFmpegSource(Source, Track, SeekMode, TCFile, Cache, CacheFile, Env);

	if (TCFile)
		fclose(TCFile);

	if (CacheFile)
		fclose(CacheFile);

	return Ret;
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* Env) {
    Env->AddFunction("FFmpegSource", "[source]s[track]i[seekmode]i[timecodes]s[cache]b[cachefile]s", CreateFFmpegSource, 0);
    return "FFmpegSource";
};

