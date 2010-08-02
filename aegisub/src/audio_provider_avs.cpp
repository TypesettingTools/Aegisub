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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_provider_avs.cpp
/// @brief Avisynth-based audio provider
/// @ingroup audio_input
///

#include "config.h"

#ifdef WITH_AVISYNTH

#ifndef AGI_PRE
#include <Mmreg.h>
#include <time.h>

#include <wx/filename.h>
#endif

#include "audio_provider_avs.h"
#include "charset_conv.h"
#include "compat.h"
#include "main.h"
#include "standard_paths.h"
#include "utils.h"

/// @brief Constructor 
/// @param _filename 
///
AvisynthAudioProvider::AvisynthAudioProvider(wxString filename) try : filename(filename) {
	AVSValue script;
	wxMutexLocker lock(AviSynthMutex);

	wxFileName fn(filename);
	if (!fn.FileExists())
		throw agi::FileNotFoundError(STD_STR(filename));

	// Include
	if (filename.EndsWith(_T(".avs"))) {
		char *fname = env->SaveString(fn.GetShortPath().mb_str(csConvLocal));
		script = env->Invoke("Import", fname);
	}

	// Use DirectShowSource
	else {
		const char * argnames[3] = { 0, "video", "audio" };
		AVSValue args[3] = { env->SaveString(fn.GetShortPath().mb_str(csConvLocal)), false, true };

		// Load DirectShowSource.dll from app dir if it exists
		wxFileName dsspath(StandardPaths::DecodePath(_T("?data/DirectShowSource.dll")));
		if (dsspath.FileExists()) {
			env->Invoke("LoadPlugin",env->SaveString(dsspath.GetShortPath().mb_str(csConvLocal)));
		}

		// Load audio with DSS if it exists
		if (env->FunctionExists("DirectShowSource")) {
			script = env->Invoke("DirectShowSource", AVSValue(args,3),argnames);
		}
		// Otherwise fail
		else {
			throw AudioOpenError("No suitable audio source filter found. Try placing DirectShowSource.dll in the Aegisub application directory.");
		}
	}

	LoadFromClip(script);
}
catch (AvisynthError &err) {
	throw AudioOpenError("Avisynth error: " + std::string(err.msg));
}

/// @brief Read from environment 
/// @param _clip 
///
void AvisynthAudioProvider::LoadFromClip(AVSValue _clip) {	
	AVSValue script;

	// Check if it has audio
	VideoInfo vi = _clip.AsClip()->GetVideoInfo();
	if (!vi.HasAudio()) throw AudioOpenError("No audio found.");

	// Convert to one channel
	char buffer[1024];
	strcpy(buffer,lagi_wxString(OPT_GET("Audio/Downmixer")->GetString()).mb_str(csConvLocal));
	script = env->Invoke(buffer, _clip);

	// Convert to 16 bits per sample
	script = env->Invoke("ConvertAudioTo16bit", script);
	vi = script.AsClip()->GetVideoInfo();

	// Convert sample rate
	int setsample = OPT_GET("Provider/Audio/AVS/Sample Rate")->GetInt();
	if (vi.SamplesPerSecond() < 32000) setsample = 44100;
	if (setsample != 0) {
		AVSValue args[2] = { script, setsample };
		script = env->Invoke("ResampleAudio", AVSValue(args,2));
	}

	// Set clip
	PClip tempclip = script.AsClip();
	vi = tempclip->GetVideoInfo();

	// Read properties
	channels = vi.AudioChannels();
	num_samples = vi.num_audio_samples;
	sample_rate = vi.SamplesPerSecond();
	bytes_per_sample = vi.BytesPerAudioSample();

	clip = tempclip;
}

/// @brief Get audio 
/// @param buf   
/// @param start 
/// @param count 
///
void AvisynthAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) {
	// Requested beyond the length of audio
	if (start+count > num_samples) {
		int64_t oldcount = count;
		count = num_samples-start;
		if (count < 0) count = 0;

		// Fill beyond with zero
		if (bytes_per_sample == 1) {
			char *temp = (char *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
		if (bytes_per_sample == 2) {
			short *temp = (short *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
	}

	if (count) {
		clip->GetAudio(buf,start,count,env);
	}
}
#endif
