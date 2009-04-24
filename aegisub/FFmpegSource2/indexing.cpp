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
#include "wave64writer.h"

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

class MatroskaAudioContext {
public:
	Wave64Writer *W64W;
	AVCodecContext *CTX;
	CompressedStream *CS;
	int64_t CurrentSample;

	MatroskaAudioContext() {
		W64W = NULL;
		CTX = NULL;
		CS = NULL;
		CurrentSample = 0;
	}

	~MatroskaAudioContext() {
		delete W64W;
		if (CTX) {
			avcodec_close(CTX);
			av_free(CTX);
		}
		if (CS)
			cs_Destroy(CS);
	}
};

class FFAudioContext {
public:
	Wave64Writer *W64W;
	AVCodecContext *CTX;
	int64_t CurrentSample;

	FFAudioContext() {
		W64W = NULL;
		CTX = NULL;
		CurrentSample = 0;
	}

	~FFAudioContext() {
		delete W64W;
		if (CTX)
			avcodec_close(CTX);
	}
};

#ifdef HAALISOURCE
class HaaliIndexMemory {
private:
	int16_t *DecodingBuffer;
	MatroskaAudioContext *AudioContexts;
public:
	HaaliIndexMemory(int Tracks, int16_t *&DecodingBuffer, MatroskaAudioContext *&AudioContexts) {
		DecodingBuffer = new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE*10];
		AudioContexts = new MatroskaAudioContext[Tracks];
		this->DecodingBuffer = DecodingBuffer;
		this->AudioContexts = AudioContexts;
	}

	~HaaliIndexMemory() {
		delete [] DecodingBuffer;
		delete [] AudioContexts;
	}
};
#endif

class MatroskaIndexMemory {
private:
	int16_t *DecodingBuffer;
	MatroskaAudioContext *AudioContexts;
	MatroskaFile *MF;
	MatroskaReaderContext *MC;
public:
	MatroskaIndexMemory(int Tracks, int16_t *&DecodingBuffer, MatroskaAudioContext *&AudioContexts, MatroskaFile *MF, MatroskaReaderContext *MC) {
		DecodingBuffer = new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE*10];
		AudioContexts = new MatroskaAudioContext[Tracks];
		this->DecodingBuffer = DecodingBuffer;
		this->AudioContexts = AudioContexts;
		this->MF = MF;
		this->MC = MC;
	}

	~MatroskaIndexMemory() {
		delete [] DecodingBuffer;
		delete [] AudioContexts;
		mkv_Close(MF);
		fclose(MC->ST.fp);
	}
};

class FFIndexMemory {
private:
	int16_t *DecodingBuffer;
	FFAudioContext *AudioContexts;
	AVFormatContext *FormatContext;
public:
	FFIndexMemory(int Tracks, int16_t *&DecodingBuffer, FFAudioContext *&AudioContexts, AVFormatContext *&FormatContext) {
		DecodingBuffer = new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE*10];
		AudioContexts = new FFAudioContext[Tracks];
		this->DecodingBuffer = DecodingBuffer;
		this->AudioContexts = AudioContexts;
		this->FormatContext = FormatContext;
	}

	~FFIndexMemory() {
		delete [] DecodingBuffer;
		delete [] AudioContexts;
		av_close_input_file(FormatContext);
	}
};

static bool DTSComparison(FrameInfo FI1, FrameInfo FI2) {
	return FI1.DTS < FI2.DTS;
}

static void SortTrackIndices(FrameIndex *TrackIndices) {
	for (FrameIndex::iterator Cur=TrackIndices->begin(); Cur!=TrackIndices->end(); Cur++)
		std::sort(Cur->begin(), Cur->end(), DTSComparison);
}

int WriteIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	std::ofstream Index(IndexFile, std::ios::out | std::ios::binary | std::ios::trunc);

	if (!Index.is_open()) {
		_snprintf(ErrorMsg, MsgSize, "Failed to open '%s' for writing", IndexFile);
		return 1;
	}

	// Write the index file header
	IndexHeader IH;
	IH.Id = INDEXID;
	IH.Version = INDEXVERSION;
	IH.Tracks = TrackIndices->size();
	IH.Decoder = TrackIndices->Decoder;
	Index.write(reinterpret_cast<char *>(&IH), sizeof(IH));
	
	for (unsigned int i = 0; i < IH.Tracks; i++) {
		int TT = (*TrackIndices)[i].TT;
		Index.write(reinterpret_cast<char *>(&TT), sizeof(TT));
		int64_t Num = (*TrackIndices)[i].TB.Num;
		Index.write(reinterpret_cast<char *>(&Num), sizeof(Num));
		int64_t Den = (*TrackIndices)[i].TB.Den;
		Index.write(reinterpret_cast<char *>(&Den), sizeof(Den));
		size_t Frames = (*TrackIndices)[i].size();
		Index.write(reinterpret_cast<char *>(&Frames), sizeof(Frames));

		for (size_t j = 0; j < Frames; j++)
			Index.write(reinterpret_cast<char *>(&(TrackIndices->at(i)[j])), sizeof(FrameInfo));
	}

	return 0;
}

