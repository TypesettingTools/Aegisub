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

void FFMatroskaVideo::Free(bool CloseCodec) {
	if (CS)
		cs_Destroy(CS);
	if (MC.ST.fp) {
		mkv_Close(MF);
		fclose(MC.ST.fp);
	}
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_freep(&CodecContext);
}

FFMatroskaVideo::FFMatroskaVideo(const char *SourceFile, int Track,
	FFIndex *Index, const char *PP,
	int Threads, char *ErrorMsg, unsigned MsgSize)
	: FFVideo(SourceFile, Index, ErrorMsg, MsgSize) {

	AVCodec *Codec = NULL;
	CodecContext = NULL;
	TrackInfo *TI = NULL;
	CS = NULL;
	VideoTrack = Track;
	Frames = (*Index)[VideoTrack];

	if (Frames.size() == 0) {
		snprintf(ErrorMsg, MsgSize, "Video track contains no frames");
		throw ErrorMsg;
	}

	MC.ST.fp = ffms_fopen(SourceFile, "rb");
	if (MC.ST.fp == NULL) {
		snprintf(ErrorMsg, MsgSize, "Can't open '%s': %s", SourceFile, strerror(errno));
		throw ErrorMsg;
	}

	setvbuf(MC.ST.fp, NULL, _IOFBF, CACHESIZE);

	MF = mkv_OpenEx(&MC.ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
	if (MF == NULL) {
		fclose(MC.ST.fp);
		snprintf(ErrorMsg, MsgSize, "Can't parse Matroska file: %s", ErrorMessage);
		throw ErrorMsg;
	}

	mkv_SetTrackMask(MF, ~(1 << VideoTrack));
	TI = mkv_GetTrackInfo(MF, VideoTrack);

	if (TI->CompEnabled) {
		CS = cs_Create(MF, VideoTrack, ErrorMessage, sizeof(ErrorMessage));
		if (CS == NULL) {
			Free(false);
			snprintf(ErrorMsg, MsgSize, "Can't create decompressor: %s", ErrorMessage);
			throw ErrorMsg;
		}
	}

	CodecContext = avcodec_alloc_context();
	CodecContext->thread_count = Threads;

	Codec = avcodec_find_decoder(MatroskaToFFCodecID(TI->CodecID, TI->CodecPrivate));
	if (Codec == NULL) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Video codec not found");
		throw ErrorMsg;
	}

	InitializeCodecContextFromMatroskaTrackInfo(TI, CodecContext);

	if (avcodec_open(CodecContext, Codec) < 0) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Could not open video codec");
		throw ErrorMsg;
	}

	// Always try to decode a frame to make sure all required parameters are known
	int64_t Dummy;
	if (DecodeNextFrame(&Dummy, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	VP.Width = CodecContext->width;
	VP.Height = CodecContext->height;;
	VP.FPSDenominator = 1;
	VP.FPSNumerator = 30;
	VP.RFFDenominator = CodecContext->time_base.num;
	VP.RFFNumerator = CodecContext->time_base.den;
	VP.NumFrames = Frames.size();
	VP.VPixelFormat = CodecContext->pix_fmt;
	VP.FirstTime = ((Frames.front().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	VP.LastTime = ((Frames.back().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;

	if (VP.Width <= 0 || VP.Height <= 0) {
		Free(true);
		snprintf(ErrorMsg, MsgSize, "Codec returned zero size video");
		throw ErrorMsg;
	}

	if (InitPP(PP, ErrorMsg, MsgSize)) {
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
	if (!OutputFrame(DecodeFrame, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	// Set AR variables
	VP.SARNum = TI->AV.Video.DisplayWidth * TI->AV.Video.PixelHeight;
	VP.SARDen = TI->AV.Video.DisplayHeight * TI->AV.Video.PixelWidth;

	// Set crop variables
	VP.CropLeft = TI->AV.Video.CropL;
	VP.CropRight = TI->AV.Video.CropR;
	VP.CropTop = TI->AV.Video.CropT;
	VP.CropBottom = TI->AV.Video.CropB;
}

FFMatroskaVideo::~FFMatroskaVideo() {
	Free(true);
}

int FFMatroskaVideo::DecodeNextFrame(int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize) {
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
			Packet.flags = AV_PKT_FLAG_KEY;
		else
			Packet.flags = 0;

		avcodec_decode_video2(CodecContext, DecodeFrame, &FrameFinished, &Packet);

		if (FrameFinished)
			goto Done;
	}

	// Flush the last frames
	if (CodecContext->has_b_frames) {
		AVPacket NullPacket;
		InitNullPacket(&NullPacket);
		avcodec_decode_video2(CodecContext, DecodeFrame, &FrameFinished, &NullPacket);
	}

	if (!FrameFinished)
		goto Error;

Error:
Done:
	return 0;
}

FFAVFrame *FFMatroskaVideo::GetFrame(int n, char *ErrorMsg, unsigned MsgSize) {
	// PPFrame always holds frame LastFrameNum even if no PP is applied
	if (LastFrameNum == n)
		return OutputFrame(DecodeFrame, ErrorMsg, MsgSize);

	bool HasSeeked = false;

	if (n < CurrentFrame || Frames.FindClosestVideoKeyFrame(n) > CurrentFrame) {
		mkv_Seek(MF, Frames[n].DTS, MKVF_SEEK_TO_PREV_KEYFRAME_STRICT);
		avcodec_flush_buffers(CodecContext);
		HasSeeked = true;
	}

	do {
		int64_t StartTime;
		if (DecodeNextFrame(&StartTime, ErrorMsg, MsgSize))
				return NULL;

		if (HasSeeked) {
			HasSeeked = false;

			if (StartTime < 0 || (CurrentFrame = Frames.FrameFromDTS(StartTime)) < 0) {
				snprintf(ErrorMsg, MsgSize, "Frame accurate seeking is not possible in this file");
				return NULL;
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	LastFrameNum = n;
	return OutputFrame(DecodeFrame, ErrorMsg, MsgSize);
}
