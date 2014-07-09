// Copyright (c) 2011, Niels Martin Hansen
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

/// @file audio_player_alsa.cpp
/// @brief ALSA-based audio output
/// @ingroup audio_output
///

#ifdef WITH_ALSA
#include "include/aegisub/audio_player.h"

#include "audio_controller.h"
#include "compat.h"
#include "frame_main.h"
#include "options.h"

#include <libaegisub/audio/provider.h>
#include <libaegisub/log.h>
#include <libaegisub/make_unique.h>

#include <atomic>
#include <algorithm>
#include <boost/scope_exit.hpp>
#include <chrono>
#include <condition_variable>
#include <alsa/asoundlib.h>
#include <memory>
#include <mutex>
#include <thread>

// X11 is the bestest
#undef None

namespace {
enum class Message {
	None,
	Start,
	Stop,
	Close
};

using clock = std::chrono::steady_clock;

class AlsaPlayer final : public AudioPlayer {
	std::mutex mutex;
	std::condition_variable cond;

	std::string device_name = OPT_GET("Player/Audio/ALSA/Device")->GetString();

	Message message = Message::None;

	std::atomic<bool> playing{false};
	std::atomic<double> volume{1.0};
	int64_t start_position = 0;
	std::atomic<int64_t> end_position{0};

	std::mutex position_mutex;
	int64_t last_position = 0;
	clock::time_point last_position_time;

	std::vector<char> decode_buffer;

	std::thread thread;

	void PlaybackThread();

	void UpdatePlaybackPosition(snd_pcm_t *pcm, int64_t position)
	{
		snd_pcm_sframes_t delay;
		if (snd_pcm_delay(pcm, &delay) == 0)
		{
			std::unique_lock<std::mutex> playback_lock;
			last_position = position - delay;
			last_position_time = clock::now();
		}
	}

public:
	AlsaPlayer(agi::AudioProvider *provider);
	~AlsaPlayer();

	void Play(int64_t start, int64_t count) override;
	void Stop() override;
	bool IsPlaying() override { return playing; }

