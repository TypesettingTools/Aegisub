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

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
#include <libavutil/sha1.h>
#include "MatroskaParser.h"
#include "stdiostream.h"
}

#include "ffms.h"

#ifdef HAALISOURCE
#	define _WIN32_DCOM
#	include <windows.h>
#	include <tchar.h>
#	include <atlbase.h>
#	include <dshow.h>
#	include "CoParser.h"
#	include <initguid.h>
#	include "guids.h"
#endif

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

int GetCPUFlags();
int ReadFrame(uint64_t FilePos, unsigned int &FrameSize, CompressedStream *CS, MatroskaReaderContext &Context, char *ErrorMsg, unsigned MsgSize);
bool AudioFMTIsFloat(SampleFormat FMT);
void InitNullPacket(AVPacket *pkt);
void FillAP(TAudioProperties &AP, AVCodecContext *CTX, FFTrack &Frames);
#ifdef HAALISOURCE
unsigned vtSize(VARIANT &vt);
void vtCopy(VARIANT& vt,void *dest);
#endif
CodecID MatroskaToFFCodecID(char *Codec, void *CodecPrivate);

#endif
