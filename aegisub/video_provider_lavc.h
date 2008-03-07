// Copyright (c) 2006-2007, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers

#ifdef WITH_FFMPEG

#ifdef WIN32
#define EMULATE_INTTYPES
#endif
extern "C" {
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
}
#include "include/aegisub/video_provider.h"
#include "mkv_wrap.h"
#include "lavc_file.h"


///////////////////////
// LibAVCodec provider
class LAVCVideoProvider : public VideoProvider {
	friend class LAVCAudioProvider;
private:
	MatroskaWrapper mkv;

	LAVCFile *lavcfile;
	AVCodecContext *codecContext;
	AVStream *stream;
	AVCodec *codec;
	AVFrame *frame;
	int vidStream;
	
	AVFrame *frameRGB;
	uint8_t *bufferRGB;
	SwsContext *sws_context;
	
	int display_w;
	int display_h;

	wxArrayInt bytePos;

	bool isMkv;
	int64_t lastDecodeTime;
	int frameNumber;
	int length;
	AegiVideoFrame curFrame;
	bool validFrame;

	uint8_t *buffer1;
	uint8_t *buffer2;
	int buffer1Size;
	int buffer2Size;

	bool GetNextFrame();
	void LoadVideo(wxString filename, double fps);
	void Close();

protected:

public:
	LAVCVideoProvider(wxString filename, double fps);
	~LAVCVideoProvider();

	const AegiVideoFrame GetFrame(int n,int formatType);
	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();
	wxString GetDecoderName() { return _T("FFMpeg/libavcodec"); }
	bool IsNativelyByFrames() { return true; }
	int GetDesiredCacheSize() { return 8; }
};




///////////
// Factory
class LAVCVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(wxString video,double fps=0.0) { return new LAVCVideoProvider(video,fps); }
};

#endif
