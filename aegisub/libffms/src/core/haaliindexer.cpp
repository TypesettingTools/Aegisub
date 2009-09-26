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

#include "indexing.h"



FFHaaliIndexer::FFHaaliIndexer(const char *Filename, enum FFMS_Sources SourceMode) : FFMS_Indexer(Filename) {
	this->SourceMode = SourceMode;
	SourceFile = Filename;
	Duration = 0;
	for (int i = 0; i < 32; i++) {
		TrackType[i] = FFMS_TYPE_UNKNOWN;
		Codec[i] = NULL;
		CodecPrivateSize[i] = 0;
	}

	pMMC = HaaliOpenFile(SourceFile, SourceMode);

	CComQIPtr<IPropertyBag> pBag2 = pMMC;
	CComVariant pV2;
	if (SUCCEEDED(pBag2->Read(L"Duration", &pV2, NULL)) && SUCCEEDED(pV2.ChangeType(VT_UI8)))
		Duration = pV2.ullVal;

	NumTracks = 0;
	CComPtr<IEnumUnknown> pEU;
	if (SUCCEEDED(pMMC->EnumTracks(&pEU))) {
		CComPtr<IUnknown> pU;
		while (pEU->Next(1, &pU, NULL) == S_OK) {
			CComQIPtr<IPropertyBag> pBag = pU;
			PropertyBags[NumTracks] = pBag;

			if (pBag) {
				CComVariant pV;
				unsigned int FourCC = 0;

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"Type", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					TrackType[NumTracks] = HaaliTrackTypeToFFTrackType(pV.uintVal);

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"CodecPrivate", &pV, NULL))) {
					CodecPrivateSize[NumTracks] = vtSize(pV);
					CodecPrivate[NumTracks].resize(CodecPrivateSize[NumTracks]);
					vtCopy(pV, FFMS_GET_VECTOR_PTR(CodecPrivate[NumTracks]));
				}

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"FOURCC", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4))) {
					FourCC = pV.uintVal;
					
					// Reconstruct the missing codec private part for VC1
					std::vector<uint8_t> bihvect;
					bihvect.resize(sizeof(FFMS_BITMAPINFOHEADER));
					FFMS_BITMAPINFOHEADER *bih = reinterpret_cast<FFMS_BITMAPINFOHEADER *>(FFMS_GET_VECTOR_PTR(bihvect));
					memset(bih, 0, sizeof(FFMS_BITMAPINFOHEADER));
					bih->biSize = sizeof(FFMS_BITMAPINFOHEADER) + CodecPrivateSize[NumTracks];
					bih->biCompression = FourCC;
					bih->biBitCount = 24;
					bih->biPlanes = 1;

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"Video.PixelWidth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
						bih->biWidth = pV.uintVal;

					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"Video.PixelHeight", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
						bih->biHeight = pV.uintVal;

					CodecPrivate[NumTracks].insert(CodecPrivate[NumTracks].begin(), bihvect.begin(), bihvect.end());
					CodecPrivateSize[NumTracks] += sizeof(FFMS_BITMAPINFOHEADER);
				}

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"CodecID", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_BSTR))) {
					char CodecStr[2048];
					wcstombs(CodecStr, pV.bstrVal, 2000);

					int BitDepth = 0;
					pV.Clear();
					if (SUCCEEDED(pBag->Read(L"Audio.BitDepth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
						BitDepth = pV.uintVal;

					Codec[NumTracks] = avcodec_find_decoder(MatroskaToFFCodecID(CodecStr, FFMS_GET_VECTOR_PTR(CodecPrivate[NumTracks]), FourCC, BitDepth));
				}
			}

			pU = NULL;
			NumTracks++;
		}
	}
}

FFMS_Index *FFHaaliIndexer::DoIndexing() {
	std::vector<SharedAudioContext> AudioContexts(NumTracks, SharedAudioContext(true));
	std::vector<SharedVideoContext> VideoContexts(NumTracks, SharedVideoContext(true));

	std::auto_ptr<FFMS_Index> TrackIndices(new FFMS_Index(Filesize, Digest));
	TrackIndices->Decoder = FFMS_SOURCE_HAALIMPEG;
	if (SourceMode == FFMS_SOURCE_HAALIOGG)
		TrackIndices->Decoder = FFMS_SOURCE_HAALIOGG;

	for (int i = 0; i < NumTracks; i++) {
		TrackIndices->push_back(FFMS_Track(1, 1000000, TrackType[i]));
		
		if (TrackType[i] == FFMS_TYPE_VIDEO && Codec[i] && (VideoContexts[i].Parser = av_parser_init(Codec[i]->id))) {

			AVCodecContext *CodecContext = avcodec_alloc_context();
			CodecContext->extradata = FFMS_GET_VECTOR_PTR(CodecPrivate[i]);
			CodecContext->extradata_size = CodecPrivateSize[i];

			InitializeCodecContextFromHaaliInfo(PropertyBags[i], CodecContext);

			if (avcodec_open(CodecContext, Codec[i]) < 0) {
				av_freep(&CodecContext);
				throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
					"Could not open video codec");
			}

			VideoContexts[i].CodecContext = CodecContext;
			VideoContexts[i].Parser->flags = PARSER_FLAG_COMPLETE_FRAMES;
		}

		if (IndexMask & (1 << i) && TrackType[i] == FFMS_TYPE_AUDIO) {
			if (Codec[i] == NULL)	
				throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_UNSUPPORTED,
					"Audio codec not found");

			AVCodecContext *CodecContext = avcodec_alloc_context();
			CodecContext->extradata = FFMS_GET_VECTOR_PTR(CodecPrivate[i]);
			CodecContext->extradata_size = CodecPrivateSize[i];
			AudioContexts[i].CodecContext = CodecContext;

			if (avcodec_open(CodecContext, Codec[i]) < 0) {
				av_freep(&CodecContext);
				AudioContexts[i].CodecContext = NULL;
				throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
					"Could not open audio codec");
			}
		} else {
			IndexMask &= ~(1 << i);
		}
	}
