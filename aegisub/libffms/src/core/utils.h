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
#include <fstream>
#include <stdio.h>
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

struct MatroskaReaderContext {
public:
	StdIoStream ST;
	uint8_t *Buffer;
	unsigned int BufferSize;

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
FFMS_TrackType HaaliTrackTypeToFFTrackType(int TT);
int ReadFrame(uint64_t FilePos, unsigned int &FrameSize, CompressedStream *CS, MatroskaReaderContext &Context, char *ErrorMsg, unsigned MsgSize);
bool AudioFMTIsFloat(SampleFormat FMT);
void InitNullPacket(AVPacket *pkt);
void FillAP(FFMS_AudioProperties &AP, AVCodecContext *CTX, FFMS_Track &Frames);
#ifdef HAALISOURCE
unsigned vtSize(VARIANT &vt);
void vtCopy(VARIANT& vt,void *dest);
void InitializeCodecContextFromHaaliInfo(CComQIPtr<IPropertyBag> pBag, AVCodecContext *CodecContext);
#endif
void InitializeCodecContextFromMatroskaTrackInfo(TrackInfo *TI, AVCodecContext *CodecContext);
CodecID MatroskaToFFCodecID(char *Codec, void *CodecPrivate, unsigned int FourCC = 0);
FILE *ffms_fopen(const char *filename, const char *mode);
size_t ffms_mbstowcs (wchar_t *wcstr, const char *mbstr, size_t max);

#endif
