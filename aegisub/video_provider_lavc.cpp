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
#include "config.h"
#ifdef WITH_FFMPEG

#ifdef WIN32
#define EMULATE_INTTYPES
#endif
extern "C" {
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
}
#include <wx/wxprec.h>
#include <wx/image.h>
#include <algorithm>
#include "video_provider.h"
#include "mkv_wrap.h"
#include "lavc_file.h"
#include "utils.h"
#include "vfr.h"
#include "ass_file.h"


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
	const AegiVideoFrame DoGetFrame(int n);

public:
	LAVCVideoProvider(wxString filename, double fps);
	~LAVCVideoProvider();

	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();
	wxString GetDecoderName() { return _T("FFMpeg/libavcodec"); }
	bool IsNativelyByFrames() { return true; }
};




///////////
// Factory
class LAVCVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(wxString video,double fps=0.0) { return new LAVCVideoProvider(video,fps); }
	LAVCVideoProviderFactory() : VideoProviderFactory(_T("ffmpeg")) {}
} registerLAVCVideo;


///////////////
// Constructor
LAVCVideoProvider::LAVCVideoProvider(wxString filename,double fps) {
	// Init variables
	codecContext = NULL;
	lavcfile = NULL;
	codec = NULL;
	stream = NULL;
	frame = NULL;
	buffer1 = NULL;
	buffer2 = NULL;
	buffer1Size = 0;
	buffer2Size = 0;
	vidStream = -1;
	validFrame = false;

	// Load
	SetCacheMax(8);
	LoadVideo(filename,fps);
}


//////////////
// Destructor
LAVCVideoProvider::~LAVCVideoProvider() {
	Close();
}


//////////////
// Load video
void LAVCVideoProvider::LoadVideo(wxString filename, double fps) {
	// Close first
	Close();

	lavcfile = LAVCFile::Create(filename);
	// Load
	try {
		int result = 0;

		// Find video stream
		vidStream = -1;
		codecContext = NULL;
		for (int i=0;i<lavcfile->fctx->nb_streams;i++) {
			codecContext = lavcfile->fctx->streams[i]->codec;
			if (codecContext->codec_type == CODEC_TYPE_VIDEO) {
				stream = lavcfile->fctx->streams[i];
				vidStream = i;
				break;
			}
		}
		if (vidStream == -1) throw _T("Could not find a video stream");

		// Find codec
		codec = avcodec_find_decoder(codecContext->codec_id);
		if (!codec) throw _T("Could not find suitable video decoder");

		// Enable truncation
		//if (codec->capabilities & CODEC_CAP_TRUNCATED) codecContext->flags |= CODEC_FLAG_TRUNCATED;

		// Open codec
		result = avcodec_open(codecContext,codec);
		if (result < 0) throw _T("Failed to open video decoder");

		// Check length
		length = stream->duration;
#if 0
		isMkv = false;
		length = stream->duration;
		if (length <= 0) {
			if (strcmp(formatContext->iformat->name,"matroska") == 0) {
				//throw _T("FFmpeg fails at seeking Matroska. If you have any idea on how to fix it, Aegisub is open source.");
				mkv.Open(filename);
				length = mkv.GetFrameCount();
				bytePos = mkv.GetBytePositions();
				isMkv = true;
			}
			if (length <= 0) throw _T("Returned invalid stream length");
		}
#endif

		// Allocate frame
		frame = avcodec_alloc_frame();

		// Set frame
		frameNumber = -1;
	}

	// Catch errors
	catch (...) {
		Close();
		throw;
	}
}


///////////////
// Close video
void LAVCVideoProvider::Close() {
	// Close mkv
#if 0
	if (isMkv) mkv.Close();
#endif

	// Clean buffers
	if (buffer1) delete buffer1;
	if (buffer2) delete buffer2;
	buffer1 = NULL;
	buffer2 = NULL;
	buffer1Size = 0;
	buffer2Size = 0;

	// Clean frame
	if (frame) av_free((void*)frame);
	frame = NULL;
	
	// Close codec context
	if (codec && codecContext) avcodec_close(codecContext);
	codecContext = NULL;
	codec = NULL;

	// Close format context
	if (lavcfile)
		lavcfile->Release();
	lavcfile = NULL;
}


