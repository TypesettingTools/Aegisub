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

/// @file audio_provider_avs.cpp
/// @brief Avisynth-based audio provider
/// @ingroup audio_input
///

#ifdef WITH_AVISYNTH
#include "include/aegisub/audio_provider.h"

#include "avisynth.h"
#include "avisynth_wrap.h"
#include "audio_controller.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/access.h>
#include <libaegisub/charset_conv.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/make_unique.h>

#include <mutex>

namespace {
class AvisynthAudioProvider final : public AudioProvider {
	AviSynthWrapper avs_wrapper;
	PClip clip;

	void LoadFromClip(AVSValue clip);
	void FillBuffer(void *buf, int64_t start, int64_t count) const;

public:
	AvisynthAudioProvider(agi::fs::path const& filename);

	bool NeedsCache() const override { return true; }
};

AvisynthAudioProvider::AvisynthAudioProvider(agi::fs::path const& filename) {
	agi::acs::CheckFileRead(filename);

	std::lock_guard<std::mutex> lock(avs_wrapper.GetMutex());

	try {
		IScriptEnvironment *env = avs_wrapper.GetEnv();

		// Include
		if (agi::fs::HasExtension(filename, "avs"))
			LoadFromClip(env->Invoke("Import", env->SaveString(agi::fs::ShortName(filename).c_str())));
		// Use DirectShowSource
		else {
			const char * argnames[3] = { 0, "video", "audio" };
			AVSValue args[3] = { env->SaveString(agi::fs::ShortName(filename).c_str()), false, true };

			// Load DirectShowSource.dll from app dir if it exists
			agi::fs::path dsspath(config::path->Decode("?data/DirectShowSource.dll"));
			if (agi::fs::FileExists(dsspath))
				env->Invoke("LoadPlugin", env->SaveString(agi::fs::ShortName(dsspath).c_str()));

			// Load audio with DSS if it exists
			if (env->FunctionExists("DirectShowSource"))
				LoadFromClip(env->Invoke("DirectShowSource", AVSValue(args, 3), argnames));
			// Otherwise fail
			else
				throw agi::AudioProviderOpenError("No suitable audio source filter found. Try placing DirectShowSource.dll in the Aegisub application directory.", 0);
		}
	}
	catch (AvisynthError &err) {
		std::string errmsg(err.msg);
		if (errmsg.find("filter graph manager won't talk to me") != errmsg.npos)
			throw agi::AudioDataNotFoundError("Avisynth error: " + errmsg, 0);
		else
			throw agi::AudioProviderOpenError("Avisynth error: " + errmsg, 0);
	}
}

void AvisynthAudioProvider::LoadFromClip(AVSValue clip) {
	// Check if it has audio
	VideoInfo vi = clip.AsClip()->GetVideoInfo();
	if (!vi.HasAudio()) throw agi::AudioDataNotFoundError("No audio found.", 0);

	IScriptEnvironment *env = avs_wrapper.GetEnv();

	// Convert to one channel
	AVSValue script = env->Invoke(OPT_GET("Audio/Downmixer")->GetString().c_str(), clip);

	// Convert to 16 bits per sample
	script = env->Invoke("ConvertAudioTo16bit", script);
	vi = script.AsClip()->GetVideoInfo();

	// Convert sample rate
	int setsample = OPT_GET("Provider/Audio/AVS/Sample Rate")->GetInt();
	if (setsample == 0 && vi.SamplesPerSecond() < 32000)
		setsample = 44100;
	if (setsample != 0) {
		AVSValue args[2] = { script, setsample };
		script = env->Invoke("ResampleAudio", AVSValue(args, 2));
	}

	// Set clip
	PClip tempclip = script.AsClip();
	vi = tempclip->GetVideoInfo();

	// Read properties
	channels = vi.AudioChannels();
	decoded_samples = num_samples = vi.num_audio_samples;
	sample_rate = vi.SamplesPerSecond();
	bytes_per_sample = vi.BytesPerAudioSample();
	float_samples = false;

	this->clip = tempclip;
}

void AvisynthAudioProvider::FillBuffer(void *buf, int64_t start, int64_t count) const {
	clip->GetAudio(buf, start, count, avs_wrapper.GetEnv());
}
}

std::unique_ptr<AudioProvider> CreateAvisynthAudioProvider(agi::fs::path const& file, agi::BackgroundRunner *) {
	return agi::make_unique<AvisynthAudioProvider>(file);
}
#endif
