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



void FFLAVFVideo::Free(bool CloseCodec) {
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_close_input_file(FormatContext);
}

FFLAVFVideo::FFLAVFVideo(const char *SourceFile, int Track, FFIndex *Index,
	const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize)
	: FFVideo(SourceFile, Index, ErrorMsg, MsgSize) {

	FormatContext = NULL;
	AVCodec *Codec = NULL;
	this->SeekMode = SeekMode;
	VideoTrack = Track;
	Frames = (*Index)[VideoTrack];

	if (Frames.size() == 0) {
		snprintf(ErrorMsg, MsgSize, "Video track contains no frames");
		throw ErrorMsg;
	}

	if (av_open_input_file(&FormatContext, SourceFile, NULL, 0, NULL) != 0) {
		snprintf(ErrorMsg, MsgSize, "Couldn't open '%s'", SourceFile);
		throw ErrorMsg;
	}
	
	if (av_find_stream_info(FormatContext) < 0) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Couldn't find stream information");
		throw ErrorMsg;
	}

	if (SeekMode >= 0 && av_seek_frame(FormatContext, VideoTrack, Frames[0].DTS, AVSEEK_FLAG_BACKWARD) < 0) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Video track is unseekable");
		throw ErrorMsg;
	}

	CodecContext = FormatContext->streams[VideoTrack]->codec;
	CodecContext->thread_count = Threads;

	Codec = avcodec_find_decoder(CodecContext->codec_id);
	if (Codec == NULL) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Video codec not found");
		throw ErrorMsg;
	}

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

	//VP.image_type = VideoInfo::IT_TFF;
	VP.Width = CodecContext->width;
	VP.Height = CodecContext->height;
	VP.FPSDenominator = FormatContext->streams[VideoTrack]->time_base.num;
	VP.FPSNumerator = FormatContext->streams[VideoTrack]->time_base.den;
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

	// sanity check framerate
	if (VP.FPSDenominator > VP.FPSNumerator || VP.FPSDenominator <= 0 || VP.FPSNumerator <= 0) {
		VP.FPSDenominator = 1;
		VP.FPSNumerator = 30;
	}

	if (InitPP(PP, ErrorMsg, MsgSize)) {
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
	if (!OutputFrame(DecodeFrame, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	// Set AR variables
	VP.SARNum = CodecContext->sample_aspect_ratio.num;
	VP.SARDen = CodecContext->sample_aspect_ratio.den;
}

FFLAVFVideo::~FFLAVFVideo() {
	Free(true);
}

int FFLAVFVideo::DecodeNextFrame(int64_t *AStartTime, char *ErrorMsg, unsigned MsgSize) {
	AVPacket Packet;
	InitNullPacket(&Packet);
	int FrameFinished = 0;
	*AStartTime = -1;

	while (av_read_frame(FormatContext, &Packet) >= 0) {
        if (Packet.stream_index == VideoTrack) {
			if (*AStartTime < 0)
				*AStartTime = Packet.dts;

			avcodec_decode_video2(CodecContext, DecodeFrame, &FrameFinished, &Packet);
        }

        av_free_packet(&Packet);

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

// Ignore errors for now
Error:
Done:
	return 0;
}

FFAVFrame *FFLAVFVideo::GetFrame(int n, char *ErrorMsg, unsigned MsgSize) {
	// PPFrame always holds frame LastFrameNum even if no PP is applied
	if (LastFrameNum == n)
		return OutputFrame(DecodeFrame, ErrorMsg, MsgSize);

	bool HasSeeked = false;
	int SeekOffset = 0;

	int ClosestKF = 0;
	if (SeekMode >= 0) {
		ClosestKF = Frames.FindClosestVideoKeyFrame(n);

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
				av_seek_frame(FormatContext, VideoTrack,
					(SeekMode == 3) ? Frames[n].DTS : Frames[ClosestKF + SeekOffset].DTS,
					AVSEEK_FLAG_BACKWARD);
				avcodec_flush_buffers(CodecContext);
				HasSeeked = true;
			}
		}
	} else if (n < CurrentFrame) {
		snprintf(ErrorMsg, MsgSize, "Non-linear access attempted");
		return NULL;
	}

	do {
		int64_t StartTime;
		if (DecodeNextFrame(&StartTime, ErrorMsg, MsgSize))
			return NULL;

		if (HasSeeked) {
			HasSeeked = false;

			// Is the seek destination time known? Does it belong to a frame?
			if (StartTime < 0 || (CurrentFrame = Frames.FrameFromDTS(StartTime)) < 0) {
				switch (SeekMode) {
					case 1:
						// No idea where we are so go back a bit further
						if (ClosestKF + SeekOffset == 0) {
							snprintf(ErrorMsg, MsgSize, "Frame accurate seeking is not possible in this file\n");
							return NULL;
						}

						SeekOffset -= FFMIN(10, ClosestKF + SeekOffset);
						goto ReSeek;
					case 2:
					case 3:
						CurrentFrame = Frames.ClosestFrameFromDTS(StartTime);
						break;
					default:
						snprintf(ErrorMsg, MsgSize, "Failed assertion");
						return NULL;
				}	
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	LastFrameNum = n;
	return OutputFrame(DecodeFrame, ErrorMsg, MsgSize);
}