#ifdef HAALISOURCE
static FrameIndex *MakeHaaliIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, int SourceMode, IndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize) {
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	CLSID clsid = HAALI_TS_Parser;
	if (SourceMode == 1)
		clsid = HAALI_OGM_Parser;

	CComPtr<IMMContainer> pMMC;
	if (FAILED(pMMC.CoCreateInstance(clsid))) {
		_snprintf(ErrorMsg, MsgSize, "Can't create parser");
		return NULL;
	}

	CComPtr<IMemAlloc>    pMA;
	if (FAILED(pMA.CoCreateInstance(CLSID_MemAlloc))) {
		_snprintf(ErrorMsg, MsgSize, "Can't create memory allocator");
		return NULL;
	}

	CComPtr<IMMStream>    pMS;
	if (FAILED(pMS.CoCreateInstance(CLSID_DiskFile))) {
		_snprintf(ErrorMsg, MsgSize, "Can't create disk file reader");
		return NULL;
	}

	WCHAR WSourceFile[2048];
	mbstowcs(WSourceFile, SourceFile, 2000);
	CComQIPtr<IMMStreamOpen> pMSO(pMS);
	if (FAILED(pMSO->Open(WSourceFile))) {
		_snprintf(ErrorMsg, MsgSize, "Can't open file");
		return NULL;
	}

	if (FAILED(pMMC->Open(pMS, 0, NULL, pMA))) {
		_snprintf(ErrorMsg, MsgSize, "Can't parse file");
		return NULL;
	}

	int NumTracks = 0;
	CComPtr<IEnumUnknown> pEU;
	if (SUCCEEDED(pMMC->EnumTracks(&pEU))) {
		CComPtr<IUnknown> pU;
		while (pEU->Next(1, &pU, NULL) == S_OK) {
			NumTracks++;
			pU = NULL;
		}
	}

	// Audio stuff

	int16_t *db;
	MatroskaAudioContext *AudioContexts;
	HaaliIndexMemory IM = HaaliIndexMemory(NumTracks, db, AudioContexts);

	FrameIndex *TrackIndices = new FrameIndex();
	TrackIndices->Decoder = 2;
	if (SourceMode == 1)
		TrackIndices->Decoder = 3;

	int TrackTypes[32];
	int CurrentTrack = 0;
	pEU = NULL;
	if (SUCCEEDED(pMMC->EnumTracks(&pEU))) {
		CComPtr<IUnknown> pU;
		while (pEU->Next(1, &pU, NULL) == S_OK) {
			CComQIPtr<IPropertyBag> pBag = pU;
			BSTR CodecID = NULL;
			TrackTypes[CurrentTrack] = -200;
			uint8_t * CodecPrivate = NULL;
			int CodecPrivateSize = 0;

			if (pBag) {
				CComVariant pV;

				if (pBag->Read(L"CodecID", &pV, NULL) == S_OK)
					CodecID = pV.bstrVal;

				if (pBag->Read(L"Type", &pV, NULL) == S_OK)
					TrackTypes[CurrentTrack] = pV.uintVal;

				if (pBag->Read(L"CodecPrivate", &pV, NULL) == S_OK) {
					CodecPrivate = (uint8_t *)pV.parray->pvData;
					CodecPrivateSize = pV.parray->cbElements;
				}
			}

			TrackIndices->push_back(FrameInfoVector(1, 1000000000, TrackTypes[CurrentTrack] - 1));

			if (IndexMask & (1 << CurrentTrack) && TrackTypes[CurrentTrack] == TT_AUDIO) {
				AVCodecContext *AudioCodecContext = avcodec_alloc_context();
				AudioCodecContext->extradata = CodecPrivate;
				AudioCodecContext->extradata_size = CodecPrivateSize;
				AudioContexts[CurrentTrack].CTX = AudioCodecContext;

				char ACodecID[2048];
				wcstombs(ACodecID, CodecID, 2000);
				AVCodec *AudioCodec = avcodec_find_decoder(MatroskaToFFCodecID(ACodecID, NULL));
				if (AudioCodec == NULL) {
					av_free(AudioCodecContext);
					AudioContexts[CurrentTrack].CTX = NULL;
					_snprintf(ErrorMsg, MsgSize, "Audio codec not found");
					return NULL;
				}

				if (avcodec_open(AudioCodecContext, AudioCodec) < 0) {
					av_free(AudioCodecContext);
					AudioContexts[CurrentTrack].CTX = NULL;
					_snprintf(ErrorMsg, MsgSize, "Could not open audio codec");
					return NULL;
				}
			} else {
				IndexMask &= ~(1 << CurrentTrack);
			}

			pU = NULL;
			CurrentTrack++;
		}
	}
//

	AVPacket TempPacket;
	init_null_packet(&TempPacket);

	for (;;) {
		if (IP) {
			if ((*IP)(0, 0, 1, Private)) {
				_snprintf(ErrorMsg, MsgSize, "Cancelled by user");
				delete TrackIndices;
				return NULL;
			}
		}

		CComPtr<IMMFrame> pMMF;
		if (pMMC->ReadFrame(NULL, &pMMF) != S_OK)
			break;

		REFERENCE_TIME  Ts, Te;
		HRESULT hr = pMMF->GetTime(&Ts, &Te);

		unsigned int CurrentTrack = pMMF->GetTrack();

		// Only create index entries for video for now to save space
		if (TrackTypes[CurrentTrack] == TT_VIDEO) {
			(*TrackIndices)[CurrentTrack].push_back(FrameInfo(Ts, pMMF->IsSyncPoint() == S_OK));
		} else if (TrackTypes[CurrentTrack] == TT_AUDIO && (IndexMask & (1 << CurrentTrack))) {
			(*TrackIndices)[CurrentTrack].push_back(FrameInfo(AudioContexts[CurrentTrack].CurrentSample, 0 /* FIXME? */, pMMF->GetActualDataLength(), pMMF->IsSyncPoint() == S_OK));
			AVCodecContext *AudioCodecContext = AudioContexts[CurrentTrack].CTX;
			pMMF->GetPointer(&TempPacket.data);
			TempPacket.size = pMMF->GetActualDataLength();

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, db, &dbsize, &TempPacket);
				if (Ret < 0) {
					if (IgnoreDecodeErrors) {
						(*TrackIndices)[CurrentTrack].clear();
						IndexMask &= ~(1 << CurrentTrack);					
						break;
					} else {
						_snprintf(ErrorMsg, MsgSize, "Audio decoding error");
						delete TrackIndices;
						return NULL;
					}
				}

				if (Ret > 0) {
					TempPacket.size -= Ret;
					TempPacket.data += Ret;
				}

				if (dbsize > 0)
					AudioContexts[CurrentTrack].CurrentSample += (dbsize * 8) / (av_get_bits_per_sample_format(AudioCodecContext->sample_fmt) * AudioCodecContext->channels);

				if (dbsize > 0 && (DumpMask & (1 << CurrentTrack))) {
					// Delay writer creation until after an audio frame has been decoded. This ensures that all parameters are known when writing the headers.
					if (!AudioContexts[CurrentTrack].W64W) {
						char ABuf[50];
						std::string WN(AudioFile);
						_snprintf(ABuf, sizeof(ABuf), ".%02d.delay.%d.w64", CurrentTrack, 0);
						WN += ABuf;
						
						AudioContexts[CurrentTrack].W64W = new Wave64Writer(WN.c_str(), av_get_bits_per_sample_format(AudioCodecContext->sample_fmt),
							AudioCodecContext->channels, AudioCodecContext->sample_rate, AudioFMTIsFloat(AudioCodecContext->sample_fmt));
					}

					AudioContexts[CurrentTrack].W64W->WriteData(db, dbsize);
				}
			}
		}
	}

	SortTrackIndices(TrackIndices);
	return TrackIndices;
}
#endif

