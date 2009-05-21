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

#ifndef FFVIDEOSOURCE_H
#define FFVIDEOSOURCE_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
}

#include <vector>
#include "indexing.h"
#include "utils.h"
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

class FFVideo {
private:
	pp_context_t *PPContext;
	pp_mode_t *PPMode;
	SwsContext *SWS;
protected:
	TVideoProperties VP;
	AVFrame *DecodeFrame;
	AVFrame *PPFrame;
	AVFrame *FinalFrame;
	int LastFrameNum;
	FFTrack Frames;
	int VideoTrack;
	int	CurrentFrame;
	AVCodecContext *CodecContext;

	FFVideo();
	int InitPP(const char *PP, PixelFormat PixelFormat, char *ErrorMsg, unsigned MsgSize);
	TAVFrameLite *OutputFrame(AVFrame *Frame);
public:
	virtual ~FFVideo();
	const TVideoProperties& GetTVideoProperties() { return VP; }
	FFTrack *GetFFTrack() { return &Frames; }
	virtual TAVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize) = 0;
	TAVFrameLite *GetFrameByTime(double Time, char *ErrorMsg, unsigned MsgSize);
	int SetOutputFormat(int TargetFormats, int Width, int Height, char *ErrorMsg, unsigned MsgSize);
	void ResetOutputFormat();
};

class FFLAVFVideo : public FFVideo {
private:
	AVFormatContext *FormatContext;
	int SeekMode;

	void Free(bool CloseCodec);
	int DecodeNextFrame(AVFrame *Frame, int64_t *DTS, char *ErrorMsg, unsigned MsgSize);
public:
	FFLAVFVideo(const char *SourceFile, int Track, FFIndex *Index, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize);
	~FFLAVFVideo();
	TAVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

class FFMatroskaVideo : public FFVideo {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
    CompressedStream *CS;
	char ErrorMessage[256];

	void Free(bool CloseCodec);
	int DecodeNextFrame(AVFrame *AFrame, int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize);
public:
	FFMatroskaVideo(const char *SourceFile, int Track, FFIndex *Index, const char *PP, int Threads, char *ErrorMsg, unsigned MsgSize);
	~FFMatroskaVideo();
    TAVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

#ifdef HAALISOURCE

class FFHaaliVideo : public FFVideo {
private:
	CComPtr<IMMContainer> pMMC;
	uint8_t * CodecPrivate;
	void Free(bool CloseCodec);
	int DecodeNextFrame(AVFrame *AFrame, int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize);
public:
	FFHaaliVideo(const char *SourceFile, int Track, FFIndex *Index, const char *PP, int Threads, int SourceMode, char *ErrorMsg, unsigned MsgSize);
	~FFHaaliVideo();
    TAVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

#endif // HAALISOURCE

#endif