//

	AVPacket TempPacket;
	InitNullPacket(TempPacket);

	for (;;) {
		CComPtr<IMMFrame> pMMF;
		if (pMMC->ReadFrame(NULL, &pMMF) != S_OK)
			break;

		REFERENCE_TIME Ts, Te;
		HRESULT hr = pMMF->GetTime(&Ts, &Te);

		if (IC) {
			if (Duration > 0) {
				if (SUCCEEDED(hr)) {
					if ((*IC)(Ts, Duration, ICPrivate))	
						throw FFMS_Exception(FFMS_ERROR_CANCELLED, FFMS_ERROR_USER,
							"Cancelled by user");
				}
			} else {
				if ((*IC)(0, 1, ICPrivate))	
					throw FFMS_Exception(FFMS_ERROR_CANCELLED, FFMS_ERROR_USER,
						"Cancelled by user");
			}
		}

		unsigned int Track = pMMF->GetTrack();
		pMMF->GetPointer(&TempPacket.data);
		TempPacket.size = pMMF->GetActualDataLength();

		// Only create index entries for video for now to save space
		if (TrackType[Track] == FFMS_TYPE_VIDEO) {
			uint8_t *OB;
			int OBSize;
			int RepeatPict = -1;

			if (VideoContexts[Track].Parser) {
				av_parser_parse2(VideoContexts[Track].Parser, VideoContexts[Track].CodecContext, &OB, &OBSize, TempPacket.data, TempPacket.size, ffms_av_nopts_value, ffms_av_nopts_value, ffms_av_nopts_value);
				RepeatPict = VideoContexts[Track].Parser->repeat_pict;
			}

			(*TrackIndices)[Track].push_back(TFrameInfo::VideoFrameInfo(Ts, RepeatPict, pMMF->IsSyncPoint() == S_OK));
		} else if (TrackType[Track] == FFMS_TYPE_AUDIO && (IndexMask & (1 << Track))) {
			int64_t StartSample = AudioContexts[Track].CurrentSample;
			AVCodecContext *AudioCodecContext = AudioContexts[Track].CodecContext;

			if (pMMF->IsSyncPoint() == S_OK)
				TempPacket.flags = AV_PKT_FLAG_KEY;
			else
				TempPacket.flags = 0;

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, &DecodingBuffer[0], &dbsize, &TempPacket);
				if (Ret < 0) {
					if (ErrorHandling == FFMS_IEH_ABORT) {
						throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
							"Audio decoding error");
					} else if (ErrorHandling == FFMS_IEH_CLEAR_TRACK) {
						(*TrackIndices)[Track].clear();
						IndexMask &= ~(1 << Track);					
						break;
					} else if (ErrorHandling == FFMS_IEH_STOP_TRACK) {
						IndexMask &= ~(1 << Track);					
						break;
					} else if (ErrorHandling == FFMS_IEH_IGNORE) {
						break;
					}
				}

				if (Ret > 0) {
					TempPacket.size -= Ret;
					TempPacket.data += Ret;
				}

				if (dbsize > 0)
					AudioContexts[Track].CurrentSample += (dbsize * 8) / (av_get_bits_per_sample_format(AudioCodecContext->sample_fmt) * AudioCodecContext->channels);

				if (DumpMask & (1 << Track))
					WriteAudio(AudioContexts[Track], TrackIndices.get(), Track, dbsize);
			}

			(*TrackIndices)[Track].push_back(TFrameInfo::AudioFrameInfo(Ts, StartSample,
				static_cast<unsigned int>(AudioContexts[Track].CurrentSample - StartSample), pMMF->IsSyncPoint() == S_OK));
		}
	}

	TrackIndices->Sort();
	return TrackIndices.release();
}

int FFHaaliIndexer::GetNumberOfTracks() { 
	return NumTracks;
}

FFMS_TrackType FFHaaliIndexer::GetTrackType(int Track) {
	return TrackType[Track];
}

const char *FFHaaliIndexer::GetTrackCodec(int Track) {
	if (Codec[Track])
		return Codec[Track]->name;
	else
		return "Unsupported codec/Unknown codec name";
}

#endif