static FrameIndex *MakeMatroskaIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, IndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize) {
	MatroskaFile *MF;
	char ErrorMessage[256];
	MatroskaReaderContext MC;
	MC.Buffer = NULL;
	MC.BufferSize = 0;

	InitStdIoStream(&MC.ST);
	MC.ST.fp = fopen(SourceFile, "rb");
	if (MC.ST.fp == NULL) {
		_snprintf(ErrorMsg, MsgSize, "Can't open '%s': %s", SourceFile, strerror(errno));
		return NULL;
	}

	setvbuf(MC.ST.fp, NULL, _IOFBF, CACHESIZE);

	MF = mkv_OpenEx(&MC.ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
	if (MF == NULL) {
		fclose(MC.ST.fp);
		_snprintf(ErrorMsg, MsgSize, "Can't parse Matroska file: %s", ErrorMessage);
		return NULL;
	}

	// Audio stuff

	int16_t *db;
	MatroskaAudioContext *AudioContexts;
	MatroskaIndexMemory IM = MatroskaIndexMemory(mkv_GetNumTracks(MF), db, AudioContexts, MF, &MC);

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

			AVCodec *AudioCodec = avcodec_find_decoder(MatroskaToFFCodecID(TI->CodecID, TI->CodecPrivate));
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

	FrameIndex *TrackIndices = new FrameIndex();
	TrackIndices->Decoder = 1;

	for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++)
		TrackIndices->push_back(FrameInfoVector(mkv_TruncFloat(mkv_GetTrackInfo(MF, i)->TimecodeScale), 1000000, mkv_GetTrackInfo(MF, i)->Type - 1));

	ulonglong StartTime, EndTime, FilePos;
	unsigned int Track, FrameFlags, FrameSize;
	AVPacket TempPacket;
	init_null_packet(&TempPacket);

	while (mkv_ReadFrame(MF, 0, &Track, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
		// Update progress
		if (IP) {
			if ((*IP)(0, _ftelli64(MC.ST.fp), SourceSize, Private)) {
				_snprintf(ErrorMsg, MsgSize, "Cancelled by user");
				delete TrackIndices;
				return NULL;
			}
		}

		// Only create index entries for video for now to save space
		if (mkv_GetTrackInfo(MF, Track)->Type == TT_VIDEO) {
			(*TrackIndices)[Track].push_back(FrameInfo(StartTime, (FrameFlags & FRAME_KF) != 0));
		} else if (mkv_GetTrackInfo(MF, Track)->Type == TT_AUDIO && (IndexMask & (1 << Track))) {
			(*TrackIndices)[Track].push_back(FrameInfo(AudioContexts[Track].CurrentSample, FilePos, FrameSize, (FrameFlags & FRAME_KF) != 0));
			ReadFrame(FilePos, FrameSize, AudioContexts[Track].CS, MC, ErrorMsg, MsgSize);
			AVCodecContext *AudioCodecContext = AudioContexts[Track].CTX;
			TempPacket.data = MC.Buffer;
			TempPacket.size = FrameSize;

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, db, &dbsize, &TempPacket);
				if (Ret < 0) {
					if (IgnoreDecodeErrors) {
						(*TrackIndices)[Track].clear();
						IndexMask &= ~(1 << Track);					
						break;
					} else {
						_snprintf(ErrorMsg, MsgSize, "Audio decoding error");
						delete TrackIndices;
						return NULL;
					}
				}

				if (Ret > 0) {
					TempPacket.size -= Ret;
					TempPacket.data += Ret;
				}

				if (dbsize > 0)
					AudioContexts[Track].CurrentSample += (dbsize * 8) / (av_get_bits_per_sample_format(AudioCodecContext->sample_fmt) * AudioCodecContext->channels);

				if (dbsize > 0 && (DumpMask & (1 << Track))) {
					// Delay writer creation until after an audio frame has been decoded. This ensures that all parameters are known when writing the headers.
					if (!AudioContexts[Track].W64W) {
						char ABuf[50];
						std::string WN(AudioFile);
						int Offset = StartTime * mkv_TruncFloat(mkv_GetTrackInfo(MF, Track)->TimecodeScale) / (double)1000000;
						_snprintf(ABuf, sizeof(ABuf), ".%02d.delay.%d.w64", Track, Offset);
						WN += ABuf;
						
						AudioContexts[Track].W64W = new Wave64Writer(WN.c_str(), av_get_bits_per_sample_format(AudioCodecContext->sample_fmt),
							AudioCodecContext->channels, AudioCodecContext->sample_rate, AudioFMTIsFloat(AudioCodecContext->sample_fmt));
					}

					AudioContexts[Track].W64W->WriteData(db, dbsize);
				}
			}
		}
	}

	SortTrackIndices(TrackIndices);
	return TrackIndices;
}

