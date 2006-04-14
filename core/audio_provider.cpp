// Copyright (c) 2005-2006, Rodrigo Braz Monteiro
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
#include "audio_provider_avs.h"
#include "audio_provider_lavc.h"
#include "audio_provider_ram.h"
#include "audio_provider_hd.h"
#include "options.h"
#include "audio_display.h"


///////////////
// Constructor
AudioProvider::AudioProvider() {
	raw = NULL;
}


//////////////
// Destructor
AudioProvider::~AudioProvider() {
	// Clear buffers
	delete raw;
}


//////////////////////////
// Get number of channels
int AudioProvider::GetChannels() {
	return channels;
}


//////////////////////////
// Get number of samples
__int64 AudioProvider::GetNumSamples() {
	return num_samples;
}


///////////////////
// Get sample rate
int AudioProvider::GetSampleRate() {
	return sample_rate;
}


////////////////////////
// Get bytes per sample
int AudioProvider::GetBytesPerSample() {
	return bytes_per_sample;
}


////////////////
// Get filename
wxString AudioProvider::GetFilename() {
	return filename;
}


////////////////
// Get waveform
void AudioProvider::GetWaveForm(int *min,int *peak,__int64 start,int w,int h,int samples,float scale) {
	// Setup
	int channels = GetChannels();
	int n = w * samples;
	for (int i=0;i<w;i++) {
		peak[i] = 0;
		min[i] = h;
	}

	// Prepare waveform
	int cur;
	int curvalue;

	// Prepare buffers
	int needLen = n*channels*bytes_per_sample;
	if (raw) {
		if (raw_len < needLen) {
			delete raw;
			raw = NULL;
		}
	}
	if (!raw) {
		raw_len = needLen;
		raw = (void*) new char[raw_len];
	}

	if (bytes_per_sample == 1) {
		// Read raw samples
		unsigned char *raw_char = (unsigned char*) raw;
		GetAudio(raw,start,n);
		int amplitude = h*scale;

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = h - (int(raw_char[i*channels])*amplitude)/0xFF;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}

	if (bytes_per_sample == 2) {
		// Read raw samples
		short *raw_short = (short*) raw;
		GetAudio(raw,start,n);
		int half_h = h/2;
		int half_amplitude = half_h * scale;

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = half_h - (int(raw_short[i*channels])*half_amplitude)/0x8000;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}
}


////////////////
// Get provider
AudioProvider *AudioProvider::GetAudioProvider(wxString filename, AudioDisplay *display) {
	// Prepare provider
	AudioProvider *provider = NULL;

	// Select provider
	#ifdef __WINDOWS__
	provider = new AvisynthAudioProvider(filename);
	#endif

	#ifdef USE_LAVC
	if (!provider) provider = new LAVCAudioProvider(filename);
	#endif

	// No provider found
	if (!provider) {
		throw _T("Could not initialize any audio provider.");
	}

	// Change provider to RAM/HD cache if needed
	int cacheMode = Options.AsInt(_T("Audio Cache"));
	if (cacheMode) {
		AudioProvider *final = NULL;
		
		// Convert to RAM
		if (cacheMode == 1) final = new RAMAudioProvider(provider);
		
		// Convert to HD
		if (cacheMode == 2) final = new HDAudioProvider(provider);
		
		// Reassign
		if (final) {
			delete provider;
			provider = final;
		}
	}

	// Return
	return provider;
}
