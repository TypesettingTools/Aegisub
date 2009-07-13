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

int FFVideo::InitPP(const char *PP, char *ErrorMsg, unsigned MsgSize) {
	if (PP == NULL || !strcmp(PP, ""))
		return 0;

	PPMode = pp_get_mode_by_name_and_quality(PP, PP_QUALITY_MAX);
	if (!PPMode) {
		snprintf(ErrorMsg, MsgSize, "Invalid postprocesing settings");
		return 1;
	}

	return 0;
}

int FFVideo::ReAdjustPP(PixelFormat VPixelFormat, int Width, int Height, char *ErrorMsg, unsigned MsgSize) {
	if (!PPMode)
		return 0;

	int Flags =  GetPPCPUFlags();

	switch (VPixelFormat) {
		case PIX_FMT_YUV420P: Flags |= PP_FORMAT_420; break;
		case PIX_FMT_YUV422P: Flags |= PP_FORMAT_422; break;
		case PIX_FMT_YUV411P: Flags |= PP_FORMAT_411; break;
		case PIX_FMT_YUV444P: Flags |= PP_FORMAT_444; break;
		default:
			snprintf(ErrorMsg, MsgSize, "Input format is not supported for postprocessing");
			return 1;
	}

	if (PPContext)
		pp_free_context(PPContext);
	PPContext = pp_get_context(Width, Height, Flags);

	avpicture_free(&PPFrame);
	avpicture_alloc(&PPFrame, VPixelFormat, Width, Height);

	return 0;
}

static void CopyAVPictureFields(AVPicture &Picture, FFAVFrame &Dst) {
	for (int i = 0; i < 4; i++) {
		Dst.Data[i] = Picture.data[i];
		Dst.Linesize[i] = Picture.linesize[i];
	}
}

FFAVFrame *FFVideo::OutputFrame(AVFrame *Frame, char *ErrorMsg, unsigned MsgSize) {
	if (LastFrameWidth != CodecContext->width || LastFrameHeight != CodecContext->height || LastFramePixelFormat != CodecContext->pix_fmt) {
		if (ReAdjustPP(CodecContext->pix_fmt, CodecContext->width, CodecContext->height, ErrorMsg, MsgSize))
			return NULL;
		if (TargetHeight > 0 && TargetWidth > 0 && TargetPixelFormats != 0)
			if (ReAdjustOutputFormat(TargetPixelFormats, TargetWidth, TargetHeight, TargetResizer, ErrorMsg, MsgSize))
				return NULL;
	}

	if (PPMode) {
		pp_postprocess(const_cast<const uint8_t **>(Frame->data), Frame->linesize, PPFrame.data, PPFrame.linesize, CodecContext->width, CodecContext->height, Frame->qscale_table, Frame->qstride, PPMode, PPContext, Frame->pict_type | (Frame->qscale_type ? PP_PICT_TYPE_QP2 : 0));
		if (SWS) {
			sws_scale(SWS, PPFrame.data, PPFrame.linesize, 0, CodecContext->height, SWSFrame.data, SWSFrame.linesize);
			CopyAVPictureFields(SWSFrame, LocalFrame);
		} else {
			CopyAVPictureFields(PPFrame, LocalFrame);			
		}
	} else {
		if (SWS) {
			sws_scale(SWS, Frame->data, Frame->linesize, 0, CodecContext->height, SWSFrame.data, SWSFrame.linesize);
			CopyAVPictureFields(SWSFrame, LocalFrame);
		} else {
			// Special case to avoid ugly casts
			for (int i = 0; i < 4; i++) {
				LocalFrame.Data[i] = Frame->data[i];
				LocalFrame.Linesize[i] = Frame->linesize[i];
			}		
		}		
	}

	LocalFrame.EncodedWidth = CodecContext->width;
	LocalFrame.EncodedHeight = CodecContext->height;
	LocalFrame.EncodedPixelFormat = CodecContext->pix_fmt;
	LocalFrame.ScaledWidth = VP.Width;
	LocalFrame.ScaledHeight = VP.Height;
	LocalFrame.ConvertedPixelFormat = VP.VPixelFormat;
	LocalFrame.KeyFrame = Frame->key_frame;
	LocalFrame.PictType = av_get_pict_type_char(Frame->pict_type);
	LocalFrame.RepeatPict = Frame->repeat_pict;
	LocalFrame.InterlacedFrame = Frame->interlaced_frame;
	LocalFrame.TopFieldFirst = Frame->top_field_first;

	LastFrameHeight = CodecContext->height;
	LastFrameWidth = CodecContext->width;
	LastFramePixelFormat = CodecContext->pix_fmt;

	return &LocalFrame;
}