FrameIndex *MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, IndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize) {
	AVFormatContext *FormatContext = NULL;
	IndexMask |= DumpMask;

	if (av_open_input_file(&FormatContext, SourceFile, NULL, 0, NULL) != 0) {
		_snprintf(ErrorMsg, MsgSize, "Can't open '%s'", SourceFile);
		return NULL;
	}

	// Do matroska indexing instead?
	if (!strcmp(FormatContext->iformat->name, "matroska")) {
		av_close_input_file(FormatContext);
		return MakeMatroskaIndex(SourceFile, IndexMask, DumpMask, AudioFile, IgnoreDecodeErrors, IP, Private, ErrorMsg, MsgSize);
	}

#ifdef HAALISOURCE
	// Do haali ts indexing instead?
	if (!strcmp(FormatContext->iformat->name, "mpeg") || !strcmp(FormatContext->iformat->name, "mpegts")) {
		av_close_input_file(FormatContext);
		return MakeHaaliIndex(SourceFile, IndexMask, DumpMask, AudioFile, IgnoreDecodeErrors, 0, IP, Private, ErrorMsg, MsgSize);
	}

	if (!strcmp(FormatContext->iformat->name, "ogg")) {
		av_close_input_file(FormatContext);
		return MakeHaaliIndex(SourceFile, IndexMask, DumpMask, AudioFile, IgnoreDecodeErrors, 1, IP, Private, ErrorMsg, MsgSize);
	}
#endif
	
	if (av_find_stream_info(FormatContext) < 0) {
		av_close_input_file(FormatContext);
		_snprintf(ErrorMsg, MsgSize, "Couldn't find stream information");
		return NULL;
	}

	// Audio stuff

	int16_t *db;
	FFAudioContext *AudioContexts;
	FFIndexMemory IM = FFIndexMemory(FormatContext->nb_streams, db, AudioContexts, FormatContext);

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

	FrameIndex *TrackIndices = new FrameIndex();
	TrackIndices->Decoder = 0;

	for (unsigned int i = 0; i < FormatContext->nb_streams; i++)
		TrackIndices->push_back(FrameInfoVector((int64_t)FormatContext->streams[i]->time_base.num * 1000, 
		FormatContext->streams[i]->time_base.den,
		FormatContext->streams[i]->codec->codec_type));

	AVPacket Packet, TempPacket;
	init_null_packet(&Packet);
	init_null_packet(&TempPacket);
	while (av_read_frame(FormatContext, &Packet) >= 0) {
		// Update progress
		if (IP) {
			if ((*IP)(0, FormatContext->pb->pos, FormatContext->file_size, Private)) {
				_snprintf(ErrorMsg, MsgSize, "Cancelled by user");
				delete TrackIndices;
				return NULL;
			}
		}

		// Only create index entries for video for now to save space
		if (FormatContext->streams[Packet.stream_index]->codec->codec_type == CODEC_TYPE_VIDEO) {
			(*TrackIndices)[Packet.stream_index].push_back(FrameInfo(Packet.dts, (Packet.flags & PKT_FLAG_KEY) ? 1 : 0));
		} else if (FormatContext->streams[Packet.stream_index]->codec->codec_type == CODEC_TYPE_AUDIO && (IndexMask & (1 << Packet.stream_index))) {
			(*TrackIndices)[Packet.stream_index].push_back(FrameInfo(Packet.dts, AudioContexts[Packet.stream_index].CurrentSample, (Packet.flags & PKT_FLAG_KEY) ? 1 : 0));
			AVCodecContext *AudioCodecContext = FormatContext->streams[Packet.stream_index]->codec;
			TempPacket.data = Packet.data;
			TempPacket.size = Packet.size;

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, db, &dbsize, &TempPacket);
				if (Ret < 0) {
					if (IgnoreDecodeErrors) {
						(*TrackIndices)[Packet.stream_index].clear();
						IndexMask &= ~(1 << Packet.stream_index);					
						break;
					} else {
						_snprintf(ErrorMsg, MsgSize, "Audio decoding error");
						delete TrackIndices;
						return NULL;
					}
				}

				if (Ret > 0) {
					TempPacket.size -= Ret;
					TempPacket.data += Ret;
				}

				if (dbsize > 0)
					AudioContexts[Packet.stream_index].CurrentSample += (dbsize * 8) / (av_get_bits_per_sample_format(AudioCodecContext->sample_fmt) * AudioCodecContext->channels);

				if (dbsize > 0 && (DumpMask & (1 << Packet.stream_index))) {
					// Delay writer creation until after an audio frame has been decoded. This ensures that all parameters are known when writing the headers.
					if (!AudioContexts[Packet.stream_index].W64W) {
						char ABuf[50];
						std::string WN(AudioFile);
						int Offset = (Packet.dts * FormatContext->streams[Packet.stream_index]->time_base.num)
							/ (double)(FormatContext->streams[Packet.stream_index]->time_base.den * 1000);
						_snprintf(ABuf, sizeof(ABuf), ".%02d.delay.%d.w64", Packet.stream_index, Offset);
						WN += ABuf;
						
						AudioContexts[Packet.stream_index].W64W = new Wave64Writer(WN.c_str(), av_get_bits_per_sample_format(AudioCodecContext->sample_fmt),
							AudioCodecContext->channels, AudioCodecContext->sample_rate, AudioFMTIsFloat(AudioCodecContext->sample_fmt));
					}

					AudioContexts[Packet.stream_index].W64W->WriteData(db, dbsize);
				}
			}
		}

		av_free_packet(&Packet);
	}

	SortTrackIndices(TrackIndices);
	return TrackIndices;
}

