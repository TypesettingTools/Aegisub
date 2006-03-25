// Copyright (c) 2006, Rodrigo Braz Monteiro
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
#ifdef USE_LAVC
#include <wx/wxprec.h>
#include <algorithm>
#include "video_provider_lavc.h"
#include "utils.h"
#include "vfr.h"
#include "mkv_wrap.h"


///////////////
// Constructor
LAVCVideoProvider::LAVCVideoProvider(wxString filename, wxString subfilename) {
	char temp[1024];
	strcpy(temp,subfilename.mb_str(wxConvUTF8));

	// Init variables
	codecContext = NULL;
	formatContext = NULL;
	codec = NULL;
	stream = NULL;
	frame = NULL;
	buffer1 = NULL;
	buffer2 = NULL;
	buffer1Size = 0;
	buffer2Size = 0;
	vidStream = -1;
	zoom = 1.0;
	validFrame = false;

	// Register types
	static bool avRegistered = false;
	if (!avRegistered) {
		av_register_all();
		avRegistered = true;
	}

	// Load
	LoadVideo(filename);
}


//////////////
// Destructor
LAVCVideoProvider::~LAVCVideoProvider() {
	Close();
}


//////////////
// Load video
void LAVCVideoProvider::LoadVideo(wxString filename) {
	// Close first
	Close();

	// Load
	try {
		// Open file
		int result = 0;
		result = av_open_input_file(&formatContext,filename.mb_str(wxConvLocal),NULL,0,NULL);
		if (result != 0) throw _T("Failed opening file.");

		// Get stream info
		result = av_find_stream_info(formatContext);
		if (result < 0) throw _T("Unable to read stream info");

		// Find video stream
		vidStream = -1;
		codecContext = NULL;
		for (int i=0;i<formatContext->nb_streams;i++) {
			codecContext = formatContext->streams[i]->codec;
			if (codecContext->codec_type == CODEC_TYPE_VIDEO) {
				stream = formatContext->streams[i];
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

		// Set size
		dar = double(GetSourceWidth()) / GetSourceHeight();
		UpdateDisplaySize();

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
	if (isMkv) mkv.Close();

	// Clean buffers
	if (buffer1) delete buffer1;
	if (buffer2) delete buffer2;
	buffer1 = NULL;
	buffer2 = NULL;
	buffer1Size = 0;
	buffer2Size = 0;

	// Clean frame
	if (frame) av_free(frame);
	frame = NULL;
	
	// Close codec context
	if (codec && codecContext) avcodec_close(codecContext);
	codecContext = NULL;
	codec = NULL;

	// Close format context
	if (formatContext) av_close_input_file(formatContext);
	formatContext = NULL;
}


///////////////////////
// Update display size
void LAVCVideoProvider::UpdateDisplaySize() {
	// Set
	display_w = MID(16,int((GetSourceHeight() * zoom * dar)+0.5),4096);
	display_h = MID(16,int((GetSourceHeight() * zoom)+0.5),4096);
}


//////////////////
// Get next frame
bool LAVCVideoProvider::GetNextFrame() {
	// Read packet
	AVPacket packet;
	while (av_read_frame(formatContext, &packet)>=0) {
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


///////////////////////////////
// Convert AVFrame to wxBitmap
wxBitmap LAVCVideoProvider::AVFrameToWX(AVFrame *source) {
	// Get sizes
	int w = codecContext->width;
	int h = codecContext->height;
#ifdef __WINDOWS__
	PixelFormat format = PIX_FMT_RGBA32;
#else
	PixelFormat format = PIX_FMT_RGB24;
#endif
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
#ifdef __WINDOWS__
	wxBitmap bmp((const char*) frameRGB->data[0],w,h,32);
#else
	wxImage img(w, h, false);
	unsigned char *data = (unsigned char *)malloc(w * h * 3);
	memcpy(data, frameRGB->data[0], w * h * 3);
	img.SetData(data);
	wxBitmap bmp(img);
#endif

	av_free(frameRGB);
	if (resized != source)
		av_free(resized);
	return bmp;
}


////////////////
// Refresh subs
void LAVCVideoProvider::RefreshSubtitles() {
}


/////////////
// Get frame
wxBitmap LAVCVideoProvider::GetFrame(int n) {
	// Return stored frame
	n = MID(0,n,GetFrameCount()-1);
	if (n == frameNumber) {
		if (!validFrame) {
			curFrame = AVFrameToWX(frame);
			validFrame = true;
		}
		return curFrame;
	}

	// Following frame, just get it
	if (n == frameNumber+1) {
		GetNextFrame();
		//wxLogMessage(wxString::Format(_T("%i"),lastDecodeTime));
	}

	// Needs to seek
	else {
		// Prepare seek
		__int64 seekTo;
		int result = 0;

		// Get time to seek to
		if (isMkv) {
			//__int64 base = AV_TIME_BASE;
			//__int64 time = VFR_Output.GetTimeAtFrame(n,true) * base / 1000000;
			//seekTo = av_rescale(time,stream->time_base.den,AV_TIME_BASE * __int64(stream->time_base.num));
			//seekTo = __int64(n) * 1000 * stream->r_frame_rate.den / stream->r_frame_rate.num;
			//seekTo = bytePos[n];

			//result = av_seek_frame(formatContext,vidStream,seekTo,AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_BYTE);

			// Prepare mkv seek
			ulonglong startTime, endTime, filePos;
			unsigned int rt, frameSize, frameFlags;
			ulonglong targetTime = __int64(VFR_Output.GetTimeAtFrame(n,true,true))*1000000;
			//ulonglong targetTime = __int64(n) * 1000 * stream->r_frame_rate.den / stream->r_frame_rate.num;
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
			seekTo = n;
			result = av_seek_frame(formatContext,vidStream,seekTo,AVSEEK_FLAG_BACKWARD);

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
		}
	}

	// Bitmap
	wxBitmap bmp;
	if (frame) bmp = AVFrameToWX(frame);
	else bmp = wxBitmap(GetWidth(),GetHeight());

	// Set current frame
	validFrame = true;
	curFrame = bmp;
	frameNumber = n;

	// Return
	return curFrame;
}


//////////////////////
// Get frame as float
void LAVCVideoProvider::GetFloatFrame(float* Buffer, int n) {
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


////////////////////
// Set aspect ratio
void LAVCVideoProvider::SetDAR(double _dar) {
	dar = _dar;
	validFrame = false;
	UpdateDisplaySize();
}


////////////
// Set zoom
void LAVCVideoProvider::SetZoom(double _zoom) {
	zoom = _zoom;
	validFrame = false;
	UpdateDisplaySize();
}


////////////
// Get zoom
double LAVCVideoProvider::GetZoom() {
	return zoom;
}


/////////////
// Get width
int LAVCVideoProvider::GetWidth() {
	return display_w;
}


//////////////
// Get height
int LAVCVideoProvider::GetHeight() {
	return display_h;
}


//////////////////////
// Get original width
int LAVCVideoProvider::GetSourceWidth() {
	return codecContext->width;
}


///////////////////////
// Get original height
int LAVCVideoProvider::GetSourceHeight() {
	return codecContext->height;
}


#endif
