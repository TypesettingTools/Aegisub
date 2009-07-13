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

#include "ffms.h"
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
#include <libavutil/sha1.h>
#include "MatroskaParser.h"
#include "stdiostream.h"
}

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


// Compatibility with older/newer ffmpegs
#if (LIBAVFORMAT_VERSION_INT) > (AV_VERSION_INT(52,34,0)) 
#define codec_bmp_tags ff_codec_bmp_tags 
#endif
#ifndef AV_PKT_FLAG_KEY
#define AV_PKT_FLAG_KEY PKT_FLAG_KEY
#endif


struct TFrameInfo {
	FFMS_FRAMEINFO_COMMON
	int64_t SampleStart;
	int64_t FilePos;
	unsigned int FrameSize;
#ifdef FFMS_EXPORTS
	TFrameInfo(int64_t DTS, bool KeyFrame);
	TFrameInfo(int64_t DTS, int64_t FilePos, unsigned int FrameSize, bool KeyFrame);
	TFrameInfo(int64_t DTS, int64_t SampleStart, bool KeyFrame);
	TFrameInfo(int64_t DTS, int64_t SampleStart, int64_t FilePos, unsigned int FrameSize, bool KeyFrame);
#endif
};

class FFTrack : public std::vector<TFrameInfo> {
public:
	FFMS_TrackType TT;
	TTrackTimeBase TB;

	int FindClosestVideoKeyFrame(int Frame);
	int FindClosestAudioKeyFrame(int64_t Sample);
	int FrameFromDTS(int64_t DTS);
	int ClosestFrameFromDTS(int64_t DTS);
	int WriteTimecodes(const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);

	FFTrack();
	FFTrack(int64_t Num, int64_t Den, FFMS_TrackType TT);
};

class FFIndex : public std::vector<FFTrack> {
public:
	int Decoder;
	int WriteIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
	int ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
};

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
FFMS_TrackType HaaliTrackTypeToFFTrackType(int TT);
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
