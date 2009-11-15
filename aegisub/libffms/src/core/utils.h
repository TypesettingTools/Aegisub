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

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <sstream>
#include <fstream>
#include <cstdio>
#include "ffms.h"
#include "matroskaparser.h"

extern "C" {
#include "stdiostream.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
}

// must be included after ffmpeg headers
#include "ffmscompat.h"

#ifdef HAALISOURCE
#	define WIN32_LEAN_AND_MEAN
#	define _WIN32_DCOM
#	include <windows.h>
#	include <tchar.h>
#	include <atlbase.h>
#	include <dshow.h>
#	include <initguid.h>
#	include "CoParser.h"
#	include "guids.h"
#endif

#define FFMS_GET_VECTOR_PTR(v) (((v).size() ? &(v)[0] : NULL))

const int64_t ffms_av_nopts_value = static_cast<int64_t>(1) << 63;

// used for matroska<->ffmpeg codec ID mapping to avoid Win32 dependency
typedef struct FFMS_BITMAPINFOHEADER {
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
} FFMS_BITMAPINFOHEADER;

class FFMS_Exception : public std::exception {
private:
	std::string _Message;
	int _ErrorType;
	int _SubType;
public:
	FFMS_Exception(int ErrorType, int SubType, const char *Message = "");
	FFMS_Exception(int ErrorType, int SubType, const std::string &Message);
	~FFMS_Exception() throw ();
	const std::string &GetErrorMessage() const;
	int CopyOut(FFMS_ErrorInfo *ErrorInfo) const;
};

template<class T>
class FFSourceResources {
private:
	T *_PrivClass;
	bool _Enabled;
	bool _Arg;
public:
	FFSourceResources(T *Target) : _PrivClass(Target), _Enabled(true), _Arg(false) {
	}

	~FFSourceResources() {
		if (_Enabled)
			_PrivClass->Free(_Arg);
	}

	void SetEnabled(bool Enabled) {
		_Enabled = Enabled;
	}

	void SetArg(bool Arg) {
		_Arg = Arg;
	}

	void CloseCodec(bool Arg) {
		_Arg = Arg;
	}
};

struct MatroskaReaderContext {
public:
	StdIoStream ST;
	uint8_t *Buffer;
	unsigned int BufferSize;
	char CSBuffer[4096];

	MatroskaReaderContext() {
		InitStdIoStream(&ST);
		Buffer = NULL;
		BufferSize = 0;
	}

	~MatroskaReaderContext() {
		free(Buffer);
	}
};

class ffms_fstream : public std::fstream {
public:
	void open(const char *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
	ffms_fstream(const char *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
};

int GetSWSCPUFlags();
int GetPPCPUFlags();
void ClearErrorInfo(FFMS_ErrorInfo *ErrorInfo);
FFMS_TrackType HaaliTrackTypeToFFTrackType(int TT);
void ReadFrame(uint64_t FilePos, unsigned int &FrameSize, CompressedStream *CS, MatroskaReaderContext &Context);
bool AudioFMTIsFloat(SampleFormat FMT);
void InitNullPacket(AVPacket &pkt);
bool IsNVOP(AVPacket &pkt);
void FillAP(FFMS_AudioProperties &AP, AVCodecContext *CTX, FFMS_Track &Frames);
#ifdef HAALISOURCE
unsigned vtSize(VARIANT &vt);
void vtCopy(VARIANT& vt,void *dest);
void InitializeCodecContextFromHaaliInfo(CComQIPtr<IPropertyBag> pBag, AVCodecContext *CodecContext);
#endif
void InitializeCodecContextFromMatroskaTrackInfo(TrackInfo *TI, AVCodecContext *CodecContext);
CodecID MatroskaToFFCodecID(char *Codec, void *CodecPrivate, unsigned int FourCC = 0, unsigned int BitsPerSample = 0);
FILE *ffms_fopen(const char *filename, const char *mode);
size_t ffms_mbstowcs (wchar_t *wcstr, const char *mbstr, size_t max);
#ifdef HAALISOURCE
CComPtr<IMMContainer> HaaliOpenFile(const char *SourceFile, enum FFMS_Sources SourceMode);
#endif
void LAVFOpenFile(const char *SourceFile, AVFormatContext *&FormatContext);

#endif
