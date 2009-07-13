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

#ifdef HAALISOURCE

void FFHaaliVideo::Free(bool CloseCodec) {
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_freep(&CodecContext);
	if (BitStreamFilter)
		av_bitstream_filter_close(BitStreamFilter);
}

FFHaaliVideo::FFHaaliVideo(const char *SourceFile, int Track,
	FFIndex *Index, const char *PP,
	int Threads, int SourceMode, char *ErrorMsg, unsigned MsgSize)
	: FFVideo(SourceFile, Index, ErrorMsg, MsgSize) {

	BitStreamFilter = NULL;
	AVCodec *Codec = NULL;
	CodecContext = NULL;
	VideoTrack = Track;
	Frames = (*Index)[VideoTrack];

	if (Frames.size() == 0) {
		snprintf(ErrorMsg, MsgSize, "Video track contains no frames");
		throw ErrorMsg;
	}

	CLSID clsid = HAALI_TS_Parser;
	if (SourceMode == 1)
		clsid = HAALI_OGM_Parser;

	if (FAILED(pMMC.CoCreateInstance(clsid))) {
		snprintf(ErrorMsg, MsgSize, "Can't create parser");
		throw ErrorMsg;
	}

	CComPtr<IMemAlloc> pMA;
	if (FAILED(pMA.CoCreateInstance(CLSID_MemAlloc))) {
		snprintf(ErrorMsg, MsgSize, "Can't create memory allocator");
		throw ErrorMsg;
	}

	CComPtr<IMMStream> pMS;
	if (FAILED(pMS.CoCreateInstance(CLSID_DiskFile))) {
		snprintf(ErrorMsg, MsgSize, "Can't create disk file reader");
		throw ErrorMsg;
	}

	WCHAR WSourceFile[2048];
	ffms_mbstowcs(WSourceFile, SourceFile, 2000);
	CComQIPtr<IMMStreamOpen> pMSO(pMS);
	if (FAILED(pMSO->Open(WSourceFile))) {
		snprintf(ErrorMsg, MsgSize, "Can't open file");
		throw ErrorMsg;
	}

	if (FAILED(pMMC->Open(pMS, 0, NULL, pMA))) {
		snprintf(ErrorMsg, MsgSize, "Can't parse file");
		throw ErrorMsg;
	}

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
					unsigned int FourCC = 0;

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"CodecPrivate", &pV, NULL))) {
						CodecPrivateSize = vtSize(pV);
						CodecPrivate.resize(CodecPrivateSize);
						vtCopy(pV, FFMS_GET_VECTOR_PTR(CodecPrivate));
					}

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"FOURCC", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
						FourCC = pV.uintVal;

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"CodecID", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_BSTR))) {
						char ACodecID[2048];
						wcstombs(ACodecID, pV.bstrVal, 2000);
						Codec = avcodec_find_decoder(MatroskaToFFCodecID(ACodecID, FFMS_GET_VECTOR_PTR(CodecPrivate), FourCC));
					}
				}
			}
			pU = NULL;
		}
	}

	CodecContext = avcodec_alloc_context();
	CodecContext->extradata = FFMS_GET_VECTOR_PTR(CodecPrivate);
	CodecContext->extradata_size = CodecPrivateSize;
	CodecContext->thread_count = Threads;

	if (Codec == NULL) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Video codec not found");
		throw ErrorMsg;
	}

	InitializeCodecContextFromHaaliInfo(pBag, CodecContext);

	if (avcodec_open(CodecContext, Codec) < 0) {
		Free(false);
		snprintf(ErrorMsg, MsgSize, "Could not open video codec");
		throw ErrorMsg;
	}

	if (Codec->id == CODEC_ID_H264 && SourceMode == 0)
		BitStreamFilter = av_bitstream_filter_init("h264_mp4toannexb");

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
		VP.FPSDenominator = (unsigned int)(DTSDiff  / (double)1000 / (double)(VP.NumFrames - 1) + 0.5);
		VP.FPSNumerator = 1000000; 
	}

	// Output the already decoded frame so it isn't wasted
	if (!OutputFrame(DecodeFrame, ErrorMsg, MsgSize)) {
		Free(true);
		throw ErrorMsg;
	}

	// Set AR variables
	CComVariant pV;

	pV.Clear();
	if (SUCCEEDED(pBag->Read(L"Video.DisplayWidth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
		VP.SARNum  = pV.uiVal;
	pV.Clear();
	if (SUCCEEDED(pBag->Read(L"Video.DisplayHeight", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
		VP.SARDen = pV.uiVal;
}

FFHaaliVideo::~FFHaaliVideo() {
	Free(true);
}

int FFHaaliVideo::DecodeNextFrame(int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize) {
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
				Packet.flags = AV_PKT_FLAG_KEY;
			else
				Packet.flags = 0;

			if (BitStreamFilter)
				av_bitstream_filter_filter(BitStreamFilter, CodecContext, NULL,
				&Packet.data, &Packet.size, Data, pMMF->GetActualDataLength(), !!Packet.flags);

			avcodec_decode_video2(CodecContext, DecodeFrame, &FrameFinished, &Packet);

			if (FrameFinished)
				goto Done;
		}
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

FFAVFrame *FFHaaliVideo::GetFrame(int n, char *ErrorMsg, unsigned MsgSize) {
	// PPFrame always holds frame LastFrameNum even if no PP is applied
	if (LastFrameNum == n)
		return OutputFrame(DecodeFrame, ErrorMsg, MsgSize);

	bool HasSeeked = false;
	int SeekOffset = 0;

	if (n < CurrentFrame || Frames.FindClosestVideoKeyFrame(n) > CurrentFrame + 10) {
ReSeek:
		pMMC->Seek(Frames[n + SeekOffset].DTS, MMSF_PREV_KF);
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
				// No idea where we are so go back a bit further
				if (n + SeekOffset == 0) {
					snprintf(ErrorMsg, MsgSize, "Frame accurate seeking is not possible in this file\n");
					return NULL;
				}

				SeekOffset -= FFMIN(20, n + SeekOffset);
				goto ReSeek;
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	LastFrameNum = n;
	return OutputFrame(DecodeFrame, ErrorMsg, MsgSize);
}

#endif // HAALISOURCE
