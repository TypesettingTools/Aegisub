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

#include "ffvideosource.h"
#include <errno.h>

#ifdef __UNIX__
#define _snprintf snprintf
#endif

int VideoBase::InitPP(const char *PP, PixelFormat PixelFormat, char *ErrorMsg, unsigned MsgSize) {
	if (PP == NULL || !strcmp(PP, ""))
		return 0;

	PPMode = pp_get_mode_by_name_and_quality(PP, PP_QUALITY_MAX);
	if (!PPMode) {
		_snprintf(ErrorMsg, MsgSize, "Invalid postprocesing settings");
		return 1;
	}

	int Flags =  GetCPUFlags();

	switch (PixelFormat) {
		case PIX_FMT_YUV420P: Flags |= PP_FORMAT_420; break;
		case PIX_FMT_YUV422P: Flags |= PP_FORMAT_422; break;
		case PIX_FMT_YUV411P: Flags |= PP_FORMAT_411; break;
		case PIX_FMT_YUV444P: Flags |= PP_FORMAT_444; break;
		default:
			_snprintf(ErrorMsg, MsgSize, "Input format is not supported for postprocessing");
			return 2;
	}

	PPContext = pp_get_context(VP.Width, VP.Height, Flags);

	if (!(PPFrame = avcodec_alloc_frame())) {
		_snprintf(ErrorMsg, MsgSize, "Failed to allocate temporary frame");
		return 3;
	}

	if (avpicture_alloc((AVPicture *)PPFrame, PixelFormat, VP.Width, VP.Height) < 0) {
		av_free(PPFrame);
		PPFrame = NULL;
		_snprintf(ErrorMsg, MsgSize, "Failed to allocate picture");
		return 4;
	}

	FinalFrame = PPFrame;

	return 0;
}

AVFrameLite *VideoBase::OutputFrame(AVFrame *Frame) {
	if (PPContext) {
		pp_postprocess(const_cast<const uint8_t **>(Frame->data), Frame->linesize, PPFrame->data, PPFrame->linesize, VP.Width, VP.Height, Frame->qscale_table, Frame->qstride, PPMode, PPContext, Frame->pict_type | (Frame->qscale_type ? PP_PICT_TYPE_QP2 : 0));
		PPFrame->key_frame = Frame->key_frame;
		PPFrame->pict_type = Frame->pict_type;
	}

	if (SWS) {
		sws_scale(SWS, PPFrame->data, PPFrame->linesize, 0, VP.Height, FinalFrame->data, FinalFrame->linesize);
		FinalFrame->key_frame = PPFrame->key_frame;
		FinalFrame->pict_type = PPFrame->pict_type;
	}

	return reinterpret_cast<AVFrameLite *>(FinalFrame);
}

VideoBase::VideoBase() {
	memset(&VP, 0, sizeof(VP));
	PPContext = NULL;
	PPMode = NULL;
	SWS = NULL;
	LastFrameNum = 0;
	CurrentFrame = 1;
	CodecContext = NULL;
	DecodeFrame = avcodec_alloc_frame();
	PPFrame = DecodeFrame;
	FinalFrame = PPFrame;
}

VideoBase::~VideoBase() {
	if (PPMode)
		pp_free_mode(PPMode);
	if (PPContext)
		pp_free_context(PPContext);
	if (SWS)
		sws_freeContext(SWS);
	if (FinalFrame != PPFrame) {
		avpicture_free((AVPicture *)FinalFrame);
		av_free(FinalFrame);
	}
	if (PPFrame != DecodeFrame) {
		avpicture_free((AVPicture *)PPFrame);
		av_free(PPFrame);
	}
	av_free(DecodeFrame);
}

AVFrameLite *VideoBase::GetFrameByTime(double Time, char *ErrorMsg, unsigned MsgSize) {
	int Frame = Frames.ClosestFrameFromDTS((Time * 1000 * Frames.TB.Den) / Frames.TB.Num);
	return GetFrame(Frame, ErrorMsg, MsgSize);
}

