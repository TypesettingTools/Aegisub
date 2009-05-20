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

#include "utils.h"
#include "indexing.h"
#include <string.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#ifdef _MSC_VER
#	include <intrin.h>
#endif

#ifdef __UNIX__
#define _fseeki64 fseeko
#define _ftelli64 ftello
#define _snprintf snprintf
#endif

TFrameInfo::TFrameInfo(int64_t DTS, bool KeyFrame) {
	this->DTS = DTS;
	this->KeyFrame = KeyFrame;
	this->SampleStart = 0;
	this->FilePos = 0;
	this->FrameSize = 0;
}

TFrameInfo::TFrameInfo(int64_t DTS, int64_t FilePos, unsigned int FrameSize, bool KeyFrame) {
	this->DTS = DTS;
	this->KeyFrame = KeyFrame;
	this->SampleStart = 0;
	this->FilePos = FilePos;
	this->FrameSize = FrameSize;
}

TFrameInfo::TFrameInfo(int64_t DTS, int64_t SampleStart, bool KeyFrame) {
	this->DTS = DTS;
	this->KeyFrame = KeyFrame;
	this->SampleStart = SampleStart;
	this->FilePos = 0;
	this->FrameSize = 0;
}

TFrameInfo::TFrameInfo(int64_t DTS, int64_t SampleStart, int64_t FilePos, unsigned int FrameSize, bool KeyFrame) {
	this->DTS = DTS;
	this->KeyFrame = KeyFrame;
	this->SampleStart = SampleStart;
	this->FilePos = FilePos;
	this->FrameSize = FrameSize;
}

int FFTrack::WriteTimecodes(const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize) {
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

int FFTrack::FrameFromDTS(int64_t DTS) {
	for (int i = 0; i < static_cast<int>(size()); i++)
		if (at(i).DTS == DTS)
			return i;
	return -1;
}

int FFTrack::ClosestFrameFromDTS(int64_t DTS) {
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

int FFTrack::FindClosestVideoKeyFrame(int Frame) {
	Frame = FFMIN(FFMAX(Frame, 0), static_cast<int>(size()) - 1);
	for (int i = Frame; i > 0; i--)
		if (at(i).KeyFrame)
			return i;
	return 0;
}

int FFTrack::FindClosestAudioKeyFrame(int64_t Sample) {
	for (size_t i = 0; i < size(); i++) {
		if (at(i).SampleStart == Sample && at(i).KeyFrame)
			return i;
		else if (at(i).SampleStart > Sample && at(i).KeyFrame)
			return i  - 1;
	}
	return size() - 1;
}

FFTrack::FFTrack() {
	this->TT = FFMS_TYPE_UNKNOWN;
	this->TB.Num = 0; 
	this->TB.Den = 0;
}

FFTrack::FFTrack(int64_t Num, int64_t Den, FFMS_TrackType TT) {
	this->TT = TT;
	this->TB.Num = Num; 
	this->TB.Den = Den;
}

int FFIndex::WriteIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize) {
	std::ofstream IndexStream(IndexFile, std::ios::out | std::ios::binary | std::ios::trunc);

	if (!IndexStream.is_open()) {
		_snprintf(ErrorMsg, MsgSize, "Failed to open '%s' for writing", IndexFile);
		return 1;
	}

	// Write the index file header
	IndexHeader IH;
	IH.Id = INDEXID;
	IH.Version = INDEXVERSION;
	IH.Tracks = size();
	IH.Decoder = Decoder;
	IH.LAVUVersion = LIBAVUTIL_VERSION_INT;
	IH.LAVFVersion = LIBAVFORMAT_VERSION_INT;
	IH.LAVCVersion = LIBAVCODEC_VERSION_INT;
	IH.LSWSVersion = LIBSWSCALE_VERSION_INT;
	IH.LPPVersion = LIBPOSTPROC_VERSION_INT;

	IndexStream.write(reinterpret_cast<char *>(&IH), sizeof(IH));
	
	for (unsigned int i = 0; i < IH.Tracks; i++) {
		int TT = at(i).TT;
		IndexStream.write(reinterpret_cast<char *>(&TT), sizeof(TT));
		int64_t Num = at(i).TB.Num;
		IndexStream.write(reinterpret_cast<char *>(&Num), sizeof(Num));
		int64_t Den = at(i).TB.Den;
		IndexStream.write(reinterpret_cast<char *>(&Den), sizeof(Den));
		size_t Frames = at(i).size();
		IndexStream.write(reinterpret_cast<char *>(&Frames), sizeof(Frames));

		for (size_t j = 0; j < Frames; j++)
			IndexStream.write(reinterpret_cast<char *>(&(at(i)[j])), sizeof(TFrameInfo));
	}

	return 0;
}

int FFIndex::ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize) {
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

	if (IH.LAVUVersion != LIBAVUTIL_VERSION_INT || IH.LAVFVersion != LIBAVFORMAT_VERSION_INT ||
		IH.LAVCVersion != LIBAVCODEC_VERSION_INT || IH.LSWSVersion != LIBSWSCALE_VERSION_INT ||
		IH.LPPVersion != LIBPOSTPROC_VERSION_INT) {
		_snprintf(ErrorMsg, MsgSize, "A different FFmpeg build was used to create this index", IndexFile);
		return NULL;
	}

	try {
		Decoder = IH.Decoder;

		for (unsigned int i = 0; i < IH.Tracks; i++) {
			// Read how many records belong to the current stream
			FFMS_TrackType TT;
			Index.read(reinterpret_cast<char *>(&TT), sizeof(TT));
			int64_t Num;
			Index.read(reinterpret_cast<char *>(&Num), sizeof(Num));
			int64_t Den;
			Index.read(reinterpret_cast<char *>(&Den), sizeof(Den));
			size_t Frames;
			Index.read(reinterpret_cast<char *>(&Frames), sizeof(Frames));
			push_back(FFTrack(Num, Den, TT));

			TFrameInfo FI(0, false);
			for (size_t j = 0; j < Frames; j++) {
				Index.read(reinterpret_cast<char *>(&FI), sizeof(TFrameInfo));
				at(i).push_back(FI);
			}
		}

	} catch (...) {
		_snprintf(ErrorMsg, MsgSize, "Unknown error while reading index information in '%s'", IndexFile);	
		return 1;
	}
}