//////////////////
// Get next frame
bool LAVCVideoProvider::GetNextFrame() {
	// Read packet
	AVPacket packet;
	while (av_read_frame(lavcfile->fctx, &packet)>=0) {
		// Check if packet is part of video stream
		if(packet.stream_index == vidStream) {
			// Decode frame
			int frameFinished;
			avcodec_decode_video(codecContext, frame, &frameFinished, packet.data, packet.size);

			// Success?
			if(frameFinished) {
				// Set time
				lastDecodeTime = packet.dts;

				// Free packet
				av_free_packet(&packet);
				return true;
			}
		}
    }

	// No more packets
	return false;
}


/////////////////////////////////
//// Convert AVFrame to wxBitmap
/*
wxBitmap LAVCVideoProvider::AVFrameToWX(AVFrame *source, int n) {
	// Get sizes
	int w = codecContext->width;
	int h = codecContext->height;
//#ifdef __WINDOWS__
//	PixelFormat format = PIX_FMT_RGBA32;
//#else
	PixelFormat format = PIX_FMT_RGB24;
//#endif
	unsigned int size1 = avpicture_get_size(codecContext->pix_fmt,display_w,display_h);
	unsigned int size2 = avpicture_get_size(format,display_w,display_h);

	// Prepare buffers
	if (!buffer1 || buffer1Size != size1) {
		if (buffer1) delete buffer1;
		buffer1 = new uint8_t[size1];
		buffer1Size = size1;
	}
	if (!buffer2 || buffer2Size != size2) {
		if (buffer2) delete buffer2;
		buffer2 = new uint8_t[size2];
		buffer2Size = size2;
	}

	// Resize
	AVFrame *resized;
	bool resize = w != display_w || h != display_h;
	if (resize) {
		// Allocate
		unsigned int resSize = avpicture_get_size(codecContext->pix_fmt,display_w,display_h);
		resized = avcodec_alloc_frame();
		avpicture_fill((AVPicture*) resized, buffer1, codecContext->pix_fmt, display_w, display_h);

		// Resize
		ImgReSampleContext *resampleContext = img_resample_init(display_w,display_h,w,h);
		img_resample(resampleContext,(AVPicture*) resized,(AVPicture*) source);
		img_resample_close(resampleContext);

		// Set new w/h
		w = display_w;
		h = display_h;
	}
	else resized = source;

	// Allocate RGB32 buffer
	AVFrame *frameRGB = avcodec_alloc_frame();
	avpicture_fill((AVPicture*) frameRGB, buffer2, format, w, h);

	// Convert to RGB32
	img_convert((AVPicture*) frameRGB, format, (AVPicture*) resized, codecContext->pix_fmt, w, h);

	// Convert to wxBitmap
	wxImage img(w, h, false);
	unsigned char *data = (unsigned char *)malloc(w * h * 3);
	memcpy(data, frameRGB->data[0], w * h * 3);
	img.SetData(data);
	if (overlay)
		overlay->Render(img, VFR_Input.GetTimeAtFrame(n));

	wxBitmap bmp(img);

	av_free(frameRGB);
	if (resized != source)
		av_free(resized);
	return bmp;
}
*/