int VideoBase::SetOutputFormat(int TargetFormats, int Width, int Height, char *ErrorMsg, unsigned MsgSize) {
//  FIXME: investigate the possible bug in avcodec_find_best_pix_fmt
//	int Loss;
//	int OutputFormat = avcodec_find_best_pix_fmt(TargetFormats,
//		CodecContext->pix_fmt, 1 /* Required to prevent pointless RGB32 => RGB24 conversion */, &Loss);
//	if (OutputFormat == -1)
//		return -1;

	PixelFormat OutputFormat = static_cast<PixelFormat>(TargetFormats);

	SwsContext *NewSWS = NULL;
	if (CodecContext->pix_fmt != OutputFormat || Width != CodecContext->width || Height != CodecContext->height) {
		NewSWS = sws_getContext(CodecContext->width, CodecContext->height, CodecContext->pix_fmt, Width, Height,
			OutputFormat, GetCPUFlags() | SWS_BICUBIC, NULL, NULL, NULL);
		if (NewSWS == NULL) {
			_snprintf(ErrorMsg, MsgSize, "Failed to allocate SWScale context");
			return 1;
		}
	}

	if (SWS)
		sws_freeContext(SWS);
	SWS = NewSWS;

	VP.Height = Height;
	VP.Width = Width;
	VP.VPixelFormat = OutputFormat;

	// FIXME: In theory the allocations in this part could fail just like in InitPP but whatever
	if (FinalFrame != PPFrame) {
		avpicture_free((AVPicture *)FinalFrame);
		av_free(FinalFrame);
	}

	if (SWS) {
		FinalFrame = avcodec_alloc_frame();
		avpicture_alloc((AVPicture *)FinalFrame, static_cast<PixelFormat>(VP.VPixelFormat), VP.Width, VP.Height);
	} else {
		FinalFrame = PPFrame;
	}

	return 0;
}

void VideoBase::ResetOutputFormat() {
	if (SWS)
		sws_freeContext(SWS);
	SWS = NULL;
	VP.Height = CodecContext->height;
	VP.Width = CodecContext->width;
	VP.VPixelFormat = CodecContext->pix_fmt;
}

void FFVideoSource::Free(bool CloseCodec) {
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_close_input_file(FormatContext);
	//FIXME how was it allocated? how was it deallocate? nobody knows
	//av_free(FormatContext);
}

