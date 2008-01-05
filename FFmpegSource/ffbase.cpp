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

int FFBase::FrameFromDTS(int64_t ADTS) {
	for (int i = 0; i < (int)Frames.size(); i++)
		if (Frames[i].DTS == ADTS)
			return i;
	return -1;
}

int FFBase::ClosestFrameFromDTS(int64_t ADTS) {
	int Frame = 0; 
	int64_t BestDiff = 0xFFFFFFFFFFFFFFLL;
	for (int i = 0; i < (int)Frames.size(); i++) {
		int64_t CurrentDiff = FFABS(Frames[i].DTS - ADTS);
		if (CurrentDiff < BestDiff) {
			BestDiff = CurrentDiff;
			Frame = i;
		}
	}
	return Frame;
}

int FFBase::FindClosestKeyFrame(int AFrame) {
	for (int i = AFrame; i > 0; i--)
		if (Frames[i].KeyFrame)
			return i;
	return 0;
}

bool FFBase::LoadFrameInfoFromFile(const char *AVideoCacheFile, const char *ASource, int AVideoTrack) {
	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffv%dcache", ASource, AVideoTrack);
	if (!strcmp(AVideoCacheFile, ""))
		AVideoCacheFile = DefaultCacheFilename;

	FILE *CacheFile = fopen(AVideoCacheFile, "r");
	if (!CacheFile)
		return false;

	if (fscanf(CacheFile, "%d\r\n", &VI.num_frames) <= 0 || VI.num_frames <= 0) {
		VI.num_frames = 0;
		fclose(CacheFile);
		return false;
	}

	for (int i = 0; i < VI.num_frames; i++) {
		int64_t DTSTemp;
		int KFTemp;
		fscanf(CacheFile, "%lld %d\r\n", &DTSTemp, &KFTemp);
		Frames.push_back(FrameInfo(DTSTemp, KFTemp != 0));
	}

	fclose(CacheFile);
	return true;
}

bool FFBase::SaveFrameInfoToFile(const char *AVideoCacheFile, const char *ASource, int AVideoTrack) {
	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffv%dcache", ASource, AVideoTrack);
	if (!strcmp(AVideoCacheFile, ""))
		AVideoCacheFile = DefaultCacheFilename;

	FILE *CacheFile = fopen(AVideoCacheFile, "wb");
	if (!CacheFile)
		return false;

	fprintf(CacheFile, "%d\r\n", VI.num_frames);
	for (int i = 0; i < VI.num_frames; i++)
		fprintf(CacheFile, "%lld %d\r\n", Frames[i].DTS, (int)(Frames[i].KeyFrame ? 1 : 0));		

	fclose(CacheFile);
	return true;
}

bool FFBase::SaveTimecodesToFile(const char *ATimecodeFile, int64_t ScaleD, int64_t ScaleN) {
	if (!strcmp(ATimecodeFile, ""))
		return true;

	FILE *TimecodeFile = fopen(ATimecodeFile, "wb");
	if (!TimecodeFile)
		return false;

	std::set<int64_t> Timecodes;
	for (int i = 0; i < VI.num_frames; i++)
		Timecodes.insert(Frames[i].DTS);

	fprintf(TimecodeFile, "# timecode format v2\r\n");

	for (std::set<int64_t>::iterator Cur=Timecodes.begin(); Cur!=Timecodes.end(); Cur++)
		fprintf(TimecodeFile, "%f\r\n", (*Cur * ScaleD) / (double)ScaleN);

	fclose(TimecodeFile);
	return true;
}

#ifdef FLAC_CACHE
static FLAC__StreamDecoderReadStatus FLACStreamDecoderReadCallback(const FLAC__StreamDecoder *ADecoder, FLAC__byte ABuffer[], size_t *ABytes, FFBase *AOwner) {
	if(*ABytes > 0) {
		*ABytes = fread(ABuffer, sizeof(FLAC__byte), *ABytes, AOwner->FCFile);
		if(ferror(AOwner->FCFile))
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
		else if(*ABytes == 0)
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		else
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	} else {
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}
}