/////////////
// Get frame
const AegiVideoFrame LAVCVideoProvider::DoGetFrame(int n) {
	// Return stored frame
	n = MID(0,n,GetFrameCount()-1);
	if (n == frameNumber) {
		if (!validFrame) validFrame = true;
		return curFrame;
	}

	// Following frame, just get it
	if (n == frameNumber+1) {
		GetNextFrame();
	}

	// Needs to seek
	else {
		// Prepare seek
		int64_t seekTo;
		int result = 0;

#if 0
		// Get time to seek to
		if (isMkv) {
			//int64_t base = AV_TIME_BASE;
			//int64_t time = VFR_Output.GetTimeAtFrame(n,true) * base / 1000000;
			//seekTo = av_rescale(time,stream->time_base.den,AV_TIME_BASE * int64_t(stream->time_base.num));
			//seekTo = int64_t(n) * 1000 * stream->r_frame_rate.den / stream->r_frame_rate.num;
			//seekTo = bytePos[n];

			//result = av_seek_frame(formatContext,vidStream,seekTo,AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_BYTE);

			// Prepare mkv seek
			ulonglong startTime, endTime, filePos;
			unsigned int rt, frameSize, frameFlags;
			ulonglong targetTime = (int64_t)(VFR_Output.GetTimeAtFrame(n,true,true))*1000000;
			//ulonglong targetTime = (int64_t)(n) * 1000 * stream->r_frame_rate.den / stream->r_frame_rate.num;
			//ulonglong targetTime = mkv.rawFrames[n].time * 1000000;
			mkv_Seek(mkv.file,targetTime,MKVF_SEEK_TO_PREV_KEYFRAME);

			// Seek
			if (mkv_ReadFrame(mkv.file,0,&rt,&startTime,&endTime,&filePos,&frameSize,&frameFlags) == 0) {
				result = av_seek_frame(formatContext,vidStream,filePos,AVSEEK_FLAG_BYTE | AVSEEK_FLAG_BACKWARD);
				int curpos = 0;
				for (unsigned int i=0;i<mkv.rawFrames.size();i++) {
					if (mkv.rawFrames[i].time == startTime / 1000000.0) curpos = i;
				}
				int seek = n - curpos;
				for (int i=0;i<seek;i++) {
					GetNextFrame();
				}
			}
		}

		// Constant frame rate
		else {
#endif
			seekTo = n;
			result = av_seek_frame(lavcfile->fctx,vidStream,seekTo,AVSEEK_FLAG_BACKWARD);

			// Seek to keyframe
			if (result == 0) {
				avcodec_flush_buffers(codecContext);

				// Seek until final frame
				bool ok = true;
				do {
					ok = GetNextFrame();
				} while (lastDecodeTime <= n && ok);
			}

			// Failed seeking
			else {
				GetNextFrame();
			}
#if 0
		}
#endif
	}

	// Convert to RGB32
	AVFrame *useFrame = frame;
	AVFrame *frameRGB = NULL;
	//if (true) {
	//	// Set properties
	//	int w = codecContext->width;
	//	int h = codecContext->height;
	//	PixelFormat convFormat = PIX_FMT_RGB24;
	//	unsigned int dstSize = avpicture_get_size(convFormat,w,h);

	//	// Allocate RGB32 buffer
	//	frameRGB = avcodec_alloc_frame();
	//	uint8_t *buffer = new uint8_t[dstSize];
	//	avpicture_fill((AVPicture*) frameRGB, buffer, convFormat, w, h);

	//	// Convert to RGB32
	//	img_convert((AVPicture*) frameRGB, convFormat, (AVPicture*) frame, codecContext->pix_fmt, w, h);
	//	useFrame = frameRGB;
	//}

	// Get aegisub frame
	AegiVideoFrame &final = curFrame;
	if (frame) {
		// Set AegiVideoFrame
		PixelFormat format = codecContext->pix_fmt;
		unsigned int size = avpicture_get_size(format,codecContext->width,codecContext->height);
		final.w = codecContext->width;
		final.h = codecContext->height;
		final.flipped = false;
		final.invertChannels = false;

		// Set format
		switch (format) {
			case PIX_FMT_BGR24: final.invertChannels = true;
			case PIX_FMT_RGB24: final.format = FORMAT_RGB24; break;
			#ifdef __WINDOWS__
			case PIX_FMT_BGR32: final.invertChannels = true;
			#endif
			case PIX_FMT_RGB32: final.format = FORMAT_RGB32; break;
			case PIX_FMT_YUYV422: final.format = FORMAT_YUY2; break;
			case PIX_FMT_YUV420P: final.format = FORMAT_YV12; break;
			default: throw _T("ffmpeg returned an unknown frame format.");
		}

		// Allocate
		for (int i=0;i<4;i++) final.pitch[i] = useFrame->linesize[i];
		final.Allocate();

		// Copy data
		if (final.format == FORMAT_YV12) {
			memcpy(final.data[0],useFrame->data[0],useFrame->linesize[0] * final.h);
			memcpy(final.data[1],useFrame->data[1],useFrame->linesize[1] * final.h / 2);
			memcpy(final.data[2],useFrame->data[2],useFrame->linesize[2] * final.h / 2);
		}
		else memcpy(final.data[0],useFrame->data[0],size);
	}

	// No frame available
	else final = AegiVideoFrame(GetWidth(),GetHeight());

	// Set current frame
	validFrame = true;
	//curFrame = final;
	frameNumber = n;
	if (frameRGB) av_free(frameRGB);

	// Return
	return curFrame;
}


////////////////
// Get position
int LAVCVideoProvider::GetPosition() {
	return frameNumber;
}


////////////////////////
// Get number of frames
int LAVCVideoProvider::GetFrameCount() {
	return length;
}


//////////////////
// Get frame rate
double LAVCVideoProvider::GetFPS() {
	return double(stream->r_frame_rate.num) / double(stream->r_frame_rate.den);
}


//////////////////////
// Get original width
int LAVCVideoProvider::GetWidth() {
	return codecContext->width;
}


///////////////////////
// Get original height
int LAVCVideoProvider::GetHeight() {
	return codecContext->height;
}

#endif // WITH_FFMPEG