FFVideoSource::FFVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices,
	const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize) {

	FormatContext = NULL;
	AVCodec *Codec = NULL;
	this->SeekMode = SeekMode;
	VideoTrack = Track;
	Frames = (*TrackIndices)[VideoTrack];

	if (Frames.size() == 0) {
		_snprintf(ErrorMsg, MsgSize, "Video track contains no frames");
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

	if (SeekMode >= 0 && av_seek_frame(FormatContext, VideoTrack, Frames[0].DTS, AVSEEK_FLAG_BACKWARD) < 0) {
		Free(false);
		_snprintf(ErrorMsg, MsgSize, "Video track is unseekable");
		throw ErrorMsg;
	}

	CodecContext = FormatContext->streams[VideoTrack]->codec;
	CodecContext->thread_count = Threads;

	Codec = avcodec_find_decoder(CodecContext->codec_id);
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
	int64_t Dummy;
	if (DecodeNextFrame(DecodeFrame, &Dummy, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	//VP.image_type = VideoInfo::IT_TFF;
	VP.Width = CodecContext->width;
	VP.Height = CodecContext->height;
	VP.FPSDenominator = FormatContext->streams[VideoTrack]->time_base.num;
	VP.FPSNumerator = FormatContext->streams[VideoTrack]->time_base.den;
	VP.NumFrames = Frames.size();
	VP.VPixelFormat = CodecContext->pix_fmt;
	VP.FirstTime = ((Frames.front().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	VP.LastTime = ((Frames.back().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;

	if (VP.Width <= 0 || VP.Height <= 0) {
		Free(true);
		_snprintf(ErrorMsg, MsgSize, "Codec returned zero size video");
		throw ErrorMsg;
	}

	// sanity check framerate
	if (VP.FPSDenominator > VP.FPSNumerator || VP.FPSDenominator <= 0 || VP.FPSNumerator <= 0) {
		VP.FPSDenominator = 1;
		VP.FPSNumerator = 30;
	}

	if (InitPP(PP, CodecContext->pix_fmt, ErrorMsg, MsgSize)) {
			Free(true);
			throw ErrorMsg;
	}

	// Adjust framerate to match the duration of the first frame
	if (Frames.size() >= 2) {
		unsigned int DTSDiff = (unsigned int)FFMAX(Frames[1].DTS - Frames[0].DTS, 1);
		VP.FPSDenominator *= DTSDiff;
	}

	// Cannot "output" to PPFrame without doing all other initialization
	// This is the additional mess required for seekmode=-1 to work in a reasonable way
	OutputFrame(DecodeFrame);

	// Set AR variables
	VP.SARNum = CodecContext->sample_aspect_ratio.num;
	VP.SARDen = CodecContext->sample_aspect_ratio.den;
}

FFVideoSource::~FFVideoSource() {
	Free(true);
}

int FFVideoSource::DecodeNextFrame(AVFrame *AFrame, int64_t *AStartTime, char *ErrorMsg, unsigned MsgSize) {
	AVPacket Packet;
	InitNullPacket(&Packet);
	int FrameFinished = 0;
	*AStartTime = -1;

	while (av_read_frame(FormatContext, &Packet) >= 0) {
        if (Packet.stream_index == VideoTrack) {
			if (*AStartTime < 0)
				*AStartTime = Packet.dts;

			avcodec_decode_video2(CodecContext, AFrame, &FrameFinished, &Packet);
        }

        av_free_packet(&Packet);

		if (FrameFinished)
			goto Done;
	}

	// Flush the last frames
	if (CodecContext->has_b_frames) {
		AVPacket NullPacket;
		InitNullPacket(&NullPacket);
		avcodec_decode_video2(CodecContext, AFrame, &FrameFinished, &NullPacket);
	}

	if (!FrameFinished)
		goto Error;

// Ignore errors for now
Error:
Done:
	return 0;
}

AVFrameLite *FFVideoSource::GetFrame(int n, char *ErrorMsg, unsigned MsgSize) {
	// PPFrame always holds frame LastFrameNum even if no PP is applied
	if (LastFrameNum == n)
		return OutputFrame(DecodeFrame);

	bool HasSeeked = false;
	int SeekOffset = 0;

	int ClosestKF = 0;
	if (SeekMode >= 0) {
		ClosestKF = Frames.FindClosestKeyFrame(n);

		if (SeekMode == 0) {
			if (n < CurrentFrame) {
				av_seek_frame(FormatContext, VideoTrack, Frames[0].DTS, AVSEEK_FLAG_BACKWARD);
				avcodec_flush_buffers(CodecContext);
				CurrentFrame = 0;
			}
		} else {
			// 10 frames is used as a margin to prevent excessive seeking since the predicted best keyframe isn't always selected by avformat
			if (n < CurrentFrame || ClosestKF > CurrentFrame + 10 || (SeekMode == 3 && n > CurrentFrame + 10)) {
ReSeek:
				av_seek_frame(FormatContext, VideoTrack, (SeekMode == 3) ? Frames[n].DTS : Frames[ClosestKF + SeekOffset].DTS, AVSEEK_FLAG_BACKWARD);
				avcodec_flush_buffers(CodecContext);
				HasSeeked = true;
			}
		}
	} else if (n < CurrentFrame) {
		_snprintf(ErrorMsg, MsgSize, "Non-linear access attempted");
		return NULL;
	}

	do {
		int64_t StartTime;
		if (DecodeNextFrame(DecodeFrame, &StartTime, ErrorMsg, MsgSize))
			return NULL;

		if (HasSeeked) {
			HasSeeked = false;

			// Is the seek destination time known? Does it belong to a frame?
			if (StartTime < 0 || (CurrentFrame = Frames.FrameFromDTS(StartTime)) < 0) {
				switch (SeekMode) {
					case 1:
						// No idea where we are so go back a bit further
						if (n + SeekOffset == 0) {
							_snprintf(ErrorMsg, MsgSize, "Frame accurate seeking is not possible in this file\n");
							return NULL;
						}

						SeekOffset -= FFMIN(20, n + SeekOffset);
						goto ReSeek;
					case 2:
					case 3:
						CurrentFrame = Frames.ClosestFrameFromDTS(StartTime);
						break;
					default:
						_snprintf(ErrorMsg, MsgSize, "Failed assertion");
						return NULL;
				}	
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	LastFrameNum = n;
	return OutputFrame(DecodeFrame);
}

void MatroskaVideoSource::Free(bool CloseCodec) {
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

MatroskaVideoSource::MatroskaVideoSource(const char *SourceFile, int Track,
	FrameIndex *TrackIndices, const char *PP,
	int Threads, char *ErrorMsg, unsigned MsgSize) {

	AVCodec *Codec = NULL;
	CodecContext = NULL;
	TrackInfo *TI = NULL;
	CS = NULL;
	VideoTrack = Track;
	Frames = (*TrackIndices)[VideoTrack];

	if (Frames.size() == 0) {
		_snprintf(ErrorMsg, MsgSize, "Video track contains no frames");
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

	mkv_SetTrackMask(MF, ~(1 << VideoTrack));
	TI = mkv_GetTrackInfo(MF, VideoTrack);

	if (TI->CompEnabled) {
		CS = cs_Create(MF, VideoTrack, ErrorMessage, sizeof(ErrorMessage));
		if (CS == NULL) {
			Free(false);
			_snprintf(ErrorMsg, MsgSize, "Can't create decompressor: %s", ErrorMessage);
			throw ErrorMsg;
		}
	}

	CodecContext = avcodec_alloc_context();
	CodecContext->extradata = (uint8_t *)TI->CodecPrivate;
	CodecContext->extradata_size = TI->CodecPrivateSize;
	CodecContext->thread_count = Threads;

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
	int64_t Dummy;
	if (DecodeNextFrame(DecodeFrame, &Dummy, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	VP.Width = CodecContext->width;
	VP.Height = CodecContext->height;;
	VP.FPSDenominator = 1;
	VP.FPSNumerator = 30;
	VP.NumFrames = Frames.size();
	VP.VPixelFormat = CodecContext->pix_fmt;
	VP.FirstTime = ((Frames.front().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	VP.LastTime = ((Frames.back().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;

	if (VP.Width <= 0 || VP.Height <= 0) {
		Free(true);
		_snprintf(ErrorMsg, MsgSize, "Codec returned zero size video");
		throw ErrorMsg;
	}

	if (InitPP(PP, CodecContext->pix_fmt, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	// Calculate the average framerate
	if (Frames.size() >= 2) {
		double DTSDiff = (double)(Frames.back().DTS - Frames.front().DTS);
		VP.FPSDenominator = (unsigned int)(DTSDiff * mkv_TruncFloat(TI->TimecodeScale) / (double)1000 / (double)(VP.NumFrames - 1) + 0.5);
		VP.FPSNumerator = 1000000; 
	}

	// Output the already decoded frame so it isn't wasted
	OutputFrame(DecodeFrame);

	// Set AR variables
	VP.SARNum = TI->AV.Video.DisplayWidth * TI->AV.Video.PixelHeight;
	VP.SARDen = TI->AV.Video.DisplayHeight * TI->AV.Video.PixelWidth;

	// Set crop variables
	VP.CropLeft = TI->AV.Video.CropL;
	VP.CropRight = TI->AV.Video.CropR;
	VP.CropTop = TI->AV.Video.CropT;
	VP.CropBottom = TI->AV.Video.CropB;
}

MatroskaVideoSource::~MatroskaVideoSource() {
	Free(true);
}

int MatroskaVideoSource::DecodeNextFrame(AVFrame *AFrame, int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize) {
	int FrameFinished = 0;
	*AFirstStartTime = -1;
	AVPacket Packet;
	InitNullPacket(&Packet);

	ulonglong StartTime, EndTime, FilePos;
	unsigned int Track, FrameFlags, FrameSize;
	
	while (mkv_ReadFrame(MF, 0, &Track, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
		if (*AFirstStartTime < 0)
			*AFirstStartTime = StartTime;

		if (ReadFrame(FilePos, FrameSize, CS, MC, ErrorMsg, MsgSize))
			return 1;

		Packet.data = MC.Buffer;
		Packet.size = FrameSize;
		if (FrameFlags & FRAME_KF)
			Packet.flags = PKT_FLAG_KEY;
		else
			Packet.flags = 0;
		avcodec_decode_video2(CodecContext, AFrame, &FrameFinished, &Packet);

		if (FrameFinished)
			goto Done;
	}

	// Flush the last frames
	if (CodecContext->has_b_frames) {
		AVPacket NullPacket;
		InitNullPacket(&NullPacket);
		avcodec_decode_video2(CodecContext, AFrame, &FrameFinished, &NullPacket);
	}

	if (!FrameFinished)
		goto Error;

Error:
Done:
	return 0;
}

AVFrameLite *MatroskaVideoSource::GetFrame(int n, char *ErrorMsg, unsigned MsgSize) {
	// PPFrame always holds frame LastFrameNum even if no PP is applied
	if (LastFrameNum == n)
		return OutputFrame(DecodeFrame);

	bool HasSeeked = false;

	if (n < CurrentFrame || Frames.FindClosestKeyFrame(n) > CurrentFrame) {
		mkv_Seek(MF, Frames[n].DTS, MKVF_SEEK_TO_PREV_KEYFRAME_STRICT);
		avcodec_flush_buffers(CodecContext);
		HasSeeked = true;
	}

	do {
		int64_t StartTime;
		if (DecodeNextFrame(DecodeFrame, &StartTime, ErrorMsg, MsgSize))
				return NULL;

		if (HasSeeked) {
			HasSeeked = false;

			if (StartTime < 0 || (CurrentFrame = Frames.FrameFromDTS(StartTime)) < 0) {
				_snprintf(ErrorMsg, MsgSize, "Frame accurate seeking is not possible in this file");
				return NULL;
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	LastFrameNum = n;
	return OutputFrame(DecodeFrame);
}

#ifdef HAALISOURCE

void HaaliVideoSource::Free(bool CloseCodec) {
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_free(CodecContext);
	delete[] CodecPrivate;
}

HaaliVideoSource::HaaliVideoSource(const char *SourceFile, int Track,
	FrameIndex *TrackIndices, const char *PP,
	int Threads, int SourceMode, char *ErrorMsg, unsigned MsgSize) {

	AVCodec *Codec = NULL;
	CodecContext = NULL;
	VideoTrack = Track;
	Frames = (*TrackIndices)[VideoTrack];

	if (Frames.size() == 0) {
		_snprintf(ErrorMsg, MsgSize, "Video track contains no frames");
		throw ErrorMsg;
	}

	::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	CLSID clsid = HAALI_TS_Parser;
	if (SourceMode == 1)
		clsid = HAALI_OGM_Parser;

	if (FAILED(pMMC.CoCreateInstance(clsid))) {
		_snprintf(ErrorMsg, MsgSize, "Can't create parser");
		throw ErrorMsg;
	}

	CComPtr<IMemAlloc> pMA;
	if (FAILED(pMA.CoCreateInstance(CLSID_MemAlloc))) {
		_snprintf(ErrorMsg, MsgSize, "Can't create memory allocator");
		throw ErrorMsg;
	}

	CComPtr<IMMStream> pMS;
	if (FAILED(pMS.CoCreateInstance(CLSID_DiskFile))) {
		_snprintf(ErrorMsg, MsgSize, "Can't create disk file reader");
		throw ErrorMsg;
	}

	WCHAR WSourceFile[2048];
	mbstowcs(WSourceFile, SourceFile, 2000);
	CComQIPtr<IMMStreamOpen> pMSO(pMS);
	if (FAILED(pMSO->Open(WSourceFile))) {
		_snprintf(ErrorMsg, MsgSize, "Can't open file");
		throw ErrorMsg;
	}

	if (FAILED(pMMC->Open(pMS, 0, NULL, pMA))) {
		_snprintf(ErrorMsg, MsgSize, "Can't parse file");
		throw ErrorMsg;
	}

	CodecPrivate = NULL;
	int CodecPrivateSize = 0;
	int CurrentTrack = 0;
	CComPtr<IEnumUnknown> pEU;
	CComQIPtr<IPropertyBag> pBag;
	if (SUCCEEDED(pMMC->EnumTracks(&pEU))) {
		CComPtr<IUnknown> pU;
		while (pEU->Next(1, &pU, NULL) == S_OK) {
			if (CurrentTrack++ == Track) {
				pBag = pU;

				if (pBag) {
					CComVariant pV;

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"CodecPrivate", &pV, NULL))) {
						CodecPrivateSize = vtSize(pV);
						CodecPrivate = new uint8_t[CodecPrivateSize];
						vtCopy(pV, CodecPrivate);
					}

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"CodecID", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_BSTR))) {
						char ACodecID[2048];
						wcstombs(ACodecID, pV.bstrVal, 2000);
						Codec = avcodec_find_decoder(MatroskaToFFCodecID(ACodecID, CodecPrivate));
					}
				}
			}
			pU = NULL;
		}
	}

	CodecContext = avcodec_alloc_context();
	CodecContext->extradata = CodecPrivate;
	CodecContext->extradata_size = CodecPrivateSize;
	CodecContext->thread_count = Threads;

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
	int64_t Dummy;
	if (DecodeNextFrame(DecodeFrame, &Dummy, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	VP.Width = CodecContext->width;
	VP.Height = CodecContext->height;;
	VP.FPSDenominator = 1;
	VP.FPSNumerator = 30;
	VP.NumFrames = Frames.size();
	VP.VPixelFormat = CodecContext->pix_fmt;
	VP.FirstTime = ((Frames.front().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	VP.LastTime = ((Frames.back().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;

	if (VP.Width <= 0 || VP.Height <= 0) {
		Free(true);
		_snprintf(ErrorMsg, MsgSize, "Codec returned zero size video");
		throw ErrorMsg;
	}

	if (InitPP(PP, CodecContext->pix_fmt, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	// Calculate the average framerate
	if (Frames.size() >= 2) {
		double DTSDiff = (double)(Frames.back().DTS - Frames.front().DTS);
		// FIXME
		VP.FPSDenominator = (unsigned int)((DTSDiff * 1000000) / (double)(VP.NumFrames - 1) + 0.5);
		VP.FPSNumerator = 1000000; 
	}

	// Output the already decoded frame so it isn't wasted
	OutputFrame(DecodeFrame);

	// Set AR variables
	CComVariant pV;

	pV.Clear();
	if (SUCCEEDED(pBag->Read(L"Video.DisplayWidth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
		VP.SARNum  = pV.uiVal;
	pV.Clear();
	if (SUCCEEDED(pBag->Read(L"Video.DisplayHeight", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
		VP.SARDen = pV.uiVal;
}

HaaliVideoSource::~HaaliVideoSource() {
	Free(true);
}

int HaaliVideoSource::DecodeNextFrame(AVFrame *AFrame, int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize) {
	int FrameFinished = 0;
	*AFirstStartTime = -1;
	AVPacket Packet;
	InitNullPacket(&Packet);

	for (;;) {
		CComPtr<IMMFrame> pMMF;
		if (pMMC->ReadFrame(NULL, &pMMF) != S_OK)
			break;

		REFERENCE_TIME  Ts, Te;
		if (*AFirstStartTime < 0 && SUCCEEDED(pMMF->GetTime(&Ts, &Te)))
			*AFirstStartTime = Ts;

		if (pMMF->GetTrack() == VideoTrack) {
			BYTE *Data = NULL;
			if (FAILED(pMMF->GetPointer(&Data)))
				goto Error;

			Packet.data = Data;
			Packet.size = pMMF->GetActualDataLength();
			if (pMMF->IsSyncPoint() == S_OK)
				Packet.flags = PKT_FLAG_KEY;
			else
				Packet.flags = 0;

			avcodec_decode_video2(CodecContext, AFrame, &FrameFinished, &Packet);

			if (FrameFinished)
				goto Done;
		}
	}

	// Flush the last frames
	if (CodecContext->has_b_frames) {
		AVPacket NullPacket;
		InitNullPacket(&NullPacket);
		avcodec_decode_video2(CodecContext, AFrame, &FrameFinished, &NullPacket);
	}

	if (!FrameFinished)
		goto Error;

Error:
Done:
	return 0;
}

AVFrameLite *HaaliVideoSource::GetFrame(int n, char *ErrorMsg, unsigned MsgSize) {
	// PPFrame always holds frame LastFrameNum even if no PP is applied
	if (LastFrameNum == n)
		return OutputFrame(DecodeFrame);

	bool HasSeeked = false;
	int SeekOffset = 0;

	if (n < CurrentFrame || Frames.FindClosestKeyFrame(n) > CurrentFrame + 10) {
ReSeek:
		pMMC->Seek(Frames[n + SeekOffset].DTS, MMSF_PREV_KF);
		avcodec_flush_buffers(CodecContext);
		HasSeeked = true;
	}

	do {
		int64_t StartTime;
		if (DecodeNextFrame(DecodeFrame, &StartTime, ErrorMsg, MsgSize))
				return NULL;

		if (HasSeeked) {
			HasSeeked = false;

			if (StartTime < 0 || (CurrentFrame = Frames.FrameFromDTS(StartTime)) < 0) {
				// No idea where we are so go back a bit further
				if (n + SeekOffset == 0) {
					_snprintf(ErrorMsg, MsgSize, "Frame accurate seeking is not possible in this file\n");
					return NULL;
				}

				SeekOffset -= FFMIN(20, n + SeekOffset);
				goto ReSeek;
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	LastFrameNum = n;
	return OutputFrame(DecodeFrame);
}

#endif // HAALISOURCE