FFVideo::FFVideo(const char *SourceFile, FFIndex *Index, char *ErrorMsg, unsigned MsgSize) {
	if (Index->CompareFileSignature(SourceFile, ErrorMsg, MsgSize))
		throw ErrorMsg;

	memset(&VP, 0, sizeof(VP));
	PPContext = NULL;
	PPMode = NULL;
	SWS = NULL;
	LastFrameNum = 0;
	CurrentFrame = 1;
	CodecContext = NULL;
	LastFrameHeight = -1;
	LastFrameWidth = -1;
	LastFramePixelFormat = PIX_FMT_NONE;
	TargetHeight = -1;
	TargetWidth = -1;
	TargetPixelFormats = 0;
	TargetResizer = 0;
	DecodeFrame = avcodec_alloc_frame();

	// Dummy allocations so the unallocated case doesn't have to be handled later
	avpicture_alloc(&PPFrame, PIX_FMT_GRAY8, 16, 16);
	avpicture_alloc(&SWSFrame, PIX_FMT_GRAY8, 16, 16);
}

FFVideo::~FFVideo() {
	if (PPMode)
		pp_free_mode(PPMode);

	if (PPContext)
		pp_free_context(PPContext);

	if (SWS)
		sws_freeContext(SWS);

	avpicture_free(&PPFrame);
	avpicture_free(&SWSFrame);
	av_freep(&DecodeFrame);
}

FFAVFrame *FFVideo::GetFrameByTime(double Time, char *ErrorMsg, unsigned MsgSize) {
	int Frame = Frames.ClosestFrameFromDTS(static_cast<int64_t>((Time * 1000 * Frames.TB.Den) / Frames.TB.Num));
	return GetFrame(Frame, ErrorMsg, MsgSize);
}

int FFVideo::SetOutputFormat(int64_t TargetFormats, int Width, int Height, int Resizer, char *ErrorMsg, unsigned MsgSize) {
	this->TargetWidth = Width;
	this->TargetHeight = Height;
	this->TargetPixelFormats = TargetFormats;
	this->TargetResizer = Resizer;
	return ReAdjustOutputFormat(TargetFormats, Width, Height, Resizer, ErrorMsg, MsgSize);
}

int FFVideo::ReAdjustOutputFormat(int64_t TargetFormats, int Width, int Height, int Resizer, char *ErrorMsg, unsigned MsgSize) {
	if (SWS) {
		sws_freeContext(SWS);
		SWS = NULL;
	}

	int Loss;
	PixelFormat OutputFormat = avcodec_find_best_pix_fmt(TargetFormats,
		CodecContext->pix_fmt, 1 /* Required to prevent pointless RGB32 => RGB24 conversion */, &Loss);
	if (OutputFormat == PIX_FMT_NONE) {
		ResetOutputFormat();
		snprintf(ErrorMsg, MsgSize, "No suitable output format found");
		return 1;
	}

	if (CodecContext->pix_fmt != OutputFormat || Width != CodecContext->width || Height != CodecContext->height) {
		SWS = sws_getContext(CodecContext->width, CodecContext->height, CodecContext->pix_fmt, Width, Height,
			OutputFormat, GetSWSCPUFlags() | Resizer, NULL, NULL, NULL);
		if (SWS == NULL) {
			ResetOutputFormat();
			snprintf(ErrorMsg, MsgSize, "Failed to allocate SWScale context");
			return 1;
		}
	}

	VP.VPixelFormat = OutputFormat;
	VP.Height = Height;
	VP.Width = Width;

	avpicture_free(&SWSFrame);
	avpicture_alloc(&SWSFrame, OutputFormat, Width, Height);

	return 0;
}

void FFVideo::ResetOutputFormat() {
	if (SWS) {
		sws_freeContext(SWS);
		SWS = NULL;
	}

	TargetWidth = -1;
	TargetHeight = -1;
	TargetPixelFormats = 0;

	VP.Height = CodecContext->height;
	VP.Width = CodecContext->width;
	VP.VPixelFormat = CodecContext->pix_fmt;
}