int GetCPUFlags() {
// FIXME Add proper feature detection when msvc isn't used
	int Flags = PP_CPU_CAPS_MMX | PP_CPU_CAPS_MMX2;

#ifdef _MSC_VER
	Flags = 0;
	int CPUInfo[4];
	__cpuid(CPUInfo, 0);

	// PP and SWS defines have the same values for their defines so this actually works
	if (CPUInfo[3] & (1 << 23))
		Flags |= PP_CPU_CAPS_MMX;
	if (CPUInfo[3] & (1 << 25))
		Flags |= PP_CPU_CAPS_MMX2;
#endif

	return Flags;
}

FFMS_TrackType HaaliTrackTypeToFFTrackType(int TT) {
	switch (TT) {
		case TT_VIDEO: return FFMS_TYPE_VIDEO; break;
		case TT_AUDIO: return FFMS_TYPE_AUDIO; break;
		case TT_SUB: return FFMS_TYPE_SUBTITLE; break;
		default: return FFMS_TYPE_UNKNOWN;
	}
}

int ReadFrame(uint64_t FilePos, unsigned int &FrameSize, CompressedStream *CS, MatroskaReaderContext &Context, char *ErrorMsg, unsigned MsgSize) {
	if (CS) {
		char CSBuffer[4096];

		unsigned int DecompressedFrameSize = 0;

		cs_NextFrame(CS, FilePos, FrameSize);

		for (;;) {
			int ReadBytes = cs_ReadData(CS, CSBuffer, sizeof(CSBuffer));
			if (ReadBytes < 0) {
				_snprintf(ErrorMsg, MsgSize, "Error decompressing data: %s", cs_GetLastError(CS));
				return 1;
			}
			if (ReadBytes == 0) {
				FrameSize = DecompressedFrameSize;
				return 0;
			}

			if (Context.BufferSize < DecompressedFrameSize + ReadBytes) {
				Context.BufferSize = FrameSize;
				Context.Buffer = (uint8_t *)realloc(Context.Buffer, Context.BufferSize + 16);
				if (Context.Buffer == NULL)  {
					_snprintf(ErrorMsg, MsgSize, "Out of memory");
					return 2;
				}
			}

			memcpy(Context.Buffer + DecompressedFrameSize, CSBuffer, ReadBytes);
			DecompressedFrameSize += ReadBytes;
		}
	} else {
		if (_fseeki64(Context.ST.fp, FilePos, SEEK_SET)) {
			_snprintf(ErrorMsg, MsgSize, "fseek(): %s", strerror(errno));
			return 3;
		}

		if (Context.BufferSize < FrameSize) {
			Context.BufferSize = FrameSize;
			Context.Buffer = (uint8_t *)realloc(Context.Buffer, Context.BufferSize + 16);
			if (Context.Buffer == NULL) {
				_snprintf(ErrorMsg, MsgSize, "Out of memory");
				return 4;
			}
		}

		size_t ReadBytes = fread(Context.Buffer, 1, FrameSize, Context.ST.fp);
		if (ReadBytes != FrameSize) {
			if (ReadBytes == 0) {
				if (feof(Context.ST.fp)) {
					_snprintf(ErrorMsg, MsgSize, "Unexpected EOF while reading frame");
					return 5;
				} else {
					_snprintf(ErrorMsg, MsgSize, "Error reading frame: %s", strerror(errno));
					return 6;
				}
			} else {
				_snprintf(ErrorMsg, MsgSize, "Short read while reading frame");
				return 7;
			}
			_snprintf(ErrorMsg, MsgSize, "Unknown read error");
			return 8;
		}

		return 0;
	}

	FrameSize = 0;
	return 0;
}