FrameIndex *ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize) {
	std::ifstream Index(IndexFile, std::ios::in | std::ios::binary);

	if (!Index.is_open()) {
		_snprintf(ErrorMsg, MsgSize, "Failed to open '%s' for reading", IndexFile);
		return NULL;
	}
	
	// Read the index file header
	IndexHeader IH;
	Index.read(reinterpret_cast<char *>(&IH), sizeof(IH));
	if (IH.Id != INDEXID) {
		_snprintf(ErrorMsg, MsgSize, "'%s' is not a valid index file", IndexFile);
		return NULL;
	}

	if (IH.Version != INDEXVERSION) {
		_snprintf(ErrorMsg, MsgSize, "'%s' is not the expected index version", IndexFile);
		return NULL;
	}

	FrameIndex *TrackIndices = new FrameIndex();

	try {

		TrackIndices->Decoder = IH.Decoder;

		for (unsigned int i = 0; i < IH.Tracks; i++) {
			// Read how many records belong to the current stream
			int TT;
			Index.read(reinterpret_cast<char *>(&TT), sizeof(TT));
			int64_t Num;
			Index.read(reinterpret_cast<char *>(&Num), sizeof(Num));
			int64_t Den;
			Index.read(reinterpret_cast<char *>(&Den), sizeof(Den));
			size_t Frames;
			Index.read(reinterpret_cast<char *>(&Frames), sizeof(Frames));
			TrackIndices->push_back(FrameInfoVector(Num, Den, TT));

			FrameInfo FI(0, false);
			for (size_t j = 0; j < Frames; j++) {
				Index.read(reinterpret_cast<char *>(&FI), sizeof(FrameInfo));
				TrackIndices->at(i).push_back(FI);
			}
		}

	} catch (...) {
		delete TrackIndices;
		_snprintf(ErrorMsg, MsgSize, "Unknown error while reading index information in '%s'", IndexFile);	
		return NULL;
	}

	return TrackIndices;
}

