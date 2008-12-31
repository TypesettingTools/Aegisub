//  Copyright (c) 2007-2008 Fredrik Mellbin
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

#ifdef HAALITS
#	define _WIN32_DCOM
#	include <windows.h>
#	include <tchar.h>
#	include <atlbase.h>
#	include <dshow.h>
#	include "CoParser.h"
#	include <initguid.h>
#	include "guids.h"
#endif

class VideoBase {
private:
	pp_context_t *PPContext;
	pp_mode_t *PPMode;
	SwsContext *SWS;
protected:
	VideoProperties VP;
	AVFrame *DecodeFrame;
	AVFrame *PPFrame;
	AVFrame *FinalFrame;
	int LastFrameNum;
	FrameInfoVector Frames;
	int VideoTrack;
	int	CurrentFrame;
	AVCodecContext *CodecContext;

	VideoBase();
	int InitPP(const char *PP, int PixelFormat, char *ErrorMsg, unsigned MsgSize);
	AVFrameLite *OutputFrame(AVFrame *Frame);
public:
	virtual ~VideoBase();
	const VideoProperties& GetVideoProperties() { return VP; }
	FrameInfoVector *GetFrameInfoVector() { return &Frames; }
	virtual AVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize) = 0;
	AVFrameLite *GetFrameByTime(double Time, char *ErrorMsg, unsigned MsgSize);
	int SetOutputFormat(int TargetFormats, int Width, int Height, char *ErrorMsg, unsigned MsgSize);
	void ResetOutputFormat();
};

class FFVideoSource : public VideoBase {
private:
	AVFormatContext *FormatContext;
	int SeekMode;

	void Free(bool CloseCodec);
	int DecodeNextFrame(AVFrame *Frame, int64_t *DTS, char *ErrorMsg, unsigned MsgSize);
public:
	FFVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize);
	~FFVideoSource();
	AVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

class MatroskaVideoSource : public VideoBase {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
    CompressedStream *CS;
	char ErrorMessage[256];

	void Free(bool CloseCodec);
	int DecodeNextFrame(AVFrame *AFrame, int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize);
public:
	MatroskaVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, const char *PP, int Threads, char *ErrorMsg, unsigned MsgSize);
	~MatroskaVideoSource();
    AVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

#ifdef HAALITS

class HaaliTSVideoSource : public VideoBase {
private:
	CComPtr<IMMContainer> pMMC;

	void Free(bool CloseCodec);
	int DecodeNextFrame(AVFrame *AFrame, int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize);
public:
	HaaliTSVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, const char *PP, int Threads, char *ErrorMsg, unsigned MsgSize);
	~HaaliTSVideoSource();
    AVFrameLite *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

#endif // HAALITS

#endif
