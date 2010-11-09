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

#include "videosource.h"

void FFMS_VideoSource::GetFrameCheck(int n) {
	if (n < 0 || n >= VP.NumFrames)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_INVALID_ARGUMENT,
			"Out of bounds frame requested");
}

void FFMS_VideoSource::SetPP(const char *PP) {

#ifdef WITH_LIBPOSTPROC
	if (PPMode)
		pp_free_mode(PPMode);
	PPMode = NULL;

	if (PP != NULL && strcmp(PP, "")) {
		PPMode = pp_get_mode_by_name_and_quality(PP, PP_QUALITY_MAX);
		if (!PPMode) {
			ResetPP();
			throw FFMS_Exception(FFMS_ERROR_POSTPROCESSING, FFMS_ERROR_INVALID_ARGUMENT,
				"Invalid postprocesing settings");
		}
		
	}

	ReAdjustPP(CodecContext->pix_fmt, CodecContext->width, CodecContext->height);
	OutputFrame(DecodeFrame);
#else
	throw FFMS_Exception(FFMS_ERROR_POSTPROCESSING, FFMS_ERROR_UNSUPPORTED,
		"FFMS2 was not compiled with postprocessing support");
#endif /* WITH_LIBPOSTPROC */
}

void FFMS_VideoSource::ResetPP() {
#ifdef WITH_LIBPOSTPROC
	if (PPContext)
		pp_free_context(PPContext);
	PPContext = NULL;

	if (PPMode)
		pp_free_mode(PPMode);
	PPMode = NULL;

#endif /* WITH_LIBPOSTPROC */
	OutputFrame(DecodeFrame);
}

void FFMS_VideoSource::ReAdjustPP(PixelFormat VPixelFormat, int Width, int Height) {
#ifdef WITH_LIBPOSTPROC
	if (PPContext)
		pp_free_context(PPContext);
	PPContext = NULL;

	if (!PPMode)
		return;

	int Flags =  GetPPCPUFlags();

	switch (VPixelFormat) {
		case PIX_FMT_YUV420P: Flags |= PP_FORMAT_420; break;
		case PIX_FMT_YUV422P: Flags |= PP_FORMAT_422; break;
		case PIX_FMT_YUV411P: Flags |= PP_FORMAT_411; break;
		case PIX_FMT_YUV444P: Flags |= PP_FORMAT_444; break;
		default:
			ResetPP();
			throw FFMS_Exception(FFMS_ERROR_POSTPROCESSING, FFMS_ERROR_UNSUPPORTED,
				"The video does not have a colorspace suitable for postprocessing");
	}

	PPContext = pp_get_context(Width, Height, Flags);

	avpicture_free(&PPFrame);
	avpicture_alloc(&PPFrame, VPixelFormat, Width, Height);
#else
	return;
#endif /* WITH_LIBPOSTPROC */

}


static void CopyAVPictureFields(AVPicture &Picture, FFMS_Frame &Dst) {
	for (int i = 0; i < 4; i++) {
		Dst.Data[i] = Picture.data[i];
		Dst.Linesize[i] = Picture.linesize[i];
	}
}