	void SetVolume(double vol) override { volume = vol; }
	int64_t GetEndPosition() override { return end_position; }
	int64_t GetCurrentPosition() override;
	void SetEndPosition(int64_t pos) override;
};

void AlsaPlayer::PlaybackThread()
{
	std::unique_lock<std::mutex> lock(mutex);

	snd_pcm_t *pcm = nullptr;
	if (snd_pcm_open(&pcm, device_name.c_str(), SND_PCM_STREAM_PLAYBACK, 0) != 0)
		return;
	LOG_D("audio/player/alsa") << "opened pcm";
	BOOST_SCOPE_EXIT_ALL(&) { snd_pcm_close(pcm); };

do_setup:
	snd_pcm_format_t pcm_format;
	switch (provider->GetBytesPerSample())
	{
	case 1:
		LOG_D("audio/player/alsa") << "format U8";
		pcm_format = SND_PCM_FORMAT_U8;
		break;
	case 2:
		LOG_D("audio/player/alsa") << "format S16_LE";
		pcm_format = SND_PCM_FORMAT_S16_LE;
		break;
	default:
		return;
	}
	if (snd_pcm_set_params(pcm,
	                       pcm_format,
	                       SND_PCM_ACCESS_RW_INTERLEAVED,
	                       provider->GetChannels(),
	                       provider->GetSampleRate(),
	                       1, // allow resample
	                       100*1000 // 100 milliseconds latency
	                      ) != 0)
		return;
	LOG_D("audio/player/alsa") << "set pcm params";

	size_t framesize = provider->GetChannels() * provider->GetBytesPerSample();

	while (true)
	{
		// Wait for condition to trigger
		while (message != Message::Start)
		{
			cond.wait(lock, [&] { return message != Message::None; });
			if (message == Message::Close)
				return;
			if (message == Message::Start && end_position > start_position)
				break;
			// Not playing, so don't need to stop...
			message = Message::None;
		}
		message = Message::None;

		LOG_D("audio/player/alsa") << "starting playback";
		int64_t position = start_position;

		// Initial buffer-fill
		{
			auto avail = std::min(snd_pcm_avail(pcm), (snd_pcm_sframes_t)(end_position-position));
			decode_buffer.resize(avail * framesize);
			provider->GetAudioWithVolume(decode_buffer.data(), position, avail, volume);

			snd_pcm_sframes_t written = 0;
			while (written <= 0)
			{
				written = snd_pcm_writei(pcm, decode_buffer.data(), avail);
				if (written == -ESTRPIPE)
					snd_pcm_recover(pcm, written, 0);
				else if (written <= 0)
				{
					LOG_D("audio/player/alsa") << "error filling buffer";
					return;
				}
			}
			position += written;
		}

		// Start playback
		LOG_D("audio/player/alsa") << "initial buffer filled, hitting start";
		snd_pcm_start(pcm);

		UpdatePlaybackPosition(pcm, position);
		playing = true;
		BOOST_SCOPE_EXIT_ALL(&) { playing = false; };
		while (true)
		{
			// Sleep a bit, or until an event
			cond.wait_for(lock, std::chrono::milliseconds{25});

			if (message == Message::Close)
			{
				snd_pcm_drop(pcm);
				return;
			}

			// Check for stop signal
			if (message == Message::Stop || message == Message::Start)
			{
				LOG_D("audio/player/alsa") << "playback loop, stop signal";
				snd_pcm_drop(pcm);
				break;
			}

			// Fill buffer
			snd_pcm_sframes_t tmp_pcm_avail = snd_pcm_avail(pcm);
			if (tmp_pcm_avail == -EPIPE)
			{
				if (snd_pcm_recover(pcm, -EPIPE, 1) < 0)
				{
					LOG_D("audio/player/alsa") << "failed to recover from underrun";
					return;
				}
				tmp_pcm_avail = snd_pcm_avail(pcm);
			}
			auto avail = std::min(tmp_pcm_avail, (snd_pcm_sframes_t)(end_position-position));
			if (avail < 0)
				continue;

			{
				decode_buffer.resize(avail * framesize);
				provider->GetAudioWithVolume(decode_buffer.data(), position, avail, volume);
				snd_pcm_sframes_t written = 0;
				while (written <= 0)
				{
					written = snd_pcm_writei(pcm, decode_buffer.data(), avail);
					if (written == -ESTRPIPE || written == -EPIPE)
						snd_pcm_recover(pcm, written, 0);
					else if (written == 0)
						break;
					else if (written < 0)
					{
						LOG_D("audio/player/alsa") << "error filling buffer, written=" << written;
						return;
					}
				}
				position += written;
			}

			UpdatePlaybackPosition(pcm, position);

			// Check for end of playback
			if (position >= end_position)
			{
				LOG_D("audio/player/alsa") << "playback loop, past end, draining";
				snd_pcm_drain(pcm);
				break;
			}
		}

		playing = false;
		LOG_D("audio/player/alsa") << "out of playback loop";

		switch (snd_pcm_state(pcm))
		{
		case SND_PCM_STATE_OPEN:
			// no clue what could have happened here, but start over
			goto do_setup;

		case SND_PCM_STATE_SETUP:
			// we lost the preparedness?
			snd_pcm_prepare(pcm);
			break;

		case SND_PCM_STATE_DISCONNECTED:
			// lost device, close the handle and return error
			return;

		default:
			// everything else should either be fine or impossible (here)
			break;
		}
	}
}

AlsaPlayer::AlsaPlayer(agi::AudioProvider *provider) try
: AudioPlayer(provider)
, thread(&AlsaPlayer::PlaybackThread, this)
{
}
catch (std::system_error const&) {
	throw AudioPlayerOpenError("AlsaPlayer: Creating the playback thread failed");
}

AlsaPlayer::~AlsaPlayer()
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		message = Message::Close;
		cond.notify_all();
	}

	thread.join();
}

void AlsaPlayer::Play(int64_t start, int64_t count)
{
	std::unique_lock<std::mutex> lock(mutex);
	message = Message::Start;
	start_position = start;
	end_position = start + count;
	cond.notify_all();
}

void AlsaPlayer::Stop()
{
	std::unique_lock<std::mutex> lock(mutex);
	message = Message::Stop;
	cond.notify_all();
}

void AlsaPlayer::SetEndPosition(int64_t pos)
{
	std::unique_lock<std::mutex> lock(mutex);
	end_position = pos;
}

int64_t AlsaPlayer::GetCurrentPosition()
{
	int64_t lastpos;
	clock::time_point lasttime;

	{
		std::unique_lock<std::mutex> playback_lock;
		lastpos = last_position;
		lasttime = last_position_time;
	}

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - lasttime).count();
	return lastpos + ms * provider->GetSampleRate() / 1000;
}
}

std::unique_ptr<AudioPlayer> CreateAlsaPlayer(agi::AudioProvider *provider, wxWindow *)
{
	return agi::make_unique<AlsaPlayer>(provider);
}

#endif // WITH_ALSA
