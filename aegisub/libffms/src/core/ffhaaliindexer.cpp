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



FFHaaliIndexer::FFHaaliIndexer(const char *Filename, int SourceMode, char *ErrorMsg, unsigned MsgSize) : FFMS_Indexer(Filename, ErrorMsg, MsgSize) {
	SourceFile = Filename;
	this->SourceMode = SourceMode;
	memset(TrackType, FFMS_TYPE_UNKNOWN, sizeof(TrackType));
	memset(Codec, 0, sizeof(Codec));
	memset(CodecPrivate, 0, sizeof(CodecPrivate));
	memset(CodecPrivateSize, 0, sizeof(CodecPrivateSize));
	Duration = 0;

	CLSID clsid = HAALI_MPEG_PARSER;
	if (SourceMode == 1)
		clsid = HAALI_OGG_PARSER;

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
		if (SourceMode == 0)
			snprintf(ErrorMsg, MsgSize, "Can't parse file, most likely a transport stream not cut at packet boundaries");
		else if (SourceMode == 1)
			snprintf(ErrorMsg, MsgSize, "Can't parse file");
		throw ErrorMsg;
	}

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
				if (SUCCEEDED(pBag->Read(L"FOURCC", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					FourCC = pV.uintVal;

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"CodecID", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_BSTR))) {
					char CodecID[2048];
					wcstombs(CodecID, pV.bstrVal, 2000);
					Codec[NumTracks] = avcodec_find_decoder(MatroskaToFFCodecID(CodecID, FFMS_GET_VECTOR_PTR(CodecPrivate[NumTracks]), FourCC));
				}
			}

			pU = NULL;
			NumTracks++;
		}
	}
}

FFMS_Index *FFHaaliIndexer::DoIndexing(char *ErrorMsg, unsigned MsgSize) {
	std::vector<SharedAudioContext> AudioContexts(NumTracks, SharedAudioContext(true));
	std::vector<SharedVideoContext> VideoContexts(NumTracks, SharedVideoContext(true));

	std::auto_ptr<FFMS_Index> TrackIndices(new FFMS_Index(Filesize, Digest));
	TrackIndices->Decoder = FFMS_SOURCE_HAALIMPEG;
	if (SourceMode == 1)
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
				snprintf(ErrorMsg, MsgSize, "Could not open video codec");
				return NULL;
			}

			VideoContexts[i].CodecContext = CodecContext;
			VideoContexts[i].Parser->flags = PARSER_FLAG_COMPLETE_FRAMES;
		}

		if (IndexMask & (1 << i) && TrackType[i] == FFMS_TYPE_AUDIO) {
			if (Codec[i] == NULL) {
				snprintf(ErrorMsg, MsgSize, "Audio codec not found");
				return NULL;
			}

			AVCodecContext *CodecContext = avcodec_alloc_context();
			CodecContext->extradata = FFMS_GET_VECTOR_PTR(CodecPrivate[i]);
			CodecContext->extradata_size = CodecPrivateSize[i];
			AudioContexts[i].CodecContext = CodecContext;

			if (avcodec_open(CodecContext, Codec[i]) < 0) {
				av_freep(&CodecContext);
				AudioContexts[i].CodecContext = NULL;
				snprintf(ErrorMsg, MsgSize, "Could not open audio codec");
				return NULL;
			}
		} else {
			IndexMask &= ~(1 << i);
		}
	}
//

	AVPacket TempPacket;
	InitNullPacket(&TempPacket);

	for (;;) {
		CComPtr<IMMFrame> pMMF;
		if (pMMC->ReadFrame(NULL, &pMMF) != S_OK)
			break;

		REFERENCE_TIME Ts, Te;
		HRESULT hr = pMMF->GetTime(&Ts, &Te);

		if (IC) {
			if (Duration > 0) {
				if (SUCCEEDED(hr)) {
					if ((*IC)(Ts, Duration, ICPrivate)) {
						snprintf(ErrorMsg, MsgSize, "Cancelled by user");
						return NULL;
					}
				}
			} else {
				if ((*IC)(0, 1, ICPrivate)) {
					snprintf(ErrorMsg, MsgSize, "Cancelled by user");
					return NULL;
				}
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
			(*TrackIndices)[Track].push_back(TFrameInfo::AudioFrameInfo(Ts, AudioContexts[Track].CurrentSample, pMMF->IsSyncPoint() == S_OK));
			AVCodecContext *AudioCodecContext = AudioContexts[Track].CodecContext;

			if (pMMF->IsSyncPoint() == S_OK)
				TempPacket.flags = AV_PKT_FLAG_KEY;
			else
				TempPacket.flags = 0;

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, &DecodingBuffer[0], &dbsize, &TempPacket);
				if (Ret < 0) {
					if (IgnoreDecodeErrors) {
						(*TrackIndices)[Track].clear();
						IndexMask &= ~(1 << Track);					
						break;
					} else {
						snprintf(ErrorMsg, MsgSize, "Audio decoding error");
						return NULL;
					}
				}

				if (Ret > 0) {
					TempPacket.size -= Ret;
					TempPacket.data += Ret;
				}

				if (dbsize > 0)
					AudioContexts[Track].CurrentSample += (dbsize * 8) / (av_get_bits_per_sample_format(AudioCodecContext->sample_fmt) * AudioCodecContext->channels);

				if (DumpMask & (1 << Track))
					WriteAudio(AudioContexts[Track], TrackIndices.get(), Track, dbsize, ErrorMsg, MsgSize);
			}
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
		return Codec[Track]->long_name;
	else
		return "Unsupported codec/Unknown codec name";
}

#endif
