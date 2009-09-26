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

#include "audiosource.h"



void FFHaaliAudio::Free(bool CloseCodec) {
	if (CloseCodec)
		avcodec_close(CodecContext);
	av_freep(&CodecContext);
}

void FFHaaliAudio::DecodeNextAudioBlock(int64_t *AFirstStartTime, int64_t *Count) {
	const size_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	int Ret = -1;
	*AFirstStartTime = -1;
	*Count = 0;
	uint8_t *Buf = &DecodingBuffer[0];
	AVPacket Packet;
	InitNullPacket(Packet);

	for (;;) {
		CComPtr<IMMFrame> pMMF;
		if (pMMC->ReadFrame(NULL, &pMMF) != S_OK)
			break;

		REFERENCE_TIME  Ts, Te;
		if (*AFirstStartTime < 0 && SUCCEEDED(pMMF->GetTime(&Ts, &Te)))
			*AFirstStartTime = Ts;

		if (pMMF->GetTrack() == AudioTrack) {
			BYTE *Data = NULL;
			if (FAILED(pMMF->GetPointer(&Data)))
				goto Done;

			Packet.data = Data;
			Packet.size = pMMF->GetActualDataLength();
			if (pMMF->IsSyncPoint() == S_OK)
				Packet.flags = AV_PKT_FLAG_KEY;
			else
				Packet.flags = 0;

			while (Packet.size > 0) {
				int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * 10;
				Ret = avcodec_decode_audio3(CodecContext, (int16_t *)Buf, &TempOutputBufSize, &Packet);

				if (Ret < 0) {// throw error or something?
					goto Done;
				}

				if (Ret > 0) {
					Packet.size -= Ret;
					Packet.data += Ret;
					Buf += TempOutputBufSize;
					if (SizeConst)
						*Count += TempOutputBufSize / SizeConst;
				}
			}

			goto Done;
        }
	}

Done:;
}

FFHaaliAudio::FFHaaliAudio(const char *SourceFile, int Track, FFMS_Index *Index, enum FFMS_Sources SourceMode)
						   : Res(FFSourceResources<FFMS_AudioSource>(this)), FFMS_AudioSource(SourceFile, Index, Track) {
	AVCodec *Codec = NULL;
	CodecContext = NULL;
	AudioTrack = Track;
	Frames = (*Index)[AudioTrack];

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

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"CodecPrivate", &pV, NULL))) {
						CodecPrivateSize = vtSize(pV);
						CodecPrivate.resize(CodecPrivateSize);
						vtCopy(pV, FFMS_GET_VECTOR_PTR(CodecPrivate));
					}

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"CodecID", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_BSTR))) {
						char ACodecID[2048];
						wcstombs(ACodecID, pV.bstrVal, 2000);

						int BitDepth = 0;
						pV.Clear();
						if (SUCCEEDED(pBag->Read(L"Audio.BitDepth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
							BitDepth = pV.uintVal;

						Codec = avcodec_find_decoder(MatroskaToFFCodecID(ACodecID, FFMS_GET_VECTOR_PTR(CodecPrivate), 0, BitDepth));
					}
				}
			}
			pU = NULL;
		}
	}

	CodecContext = avcodec_alloc_context();
	CodecContext->extradata = FFMS_GET_VECTOR_PTR(CodecPrivate);
	CodecContext->extradata_size = CodecPrivateSize;

	if (Codec == NULL)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Audio codec not found");

	InitializeCodecContextFromHaaliInfo(pBag, CodecContext);

	if (avcodec_open(CodecContext, Codec) < 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Could not open audio codec");

	Res.CloseCodec(true);

	// Always try to decode a frame to make sure all required parameters are known
	int64_t Dummy1, Dummy2;
	DecodeNextAudioBlock(&Dummy1, &Dummy2);

	pMMC->Seek(Frames[0].DTS, MKVF_SEEK_TO_PREV_KEYFRAME_STRICT);
	avcodec_flush_buffers(CodecContext);

	FillAP(AP, CodecContext, Frames);

	if (AP.SampleRate <= 0 || AP.BitsPerSample <= 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Codec returned zero size audio");

	AudioCache.Initialize((AP.Channels * AP.BitsPerSample) / 8, 50);
}

void FFHaaliAudio::GetAudio(void *Buf, int64_t Start, int64_t Count) {
	GetAudioCheck(Start, Count);

	const int64_t SizeConst = (av_get_bits_per_sample_format(CodecContext->sample_fmt) * CodecContext->channels) / 8;
	memset(Buf, 0, static_cast<size_t>(SizeConst * Count));
	bool HasSeeked = false;

	int PreDecBlocks = 0;
	uint8_t *DstBuf = static_cast<uint8_t *>(Buf);

	// Fill with everything in the cache
	int64_t CacheEnd = AudioCache.FillRequest(Start, Count, DstBuf);
	// Was everything in the cache?
	if (CacheEnd == Start + Count)
		return;

	int CurrentAudioBlock;
	// Is seeking required to decode the requested samples?
//	if (!(CurrentSample >= Start && CurrentSample <= CacheEnd)) {
	if (CurrentSample != CacheEnd) {
		PreDecBlocks = 15;
		CurrentAudioBlock = FFMAX((int64_t)Frames.FindClosestAudioKeyFrame(CacheEnd) - PreDecBlocks - 20, (int64_t)0);
		pMMC->Seek(Frames[CurrentAudioBlock].DTS, MKVF_SEEK_TO_PREV_KEYFRAME_STRICT);
		avcodec_flush_buffers(CodecContext);
		HasSeeked = true;
	} else {
		CurrentAudioBlock = Frames.FindClosestAudioKeyFrame(CurrentSample);
	}

	int64_t FirstTime, DecodeCount;

	do {
		DecodeNextAudioBlock(&FirstTime, &DecodeCount);

		if (HasSeeked) {
			CurrentAudioBlock = Frames.ClosestFrameFromDTS(FirstTime);	
			HasSeeked = false;
		}

		// Cache the block if enough blocks before it have been decoded to avoid garbage
		if (PreDecBlocks == 0) {
			AudioCache.CacheBlock(Frames[CurrentAudioBlock].SampleStart, DecodeCount, &DecodingBuffer[0]);
			CacheEnd = AudioCache.FillRequest(CacheEnd, Start + Count - CacheEnd, DstBuf + (CacheEnd - Start) * SizeConst);
		} else {
			PreDecBlocks--;
		}

		CurrentAudioBlock++;
		if (CurrentAudioBlock < static_cast<int>(Frames.size()))
			CurrentSample = Frames[CurrentAudioBlock].SampleStart;
	} while (Start + Count - CacheEnd > 0 && CurrentAudioBlock < static_cast<int>(Frames.size()));
}

#endif // HAALISOURCE
