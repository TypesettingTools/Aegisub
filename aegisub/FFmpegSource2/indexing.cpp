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

#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <memory>
#include <errno.h>
#include "indexing.h"

#ifdef __UNIX__
#define _fseeki64 fseeko
#define _ftelli64 ftello
#define _snprintf snprintf
#endif

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "MatroskaParser.h"
#include "stdiostream.h"
}

#ifdef _WIN32
#	define _WIN32_DCOM
#	include <windows.h>
#	include <tchar.h>
#	include <atlbase.h>
#	include <dshow.h>
#	include "CoParser.h"
#	include <initguid.h>
#	include "guids.h"
#endif

class MatroskaAudioContext : public SharedAudioContext {
public:
	CompressedStream *CS;
	uint8_t *CodecPrivate;

	MatroskaAudioContext() {
		CS = NULL;
		CodecPrivate = NULL;
	}

	~MatroskaAudioContext() {
		if (CTX) {
			avcodec_close(CTX);
			av_free(CTX);
		}
		if (CS)
			cs_Destroy(CS);
		delete[] CodecPrivate;
	}
};

class FFAudioContext : public SharedAudioContext {
public:
	~FFAudioContext() {
		if (CTX)
			avcodec_close(CTX);
	}
};

#ifdef HAALISOURCE
class HaaliIndexMemory {
private:
	MatroskaAudioContext *AudioContexts;
public:
	HaaliIndexMemory(int Tracks, MatroskaAudioContext *&AudioContexts) {
		AudioContexts = new MatroskaAudioContext[Tracks];
		this->AudioContexts = AudioContexts;
	}

	~HaaliIndexMemory() {
		delete[] AudioContexts;
	}
};
#endif

class MatroskaIndexMemory {
private:
	MatroskaAudioContext *AudioContexts;
	MatroskaFile *MF;
	MatroskaReaderContext *MC;
public:
	MatroskaIndexMemory(int Tracks, MatroskaAudioContext *&AudioContexts, MatroskaFile *MF, MatroskaReaderContext *MC) {
		AudioContexts = new MatroskaAudioContext[Tracks];
		this->AudioContexts = AudioContexts;
		this->MF = MF;
		this->MC = MC;
	}

	~MatroskaIndexMemory() {
		delete[] AudioContexts;
		mkv_Close(MF);
		fclose(MC->ST.fp);
	}
};

class FFIndexMemory {
private:
	FFAudioContext *AudioContexts;
	AVFormatContext *FormatContext;
public:
	FFIndexMemory(int Tracks, FFAudioContext *&AudioContexts, AVFormatContext *&FormatContext) {
		AudioContexts = new FFAudioContext[Tracks];
		this->AudioContexts = AudioContexts;
		this->FormatContext = FormatContext;
	}

	~FFIndexMemory() {
		delete[] AudioContexts;
		av_close_input_file(FormatContext);
	}
};

static bool DTSComparison(TFrameInfo FI1, TFrameInfo FI2) {
	return FI1.DTS < FI2.DTS;
}

static void SortTrackIndex(FFIndex *Index) {
	for (FFIndex::iterator Cur=Index->begin(); Cur!=Index->end(); Cur++)
		std::sort(Cur->begin(), Cur->end(), DTSComparison);
}

bool FFIndexer::WriteAudio(SharedAudioContext &AudioContext, FFIndex *Index, int Track, int DBSize, char *ErrorMsg, unsigned MsgSize) {
	// Delay writer creation until after an audio frame has been decoded. This ensures that all parameters are known when writing the headers.
	if (DBSize > 0) {
		if (!AudioContext.W64W) {
			TAudioProperties AP;
			FillAP(AP, AudioContext.CTX, (*Index)[Track]);
			char *WName = new char[(*ANC)(SourceFile, Track, &AP, NULL, ANCPrivate)];
			(*ANC)(SourceFile, Track, &AP, WName, ANCPrivate);
			std::string WN(WName);
			delete[] WName;
			try {
				AudioContext.W64W = new Wave64Writer(WN.c_str(), av_get_bits_per_sample_format(AudioContext.CTX->sample_fmt),
					AudioContext.CTX->channels, AudioContext.CTX->sample_rate, AudioFMTIsFloat(AudioContext.CTX->sample_fmt));
			} catch (...) {
				_snprintf(ErrorMsg, MsgSize, "Failed to write wave data");
				return false;
			}
		}

		AudioContext.W64W->WriteData(DecodingBuffer, DBSize);
	}

	return true;
}

