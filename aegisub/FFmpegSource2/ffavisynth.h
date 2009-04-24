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

#ifndef FFAVISYNTH_H
#define FFAVISYNTH_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
}

#include <windows.h>
#include "avisynth.h"
#include "ffms.h"

class AvisynthVideoSource : public IClip {
private:
	VideoInfo VI;
	VideoBase *VS;
	SwsContext *SWS;
	PixelFormat ConvertToFormat;
	int FPSNum;
	int FPSDen;

	void InitOutputFormat(enum PixelFormat CurrentFormat, IScriptEnvironment *Env);
	PVideoFrame OutputFrame(const AVFrameLite *SrcPicture, IScriptEnvironment *Env);
public:
	AvisynthVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, int FPSNum, int FPSDen, const char *PP, int Threads, int SeekMode, IScriptEnvironment* Env, char *ErrorMsg, unsigned MsgSize);
	~AvisynthVideoSource();
	bool __stdcall GetParity(int n) { return false; }
	void __stdcall SetCacheHints(int cachehints, int frame_range) { }
	const VideoInfo& __stdcall GetVideoInfo() { return VI; }
	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env) { }
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *Env);
};

class AvisynthAudioSource : public IClip {
private:
	VideoInfo VI;
	AudioBase *AS;
public:
	AvisynthAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, IScriptEnvironment* Env, char *ErrorMsg, unsigned MsgSize);
	~AvisynthAudioSource();
	bool __stdcall GetParity(int n) { return false; }
	void __stdcall SetCacheHints(int cachehints, int frame_range) { }
	const VideoInfo& __stdcall GetVideoInfo() { return VI; }
	void __stdcall GetAudio(void* Buf, __int64 Start, __int64 Count, IScriptEnvironment *Env);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *Env) { return NULL; };
};

#endif