static FLAC__StreamDecoderSeekStatus FLACStreamDecoderSeekCallback(const FLAC__StreamDecoder *ADecoder, FLAC__uint64 AAbsoluteByteOffset, FFBase *AOwner) {
	if(_fseeki64(AOwner->FCFile, AAbsoluteByteOffset, SEEK_SET) < 0)
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	else
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus FLACStreamDecoderTellCallback(const FLAC__StreamDecoder *ADecoder, FLAC__uint64 *AAbsoluteByteOffset, FFBase *AOwner) {
	__int64 Pos;
	if ((Pos = _ftelli64(AOwner->FCFile)) < 0) {
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
	} else {
		*AAbsoluteByteOffset = (FLAC__uint64)Pos;
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}
}

static FLAC__StreamDecoderLengthStatus FLACStreamDecoderLengthCallback(const FLAC__StreamDecoder *ADecoder, FLAC__uint64 *AStreamLength, FFBase *AOwner) {
	__int64 OriginalPos;
	__int64 Length;
	if ((OriginalPos = _ftelli64(AOwner->FCFile)) < 0)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	_fseeki64(AOwner->FCFile, 0, SEEK_END);
	if ((Length = _ftelli64(AOwner->FCFile)) < 0)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	_fseeki64(AOwner->FCFile, OriginalPos, SEEK_SET);
	*AStreamLength = Length;
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool FLACStreamDecoderEofCallback(const FLAC__StreamDecoder *ADecoder, FFBase *AOwner) {
	return feof(AOwner->FCFile) ? true : false;
}

static FLAC__StreamDecoderWriteStatus FLACStreamDecoderWriteCallback(const FLAC__StreamDecoder *ADecoder, const FLAC__Frame *AFrame, const FLAC__int32 *const ABuffer[], FFBase *AOwner) {
	unsigned Blocksize = AFrame->header.blocksize;
	const VideoInfo VI = AOwner->GetVideoInfo();
	int16_t *Buffer = (int16_t *)AOwner->FCBuffer;

	int j = 0;
	while (AOwner->FCCount > 0 && Blocksize > 0) {
		for (int i = 0; i < VI.nchannels; i++)
			*Buffer++ = ABuffer[i][j];
		j++;
		AOwner->FCCount--;
		Blocksize--;
	}

	AOwner->FCBuffer = Buffer;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void FLACStreamDecoderMetadataCallback(const FLAC__StreamDecoder *ADecoder, const FLAC__StreamMetadata *AMetadata, FFBase *AOwner) {
	AOwner->FCError = (AMetadata->data.stream_info.total_samples <= 0);
}

static void FLACStreamDecoderErrorCallback(const FLAC__StreamDecoder *ADecoder, FLAC__StreamDecoderErrorStatus AStatus, FFBase *AOwner) {
	AOwner->FCError = true;
}
#endif // FLAC_CACHE

bool FFBase::OpenAudioCache(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, IScriptEnvironment *Env) {
	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffa%dcache", ASource, AAudioTrack);
	if (!strcmp(AAudioCacheFile, ""))
		AAudioCacheFile = DefaultCacheFilename;

	// Is an empty file?
	FCFile = fopen(AAudioCacheFile, "rb");
	int64_t CacheSize;
	if (FCFile) {
		_fseeki64(FCFile, 0, SEEK_END);
		CacheSize = _ftelli64(FCFile);
		_fseeki64(FCFile, 0, SEEK_SET);
		if (CacheSize <= 0) {
			fclose(FCFile);
			FCFile = NULL;
			return false;
		}
	} else {
		return false;
	}

#ifdef FLAC_CACHE
	// is FLAC?
	FLACAudioCache = FLAC__stream_decoder_new();
	if (FLAC__stream_decoder_init_stream(FLACAudioCache,
		&(FLAC__StreamDecoderReadCallback)FLACStreamDecoderReadCallback,
		&(FLAC__StreamDecoderSeekCallback)FLACStreamDecoderSeekCallback,
		&(FLAC__StreamDecoderTellCallback)FLACStreamDecoderTellCallback,
		&(FLAC__StreamDecoderLengthCallback)FLACStreamDecoderLengthCallback,
		&(FLAC__StreamDecoderEofCallback)FLACStreamDecoderEofCallback,
		&(FLAC__StreamDecoderWriteCallback)FLACStreamDecoderWriteCallback,
		&(FLAC__StreamDecoderMetadataCallback)FLACStreamDecoderMetadataCallback, &(FLAC__StreamDecoderErrorCallback)FLACStreamDecoderErrorCallback, this) == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		FCError = true;
		FLAC__stream_decoder_process_until_end_of_metadata(FLACAudioCache);
		if (!FCError) {
			VI.num_audio_samples = FLAC__stream_decoder_get_total_samples(FLACAudioCache);
			AudioCacheType = acFLAC;
			return true;
		}
	}
	FLAC__stream_decoder_delete(FLACAudioCache);
	FLACAudioCache = NULL;
#endif // FLAC_CACHE

	// Raw audio
	VI.num_audio_samples = VI.AudioSamplesFromBytes(CacheSize);
	AudioCacheType = acRaw;
	RawAudioCache = FCFile;
	FCFile = NULL;
	return true;
}

#ifdef FLAC_CACHE
static FLAC__StreamEncoderWriteStatus FLACStreamEncoderWriteCallback(const FLAC__StreamEncoder *ASncoder, const FLAC__byte ABuffer[], size_t ABytes, unsigned ABamples, unsigned ACurrentFrame, FFBase *AOwner) {
	fwrite(ABuffer, sizeof(FLAC__byte), ABytes, AOwner->FCFile);
	if(ferror(AOwner->FCFile))
		return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
	else
		return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

static FLAC__StreamEncoderSeekStatus FLACStreamEncoderSeekCallback(const FLAC__StreamEncoder *AEncoder, FLAC__uint64 AAbsoluteByteOffset, FFBase *AOwner) {
	if(_fseeki64(AOwner->FCFile, AAbsoluteByteOffset, SEEK_SET) < 0)
		return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
	else
		return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
}

static FLAC__StreamEncoderTellStatus FLACStreamEncoderTellCallback(const FLAC__StreamEncoder *AEncoder, FLAC__uint64 *AAbsoluteByteOffset, FFBase *AOwner) {
	__int64 Pos;
	if((Pos = _ftelli64(AOwner->FCFile)) < 0) {
		return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
	} else {
		*AAbsoluteByteOffset = (FLAC__uint64)Pos;
		return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
	}
}

FLAC__StreamEncoder *FFBase::NewFLACCacheWriter(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, int ACompression, IScriptEnvironment *Env) {
	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffa%dcache", ASource, AAudioTrack);
	if (!strcmp(AAudioCacheFile, ""))
		AAudioCacheFile = DefaultCacheFilename;

	FCFile = fopen(AAudioCacheFile, "wb");
	if (!FCFile)
		Env->ThrowError("FFmpegSource: Failed to open '%s' for writing", AAudioCacheFile);
	FLAC__StreamEncoder *FSE;
	FSE = FLAC__stream_encoder_new();
	FLAC__stream_encoder_set_channels(FSE, VI.nchannels);
	FLAC__stream_encoder_set_bits_per_sample(FSE, VI.BytesPerChannelSample() * 8);
	FLAC__stream_encoder_set_sample_rate(FSE, VI.audio_samples_per_second);
	FLAC__stream_encoder_set_compression_level(FSE, ACompression);
	if (FLAC__stream_encoder_init_stream(FSE, &(FLAC__StreamEncoderWriteCallback)FLACStreamEncoderWriteCallback,
		&(FLAC__StreamEncoderSeekCallback)FLACStreamEncoderSeekCallback,
		&(FLAC__StreamEncoderTellCallback)FLACStreamEncoderTellCallback, NULL, this)
		!= FLAC__STREAM_ENCODER_INIT_STATUS_OK)
		Env->ThrowError("FFmpegSource: Failed to initialize the FLAC encoder for '%s'", AAudioCacheFile);

	return FSE;
}

void FFBase::CloseFLACCacheWriter(FLAC__StreamEncoder *AFSE) {
	FLAC__stream_encoder_finish(AFSE);
	FLAC__stream_encoder_delete(AFSE);
	fclose(FCFile);
	FCFile = NULL;
}
#endif // FLAC_CACHE

FILE *FFBase::NewRawCacheWriter(const char *AAudioCacheFile, const char *ASource, int AAudioTrack, IScriptEnvironment *Env) {
	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffa%dcache", ASource, AAudioTrack);
	if (!strcmp(AAudioCacheFile, ""))
		AAudioCacheFile = DefaultCacheFilename;

	FILE *RCF = fopen(AAudioCacheFile, "wb");
	if (RCF == NULL)
		Env->ThrowError("FFmpegSource: Failed to open '%s' for writing", AAudioCacheFile);
	return RCF;
}

void FFBase::CloseRawCacheWriter(FILE *ARawCache) {
	fclose(ARawCache);
}

void FFBase::InitPP(int AWidth, int AHeight, const char *APPString, int AQuality, int APixelFormat, IScriptEnvironment *Env) {
	if (!strcmp(APPString, ""))
		return;

	if (AQuality < 0 || AQuality > PP_QUALITY_MAX)
		Env->ThrowError("FFmpegSource: Quality is out of range");

	// Unsafe?
	PPMode = pp_get_mode_by_name_and_quality((char *)APPString, AQuality);
	if (!PPMode)
		Env->ThrowError("FFmpegSource: Invalid postprocesing settings");

	int Flags = GetPPCPUFlags(Env);

	switch (APixelFormat) {
		case PIX_FMT_YUV420P: Flags |= PP_FORMAT_420; break;
		case PIX_FMT_YUV422P: Flags |= PP_FORMAT_422; break;
		case PIX_FMT_YUV411P: Flags |= PP_FORMAT_411; break;
		case PIX_FMT_YUV444P: Flags |= PP_FORMAT_444; break;
		default:
			Env->ThrowError("FFmpegSource: Input format is not supported for postprocessing");
	}

	PPContext = pp_get_context(VI.width, VI.height, Flags);

	if (avpicture_alloc(&PPPicture, APixelFormat, AWidth, AHeight) < 0)
			Env->ThrowError("FFmpegSource: Failed to allocate picture");
}

void FFBase::SetOutputFormat(int ACurrentFormat, IScriptEnvironment *Env) {
	int Loss;
	int BestFormat = avcodec_find_best_pix_fmt((1 << PIX_FMT_YUVJ420P) | (1 << PIX_FMT_YUV420P) | (1 << PIX_FMT_YUYV422) | (1 << PIX_FMT_RGB32) | (1 << PIX_FMT_BGR24), ACurrentFormat, 1 /* Required to prevent pointless RGB32 => RGB24 conversion */, &Loss);

	switch (BestFormat) {
		case PIX_FMT_YUVJ420P: // stupid yv12 distinctions, also inexplicably completely undeniably incompatible with all other supported output formats
		case PIX_FMT_YUV420P: VI.pixel_type = VideoInfo::CS_I420; break;
		case PIX_FMT_YUYV422: VI.pixel_type = VideoInfo::CS_YUY2; break;
		case PIX_FMT_RGB32: VI.pixel_type = VideoInfo::CS_BGR32; break;
		case PIX_FMT_BGR24: VI.pixel_type = VideoInfo::CS_BGR24; break;
		default:
			Env->ThrowError("FFmpegSource: No suitable output format found");
	}

	if (BestFormat != ACurrentFormat) {
		ConvertToFormat = BestFormat;
		SWS = sws_getContext(VI.width, VI.height, ACurrentFormat, VI.width, VI.height, ConvertToFormat, GetSWSCPUFlags(Env) | SWS_BICUBIC, NULL, NULL, NULL);
	}

	if (BestFormat == PIX_FMT_YUVJ420P || BestFormat == PIX_FMT_YUV420P) {
		VI.height -= VI.height & 1;
		VI.width -= VI.width & 1;
	}

	if (BestFormat == PIX_FMT_YUYV422) {
		VI.width -= VI.width & 1;
	}
}

PVideoFrame FFBase::OutputFrame(AVFrame *AFrame, IScriptEnvironment *Env) {
	AVPicture *SrcPicture = (AVPicture *)AFrame;

	if (PPContext) {
		pp_postprocess(AFrame->data, AFrame->linesize, PPPicture.data, PPPicture.linesize, VI.width, VI.height, AFrame->qscale_table, AFrame->qstride, PPMode, PPContext, AFrame->pict_type | (AFrame->qscale_type ? PP_PICT_TYPE_QP2 : 0));
		SrcPicture = &PPPicture;
	}

	PVideoFrame Dst = Env->NewVideoFrame(VI);

	if (ConvertToFormat != PIX_FMT_NONE && VI.pixel_type == VideoInfo::CS_I420) {
		uint8_t *DstData[3] = {Dst->GetWritePtr(PLANAR_Y), Dst->GetWritePtr(PLANAR_U), Dst->GetWritePtr(PLANAR_V)};
		int DstStride[3] = {Dst->GetPitch(PLANAR_Y), Dst->GetPitch(PLANAR_U), Dst->GetPitch(PLANAR_V)};
		sws_scale(SWS, SrcPicture->data, SrcPicture->linesize, 0, VI.height, DstData, DstStride);
	} else if (ConvertToFormat != PIX_FMT_NONE) {
		if (VI.IsRGB()) {
			uint8_t *DstData[1] = {Dst->GetWritePtr() + Dst->GetPitch() * (Dst->GetHeight() - 1)};
			int DstStride[1] = {-Dst->GetPitch()};
			sws_scale(SWS, SrcPicture->data, SrcPicture->linesize, 0, VI.height, DstData, DstStride);
		} else {
			uint8_t *DstData[1] = {Dst->GetWritePtr()};
			int DstStride[1] = {Dst->GetPitch()};
			sws_scale(SWS, SrcPicture->data, SrcPicture->linesize, 0, VI.height, DstData, DstStride);
		}
	} else if (VI.pixel_type == VideoInfo::CS_I420) {
		Env->BitBlt(Dst->GetWritePtr(PLANAR_Y), Dst->GetPitch(PLANAR_Y), SrcPicture->data[0], SrcPicture->linesize[0], Dst->GetRowSize(PLANAR_Y), Dst->GetHeight(PLANAR_Y)); 
		Env->BitBlt(Dst->GetWritePtr(PLANAR_U), Dst->GetPitch(PLANAR_U), SrcPicture->data[1], SrcPicture->linesize[1], Dst->GetRowSize(PLANAR_U), Dst->GetHeight(PLANAR_U)); 
		Env->BitBlt(Dst->GetWritePtr(PLANAR_V), Dst->GetPitch(PLANAR_V), SrcPicture->data[2], SrcPicture->linesize[2], Dst->GetRowSize(PLANAR_V), Dst->GetHeight(PLANAR_V)); 
	} else {
		if (VI.IsRGB())
			Env->BitBlt(Dst->GetWritePtr() + Dst->GetPitch() * (Dst->GetHeight() - 1), -Dst->GetPitch(), SrcPicture->data[0], SrcPicture->linesize[0], Dst->GetRowSize(), Dst->GetHeight()); 
		else
			Env->BitBlt(Dst->GetWritePtr(), Dst->GetPitch(), SrcPicture->data[0], SrcPicture->linesize[0], Dst->GetRowSize(), Dst->GetHeight()); 
	}

	return Dst;
}

void FFBase::GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment* Env) {
	if (AudioCacheType == acRaw) {
		_fseeki64(RawAudioCache, VI.BytesFromAudioSamples(Start), SEEK_SET);
		fread(Buf, 1, (size_t)VI.BytesFromAudioSamples(Count), RawAudioCache);
#ifdef FLAC_CACHE
	} else if (AudioCacheType == acFLAC) {
		FCCount = Count;
		FCBuffer = Buf;
		FLAC__stream_decoder_seek_absolute(FLACAudioCache, Start);
		while (FCCount > 0)
			FLAC__stream_decoder_process_single(FLACAudioCache);
#endif // FLAC_CACHE
	} else {
		Env->ThrowError("FFmpegSource: Audio requested but none available");
	}
}

FFBase::FFBase() {
	memset(&VI, 0, sizeof(VI));
	AudioCacheType = acNone;
	FCError = false;
	RawAudioCache = NULL;
	PPContext = NULL;
	PPMode = NULL;
	SWS = NULL;
	LastFrameNum = -1;
	DecodingBuffer = new uint8_t[AVCODEC_MAX_AUDIO_FRAME_SIZE];
#ifdef FLAC_CACHE
	FLACAudioCache = NULL;
	FLACBuffer = new FLAC__int32[AVCODEC_MAX_AUDIO_FRAME_SIZE];
#endif // FLAC_CACHE
	FCFile = NULL;
	ConvertToFormat = PIX_FMT_NONE;
	memset(&PPPicture, 0, sizeof(PPPicture));
	DecodeFrame = avcodec_alloc_frame();
}

FFBase::~FFBase() {
	delete [] DecodingBuffer;
	if (RawAudioCache)
		fclose(RawAudioCache);
#ifdef FLAC_CACHE
	delete [] FLACBuffer;
	if (FLACAudioCache) {
		FLAC__stream_decoder_finish(FLACAudioCache);
		FLAC__stream_decoder_delete(FLACAudioCache);
	}
#endif // FLAC_CACHE
	if (FCFile)
		fclose(FCFile);
	if (SWS)
		sws_freeContext(SWS);
	if (PPMode)
		pp_free_mode(PPMode);
	if (PPContext)
		pp_free_context(PPContext);
	avpicture_free(&PPPicture);
	av_free(DecodeFrame);
}
