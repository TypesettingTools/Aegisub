// Copyright (c) 2005-2006, Rodrigo Braz Monteiro, Fredrik Mellbin
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
#include <wx/wxprec.h>
#include <wx/filename.h>
#include "audio_provider_lavc.h"
#include "options.h"


//////////////
// Constructor
LAVCAudioProvider::LAVCAudioProvider(wxString _filename) {
	// Set filename
	filename = _filename;

	// Init lavc variables
	codecContext = NULL;
	formatContext = NULL;
	codec = NULL;
	stream = NULL;
	frame = NULL;

	// Register types
	static bool avRegistered = false;
	if (!avRegistered) {
		av_register_all();
		avRegistered = true;
	}

	// Load audio
	LoadAudio(filename);
}


//////////////
// Destructor
LAVCAudioProvider::~LAVCAudioProvider() {
	Close();
}


////////////////
// Get filename
wxString LAVCAudioProvider::GetFilename() {
	return filename;
}


//////////////
// Load audio
void LAVCAudioProvider::LoadAudio(wxString file) {
	// Close first
	Close();

	try {
		// Open file
		int result = 0;
		result = av_open_input_file(&formatContext,filename.mb_str(wxConvLocal),NULL,0,NULL);
		if (result != 0) throw _T("Failed opening file.");

		// Get stream info
		result = av_find_stream_info(formatContext);
		if (result < 0) throw _T("Unable to read stream info");

		// Find audio stream
		audStream = -1;
		codecContext = NULL;
		for (int i=0;i<formatContext->nb_streams;i++) {
			codecContext = formatContext->streams[i]->codec;
			if (codecContext->codec_type == CODEC_TYPE_AUDIO) {
				stream = formatContext->streams[i];
				audStream = i;
				break;
			}
		}
		if (audStream == -1) throw _T("Could not find an audio stream");

		// Find codec
		codec = avcodec_find_decoder(codecContext->codec_id);
		if (!codec) throw _T("Could not find suitable audio decoder");

		// Enable truncation
		//if (codec->capabilities & CODEC_CAP_TRUNCATED) codecContext->flags |= CODEC_FLAG_TRUNCATED;

		// Open codec
		result = avcodec_open(codecContext,codec);
		if (result < 0) throw _T("Failed to open audio decoder");

		// TODO: rest of opening
	}

	// Catch errors
	catch (...) {
		Close();
		throw;
	}
}


//////////
// Unload
void LAVCAudioProvider::Close() {
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


/////////////
// Get audio
void LAVCAudioProvider::GetAudio(void *buf, __int64 start, __int64 count) {
	// TODO
}