int FrameInfoVector::WriteTimecodes(const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize) {
	std::ofstream Timecodes(TimecodeFile, std::ios::out | std::ios::trunc);

	if (!Timecodes.is_open()) {
		_snprintf(ErrorMsg, MsgSize, "Failed to open '%s' for writing", TimecodeFile);
		return 1;
	}

	Timecodes << "# timecode format v2\n";

	for (iterator Cur=begin(); Cur!=end(); Cur++)
		Timecodes << std::fixed << ((Cur->DTS * TB.Num) / (double)TB.Den) << "\n";

	return 0;
}

int FrameInfoVector::FrameFromDTS(int64_t DTS) {
	for (int i = 0; i < static_cast<int>(size()); i++)
		if (at(i).DTS == DTS)
			return i;
	return -1;
}

int FrameInfoVector::ClosestFrameFromDTS(int64_t DTS) {
	int Frame = 0; 
	int64_t BestDiff = 0xFFFFFFFFFFFFFFLL; // big number
	for (int i = 0; i < static_cast<int>(size()); i++) {
		int64_t CurrentDiff = FFABS(at(i).DTS - DTS);
		if (CurrentDiff < BestDiff) {
			BestDiff = CurrentDiff;
			Frame = i;
		}
	}

	return Frame;
}

int FrameInfoVector::FindClosestKeyFrame(int Frame) {
	Frame = FFMIN(FFMAX(Frame, 0), size() - 1);
	for (int i = Frame; i > 0; i--)
		if (at(i).KeyFrame)
			return i;
	return 0;
}

FrameInfoVector::FrameInfoVector() {
	this->TT = 0;
	this->TB.Num = 0; 
	this->TB.Den = 0;
}

FrameInfoVector::FrameInfoVector(int64_t Num, int64_t Den, int TT) {
	this->TT = TT;
	this->TB.Num = Num; 
	this->TB.Den = Den;
}
