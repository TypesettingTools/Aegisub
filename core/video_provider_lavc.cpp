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
#include "video_provider_lavc.h"


///////////////
// Constructor
LAVCVideoProvider::LAVCVideoProvider(wxString filename, wxString subfilename, double zoom) {
	// Init variables
	codecContext = NULL;
	formatContext = NULL;
	codec = NULL;
	stream = NULL;
	frame = NULL;
	buffer = NULL;
	bufferSize = 0;
	vidStream = -1;

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

		// Check length
		if (stream->duration <= 0) throw _T("Returned invalid stream length");

		// Find codec
		codec = avcodec_find_decoder(codecContext->codec_id);
		if (!codec) throw _T("Could not find suitable video decoder");

		// Enable truncation
		if (codec->capabilities & CODEC_CAP_TRUNCATED) codecContext->flags |= CODEC_FLAG_TRUNCATED;

		// Open codec
		result = avcodec_open(codecContext,codec);
		if (result < 0) throw _T("Failed to open video decoder");

		// Allocate frame
		frame = avcodec_alloc_frame();
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
	// Clean buffer
	if (buffer) delete buffer;
	buffer = NULL;
	bufferSize = 0;

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


//////////////////
// Get next frame
void LAVCVideoProvider::GetNextFrame() {
	static AVPacket packet;
	static bool firstTime = true;
	static int bytesRemaining = 0;
	static uint8_t *rawdata;
	int decoded;
	int frameFinished;

	// First call
	if (firstTime) {
		firstTime = false;
		packet.data = NULL;
	}

	// Start processing packets
	bool run = true;
	while (run) {
		// Current packet has more bytes
		while (bytesRemaining > 0) {
			// Decode
			decoded = avcodec_decode_video(codecContext,frame,&frameFinished,rawdata,bytesRemaining);

			// Error?
			if (decoded < 0) throw _T("Error reading frame");

			// Update counts
			bytesRemaining -= decoded;
			rawdata += decoded;

			// Finished?
			if (frameFinished) return;
		}

		// Get a packet
		do {
			// Free packet
			if (packet.data != NULL) av_free_packet(&packet);
			
			// Get new
			int result = av_read_packet(formatContext,&packet);
			if (result < 0) {
				run = false;
				break;
			}
		} while (packet.stream_index != vidStream);

		// Get packet data
		if (run) {
			bytesRemaining = packet.size;
			rawdata = packet.data;
		}
	}

	// End of last pack
	decoded = avcodec_decode_video(codecContext,frame,&frameFinished,rawdata,bytesRemaining);
	if (packet.data != NULL) av_free_packet(&packet);

	// Frame finished?
	if (!frameFinished) throw _T("Unable to finish decoding frame");
}


///////////////////////////////
// Convert AVFrame to wxBitmap
wxBitmap LAVCVideoProvider::AVFrameToWX(AVFrame *source) {
	// Get sizes
	int w = codecContext->width;
	int h = codecContext->height;
	PixelFormat format = PIX_FMT_RGBA32;
	unsigned int size = avpicture_get_size(format,w,h);

	// Prepare buffer
	if (!buffer || bufferSize != size) {
		if (buffer) delete buffer;
		buffer = new uint8_t[size];
		bufferSize = size;
	}

	// Allocate RGB32 buffer
	AVFrame *frameRGB = avcodec_alloc_frame();
	avpicture_fill((AVPicture*) frameRGB, buffer, format, w, h);

	// Convert to RGB32
	img_convert((AVPicture*) frameRGB, format, (AVPicture*) source, codecContext->pix_fmt, w, h);

	// Convert to wxBitmap
	wxBitmap bmp((const char*) frameRGB->data[0],w,h,32);
	return bmp;
}


////////////////
// Refresh subs
void LAVCVideoProvider::RefreshSubtitles() {
}


/////////////
// Get frame
wxBitmap LAVCVideoProvider::GetFrame(int n) {
	// Get frame
	GetNextFrame();

	// Bitmap
	wxBitmap bmp = AVFrameToWX(frame);

	// Return
	return bmp;
}


//////////////////////
// Get frame as float
void LAVCVideoProvider::GetFloatFrame(float* Buffer, int n) {
}


////////////////
// Get position
int LAVCVideoProvider::GetPosition() {
	return 0;
}


////////////////////////
// Get number of frames
int LAVCVideoProvider::GetFrameCount() {
	return stream->duration;
}


//////////////////
// Get frame rate
double LAVCVideoProvider::GetFPS() {
	return double(stream->r_frame_rate.num) / double(stream->r_frame_rate.den);
}


////////////////////
// Set aspect ratio
void LAVCVideoProvider::SetDAR(double dar) {
}


////////////
// Set zoom
void LAVCVideoProvider::SetZoom(double zoom) {
}


////////////
// Get zoom
double LAVCVideoProvider::GetZoom() {
	return 1;
}


/////////////
// Get width
int LAVCVideoProvider::GetWidth() {
	return codecContext->coded_width;
}


//////////////
// Get height
int LAVCVideoProvider::GetHeight() {
	return codecContext->coded_height;
}


//////////////////////
// Get original width
int LAVCVideoProvider::GetSourceWidth() {
	return codecContext->coded_width;
}


///////////////////////
// Get original height
int LAVCVideoProvider::GetSourceHeight() {
	return codecContext->coded_height;
}


#endif