FFMS_Frame *FFMS_VideoSource::OutputFrame(AVFrame *Frame) {
	if (LastFrameWidth != CodecContext->width || LastFrameHeight != CodecContext->height || LastFramePixelFormat != CodecContext->pix_fmt) {
		ReAdjustPP(CodecContext->pix_fmt, CodecContext->width, CodecContext->height);

		if (TargetHeight > 0 && TargetWidth > 0 && TargetPixelFormats != 0)
			ReAdjustOutputFormat(TargetPixelFormats, TargetWidth, TargetHeight, TargetResizer);
	}

#ifdef WITH_LIBPOSTPROC
	if (PPMode) {
		pp_postprocess(const_cast<const uint8_t **>(Frame->data), Frame->linesize, PPFrame.data, PPFrame.linesize, CodecContext->width, CodecContext->height, Frame->qscale_table, Frame->qstride, PPMode, PPContext, Frame->pict_type | (Frame->qscale_type ? PP_PICT_TYPE_QP2 : 0));
		if (SWS) {
			sws_scale(SWS, const_cast<FFMS_SWS_CONST_PARAM uint8_t **>(PPFrame.data), PPFrame.linesize, 0, CodecContext->height, SWSFrame.data, SWSFrame.linesize);
			CopyAVPictureFields(SWSFrame, LocalFrame);
		} else {
			CopyAVPictureFields(PPFrame, LocalFrame);			
		}
	} else {
		if (SWS) {
			sws_scale(SWS, const_cast<FFMS_SWS_CONST_PARAM uint8_t **>(Frame->data), Frame->linesize, 0, CodecContext->height, SWSFrame.data, SWSFrame.linesize);
			CopyAVPictureFields(SWSFrame, LocalFrame);
		} else {
			// Special case to avoid ugly casts
			for (int i = 0; i < 4; i++) {
				LocalFrame.Data[i] = Frame->data[i];
				LocalFrame.Linesize[i] = Frame->linesize[i];
			}		
		}		
	}
#else // WITH_LIBPOSTPROC
	if (SWS) {
		sws_scale(SWS, const_cast<FFMS_SWS_CONST_PARAM uint8_t **>(Frame->data), Frame->linesize, 0, CodecContext->height, SWSFrame.data, SWSFrame.linesize);
		CopyAVPictureFields(SWSFrame, LocalFrame);
	} else {
		// Special case to avoid ugly casts
		for (int i = 0; i < 4; i++) {
			LocalFrame.Data[i] = Frame->data[i];
			LocalFrame.Linesize[i] = Frame->linesize[i];
		}		
	}		
#endif // WITH_LIBPOSTPROC

	LocalFrame.EncodedWidth = CodecContext->width;
	LocalFrame.EncodedHeight = CodecContext->height;
	LocalFrame.EncodedPixelFormat = CodecContext->pix_fmt;
	LocalFrame.ScaledWidth = TargetWidth;
	LocalFrame.ScaledHeight = TargetHeight;
	LocalFrame.ConvertedPixelFormat = OutputFormat;
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

FFMS_VideoSource::FFMS_VideoSource(const char *SourceFile, FFMS_Index *Index, int Track) {
	if (Track < 0 || Track >= static_cast<int>(Index->size()))
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Out of bounds track index selected");

	if (Index->at(Track).TT != FFMS_TYPE_VIDEO)
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Not a video track");

	if (Index->at(Track).size() == 0)
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Video track contains no frames");

	if (!Index->CompareFileSignature(SourceFile))
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_FILE_MISMATCH,
			"The index does not match the source file");

	memset(&VP, 0, sizeof(VP));
#ifdef WITH_LIBPOSTPROC
	PPContext = NULL;
	PPMode = NULL;
#endif // WITH_LIBPOSTPROC
	SWS = NULL;
	LastFrameNum = 0;
	CurrentFrame = 1;
	DelayCounter = 0;
	InitialDecode = 1;
	CodecContext = NULL;
	LastFrameHeight = -1;
	LastFrameWidth = -1;
	LastFramePixelFormat = PIX_FMT_NONE;
	TargetHeight = -1;
	TargetWidth = -1;
	TargetPixelFormats = 0;
	TargetResizer = 0;
	OutputFormat = PIX_FMT_NONE;
	DecodeFrame = avcodec_alloc_frame();

	// Dummy allocations so the unallocated case doesn't have to be handled later
#ifdef WITH_LIBPOSTPROC
	avpicture_alloc(&PPFrame, PIX_FMT_GRAY8, 16, 16);
#endif // WITH_LIBPOSTPROC
	avpicture_alloc(&SWSFrame, PIX_FMT_GRAY8, 16, 16);
}

FFMS_VideoSource::~FFMS_VideoSource() {
#ifdef WITH_LIBPOSTPROC
	if (PPMode)
		pp_free_mode(PPMode);

	if (PPContext)
		pp_free_context(PPContext);

	avpicture_free(&PPFrame);
#endif // WITH_LIBPOSTPROC

	if (SWS)
		sws_freeContext(SWS);

	avpicture_free(&SWSFrame);
	av_freep(&DecodeFrame);
}

FFMS_Frame *FFMS_VideoSource::GetFrameByTime(double Time) {
	int Frame = Frames.ClosestFrameFromPTS(static_cast<int64_t>((Time * 1000 * Frames.TB.Den) / Frames.TB.Num));
	return GetFrame(Frame);
}

void FFMS_VideoSource::SetOutputFormat(int64_t TargetFormats, int Width, int Height, int Resizer) {
	this->TargetWidth = Width;
	this->TargetHeight = Height;
	this->TargetPixelFormats = TargetFormats;
	this->TargetResizer = Resizer;
	ReAdjustOutputFormat(TargetFormats, Width, Height, Resizer);
	OutputFrame(DecodeFrame);
}

void FFMS_VideoSource::ReAdjustOutputFormat(int64_t TargetFormats, int Width, int Height, int Resizer) {
	if (SWS) {
		sws_freeContext(SWS);
		SWS = NULL;
	}

	int Loss;
	OutputFormat = avcodec_find_best_pix_fmt(TargetFormats,
		CodecContext->pix_fmt, 1 /* Required to prevent pointless RGB32 => RGB24 conversion */, &Loss);
	if (OutputFormat == PIX_FMT_NONE) {
		ResetOutputFormat();
		throw FFMS_Exception(FFMS_ERROR_SCALING, FFMS_ERROR_INVALID_ARGUMENT,
			"No suitable output format found");
	}

	if (CodecContext->pix_fmt != OutputFormat || Width != CodecContext->width || Height != CodecContext->height) {
		SWS = sws_getContext(CodecContext->width, CodecContext->height, CodecContext->pix_fmt, Width, Height,
			OutputFormat, GetSWSCPUFlags() | Resizer, NULL, NULL, NULL);
		if (SWS == NULL) {
			ResetOutputFormat();
			throw FFMS_Exception(FFMS_ERROR_SCALING, FFMS_ERROR_INVALID_ARGUMENT,
				"Failed to allocate SWScale context");
		}
	}

	avpicture_free(&SWSFrame);
	avpicture_alloc(&SWSFrame, OutputFormat, Width, Height);
}

void FFMS_VideoSource::ResetOutputFormat() {
	if (SWS) {
		sws_freeContext(SWS);
		SWS = NULL;
	}

	TargetWidth = -1;
	TargetHeight = -1;
	TargetPixelFormats = 0;
	OutputFormat = PIX_FMT_NONE;
	
	OutputFrame(DecodeFrame);
}