#ifdef HAALISOURCE
FFHaaliIndexer::FFHaaliIndexer(const char *Filename, int SourceMode, char *ErrorMsg, unsigned MsgSize) {
	SourceFile = Filename;
	this->SourceMode = SourceMode;
	memset(TrackType, FFMS_TYPE_UNKNOWN, sizeof(TrackType));
	memset(Codec, 0, sizeof(Codec));
	memset(CodecPrivate, 0, sizeof(CodecPrivate));
	memset(CodecPrivateSize, 0, sizeof(CodecPrivateSize));

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

	NumTracks = 0;
	CComPtr<IEnumUnknown> pEU;
	if (SUCCEEDED(pMMC->EnumTracks(&pEU))) {
		CComPtr<IUnknown> pU;
		while (pEU->Next(1, &pU, NULL) == S_OK) {
			CComQIPtr<IPropertyBag> pBag = pU;

			if (pBag) {
				CComVariant pV;

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"Type", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					TrackType[NumTracks] = HaaliTrackTypeToFFTrackType(pV.uintVal);

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"CodecPrivate", &pV, NULL))) {
					CodecPrivateSize[NumTracks] = vtSize(pV);
					CodecPrivate[NumTracks] = new uint8_t[CodecPrivateSize[NumTracks]];
					vtCopy(pV, CodecPrivate[NumTracks]);
				}

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"CodecID", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_BSTR))) {
					char CodecID[2048];
					wcstombs(CodecID, pV.bstrVal, 2000);
					Codec[NumTracks] = avcodec_find_decoder(MatroskaToFFCodecID(CodecID, CodecPrivate[NumTracks]));
				}
			}

			pU = NULL;
			NumTracks++;
		}
	}
}

