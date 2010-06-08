// Copyright (c) 2005-2007, Rodrigo Braz Monteiro
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

/// @file audio_player_portaudio.cpp
/// @brief PortAudio v18-based audio output
/// @ingroup audio_output
///


#include "config.h"

#ifdef WITH_PORTAUDIO


// Headers
#include <libaegisub/log.h>

#include "audio_player_portaudio.h"
#include "audio_provider_manager.h"
#include "charset_conv.h"
#include "main.h"
#include "options.h"
#include "utils.h"
#include <wx/log.h>

//#define PORTAUDIO_DEBUG

// Init reference counter
int PortAudioPlayer::pa_refcount = 0;

/// @brief Constructor
PortAudioPlayer::PortAudioPlayer() {
	// Initialize portaudio
	if (!pa_refcount) {
		PaError err = Pa_Initialize();

		if (err != paNoError) {
			static wchar_t errormsg[2048];
			swprintf(errormsg, 2048, L"Failed opening PortAudio: %s", Pa_GetErrorText(err));
			throw (const wchar_t *)errormsg;
		}
		pa_refcount++;
	}

	volume = 1.0f;
	pos.pa_start = 0.0;
}


/// @brief Destructor
PortAudioPlayer::~PortAudioPlayer() {
	// Deinit portaudio
	if (!--pa_refcount) Pa_Terminate();
}


/// @brief Open stream
void PortAudioPlayer::OpenStream() {
	// Open stream
	PaStreamParameters pa_output_p;

	int pa_config_default = OPT_GET("Player/Audio/PortAudio/Device")->GetInt();
	PaDeviceIndex pa_device;

	if (pa_config_default < 0) {
		pa_device = Pa_GetDefaultOutputDevice();
		LOG_D("audio/player/portaudio") << "using default output device:" << pa_device;
	} else {
		pa_device = pa_config_default;
		LOG_D("audio/player/portaudio") << "using config device: " << pa_device;
	}

	pa_output_p.device = pa_device;
	pa_output_p.channelCount = provider->GetChannels();
	pa_output_p.sampleFormat = paInt16;
	pa_output_p.suggestedLatency = Pa_GetDeviceInfo(pa_device)->defaultLowOutputLatency;
	pa_output_p.hostApiSpecificStreamInfo = NULL;

	LOG_D("audio/player/portaudio") << "output channels: " << pa_output_p.channelCount << ", latency: " << pa_output_p.suggestedLatency << "  sample rate: " << pa_output_p.sampleFormat;

	PaError err = Pa_OpenStream(&stream, NULL, &pa_output_p, provider->GetSampleRate(), 256, paPrimeOutputBuffersUsingStreamCallback, paCallback, this);

	if (err != paNoError) {

		const PaHostErrorInfo *pa_err = Pa_GetLastHostErrorInfo();
		LOG_D_IF(pa_err->errorCode != 0, "audio/player/portaudio") << "HostError: API: " << pa_err->hostApiType << ", " << pa_err->errorText << ", " << pa_err->errorCode;
		LOG_D("audio/player/portaudio") << "Failed initializing PortAudio stream with error: " << Pa_GetErrorText(err);
		throw wxString(_T("Failed initializing PortAudio stream with error: ") + wxString(Pa_GetErrorText(err),csConvLocal));
	}
}


/// @brief Close stream
void PortAudioPlayer::CloseStream() {
	Stop(false);
	Pa_CloseStream(stream);
}


/// @brief Called when the callback has finished.
/// @param userData Local data to be handed to the callback.
void PortAudioPlayer::paStreamFinishedCallback(void *userData) {
    PortAudioPlayer *player = (PortAudioPlayer *) userData;

	if (player->displayTimer) {
		player->displayTimer->Stop();
	}
	LOG_D("audio/player/portaudio") << "stopping stream";
}