bool AudioFMTIsFloat(SampleFormat FMT){
	switch (FMT) {
		case SAMPLE_FMT_FLT:
		case SAMPLE_FMT_DBL:
			  return true;
		default:
			return false;
	}
}

void InitNullPacket(AVPacket *pkt) {
	av_init_packet(pkt);
	pkt->data = NULL;
	pkt->size = 0;
}

void FillAP(TAudioProperties &AP, AVCodecContext *CTX, FFTrack &Frames) {
	AP.BitsPerSample = av_get_bits_per_sample_format(CTX->sample_fmt);
	AP.Channels = CTX->channels;;
	AP.Float = AudioFMTIsFloat(CTX->sample_fmt);
	AP.SampleRate = CTX->sample_rate;
	AP.NumSamples = (Frames.back()).SampleStart;
	AP.FirstTime = ((Frames.front().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	AP.LastTime = ((Frames.back().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
}

#ifdef HAALISOURCE

unsigned vtSize(VARIANT &vt) {
	if (V_VT(&vt) != (VT_ARRAY | VT_UI1))
		return 0;
	long lb,ub;
	if (FAILED(SafeArrayGetLBound(V_ARRAY(&vt),1,&lb)) ||
		FAILED(SafeArrayGetUBound(V_ARRAY(&vt),1,&ub)))
		return 0;
	return ub - lb + 1;
}

void vtCopy(VARIANT& vt,void *dest) {
	unsigned sz = vtSize(vt);
	if (sz > 0) {
		void  *vp;
		if (SUCCEEDED(SafeArrayAccessData(V_ARRAY(&vt),&vp))) {
			memcpy(dest,vp,sz);
			SafeArrayUnaccessData(V_ARRAY(&vt));
		}
	}
}

#else

// used for matroska<->ffmpeg codec ID mapping to avoid Win32 dependency
typedef struct BITMAPINFOHEADER {
        uint32_t      biSize;
        int32_t       biWidth;
        int32_t       biHeight;
        uint16_t      biPlanes;
        uint16_t      biBitCount;
        uint32_t      biCompression;
        uint32_t      biSizeImage;
        int32_t       biXPelsPerMeter;
        int32_t       biYPelsPerMeter;
        uint32_t      biClrUsed;
        uint32_t      biClrImportant;
} BITMAPINFOHEADER;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)\
	((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |\
	((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

#endif

CodecID MatroskaToFFCodecID(char *Codec, void *CodecPrivate) {
/* Video Codecs */
	if (!strcmp(Codec, "V_MS/VFW/FOURCC")) {
		// fourcc list from ffdshow
		switch (((BITMAPINFOHEADER *)CodecPrivate)->biCompression) {
			case MAKEFOURCC('F', 'F', 'D', 'S'):
			case MAKEFOURCC('F', 'V', 'F', 'W'):
			case MAKEFOURCC('X', 'V', 'I', 'D'):
			case MAKEFOURCC('D', 'I', 'V', 'X'):
			case MAKEFOURCC('D', 'X', '5', '0'):
			case MAKEFOURCC('M', 'P', '4', 'V'):
			case MAKEFOURCC('m', 'p', '4', 'v'):
			case MAKEFOURCC('3', 'I', 'V', 'X'):
			case MAKEFOURCC('W', 'V', '1', 'F'):
			case MAKEFOURCC('F', 'M', 'P', '4'):
			case MAKEFOURCC('S', 'M', 'P', '4'):
				return CODEC_ID_MPEG4;
			case MAKEFOURCC('D', 'I', 'V', '3'):
			case MAKEFOURCC('D', 'V', 'X', '3'):
			case MAKEFOURCC('M', 'P', '4', '3'):
				return CODEC_ID_MSMPEG4V3;
			case MAKEFOURCC('M', 'P', '4', '2'):
				return CODEC_ID_MSMPEG4V2;
			case MAKEFOURCC('M', 'P', '4', '1'):
				return CODEC_ID_MSMPEG4V1;
			case MAKEFOURCC('W', 'M', 'V', '1'):
				return CODEC_ID_WMV1;
			case MAKEFOURCC('W', 'M', 'V', '2'):
				return CODEC_ID_WMV2;
			case MAKEFOURCC('W', 'M', 'V', '3'):
				return CODEC_ID_WMV3;
/*
			case MAKEFOURCC('M', 'S', 'S', '1'):
			case MAKEFOURCC('M', 'S', 'S', '2'):
			case MAKEFOURCC('W', 'V', 'P', '2'):
			case MAKEFOURCC('W', 'M', 'V', 'P'):
				return CODEC_ID_WMV9_LIB;
*/
			case MAKEFOURCC('W', 'V', 'C', '1'):
				return CODEC_ID_VC1;
			case MAKEFOURCC('V', 'P', '5', '0'):
				return CODEC_ID_VP5;
			case MAKEFOURCC('V', 'P', '6', '0'):
			case MAKEFOURCC('V', 'P', '6', '1'):
			case MAKEFOURCC('V', 'P', '6', '2'):
				return CODEC_ID_VP6;
			case MAKEFOURCC('V', 'P', '6', 'F'):
			case MAKEFOURCC('F', 'L', 'V', '4'):
				return CODEC_ID_VP6F;	
			case MAKEFOURCC('C', 'A', 'V', 'S'):
				return CODEC_ID_CAVS;	
			case MAKEFOURCC('M', 'P', 'G', '1'):
			case MAKEFOURCC('M', 'P', 'E', 'G'):
				return CODEC_ID_MPEG2VIDEO;	// not a typo
			case MAKEFOURCC('M', 'P', 'G', '2'):
			case MAKEFOURCC('E', 'M', '2', 'V'):
			case MAKEFOURCC('M', 'M', 'E', 'S'):
				return CODEC_ID_MPEG2VIDEO;
			case MAKEFOURCC('H', '2', '6', '3'):
			case MAKEFOURCC('S', '2', '6', '3'):
			case MAKEFOURCC('L', '2', '6', '3'):
			case MAKEFOURCC('M', '2', '6', '3'):
			case MAKEFOURCC('U', '2', '6', '3'):
			case MAKEFOURCC('X', '2', '6', '3'):
				return CODEC_ID_H263;
			case MAKEFOURCC('H', '2', '6', '4'):
			case MAKEFOURCC('X', '2', '6', '4'):
			case MAKEFOURCC('V', 'S', 'S', 'H'):
			case MAKEFOURCC('D', 'A', 'V', 'C'):
			case MAKEFOURCC('P', 'A', 'V', 'C'):
			case MAKEFOURCC('A', 'V', 'C', '1'):
				return CODEC_ID_H264;
			case MAKEFOURCC('M', 'J', 'P', 'G'):
			case MAKEFOURCC('L', 'J', 'P', 'G'):
			case MAKEFOURCC('M', 'J', 'L', 'S'):
			case MAKEFOURCC('J', 'P', 'E', 'G'):
			case MAKEFOURCC('A', 'V', 'R', 'N'):
			case MAKEFOURCC('M', 'J', 'P', 'A'):
				return CODEC_ID_MJPEG;
			case MAKEFOURCC('D', 'V', 'S', 'D'):
			case MAKEFOURCC('D', 'V', '2', '5'):
			case MAKEFOURCC('D', 'V', '5', '0'):
			case MAKEFOURCC('C', 'D', 'V', 'C'):
			case MAKEFOURCC('C', 'D', 'V', '5'):
			case MAKEFOURCC('D', 'V', 'I', 'S'):
			case MAKEFOURCC('P', 'D', 'V', 'C'):
				return CODEC_ID_DVVIDEO;
			case MAKEFOURCC('H', 'F', 'Y', 'U'):
				return CODEC_ID_HUFFYUV;
			case MAKEFOURCC('F', 'F', 'V', 'H'):
				return CODEC_ID_FFVHUFF;
			case MAKEFOURCC('C', 'Y', 'U', 'V'):
				return CODEC_ID_CYUV;
			case MAKEFOURCC('A', 'S', 'V', '1'):
				return CODEC_ID_ASV1;
			case MAKEFOURCC('A', 'S', 'V', '2'):
				return CODEC_ID_ASV2;
			case MAKEFOURCC('V', 'C', 'R', '1'):
				return CODEC_ID_VCR1;
			case MAKEFOURCC('T', 'H', 'E', 'O'):
				return CODEC_ID_THEORA;
			case MAKEFOURCC('S', 'V', 'Q', '1'):
				return CODEC_ID_SVQ1;
			case MAKEFOURCC('S', 'V', 'Q', '3'):
				return CODEC_ID_SVQ3;
			case MAKEFOURCC('R', 'P', 'Z', 'A'):
				return CODEC_ID_RPZA;
			case MAKEFOURCC('F', 'F', 'V', '1'):
				return CODEC_ID_FFV1;
			case MAKEFOURCC('V', 'P', '3', '1'):
				return CODEC_ID_VP3;
			case MAKEFOURCC('R', 'L', 'E', '8'):
				return CODEC_ID_MSRLE;
			case MAKEFOURCC('M', 'S', 'Z', 'H'):
				return CODEC_ID_MSZH;
			case MAKEFOURCC('Z', 'L', 'I', 'B'):
				return CODEC_ID_ZLIB;
			case MAKEFOURCC('F', 'L', 'V', '1'):
				return CODEC_ID_FLV1;
			case MAKEFOURCC('P', 'N', 'G', '1'):
				return CODEC_ID_PNG;
			case MAKEFOURCC('M', 'P', 'N', 'G'):
				return CODEC_ID_PNG;
/*
			case MAKEFOURCC('A', 'V', 'I', 'S'):
				return CODEC_ID_AVISYNTH;
*/
			case MAKEFOURCC('C', 'R', 'A', 'M'):
				return CODEC_ID_MSVIDEO1;
			case MAKEFOURCC('R', 'T', '2', '1'):
				return CODEC_ID_INDEO2;
			case MAKEFOURCC('I', 'V', '3', '2'):
			case MAKEFOURCC('I', 'V', '3', '1'):
				return CODEC_ID_INDEO3;
			case MAKEFOURCC('C', 'V', 'I', 'D'):
				return CODEC_ID_CINEPAK;
			case MAKEFOURCC('R', 'V', '1', '0'):
				return CODEC_ID_RV10;
			case MAKEFOURCC('R', 'V', '2', '0'):
				return CODEC_ID_RV20;
			case MAKEFOURCC('8', 'B', 'P', 'S'):
				return CODEC_ID_8BPS;
			case MAKEFOURCC('Q', 'R', 'L', 'E'):
				return CODEC_ID_QTRLE;
			case MAKEFOURCC('D', 'U', 'C', 'K'):
				return CODEC_ID_TRUEMOTION1;
			case MAKEFOURCC('T', 'M', '2', '0'):
				return CODEC_ID_TRUEMOTION2;
			case MAKEFOURCC('T', 'S', 'C', 'C'):
				return CODEC_ID_TSCC;
			case MAKEFOURCC('S', 'N', 'O', 'W'):
				return CODEC_ID_SNOW;
			case MAKEFOURCC('Q', 'P', 'E', 'G'):
			case MAKEFOURCC('Q', '1', '_', '0'):
			case MAKEFOURCC('Q', '1', '_', '1'):
				return CODEC_ID_QPEG;
			case MAKEFOURCC('H', '2', '6', '1'):
			case MAKEFOURCC('M', '2', '6', '1'):
				return CODEC_ID_H261;
			case MAKEFOURCC('L', 'O', 'C', 'O'):
				return CODEC_ID_LOCO;
			case MAKEFOURCC('W', 'N', 'V', '1'):
				return CODEC_ID_WNV1;
			case MAKEFOURCC('C', 'S', 'C', 'D'):
				return CODEC_ID_CSCD;
			case MAKEFOURCC('Z', 'M', 'B', 'V'):
				return CODEC_ID_ZMBV;
			case MAKEFOURCC('U', 'L', 'T', 'I'):
				return CODEC_ID_ULTI;
			case MAKEFOURCC('V', 'I', 'X', 'L'):
				return CODEC_ID_VIXL;
			case MAKEFOURCC('A', 'A', 'S', 'C'):
				return CODEC_ID_AASC;
			case MAKEFOURCC('F', 'P', 'S', '1'):
				return CODEC_ID_FRAPS;
			default:
				return CODEC_ID_NONE;
		}

	} else if (!strcmp(Codec, "V_MPEG4/ISO/AVC"))
		return CODEC_ID_H264;
	else if (!strcmp(Codec, "V_MPEG4/ISO/AP"))
		return CODEC_ID_MPEG4;
	else if (!strcmp(Codec, "V_MPEG4/ISO/ASP"))
		return CODEC_ID_MPEG4;
	else if (!strcmp(Codec, "V_MPEG4/ISO/SP"))
		return CODEC_ID_MPEG4;
	else if (!strcmp(Codec, "V_MPEG4/MS/V3"))
		return CODEC_ID_MSMPEG4V3;
	else if (!strcmp(Codec, "V_MPEG2"))
		return CODEC_ID_MPEG2VIDEO;
	else if (!strcmp(Codec, "V_MPEG1"))
		return CODEC_ID_MPEG2VIDEO; // still not a typo
	else if (!strcmp(Codec, "V_VC1"))
		return CODEC_ID_VC1;
	else if (!strcmp(Codec, "V_SNOW"))
		return CODEC_ID_SNOW;
	else if (!strcmp(Codec, "V_THEORA"))
		return CODEC_ID_THEORA;
	else if (!strcmp(Codec, "V_UNCOMPRESSED"))
		return CODEC_ID_NONE; // FIXME: bleh
	else if (!strcmp(Codec, "V_QUICKTIME"))
		return CODEC_ID_SVQ3; // no idea if this is right
	else if (!strcmp(Codec, "V_CIPC"))
		return CODEC_ID_NONE; // don't know, don't care
	else if (!strncmp(Codec, "V_REAL/RV", 9)) {
		switch (Codec[9]) {
			case '1': 
				return CODEC_ID_RV10;
			case '2': 
				return CODEC_ID_RV20;
			case '3': 
				return CODEC_ID_RV30;
			case '4': 
				return CODEC_ID_RV40;
			default:
				return CODEC_ID_NONE;
		}
/* Audio Codecs */
	} else if (!strcmp(Codec, "A_AC3"))
		return CODEC_ID_AC3;
	else if (!strcmp(Codec, "A_EAC3"))
		return CODEC_ID_AC3;
	else if (!strcmp(Codec, "A_MPEG/L3"))
		return CODEC_ID_MP3;
	else if (!strcmp(Codec, "A_MPEG/L2"))
		return CODEC_ID_MP2;
	else if (!strcmp(Codec, "A_MPEG/L1"))
		return CODEC_ID_MP2; // correct?
	else if (!strcmp(Codec, "A_DTS"))
		return CODEC_ID_DTS;
	else if (!strcmp(Codec, "A_PCM/INT/LIT")) {
/* FIXME
		switch (TI->AV.Audio.BitDepth) {
			case 8: return CODEC_ID_PCM_S8;
			case 16: return CODEC_ID_PCM_S16LE;
			case 24: return CODEC_ID_PCM_S24LE;
			case 32: return CODEC_ID_PCM_S32LE;
			default: return CODEC_ID_NONE;
		}
*/
		return CODEC_ID_NONE;
	} else if (!strcmp(Codec, "A_PCM/INT/BIG")) {
/* FIXME
		switch (TI->AV.Audio.BitDepth) {
			case 8: return CODEC_ID_PCM_S8;
			case 16: return CODEC_ID_PCM_S16BE;
			case 24: return CODEC_ID_PCM_S24BE;
			case 32: return CODEC_ID_PCM_S32BE;
			default: return CODEC_ID_NONE;
		}
*/
		return CODEC_ID_NONE;
	} else if (!strcmp(Codec, "A_PCM/FLOAT/IEEE"))
		return CODEC_ID_PCM_F32LE; // FIXME: only a most likely guess, may do bad things
	else if (!strcmp(Codec, "A_FLAC"))
		return CODEC_ID_FLAC;
	else if (!strcmp(Codec, "A_MPC"))
		return CODEC_ID_MUSEPACK8; // or is it CODEC_ID_MUSEPACK7? both?
	else if (!strcmp(Codec, "A_TTA1"))
		return CODEC_ID_TTA;
	else if (!strcmp(Codec, "A_WAVPACK4"))
		return CODEC_ID_WAVPACK;
	else if (!strcmp(Codec, "A_VORBIS"))
		return CODEC_ID_VORBIS;
	else if (!strcmp(Codec, "A_REAL/14_4"))
		return CODEC_ID_RA_144;
	else if (!strcmp(Codec, "A_REAL/28_8"))
		return CODEC_ID_RA_288;
	else if (!strcmp(Codec, "A_REAL/COOK"))
		return CODEC_ID_COOK;
	else if (!strcmp(Codec, "A_REAL/SIPR"))
		return CODEC_ID_SIPR;
	else if (!strcmp(Codec, "A_REAL/ATRC"))
		return CODEC_ID_ATRAC3;
	else if (!strncmp(Codec, "A_AAC", 5))
		return CODEC_ID_AAC;
	else if (!strcmp(Codec, "A_SPEEX"))
		return CODEC_ID_SPEEX;
	else if (!strcmp(Codec, "A_QUICKTIME"))
		return CODEC_ID_NONE; // no
	else if (!strcmp(Codec, "A_MS/ACM")) {
		// nothing useful here anyway?
		//#include "Mmreg.h"
		//((WAVEFORMATEX *)TI->CodecPrivate)->wFormatTag
		return CODEC_ID_NONE;
	} else {
		return CODEC_ID_NONE;
	}
}