FFIndex *FFHaaliIndexer::DoIndexing(char *ErrorMsg, unsigned MsgSize) {
	// Audio stuff
	MatroskaAudioContext *AudioContexts;
	HaaliIndexMemory IM = HaaliIndexMemory(NumTracks, AudioContexts);

	std::auto_ptr<FFIndex> TrackIndices(new FFIndex());
	TrackIndices->Decoder = 2;
	if (SourceMode == 1)
		TrackIndices->Decoder = 3;


	for (int i = 0; i < NumTracks; i++) {
		TrackIndices->push_back(FFTrack(1, 1000000000, TrackType[i]));

		if (IndexMask & (1 << i) && TrackType[i] == FFMS_TYPE_AUDIO) {
			AVCodecContext *AudioCodecContext = avcodec_alloc_context();
			AudioCodecContext->extradata = CodecPrivate[i];
			AudioCodecContext->extradata_size = CodecPrivateSize[i];
			AudioContexts[i].CTX = AudioCodecContext;

			if (Codec[i] == NULL) {
				av_free(AudioCodecContext);
				AudioContexts[i].CTX = NULL;
				_snprintf(ErrorMsg, MsgSize, "Audio codec not found");
				return NULL;
			}

			if (avcodec_open(AudioCodecContext, Codec[i]) < 0) {
				av_free(AudioCodecContext);
				AudioContexts[i].CTX = NULL;
				_snprintf(ErrorMsg, MsgSize, "Could not open audio codec");
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
		if (IC) {
			if ((*IC)(0, 1, ICPrivate)) {
				_snprintf(ErrorMsg, MsgSize, "Cancelled by user");
				return NULL;
			}
		}

		CComPtr<IMMFrame> pMMF;
		if (pMMC->ReadFrame(NULL, &pMMF) != S_OK)
			break;

		REFERENCE_TIME Ts, Te;
		HRESULT hr = pMMF->GetTime(&Ts, &Te);

		unsigned int Track = pMMF->GetTrack();

		// Only create index entries for video for now to save space
		if (TrackType[Track] == FFMS_TYPE_VIDEO) {
			(*TrackIndices)[Track].push_back(TFrameInfo(Ts, pMMF->IsSyncPoint() == S_OK));
		} else if (TrackType[Track] == FFMS_TYPE_AUDIO && (IndexMask & (1 << Track))) {
			(*TrackIndices)[Track].push_back(TFrameInfo(Ts, AudioContexts[Track].CurrentSample, 0 /* FIXME? */, pMMF->GetActualDataLength(), pMMF->IsSyncPoint() == S_OK));
			AVCodecContext *AudioCodecContext = AudioContexts[Track].CTX;
			pMMF->GetPointer(&TempPacket.data);
			TempPacket.size = pMMF->GetActualDataLength();

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, DecodingBuffer, &dbsize, &TempPacket);
				if (Ret < 0) {
					if (IgnoreDecodeErrors) {
						(*TrackIndices)[Track].clear();
						IndexMask &= ~(1 << Track);					
						break;
					} else {
						_snprintf(ErrorMsg, MsgSize, "Audio decoding error");
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

	SortTrackIndex(TrackIndices.get());
	return TrackIndices.release();
}
#endif

FFMatroskaIndexer::FFMatroskaIndexer(const char *Filename, char *ErrorMsg, unsigned MsgSize) {
	memset(Codec, 0, sizeof(Codec));
	SourceFile = Filename;
	char ErrorMessage[256];

	InitStdIoStream(&MC.ST);
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

	for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++) {
		TrackInfo *TI = mkv_GetTrackInfo(MF, i);
		Codec[i] = avcodec_find_decoder(MatroskaToFFCodecID(TI->CodecID, TI->CodecPrivate));
	}
}

FFIndex *FFMatroskaIndexer::DoIndexing(char *ErrorMsg, unsigned MsgSize) {
	char ErrorMessage[256];

	// Audio stuff
	MatroskaAudioContext *AudioContexts;
	MatroskaIndexMemory IM = MatroskaIndexMemory(mkv_GetNumTracks(MF), AudioContexts, MF, &MC);

	for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++) {
		TrackInfo *TI = mkv_GetTrackInfo(MF, i);
		if (IndexMask & (1 << i) && TI->Type == TT_AUDIO) {
			AVCodecContext *AudioCodecContext = avcodec_alloc_context();
			AudioCodecContext->extradata = (uint8_t *)TI->CodecPrivate;
			AudioCodecContext->extradata_size = TI->CodecPrivateSize;
			AudioContexts[i].CTX = AudioCodecContext;

			if (TI->CompEnabled) {
				AudioContexts[i].CS = cs_Create(MF, i, ErrorMessage, sizeof(ErrorMessage));
				if (AudioContexts[i].CS == NULL) {
					av_free(AudioCodecContext);
					AudioContexts[i].CTX = NULL;
					_snprintf(ErrorMsg, MsgSize, "Can't create decompressor: %s", ErrorMessage);
					return NULL;
				}
			}

			AVCodec *AudioCodec = Codec[i];
			if (AudioCodec == NULL) {
				av_free(AudioCodecContext);
				AudioContexts[i].CTX = NULL;
				_snprintf(ErrorMsg, MsgSize, "Audio codec not found");
				return NULL;
			}

			if (avcodec_open(AudioCodecContext, AudioCodec) < 0) {
				av_free(AudioCodecContext);
				AudioContexts[i].CTX = NULL;
				_snprintf(ErrorMsg, MsgSize, "Could not open audio codec");
				return NULL;
			}
		} else {
			IndexMask &= ~(1 << i);
		}
	}

	//

	int64_t CurrentPos = _ftelli64(MC.ST.fp);
	_fseeki64(MC.ST.fp, 0, SEEK_END);
	int64_t SourceSize = _ftelli64(MC.ST.fp);
	_fseeki64(MC.ST.fp, CurrentPos, SEEK_SET);

	std::auto_ptr<FFIndex> TrackIndices(new FFIndex());
	TrackIndices->Decoder = 1;

	for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++)
		TrackIndices->push_back(FFTrack(mkv_TruncFloat(mkv_GetTrackInfo(MF, i)->TimecodeScale), 1000000, HaaliTrackTypeToFFTrackType(mkv_GetTrackInfo(MF, i)->Type)));

	ulonglong StartTime, EndTime, FilePos;
	unsigned int Track, FrameFlags, FrameSize;
	AVPacket TempPacket;
	InitNullPacket(&TempPacket);

	while (mkv_ReadFrame(MF, 0, &Track, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
		// Update progress
		if (IC) {
			if ((*IC)(_ftelli64(MC.ST.fp), SourceSize, ICPrivate)) {
				_snprintf(ErrorMsg, MsgSize, "Cancelled by user");
				return NULL;
			}
		}
	
		// Only create index entries for video for now to save space
		if (mkv_GetTrackInfo(MF, Track)->Type == TT_VIDEO) {
			(*TrackIndices)[Track].push_back(TFrameInfo(StartTime, FilePos, FrameSize, (FrameFlags & FRAME_KF) != 0));
		} else if (mkv_GetTrackInfo(MF, Track)->Type == TT_AUDIO && (IndexMask & (1 << Track))) {
			(*TrackIndices)[Track].push_back(TFrameInfo(StartTime, AudioContexts[Track].CurrentSample, FilePos, FrameSize, (FrameFlags & FRAME_KF) != 0));
			ReadFrame(FilePos, FrameSize, AudioContexts[Track].CS, MC, ErrorMsg, MsgSize);
			AVCodecContext *AudioCodecContext = AudioContexts[Track].CTX;
			TempPacket.data = MC.Buffer;
			TempPacket.size = FrameSize;
			if ((FrameFlags & FRAME_KF) != 0)
				TempPacket.flags = PKT_FLAG_KEY;

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, DecodingBuffer, &dbsize, &TempPacket);
				if (Ret < 0) {
					if (IgnoreDecodeErrors) {
						(*TrackIndices)[Track].clear();
						IndexMask &= ~(1 << Track);					
						break;
					} else {
						_snprintf(ErrorMsg, MsgSize, "Audio decoding error");
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

	SortTrackIndex(TrackIndices.get());
	return TrackIndices.release();
}

FFIndexer *FFIndexer::CreateFFIndexer(const char *Filename, char *ErrorMsg, unsigned MsgSize) {
	AVFormatContext *FormatContext = NULL;

	if (av_open_input_file(&FormatContext, Filename, NULL, 0, NULL) != 0) {
		_snprintf(ErrorMsg, MsgSize, "Can't open '%s'", Filename);
		return NULL;
	}

	// Do matroska indexing instead?
	if (!strcmp(FormatContext->iformat->name, "matroska")) {
		av_close_input_file(FormatContext);
		return new FFMatroskaIndexer(Filename, ErrorMsg, MsgSize);
	}

#ifdef HAALISOURCE
	// Do haali ts indexing instead?
	if (!strcmp(FormatContext->iformat->name, "mpeg") || !strcmp(FormatContext->iformat->name, "mpegts")) {
		av_close_input_file(FormatContext);
		return new FFHaaliIndexer(Filename, 0, ErrorMsg, MsgSize);
	}

	if (!strcmp(FormatContext->iformat->name, "ogg")) {
		av_close_input_file(FormatContext);
		return new FFHaaliIndexer(Filename, 1, ErrorMsg, MsgSize);
	}
#endif

	return new FFLAVFIndexer(Filename, FormatContext, ErrorMsg, MsgSize);
}

FFIndexer::FFIndexer() {
	DecodingBuffer = new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE * 5];
}

FFIndexer::~FFIndexer() {
	delete[] DecodingBuffer;
}

FFLAVFIndexer::FFLAVFIndexer(const char *Filename, AVFormatContext *FormatContext, char *ErrorMsg, unsigned MsgSize) {
	SourceFile = Filename;
	this->FormatContext = FormatContext;
	IsIndexing = false;
	
	if (av_find_stream_info(FormatContext) < 0) {
		av_close_input_file(FormatContext);
		_snprintf(ErrorMsg, MsgSize, "Couldn't find stream information");
		throw ErrorMsg;
	}
}

FFLAVFIndexer::~FFLAVFIndexer() {
	if (!IsIndexing)
		av_close_input_file(FormatContext);
}

FFIndex *FFLAVFIndexer::DoIndexing(char *ErrorMsg, unsigned MsgSize) {
	IsIndexing = true;

	// Audio stuff
	FFAudioContext *AudioContexts;
	FFIndexMemory IM = FFIndexMemory(FormatContext->nb_streams, AudioContexts, FormatContext);

	for (unsigned int i = 0; i < FormatContext->nb_streams; i++) {
		if (IndexMask & (1 << i) && FormatContext->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) {
			AVCodecContext *AudioCodecContext = FormatContext->streams[i]->codec;

			AVCodec *AudioCodec = avcodec_find_decoder(AudioCodecContext->codec_id);
			if (AudioCodec == NULL) {
				_snprintf(ErrorMsg, MsgSize, "Audio codec not found");
				return NULL;
			}

			if (avcodec_open(AudioCodecContext, AudioCodec) < 0) {
				_snprintf(ErrorMsg, MsgSize, "Could not open audio codec");
				return NULL;
			}

			AudioContexts[i].CTX = AudioCodecContext;
		} else {
			IndexMask &= ~(1 << i);
		}
	}

	//

	std::auto_ptr<FFIndex> TrackIndices(new FFIndex());
	TrackIndices->Decoder = 0;

	for (unsigned int i = 0; i < FormatContext->nb_streams; i++)
		TrackIndices->push_back(FFTrack((int64_t)FormatContext->streams[i]->time_base.num * 1000, 
		FormatContext->streams[i]->time_base.den,
		static_cast<FFMS_TrackType>(FormatContext->streams[i]->codec->codec_type)));

	AVPacket Packet, TempPacket;
	InitNullPacket(&Packet);
	InitNullPacket(&TempPacket);
	while (av_read_frame(FormatContext, &Packet) >= 0) {
		// Update progress
		if (IC) {
			if ((*IC)(FormatContext->pb->pos, FormatContext->file_size, ICPrivate)) {
				_snprintf(ErrorMsg, MsgSize, "Cancelled by user");
				return NULL;
			}
		}

		// Only create index entries for video for now to save space
		if (FormatContext->streams[Packet.stream_index]->codec->codec_type == CODEC_TYPE_VIDEO) {
			(*TrackIndices)[Packet.stream_index].push_back(TFrameInfo(Packet.dts, (Packet.flags & PKT_FLAG_KEY) ? 1 : 0));
		} else if (FormatContext->streams[Packet.stream_index]->codec->codec_type == CODEC_TYPE_AUDIO && (IndexMask & (1 << Packet.stream_index))) {
			(*TrackIndices)[Packet.stream_index].push_back(TFrameInfo(Packet.dts, AudioContexts[Packet.stream_index].CurrentSample, (Packet.flags & PKT_FLAG_KEY) ? 1 : 0));
			AVCodecContext *AudioCodecContext = FormatContext->streams[Packet.stream_index]->codec;
			TempPacket.data = Packet.data;
			TempPacket.size = Packet.size;

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, DecodingBuffer, &dbsize, &TempPacket);
				if (Ret < 0) {
					if (IgnoreDecodeErrors) {
						(*TrackIndices)[Packet.stream_index].clear();
						IndexMask &= ~(1 << Packet.stream_index);					
						break;
					} else {
						_snprintf(ErrorMsg, MsgSize, "Audio decoding error");
						return NULL;
					}
				}

				if (Ret > 0) {
					TempPacket.size -= Ret;
					TempPacket.data += Ret;
				}

				if (dbsize > 0)
					AudioContexts[Packet.stream_index].CurrentSample += (dbsize * 8) / (av_get_bits_per_sample_format(AudioCodecContext->sample_fmt) * AudioCodecContext->channels);

				if (DumpMask & (1 << Packet.stream_index))
					WriteAudio(AudioContexts[Packet.stream_index], TrackIndices.get(), Packet.stream_index, dbsize, ErrorMsg, MsgSize);
			}
		}

		av_free_packet(&Packet);
	}

	SortTrackIndex(TrackIndices.get());
	return TrackIndices.release();
}
