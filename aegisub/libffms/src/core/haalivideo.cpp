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

#ifdef HAALISOURCE

#include "videosource.h"



void FFHaaliVideo::Free(bool CloseCodec) {
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_freep(&CodecContext);
	if (BitStreamFilter)
		av_bitstream_filter_close(BitStreamFilter);
}

FFHaaliVideo::FFHaaliVideo(const char *SourceFile, int Track,
	FFMS_Index *Index, int Threads, enum FFMS_Sources SourceMode)
	: Res(FFSourceResources<FFMS_VideoSource>(this)), FFMS_VideoSource(SourceFile, Index, Track) {

	BitStreamFilter = NULL;
	AVCodec *Codec = NULL;
	CodecContext = NULL;
	VideoTrack = Track;
	Frames = (*Index)[VideoTrack];

	pMMC = HaaliOpenFile(SourceFile, SourceMode);

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
					if (SUCCEEDED(pBag->Read(L"FOURCC", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4))) {
						FourCC = pV.uintVal;
						
						// Reconstruct the missing codec private part for VC1
						std::vector<uint8_t> bihvect;
						bihvect.resize(sizeof(FFMS_BITMAPINFOHEADER));
						FFMS_BITMAPINFOHEADER *bih = reinterpret_cast<FFMS_BITMAPINFOHEADER *>(FFMS_GET_VECTOR_PTR(bihvect));
						memset(bih, 0, sizeof(FFMS_BITMAPINFOHEADER));
						bih->biSize = sizeof(FFMS_BITMAPINFOHEADER) + CodecPrivateSize;
						bih->biCompression = FourCC;
						bih->biBitCount = 24;
						bih->biPlanes = 1;

						pV.Clear();
						if (SUCCEEDED(pBag->Read(L"Video.PixelWidth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
							bih->biWidth = pV.uintVal;

						pV.Clear();
						if (SUCCEEDED(pBag->Read(L"Video.PixelHeight", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
							bih->biHeight = pV.uintVal;

						CodecPrivate.insert(CodecPrivate.begin(), bihvect.begin(), bihvect.end());
						CodecPrivateSize += sizeof(FFMS_BITMAPINFOHEADER);
					}

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

	if (Codec == NULL)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Video codec not found");

	InitializeCodecContextFromHaaliInfo(pBag, CodecContext);

	if (avcodec_open(CodecContext, Codec) < 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Could not open video codec");

	if (Codec->id == CODEC_ID_H264 && SourceMode == FFMS_SOURCE_HAALIMPEG)
		BitStreamFilter = av_bitstream_filter_init("h264_mp4toannexb");

	Res.CloseCodec(true);

	// Always try to decode a frame to make sure all required parameters are known
	int64_t Dummy;
	DecodeNextFrame(&Dummy);

	VP.FPSDenominator = 1;
	VP.FPSNumerator = 30;
	VP.RFFDenominator = CodecContext->time_base.num;
	VP.RFFNumerator = CodecContext->time_base.den;
	VP.NumFrames = Frames.size();
	VP.TopFieldFirst = DecodeFrame->top_field_first;
	VP.ColorSpace = CodecContext->colorspace;
	VP.ColorRange = CodecContext->color_range;
	VP.FirstTime = ((Frames.front().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	VP.LastTime = ((Frames.back().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;

	if (CodecContext->width <= 0 || CodecContext->height <= 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Codec returned zero size video");

	// Calculate the average framerate
	if (Frames.size() >= 2) {
		double DTSDiff = (double)(Frames.back().DTS - Frames.front().DTS);
		VP.FPSDenominator = (unsigned int)(DTSDiff  / (double)1000 / (double)(VP.NumFrames - 1) + 0.5);
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

void FFHaaliVideo::DecodeNextFrame(int64_t *AFirstStartTime) {
	int FrameFinished = 0;
	*AFirstStartTime = -1;
	AVPacket Packet;
	InitNullPacket(Packet);

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

			if (CodecContext->codec_id == CODEC_ID_MPEG4 && IsNVOP(Packet))
				goto Done;

			avcodec_decode_video2(CodecContext, DecodeFrame, &FrameFinished, &Packet);

			if (FrameFinished)
				goto Done;
		}
	}

	// Flush the last frames
	if (CodecContext->has_b_frames) {
		AVPacket NullPacket;
		InitNullPacket(NullPacket);
		avcodec_decode_video2(CodecContext, DecodeFrame, &FrameFinished, &NullPacket);
	}

	if (!FrameFinished)
		goto Error;

Error:
Done:;
}

FFMS_Frame *FFHaaliVideo::GetFrame(int n) {
	GetFrameCheck(n);

	if (LastFrameNum == n)
		return &LocalFrame;

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
		DecodeNextFrame(&StartTime);

		if (HasSeeked) {
			HasSeeked = false;

			if (StartTime < 0 || (CurrentFrame = Frames.FrameFromDTS(StartTime)) < 0) {
				// No idea where we are so go back a bit further
				if (n + SeekOffset == 0)
					throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
						"Frame accurate seeking is not possible in this file");

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
