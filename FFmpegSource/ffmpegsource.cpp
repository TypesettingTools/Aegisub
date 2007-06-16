#include <windows.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
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
#include "stdiostream.cpp"

class FFBase : public IClip {
private:
    SwsContext *SWS;
	int ConvertToFormat;
	int ConvertFromFormat;
protected:
	VideoInfo VI;

	void SetOutputFormat(int CurrentFormat, IScriptEnvironment *Env) {
		int Loss;
		int BestFormat = avcodec_find_best_pix_fmt((1 << PIX_FMT_YUV420P) | (1 << PIX_FMT_YUYV422) | (1 << PIX_FMT_RGB32) | (1 << PIX_FMT_BGR24), CurrentFormat, 1, &Loss);

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
			SWS = sws_getContext(VI.width, VI.height, ConvertFromFormat, VI.width, VI.height, ConvertToFormat, SWS_LANCZOS, NULL, NULL, NULL);
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

	struct FFMKVFrameInfo {
		ulonglong DTS;
		bool KeyFrame;
	};

	std::vector<FFMKVFrameInfo> FrameToDTS;
	std::map<ulonglong, int> DTSToFrame;

	StdIoStream	ST; 
	MatroskaFile *MF;

	char ErrorMessage[256];
    unsigned int BufferSize;
    void *Buffer;
    CompressedStream *CS;

	int ReadNextFrame(AVFrame *Frame, ulonglong *StartTime, IScriptEnvironment* Env);

	CodecID MatroskaToFFCodecID(TrackInfo *TI) {
		char *Codec = TI->CodecID;
		// fourcc list from ffdshow
		if (!strcmp(Codec, "V_MS/VFW/FOURCC")) {
			switch (((BITMAPINFOHEADER *)TI->CodecPrivate)->biCompression) {
				case MAKEFOURCC('F', 'F', 'D', 'S'):
				case MAKEFOURCC('F', 'V', 'F', 'W'):
				case MAKEFOURCC('X', 'V', 'I', 'D'):
				case MAKEFOURCC('D', 'I', 'V', 'X'):
				case MAKEFOURCC('D', 'X', '5', '0'):
				case MAKEFOURCC('M', 'P', '4', 'V'):
				case MAKEFOURCC('3', 'I', 'V', 'X'):
				case MAKEFOURCC('W', 'V', '1', 'F'):
				case MAKEFOURCC('F', 'M', 'P', '4'):
				case MAKEFOURCC('S', 'M', 'P', '4'):
					return CODEC_ID_MPEG4;
				case MAKEFOURCC('D', 'I', 'V', '3'):
				case MAKEFOURCC('D', 'V', 'X', '3'):
				case MAKEFOURCC('M', 'P', '4', '3'):
					return CODEC_ID_MSMPEG4V3;
				case MAKEFOURCC('M', 'P', '4', '2'):
					return CODEC_ID_MSMPEG4V2;
				case MAKEFOURCC('M', 'P', '4', '1'):
					return CODEC_ID_MSMPEG4V1;
				case MAKEFOURCC('W', 'M', 'V', '1'):
					return CODEC_ID_WMV1;
				case MAKEFOURCC('W', 'M', 'V', '2'):
					return CODEC_ID_WMV2;
				case MAKEFOURCC('W', 'M', 'V', '3'):
					return CODEC_ID_WMV3;
/*
				case MAKEFOURCC('M', 'S', 'S', '1'):
				case MAKEFOURCC('M', 'S', 'S', '2'):
				case MAKEFOURCC('W', 'V', 'P', '2'):
				case MAKEFOURCC('W', 'M', 'V', 'P'):
					return CODEC_ID_WMV9_LIB;
*/
				case MAKEFOURCC('W', 'V', 'C', '1'):
					return CODEC_ID_VC1;
				case MAKEFOURCC('V', 'P', '5', '0'):
					return CODEC_ID_VP5;
				case MAKEFOURCC('V', 'P', '6', '0'):
				case MAKEFOURCC('V', 'P', '6', '1'):
				case MAKEFOURCC('V', 'P', '6', '2'):
					return CODEC_ID_VP6;
				case MAKEFOURCC('V', 'P', '6', 'F'):
				case MAKEFOURCC('F', 'L', 'V', '4'):
					return CODEC_ID_VP6F;	
				case MAKEFOURCC('C', 'A', 'V', 'S'):
					return CODEC_ID_CAVS;	
				case MAKEFOURCC('M', 'P', 'G', '1'):
				case MAKEFOURCC('M', 'P', 'E', 'G'):
					return CODEC_ID_MPEG2VIDEO;	// not a typo
				case MAKEFOURCC('M', 'P', 'G', '2'):
				case MAKEFOURCC('E', 'M', '2', 'V'):
				case MAKEFOURCC('M', 'M', 'E', 'S'):
					return CODEC_ID_MPEG2VIDEO;
				case MAKEFOURCC('H', '2', '6', '3'):
				case MAKEFOURCC('S', '2', '6', '3'):
				case MAKEFOURCC('L', '2', '6', '3'):
				case MAKEFOURCC('M', '2', '6', '3'):
				case MAKEFOURCC('U', '2', '6', '3'):
				case MAKEFOURCC('X', '2', '6', '3'):
					return CODEC_ID_H263;
				case MAKEFOURCC('H', '2', '6', '4'):
				case MAKEFOURCC('X', '2', '6', '4'):
				case MAKEFOURCC('V', 'S', 'S', 'H'):
				case MAKEFOURCC('D', 'A', 'V', 'C'):
				case MAKEFOURCC('P', 'A', 'V', 'C'):
				case MAKEFOURCC('A', 'V', 'C', '1'):
					return CODEC_ID_H264;
				case MAKEFOURCC('M', 'J', 'P', 'G'):
				case MAKEFOURCC('L', 'J', 'P', 'G'):
				case MAKEFOURCC('M', 'J', 'L', 'S'):
				case MAKEFOURCC('J', 'P', 'E', 'G'): // questionable fourcc?
				case MAKEFOURCC('A', 'V', 'R', 'N'):
				case MAKEFOURCC('M', 'J', 'P', 'A'):
					return CODEC_ID_MJPEG;
				case MAKEFOURCC('D', 'V', 'S', 'D'):
				case MAKEFOURCC('D', 'V', '2', '5'):
				case MAKEFOURCC('D', 'V', '5', '0'):
				case MAKEFOURCC('C', 'D', 'V', 'C'):
				case MAKEFOURCC('C', 'D', 'V', '5'):
				case MAKEFOURCC('D', 'V', 'I', 'S'):
				case MAKEFOURCC('P', 'D', 'V', 'C'):
					return CODEC_ID_DVVIDEO;
				case MAKEFOURCC('H', 'F', 'Y', 'U'):
				case MAKEFOURCC('F', 'F', 'V', 'H'):
					return CODEC_ID_HUFFYUV;
				case MAKEFOURCC('C', 'Y', 'U', 'V'):
					return CODEC_ID_CYUV;
				case MAKEFOURCC('A', 'S', 'V', '1'):
					return CODEC_ID_ASV1;
				case MAKEFOURCC('A', 'S', 'V', '2'):
					return CODEC_ID_ASV2;
				case MAKEFOURCC('V', 'C', 'R', '1'):
					return CODEC_ID_VCR1;
				case MAKEFOURCC('T', 'H', 'E', 'O'):
					return CODEC_ID_THEORA;
				case MAKEFOURCC('S', 'V', 'Q', '1'):
					return CODEC_ID_SVQ1;
				case MAKEFOURCC('S', 'V', 'Q', '3'):
					return CODEC_ID_SVQ3;
				case MAKEFOURCC('R', 'P', 'Z', 'A'):
					return CODEC_ID_RPZA;
				case MAKEFOURCC('F', 'F', 'V', '1'):
					return CODEC_ID_FFV1;
				case MAKEFOURCC('V', 'P', '3', '1'):
					return CODEC_ID_VP3;
				case MAKEFOURCC('R', 'L', 'E', '8'):
					return CODEC_ID_MSRLE;
				case MAKEFOURCC('M', 'S', 'Z', 'H'):
					return CODEC_ID_MSZH;
				case MAKEFOURCC('Z', 'L', 'I', 'B'):
					return CODEC_ID_FLV1;
				case MAKEFOURCC('F', 'L', 'V', '1'):
					return CODEC_ID_ZLIB;
/*
				case MAKEFOURCC('P', 'N', 'G', '1'):
					return CODEC_ID_COREPNG;
*/
				case MAKEFOURCC('M', 'P', 'N', 'G'):
					return CODEC_ID_PNG;
/*
				case MAKEFOURCC('A', 'V', 'I', 'S'):
					return CODEC_ID_AVISYNTH;
*/
				case MAKEFOURCC('C', 'R', 'A', 'M'):
					return CODEC_ID_MSVIDEO1;
				case MAKEFOURCC('R', 'T', '2', '1'):
					return CODEC_ID_INDEO2;
				case MAKEFOURCC('I', 'V', '3', '2'):
				case MAKEFOURCC('I', 'V', '3', '1'):
					return CODEC_ID_INDEO3;
				case MAKEFOURCC('C', 'V', 'I', 'D'):
					return CODEC_ID_CINEPAK;
				case MAKEFOURCC('R', 'V', '1', '0'):
					return CODEC_ID_RV10;
				case MAKEFOURCC('R', 'V', '2', '0'):
					return CODEC_ID_RV20;
				case MAKEFOURCC('8', 'B', 'P', 'S'):
					return CODEC_ID_8BPS;
				case MAKEFOURCC('Q', 'R', 'L', 'E'):
					return CODEC_ID_QTRLE;
				case MAKEFOURCC('D', 'U', 'C', 'K'):
					return CODEC_ID_TRUEMOTION1;
				case MAKEFOURCC('T', 'M', '2', '0'):
					return CODEC_ID_TRUEMOTION2;
				case MAKEFOURCC('T', 'S', 'C', 'C'):
					return CODEC_ID_TSCC;
				case MAKEFOURCC('S', 'N', 'O', 'W'):
					return CODEC_ID_SNOW;
				case MAKEFOURCC('Q', 'P', 'E', 'G'):
				case MAKEFOURCC('Q', '1', '_', '0'):
				case MAKEFOURCC('Q', '1', '_', '1'):
					return CODEC_ID_QPEG;
				case MAKEFOURCC('H', '2', '6', '1'):
				case MAKEFOURCC('M', '2', '6', '1'):
					return CODEC_ID_H261;
				case MAKEFOURCC('L', 'O', 'C', 'O'):
					return CODEC_ID_LOCO;
				case MAKEFOURCC('W', 'N', 'V', '1'):
					return CODEC_ID_WNV1;
				case MAKEFOURCC('C', 'S', 'C', 'D'):
					return CODEC_ID_CSCD;
				case MAKEFOURCC('Z', 'M', 'B', 'V'):
					return CODEC_ID_ZMBV;
				case MAKEFOURCC('U', 'L', 'T', 'I'):
					return CODEC_ID_ULTI;
				case MAKEFOURCC('V', 'I', 'X', 'L'):
					return CODEC_ID_VIXL;
				case MAKEFOURCC('A', 'A', 'S', 'C'):
					return CODEC_ID_AASC;
				case MAKEFOURCC('F', 'P', 'S', '1'):
					return CODEC_ID_FRAPS;
				default:
					return CODEC_ID_NONE;
			}
		} else if (!strcmp(Codec, "V_MPEG4/ISO/AVC"))
			return CODEC_ID_H264;
		else if (!strcmp(Codec, "V_MPEG4/ISO/ASP"))
			return CODEC_ID_MPEG4;
		else if (!strcmp(Codec, "V_MPEG2"))
			return CODEC_ID_MPEG2VIDEO;
		else if (!strcmp(Codec, "V_MPEG1"))
			return CODEC_ID_MPEG2VIDEO; // still not a typo
		else if (!strcmp(Codec, "V_SNOW"))
			return CODEC_ID_SNOW;
		else if (!strcmp(Codec, "V_THEORA"))
			return CODEC_ID_THEORA;
		else if (!strncmp(Codec, "V_REAL/RV", 9)) {
			switch (Codec[9]) {
				case '1': 
					return CODEC_ID_RV10;
				case '2': 
					return CODEC_ID_RV20;
				case '3': 
					return CODEC_ID_RV30;
				case '4': 
					return CODEC_ID_RV40;
				default:
					return CODEC_ID_NONE;
			}
		} else
			return CODEC_ID_NONE;
	}
public:
	FFMKVSource(const char *Source, int Track, IScriptEnvironment* Env) {
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

		ST.fp = fopen(Source ,"rb");
		if (ST.fp == NULL)
			Env->ThrowError("Can't open '%s': %s\n", Source, strerror(errno));

		setvbuf(ST.fp, NULL, _IOFBF, CACHESIZE);

		MF = mkv_OpenEx(&ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
		if (MF == NULL) {
			fclose(ST.fp);
			Env->ThrowError("Can't parse Matroska file: %s\n", ErrorMessage);
		}

		if (Track < 0)
			for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++)
				if (mkv_GetTrackInfo(MF, i)->Type == TT_VIDEO) {
					Track = i;
					break;
				}

		if (Track < 0)
			Env->ThrowError("No video track found");

		if ((unsigned)Track >= mkv_GetNumTracks(MF))
			Env->ThrowError("Invalid track number: %d\n", Track);

		TrackInfo *TI = mkv_GetTrackInfo(MF, Track);

		if (TI->Type != TT_VIDEO)
			Env->ThrowError("Selected track is not video");

		mkv_SetTrackMask(MF, ~(1 << Track));

		if (TI->CompEnabled) {
			CS = cs_Create(MF, Track, ErrorMessage, sizeof(ErrorMessage));
			if (CS == NULL)
				Env->ThrowError("Can't create decompressor: %s\n", ErrorMessage);
		}

		avcodec_get_context_defaults(&CodecContext);
		CodecContext.extradata = (uint8_t *)TI->CodecPrivate;
		CodecContext.extradata_size = TI->CodecPrivateSize;

		Codec = avcodec_find_decoder(MatroskaToFFCodecID(TI));
		if(Codec == NULL)
			Env->ThrowError("Codec not found");

		if(avcodec_open(&CodecContext, Codec) < 0)
			Env->ThrowError("Could not open codec");

		VI.image_type = VideoInfo::IT_TFF;
		VI.width = TI->AV.Video.PixelWidth;
		VI.height = TI->AV.Video.PixelHeight;
		VI.fps_denominator = 1;
		VI.fps_numerator = 30;

		SetOutputFormat(CodecContext.pix_fmt, Env);	

		unsigned TrackNumber, FrameSize, FrameFlags;
		ulonglong StartTime, EndTime, FilePos;

		while (mkv_ReadFrame(MF, 0, &TrackNumber, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
			FFMKVFrameInfo FI;
			FI.DTS = StartTime;
			FI.KeyFrame = (FrameFlags & FRAME_KF) != 0;

			FrameToDTS.push_back(FI);
			DTSToFrame[StartTime] = VI.num_frames;
			VI.num_frames++;
		}

		Frame = avcodec_alloc_frame();

		mkv_Seek(MF, FrameToDTS[0].DTS, MKVF_SEEK_TO_PREV_KEYFRAME);
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

int FFMKVSource::ReadNextFrame(AVFrame *Frame, ulonglong *StartTime, IScriptEnvironment* Env) {
	unsigned TrackNumber, FrameFlags, FrameSize;
	ulonglong EndTime, FilePos, StartTime2;
	*StartTime = -1;
	int FrameFinished = 0;
	int Ret = -1;

	while (mkv_ReadFrame(MF, 0, &TrackNumber, &StartTime2, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
		if ((longlong)*StartTime < 0)
			*StartTime = StartTime2;
		if (CS) {
			char CSBuffer[1024];

			cs_NextFrame(CS, FilePos, FrameSize);
			for (;;) {
				int ReadBytes = cs_ReadData(CS, CSBuffer, sizeof(CSBuffer));
				if (ReadBytes < 0)
					Env->ThrowError("Error decompressing data: %s\n", cs_GetLastError(CS));
				if (ReadBytes == 0)
					break;
				Ret = avcodec_decode_video(&CodecContext, Frame, &FrameFinished, (uint8_t *)CSBuffer, ReadBytes);
				if (FrameFinished)
					goto Done;
			}
		} else {
			size_t ReadBytes;

			if (fseek(ST.fp, FilePos, SEEK_SET))
				Env->ThrowError("fseek(): %s\n", strerror(errno));

			if (BufferSize < FrameSize) {
				BufferSize = FrameSize;
				Buffer = realloc(Buffer, BufferSize);
				if (Buffer == NULL) 
					Env->ThrowError("Out of memory\n");
			}

			ReadBytes = fread(Buffer, 1, FrameSize, ST.fp);
			if (ReadBytes != FrameSize) {
				if (ReadBytes == 0) {
					if (feof(ST.fp))
						fprintf(stderr, "Unexpected EOF while reading frame\n");
					else
						fprintf(stderr, "Error reading frame: %s\n", strerror(errno));
				} else
					fprintf(stderr,"Short read while reading frame\n");
				goto Done;
			}

			Ret = avcodec_decode_video(&CodecContext, Frame, &FrameFinished, (uint8_t *)Buffer, FrameSize);

			if (FrameFinished)
				goto Done;
		}
	}

Done:
	return Ret;
}

PVideoFrame __stdcall FFMKVSource::GetFrame(int n, IScriptEnvironment* Env) {
	bool HasSeeked = false;
	bool HasBiggerKF = false;

	for (int i = CurrentFrame + 1; i <= n; i++)
		if (FrameToDTS[i].KeyFrame) {
			HasBiggerKF = true;
			break;
		}	

	if (n < CurrentFrame || HasBiggerKF) {
		mkv_Seek(MF, FrameToDTS[n].DTS, MKVF_SEEK_TO_PREV_KEYFRAME);
		avcodec_flush_buffers(&CodecContext);
		HasSeeked = true;
	}

	do {
		ulonglong StartTime;
		int Ret = ReadNextFrame(Frame, &StartTime, Env);

		if (HasSeeked) {
			CurrentFrame = DTSToFrame[StartTime];
			HasSeeked = false;
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
	std::vector<int64_t> FrameToDTS;
	std::map<int64_t, int> DTSToFrame;
	bool ForceSeek;

	int ReadNextFrame(AVFrame *Frame, int64_t *DTS);
public:
	FFmpegSource(const char *Source, int _Track, bool _ForceSeek, IScriptEnvironment* Env) : Track(_Track), ForceSeek(_ForceSeek) {
		CurrentFrame = 0;

		if(av_open_input_file(&FormatContext, Source, NULL, 0, NULL) != 0)
			Env->ThrowError("Couldn't open \"%s\"", Source);

		if(av_find_stream_info(FormatContext) < 0)
			Env->ThrowError("Couldn't find stream information");

		if (Track >= (int)FormatContext->nb_streams)
			Env->ThrowError("Invalid track number");

		if (Track < 0)
			for(unsigned int i = 0; i < FormatContext->nb_streams; i++)
				if(FormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
					Track = i;
					break;
				}

		if(Track < -1)
			Env->ThrowError("Couldn't find a video stream");

		if (FormatContext->streams[Track]->codec->codec_type != CODEC_TYPE_VIDEO)
			Env->ThrowError("Selected stream doesn't contain video");

		CodecContext = FormatContext->streams[Track]->codec;

		Codec = avcodec_find_decoder(CodecContext->codec_id);
		if(Codec == NULL)
			Env->ThrowError("Codec not found");

		if(avcodec_open(CodecContext, Codec) < 0)
			Env->ThrowError("Could not open codec");

		VI.image_type = VideoInfo::IT_TFF;
		VI.width = CodecContext->width;
		VI.height = CodecContext->height;
		VI.fps_denominator = CodecContext->time_base.num * 1000;
		VI.fps_numerator = CodecContext->time_base.den;

		// sanity check framerate
		if (VI.fps_numerator < VI.fps_denominator || CodecContext->time_base.num <= 0 || CodecContext->time_base.den <= 0) {
			VI.fps_denominator = 1;
			VI.fps_numerator = 30;
		}

		SetOutputFormat(CodecContext->pix_fmt, Env);

		AVPacket Packet;

		while (av_read_frame(FormatContext, &Packet) >= 0) {
			if (Packet.stream_index == Track) {
				FrameToDTS.push_back(Packet.dts);
				DTSToFrame[Packet.dts] = VI.num_frames;
				VI.num_frames++;
			}
			av_free_packet(&Packet);
		}

		Frame = avcodec_alloc_frame();

		av_seek_frame(FormatContext, Track, 0, AVSEEK_FLAG_BACKWARD);
	}

	~FFmpegSource() {
		av_free(Frame);
		avcodec_close(CodecContext);
		av_close_input_file(FormatContext);
	}

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* Env);
};

int FFmpegSource::ReadNextFrame(AVFrame *Frame, int64_t *DTS) {
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

	int IndexPosition = av_index_search_timestamp(FormatContext->streams[Track], FrameToDTS[n], AVSEEK_FLAG_BACKWARD);
	int64_t NearestIndexDTS = -1;
	if (IndexPosition >= 0)	
		NearestIndexDTS = FormatContext->streams[Track]->index_entries[IndexPosition].timestamp;

	if (n < CurrentFrame || NearestIndexDTS > FrameToDTS[CurrentFrame] || (ForceSeek && IndexPosition == -1 && n > CurrentFrame + 10)) {
		av_seek_frame(FormatContext, Track, FrameToDTS[n], AVSEEK_FLAG_BACKWARD);
		avcodec_flush_buffers(CodecContext);
		HasSeeked = true;
	}

	do {
		int64_t DTS;
		int Ret = ReadNextFrame(Frame, &DTS);

		if (HasSeeked) {
			CurrentFrame = DTSToFrame[DTS];
			HasSeeked = DTS < 0;
		}

		CurrentFrame++;
	} while (CurrentFrame <= n || HasSeeked);

	return OutputFrame(Frame, Env);
}

AVSValue __cdecl CreateFFmpegSource(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
	if (!Args[0].Defined())
    	Env->ThrowError("No source specified");

	av_register_all();

	AVFormatContext *FormatContext;

	if (av_open_input_file(&FormatContext, Args[0].AsString(), NULL, 0, NULL) != 0)
		Env->ThrowError("Couldn't open \"%s\"", Args[0].AsString());

	bool IsMatroska = !strcmp(FormatContext->iformat->name, "matroska");

	av_close_input_file(FormatContext);

	if (IsMatroska)
		return new FFMKVSource(Args[0].AsString(), Args[1].AsInt(-1), Env);
	else
		return new FFmpegSource(Args[0].AsString(), Args[1].AsInt(-1), Args[2].AsBool(false), Env);
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* Env) {
    Env->AddFunction("FFmpegSource", "[source]s[track]i[forceseek]b", CreateFFmpegSource, 0);
    return "FFmpegSource";
};

