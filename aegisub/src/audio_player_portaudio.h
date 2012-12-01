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

/// @file audio_player_portaudio.h
/// @see audio_player_portaudio.cpp
/// @ingroup audio_output
///

#ifdef WITH_PORTAUDIO

#include "include/aegisub/audio_player.h"

extern "C" {
#include <portaudio.h>
}

#include <map>
#include <string>
#include <vector>

class wxArrayString;

/// @class PortAudioPlayer
/// @brief PortAudio Player
///
class PortAudioPlayer : public AudioPlayer {
	typedef std::vector<PaDeviceIndex> DeviceVec;
	/// Map of supported output devices from name -> device index
	std::map<std::string, DeviceVec> devices;

	/// The index of the default output devices sorted by host API priority
	DeviceVec default_device;

	float volume;    ///< Current volume level
	int64_t current; ///< Current position
	int64_t start;   ///< Start position
	int64_t end;     ///< End position
	PaTime pa_start; ///< PortAudio internal start position

	PaStream *stream; ///< PortAudio stream

	/// @brief PortAudio callback, used to fill buffer for playback, and prime the playback buffer.
	/// @param inputBuffer     Input buffer.
	/// @param outputBuffer    Output buffer.
	/// @param framesPerBuffer Frames per buffer.
	/// @param timeInfo        PortAudio time information.
	/// @param statusFlags     Status flags
	/// @param userData        Local data to hand callback
	/// @return Whether to stop playback.
	static int paCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo*
		timeInfo,
		PaStreamCallbackFlags
		statusFlags,
		void *userData);

	/// @brief Called when the callback has finished.
	/// @param userData Local data to be handed to the callback.
	static void paStreamFinishedCallback(void *userData);

	/// Gather the list of output devices supported by a host API
	/// @param host_idx Host API ID
	void GatherDevices(PaHostApiIndex host_idx);

	void OpenStream();

public:
	/// @brief Constructor
	PortAudioPlayer(AudioProvider *provider);

	/// @brief Destructor
	~PortAudioPlayer();

	/// @brief Play audio.
	/// @param start Start position.
	/// @param count Frame count
	void Play(int64_t start,int64_t count);
	/// @brief Stop Playback
	/// @param timerToo Stop display timer?
	void Stop();

	/// @brief Whether audio is currently being played.
	/// @return Status
	bool IsPlaying();

	/// @brief Position audio will be played from.
	/// @return Start position.
	int64_t GetStartPosition() { return start; }

	/// @brief End position playback will stop at.
	/// @return End position.
	int64_t GetEndPosition() { return end; }
	/// @brief Get current stream position.
	/// @return Stream position
	int64_t GetCurrentPosition();

	/// @brief Set end position of playback
	/// @param pos End position
	void SetEndPosition(int64_t position) { end = position; }

	/// @brief Set current position of playback.
	/// @param pos Current position
	void SetCurrentPosition(int64_t position) { current = position; }


	/// @brief Set volume level
	/// @param vol Volume
	void SetVolume(double vol) { volume = vol; }

	/// @brief Get current volume level
	/// @return Volume level
	double GetVolume() { return volume; }

	/// Get list of available output devices
	static wxArrayString GetOutputDevices();
};
#endif //ifdef WITH_PORTAUDIO
