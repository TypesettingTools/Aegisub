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

#include "ffavisynth.h"
#include "utils.h"

AvisynthVideoSource::AvisynthVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, int FPSNum, int FPSDen, const char *PP, int Threads, int SeekMode, IScriptEnvironment* Env, char *ErrorMsg, unsigned MsgSize) {
	memset(&VI, 0, sizeof(VI));
	SWS = NULL;
	ConvertToFormat = PIX_FMT_NONE;
	this->FPSNum = FPSNum;
	this->FPSDen = FPSDen;

	VS = FFMS_CreateVideoSource(SourceFile, Track, TrackIndices, PP, Threads, SeekMode, ErrorMsg, MsgSize);
	if (!VS)
		Env->ThrowError(ErrorMsg);

	const VideoProperties VP = *FFMS_GetVideoProperties(VS);

	VI.image_type = VideoInfo::IT_TFF;
	VI.width = VP.Width;
	VI.height = VP.Height;

	if (FPSNum > 0 && FPSDen > 0) {
		VI.fps_denominator = FPSDen;
		VI.fps_numerator = FPSNum;
		VI.num_frames = ceil(((VP.LastTime - VP.FirstTime) * FPSNum) / FPSDen);
	} else {
		VI.fps_denominator = VP.FPSDenominator;
		VI.fps_numerator = VP.FPSNumerator;
		VI.num_frames = VP.NumFrames;
	}

	try {
		InitOutputFormat(VP.VPixelFormat, Env);
	} catch (AvisynthError &) {
		FFMS_DestroyVideoSource(VS);
		throw;
	}

	// Set AR variables
	Env->SetVar("FFSAR_NUM", VP.SARNum);
	Env->SetVar("FFSAR_DEN", VP.SARDen);
	if (VP.SARNum > 0 && VP.SARDen > 0)
		Env->SetVar("FFSAR", VP.SARNum / (double)VP.SARDen);

	// Set crop variables
	Env->SetVar("FFCROP_LEFT", VP.CropLeft);
	Env->SetVar("FFCROP_RIGHT", VP.CropRight);
	Env->SetVar("FFCROP_TOP", VP.CropTop);
	Env->SetVar("FFCROP_BOTTOM", VP.CropBottom);
}

AvisynthVideoSource::~AvisynthVideoSource() {
	if (SWS)
		sws_freeContext(SWS);
	FFMS_DestroyVideoSource(VS);
}

void AvisynthVideoSource::InitOutputFormat(PixelFormat CurrentFormat, IScriptEnvironment *Env) {
	int Loss;
	PixelFormat BestFormat = avcodec_find_best_pix_fmt((1 << PIX_FMT_YUVJ420P) | (1 << PIX_FMT_YUV420P) | (1 << PIX_FMT_YUYV422) | (1 << PIX_FMT_RGB32) | (1 << PIX_FMT_BGR24), CurrentFormat, 1 /* Required to prevent pointless RGB32 => RGB24 conversion */, &Loss);

	switch (BestFormat) {
		case PIX_FMT_YUVJ420P: // stupid yv12 distinctions, also inexplicably completely undeniably incompatible with all other supported output formats
		case PIX_FMT_YUV420P: VI.pixel_type = VideoInfo::CS_I420; break;
		case PIX_FMT_YUYV422: VI.pixel_type = VideoInfo::CS_YUY2; break;
		case PIX_FMT_RGB32: VI.pixel_type = VideoInfo::CS_BGR32; break;
		case PIX_FMT_BGR24: VI.pixel_type = VideoInfo::CS_BGR24; break;
		default:
			Env->ThrowError("FFVideoSource: No suitable output format found");
	}

	if (BestFormat != CurrentFormat) {
		ConvertToFormat = BestFormat;
		SWS = sws_getContext(VI.width, VI.height, CurrentFormat, VI.width, VI.height, ConvertToFormat, GetCPUFlags() | SWS_BICUBIC, NULL, NULL, NULL);
	}

	if (BestFormat == PIX_FMT_YUVJ420P || BestFormat == PIX_FMT_YUV420P) {
		VI.height -= VI.height & 1;
		VI.width -= VI.width & 1;
	}

	if (BestFormat == PIX_FMT_YUYV422) {
		VI.width -= VI.width & 1;
	}
}

PVideoFrame AvisynthVideoSource::OutputFrame(const AVFrameLite *Frame, IScriptEnvironment *Env) {
	// Yes, this function is overly complex and could probably be simplified
	AVPicture *SrcPicture = reinterpret_cast<AVPicture *>(const_cast<AVFrameLite *>(Frame));
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

PVideoFrame AvisynthVideoSource::GetFrame(int n, IScriptEnvironment *Env) {
	char ErrorMsg[1024];
	unsigned MsgSize = sizeof(ErrorMsg);
	const AVFrameLite *Frame;

	if (FPSNum > 0 && FPSDen > 0)
		Frame = FFMS_GetFrameByTime(VS, FFMS_GetVideoProperties(VS)->FirstTime + (double)(n * (int64_t)FPSDen) / FPSNum, ErrorMsg, MsgSize);
	else
		Frame = FFMS_GetFrame(VS, n, ErrorMsg, MsgSize);

	if (Frame == NULL)
		Env->ThrowError("FFVideoSource: %s", ErrorMsg);

	Env->SetVar("FFPICT_TYPE", Frame->PictType);
	return OutputFrame(Frame, Env);
}

AvisynthAudioSource::AvisynthAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, IScriptEnvironment* Env, char *ErrorMsg, unsigned MsgSize) {
	memset(&VI, 0, sizeof(VI));

	AS = FFMS_CreateAudioSource(SourceFile, Track, TrackIndices, ErrorMsg, MsgSize);
	if (!AS)
		Env->ThrowError(ErrorMsg);

	const AudioProperties AP = *FFMS_GetAudioProperties(AS);

	VI.nchannels = AP.Channels;
	VI.num_audio_samples = AP.NumSamples;
	VI.audio_samples_per_second = AP.SampleRate;

	if (AP.Float && AP.BitsPerSample == 32) {
		VI.sample_type = SAMPLE_FLOAT;
	} else {
		switch (AP.BitsPerSample) {
			case 8: VI.sample_type = SAMPLE_INT8; break;
			case 16: VI.sample_type = SAMPLE_INT16; break;
			case 24: VI.sample_type = SAMPLE_INT24; break;
			case 32: VI.sample_type = SAMPLE_INT32; break;
			default: Env->ThrowError("FFAudioSource: Bad audio format");
		}
	}
}

AvisynthAudioSource::~AvisynthAudioSource() {
	FFMS_DestroyAudioSource(AS);
}

void AvisynthAudioSource::GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env) {
	char ErrorMsg[1024];
	unsigned MsgSize = sizeof(ErrorMsg);
	if (FFMS_GetAudio(AS, Buf, Start, Count, ErrorMsg, MsgSize))
		Env->ThrowError(ErrorMsg);
}