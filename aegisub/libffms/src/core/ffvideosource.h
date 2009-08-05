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

class FFMS_VideoSource {
private:
	pp_context_t *PPContext;
	pp_mode_t *PPMode;
	SwsContext *SWS;
	int LastFrameHeight;
	int LastFrameWidth;
	PixelFormat LastFramePixelFormat;
	int TargetHeight;
	int TargetWidth;
	int64_t TargetPixelFormats;
	int TargetResizer;
	AVPicture PPFrame;
	AVPicture SWSFrame;
protected:
	FFMS_VideoProperties VP;
	FFMS_Frame LocalFrame;
	AVFrame *DecodeFrame;
	int LastFrameNum;
	FFMS_Track Frames;
	int VideoTrack;
	int	CurrentFrame;
	AVCodecContext *CodecContext;

	FFMS_VideoSource(const char *SourceFile, FFMS_Index *Index, char *ErrorMsg, unsigned MsgSize);
	int InitPP(const char *PP, char *ErrorMsg, unsigned MsgSize);
	int ReAdjustPP(PixelFormat VPixelFormat, int Width, int Height, char *ErrorMsg, unsigned MsgSize);
	FFMS_Frame *OutputFrame(AVFrame *Frame, char *ErrorMsg, unsigned MsgSize);
public:
	virtual ~FFMS_VideoSource();
	const FFMS_VideoProperties& GetVideoProperties() { return VP; }
	FFMS_Track *GetFFTrack() { return &Frames; }
	virtual FFMS_Frame *GetFrame(int n, char *ErrorMsg, unsigned MsgSize) = 0;
	FFMS_Frame *GetFrameByTime(double Time, char *ErrorMsg, unsigned MsgSize);
	int SetOutputFormat(int64_t TargetFormats, int Width, int Height, int Resizer, char *ErrorMsg, unsigned MsgSize);
	int ReAdjustOutputFormat(int64_t TargetFormats, int Width, int Height, int Resizer, char *ErrorMsg, unsigned MsgSize);
	void ResetOutputFormat();
};

class FFLAVFVideo : public FFMS_VideoSource {
private:
	AVFormatContext *FormatContext;
	int SeekMode;

	void Free(bool CloseCodec);
	int DecodeNextFrame(int64_t *DTS, char *ErrorMsg, unsigned MsgSize);
public:
	FFLAVFVideo(const char *SourceFile, int Track, FFMS_Index *Index, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize);
	~FFLAVFVideo();
	FFMS_Frame *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

class FFMatroskaVideo : public FFMS_VideoSource {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
    CompressedStream *CS;
	char ErrorMessage[256];

	void Free(bool CloseCodec);
	int DecodeNextFrame(int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize);
public:
	FFMatroskaVideo(const char *SourceFile, int Track, FFMS_Index *Index, const char *PP, int Threads, char *ErrorMsg, unsigned MsgSize);
	~FFMatroskaVideo();
    FFMS_Frame *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

#ifdef HAALISOURCE

class FFHaaliVideo : public FFMS_VideoSource {
private:
	CComPtr<IMMContainer> pMMC;
	std::vector<uint8_t> CodecPrivate;
	AVBitStreamFilterContext *BitStreamFilter;

	void Free(bool CloseCodec);
	int DecodeNextFrame(int64_t *AFirstStartTime, char *ErrorMsg, unsigned MsgSize);
public:
	FFHaaliVideo(const char *SourceFile, int Track, FFMS_Index *Index, const char *PP, int Threads, int SourceMode, char *ErrorMsg, unsigned MsgSize);
	~FFHaaliVideo();
    FFMS_Frame *GetFrame(int n, char *ErrorMsg, unsigned MsgSize);
};

#endif // HAALISOURCE

#endif