/// @brief Play audio.
/// @param start Start position.
/// @param count Frame count
void PortAudioPlayer::Play(int64_t start,int64_t count) {
	PaError err;

	// Set values
	pos.end = start + count;
	pos.current = start;
	pos.start = start;

	// Start playing
	if (!IsPlaying()) {

		err = Pa_SetStreamFinishedCallback(stream, paStreamFinishedCallback);

		if (err != paNoError) {
			LOG_D("audio/player/portaudio") << "could not set FinishedCallback";
			return;
		}

		err = Pa_StartStream(stream);

		if (err != paNoError) {
			LOG_D("audio/player/portaudio") << "error playing stream";
			return;
		}
	}
	pos.pa_start = Pa_GetStreamTime(stream);

	// Update timer
	if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(15);
}


/// @brief Stop Playback
/// @param timerToo Stop display timer?
///
void PortAudioPlayer::Stop(bool timerToo) {
	// Stop stream
	Pa_StopStream (stream);

	// Stop timer
	if (timerToo && displayTimer) {
		displayTimer->Stop();
	}
}


/// @brief PortAudio callback, used to fill buffer for playback, and prime the playback buffer.
/// @param inputBuffer     Input buffer.
/// @param outputBuffer    Output buffer.
/// @param framesPerBuffer Frames per buffer.
/// @param timeInfo        PortAudio time information.
/// @param statusFlags     Status flags
/// @param userData        Local data to hand callback
/// @return Whether to stop playback.
///
int PortAudioPlayer::paCallback(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData) {

	// Get provider
	PortAudioPlayer *player = (PortAudioPlayer *) userData;
	AudioProvider *provider = player->GetProvider();

#ifdef PORTAUDIO_DEBUG
	printf("paCallBack: pos.current: %lld  pos.start: %lld  paStart: %f Pa_GetStreamTime: %f  AdcTime: %f  DacTime: %f  framesPerBuffer: %lu  CPU: %f\n", 
	player->pos.current, player->pos.start, player->paStart, Pa_GetStreamTime(player->stream),
	timeInfo->inputBufferAdcTime, timeInfo->outputBufferDacTime,  framesPerBuffer, Pa_GetStreamCpuLoad(player->stream));
#endif

	// Calculate how much left
	int64_t lenAvailable = (player->pos.end - player->pos.current) > 0 ? framesPerBuffer : 0;

	// Play something
	if (lenAvailable > 0) {
		provider->GetAudioWithVolume(outputBuffer, player->pos.current, lenAvailable, player->GetVolume());

		// Set play position
		player->pos.current += framesPerBuffer;

		// Continue as normal
		return 0;
	}

	// Abort stream and stop the callback.
	return paAbort;
}



/// @brief Get current stream position.
/// @return Stream position
int64_t PortAudioPlayer::GetCurrentPosition()
{

	if (!IsPlaying()) return 0;

	const PaStreamInfo* streamInfo = Pa_GetStreamInfo(stream);

#ifdef PORTAUDIO_DEBUG
	PaTime pa_getstream = Pa_GetStreamTime(stream);
	int64_t real = ((pa_getstream - paStart) * streamInfo->sampleRate) + pos.start;
	printf("GetCurrentPosition: Pa_GetStreamTime: %f  pos.start: %lld  pos.current: %lld  paStart: %f  real: %lld  diff: %f\n",
	pa_getstream, pos.start, pos.current, paStart, real, pa_getstream-paStart);

	return real;
#endif

	return ((Pa_GetStreamTime(stream) - pos.pa_start) * streamInfo->sampleRate) + pos.start;

}


/// @brief Get list of available output devices
/// @param favorite Favorite output device
/// @return List of available output devices with the 'favorite' being first in the list.
wxArrayString PortAudioPlayer::GetOutputDevices(wxString favorite) {
	wxArrayString list;
	int devices = Pa_GetDeviceCount();
	int i;

	if (devices < 0) {
		// some error here
	}

	for (i=0; i<devices; i++) {
		const PaDeviceInfo *dev_info = Pa_GetDeviceInfo(i);
		wxString name(dev_info->name, wxConvUTF8);
		list.Insert(name, i);
	}

	return list;
}

bool PortAudioPlayer::IsPlaying() {
	return Pa_IsStreamActive(stream) ? true : false;
}

#endif // WITH_PORTAUDIO
