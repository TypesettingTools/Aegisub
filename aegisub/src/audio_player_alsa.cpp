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

#include "config.h"

#ifdef WITH_ALSA

#include <libaegisub/log.h>

#include "audio_player_alsa.h"

#include "audio_controller.h"
#include "include/aegisub/audio_provider.h"
#include "compat.h"
#include "frame_main.h"
#include "main.h"

#include <algorithm>

#include <inttypes.h>

class PthreadMutexLocker {
	pthread_mutex_t &mutex;

	PthreadMutexLocker(const PthreadMutexLocker &); // uncopyable
	PthreadMutexLocker(); // no default
	PthreadMutexLocker& operator=(PthreadMutexLocker const&);

public:
	explicit PthreadMutexLocker(pthread_mutex_t &mutex) : mutex(mutex)
	{
		pthread_mutex_lock(&mutex);
	}

	~PthreadMutexLocker()
	{
		pthread_mutex_unlock(&mutex);
	}

	int WaitCondition(pthread_cond_t &cond)
	{
		return pthread_cond_wait(&cond, &mutex);
	}

	int WaitConditionTimeout(pthread_cond_t &cond, int ms)
	{
		timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_nsec += ms * 1000000;
		return pthread_cond_timedwait(&cond, &mutex, &abstime);
	}
};


class ScopedAliveFlag {
	bool &flag;

	ScopedAliveFlag(const ScopedAliveFlag &); // uncopyable
	ScopedAliveFlag(); // no default
	ScopedAliveFlag& operator=(ScopedAliveFlag const&);

public:
	explicit ScopedAliveFlag(bool &var) : flag(var) { flag = true; }
	~ScopedAliveFlag() { flag = false; }
};


struct PlaybackState {
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	bool playing;
	bool alive;

	bool signal_start;
	bool signal_stop;
	bool signal_close;
	bool signal_volume;

	double volume;
	int64_t start_position;
	int64_t end_position;

	AudioProvider *provider;
	std::string device_name;

	int64_t last_position;
	timespec last_position_time;

	PlaybackState()
	{
		pthread_mutex_init(&mutex, 0);
		pthread_cond_init(&cond, 0);

		Reset();
		volume = 1.0;
	}

	~PlaybackState()
	{
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);
	}

	void Reset()
	{
		playing = false;
		alive = false;
		signal_start = false;
		signal_stop = false;
		signal_close = false;
		signal_volume = false;
		start_position = 0;
		end_position = 0;
		last_position = 0;
		memset(&last_position_time, 0, sizeof last_position_time);
		provider = 0;
	}
};

void *playback_thread(void *arg)
{
	// This is exception-free territory!
	// Return a pointer to a static string constant describing the error, or 0 on no error
	PlaybackState &ps = *(PlaybackState*)arg;

	PthreadMutexLocker ml(ps.mutex);
	ScopedAliveFlag alive_flag(ps.alive);

	snd_pcm_t *pcm = 0;
	if (snd_pcm_open(&pcm, ps.device_name.c_str(), SND_PCM_STREAM_PLAYBACK, 0) != 0)
		return (void*)"snd_pcm_open";
	LOG_D("audio/player/alsa") << "opened pcm";

do_setup:
	snd_pcm_format_t pcm_format;
	switch (ps.provider->GetBytesPerSample())
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
		snd_pcm_close(pcm);
		return (void*)"snd_pcm_format_t";
	}
	if (snd_pcm_set_params(pcm,
	                       pcm_format,
	                       SND_PCM_ACCESS_RW_INTERLEAVED,
	                       ps.provider->GetChannels(),
	                       ps.provider->GetSampleRate(),
	                       1, // allow resample
	                       100*1000 // 100 milliseconds latency
	                      ) != 0)
		return (void*)"snd_pcm_set_params";
	LOG_D("audio/player/alsa") << "set pcm params";

	size_t framesize = ps.provider->GetChannels() * ps.provider->GetBytesPerSample();

	ps.signal_close = false;
	while (ps.signal_close == false)
	{
		// Wait for condition to trigger
		if (!ps.signal_start)
			ml.WaitCondition(ps.cond);
		LOG_D("audio/player/alsa") << "outer loop, condition happened";

		if (ps.signal_start == false || ps.end_position <= ps.start_position)
		{
			LOG_D("audio/player/alsa") << "nothing to play, rewaiting";
			ps.signal_start = false;
			continue;
		}

		LOG_D("audio/player/alsa") << "starting playback";
		int64_t position = ps.start_position;

		// Playback position
		ps.last_position = position;
		clock_gettime(CLOCK_REALTIME, &ps.last_position_time);

		// Initial buffer-fill
		snd_pcm_sframes_t avail = std::min(snd_pcm_avail(pcm), (snd_pcm_sframes_t)(ps.end_position-position));
		char *buf = new char[avail*framesize];
		ps.provider->GetAudioWithVolume(buf, position, avail, ps.volume);
		snd_pcm_sframes_t written = 0;
		while (written <= 0)
		{
			written = snd_pcm_writei(pcm, buf, avail);
			if (written == -ESTRPIPE)
			{
				snd_pcm_recover(pcm, written, 0);
			}
			else if (written <= 0)
			{
				delete[] buf;
				snd_pcm_close(pcm);
				LOG_D("audio/player/alsa") << "error filling buffer";
				return (void*)"snd_pcm_writei";
			}
		}
		delete[] buf;
		position += written;

		// Start playback
		LOG_D("audio/player/alsa") << "initial buffer filled, hitting start";
		snd_pcm_start(pcm);

		ps.signal_start = false;
		ps.signal_stop = false;

		while (ps.signal_stop == false)
		{
			int64_t orig_position = position;
			int64_t orig_ps_end_position = ps.end_position;
			
			ScopedAliveFlag playing_flag(ps.playing);

			// Sleep a bit, or until an event
			ml.WaitConditionTimeout(ps.cond, 50);
			//LOG_D("audio/player/alsa") << "playback loop, out of wait";

			// Check for stop signal
			if (ps.signal_stop == true)
			{
				LOG_D("audio/player/alsa") << "playback loop, stop signal";
				snd_pcm_drop(pcm);
				break;
			}

			// Playback position
			snd_pcm_sframes_t delay;
			if (snd_pcm_delay(pcm, &delay) == 0)
			{
				ps.last_position = position - delay;
				clock_gettime(CLOCK_REALTIME, &ps.last_position_time);
			}

			// Fill buffer
			snd_pcm_sframes_t tmp_pcm_avail = snd_pcm_avail(pcm);
			if (tmp_pcm_avail == -EPIPE)
			{
				if (snd_pcm_recover(pcm, -EPIPE, 1) < 0)
				{
					LOG_D("audio/player/alsa") << "failed to recover from underrun";
					return (void*)"snd_pcm_avail";
				}
				tmp_pcm_avail = snd_pcm_avail(pcm);
			}
			avail = std::min(tmp_pcm_avail, (snd_pcm_sframes_t)(ps.end_position-position));
			if (avail < 0)
			{
				printf("\n--------- avail was less than 0: %ld\n", (long)avail);
				printf("snd_pcm_avail(pcm): %ld\n", (long)tmp_pcm_avail);
				printf("original position: %" PRId64 "\n", orig_position);
				printf("current  position: %" PRId64 "\n", position);
				printf("original ps.end_position: %" PRId64 "\n", orig_ps_end_position);
				printf("current  ps.end_position: %" PRId64 "\n", ps.end_position);
				printf("---------\n\n");
				continue;
			}
			buf = new char[avail*framesize];
			ps.provider->GetAudioWithVolume(buf, position, avail, ps.volume);
			written = 0;
			while (written <= 0)
			{
				written = snd_pcm_writei(pcm, buf, avail);
				if (written == -ESTRPIPE || written == -EPIPE)
				{
					snd_pcm_recover(pcm, written, 0);
				}
				else if (written == 0)
				{
					break;
				}
				else if (written < 0)
				{
					delete[] buf;
					snd_pcm_close(pcm);
					LOG_D("audio/player/alsa") << "error filling buffer, written=" << written;
					return (void*)"snd_pcm_writei";
				}
			}
			delete[] buf;
			position += written;
			//LOG_D("audio/player/alsa") << "playback loop, filled buffer";

			// Check for end of playback
			if (position >= ps.end_position)
			{
				LOG_D("audio/player/alsa") << "playback loop, past end, draining";
				snd_pcm_drain(pcm);
				break;
			}
		}
		ps.signal_stop = false;
		LOG_D("audio/player/alsa") << "out of playback loop";

		switch (snd_pcm_state(pcm))
		{
		case SND_PCM_STATE_OPEN:
			// no clue what could have happened here, but start over
			ps.signal_start = false;
			ps.signal_stop = false;
			goto do_setup;

		case SND_PCM_STATE_SETUP:
			// we lost the preparedness?
			snd_pcm_prepare(pcm);
			break;

		case SND_PCM_STATE_DISCONNECTED:
			// lost device, close the handle and return error
			snd_pcm_close(pcm);
			return (void*)"SND_PCM_STATE_DISCONNECTED";

		default:
			// everything else should either be fine or impossible (here)
			break;
		}
	}
	ps.signal_close = false;
	LOG_D("audio/player/alsa") << "out of outer loop";

	snd_pcm_close(pcm);

	return 0;
}


AlsaPlayer::AlsaPlayer(AudioProvider *provider)
: AudioPlayer(provider)
, ps(new PlaybackState)
{
	ps->provider = provider;

	wxString device_name = lagi_wxString(OPT_GET("Player/Audio/ALSA/Device")->GetString());
	ps->device_name = std::string(device_name.utf8_str());

	if (pthread_create(&thread, 0, &playback_thread, ps.get()) != 0)
		throw agi::AudioPlayerOpenError("AlsaPlayer: Creating the playback thread failed", 0);
}


AlsaPlayer::~AlsaPlayer()
{
	{
		PthreadMutexLocker ml(ps->mutex);
		ps->signal_stop = true;
		ps->signal_close = true;
		LOG_D("audio/player/alsa") << "close stream, stop+close signal";
		pthread_cond_signal(&ps->cond);
	}

	pthread_join(thread, 0); // FIXME: check for errors
}


void AlsaPlayer::Play(int64_t start, int64_t count)
{
	PthreadMutexLocker ml(ps->mutex);
	ps->signal_start = true;
	ps->signal_stop = true; // make sure to stop any ongoing playback first
	ps->start_position = start;
	ps->end_position = start + count;
	pthread_cond_signal(&ps->cond);
}


void AlsaPlayer::Stop()
{
	PthreadMutexLocker ml(ps->mutex);
	ps->signal_stop = true;
	LOG_D("audio/player/alsa") << "stop stream, stop signal";
	pthread_cond_signal(&ps->cond);
}

bool AlsaPlayer::IsPlaying()
{
	PthreadMutexLocker ml(ps->mutex);
	return ps->playing;
}


void AlsaPlayer::SetEndPosition(int64_t pos)
{
	PthreadMutexLocker ml(ps->mutex);
	ps->end_position = pos;
}


void AlsaPlayer::SetCurrentPosition(int64_t pos)
{
	PthreadMutexLocker ml(ps->mutex);

	if (!ps->playing) return;

	ps->start_position = pos;
	ps->signal_start = true;
	ps->signal_stop = true;
	LOG_D("audio/player/alsa") << "set position, stop+start signal";
	pthread_cond_signal(&ps->cond);
}

int64_t AlsaPlayer::GetStartPosition()
{
	PthreadMutexLocker ml(ps->mutex);
	return ps->start_position;
}

int64_t AlsaPlayer::GetEndPosition()
{
	PthreadMutexLocker ml(ps->mutex);
	return ps->end_position;
}


int64_t AlsaPlayer::GetCurrentPosition()
{
	int64_t lastpos;
	timespec lasttime;
	int64_t samplerate;

	{
		PthreadMutexLocker ml(ps->mutex);
		lastpos = ps->last_position;
		lasttime = ps->last_position_time;
		samplerate = ps->provider->GetSampleRate();
	}

	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	const double NANO = 1000000000; // nano- is 10^-9

	double now_sec = now.tv_sec + now.tv_nsec/NANO;
	double last_sec = lasttime.tv_sec + lasttime.tv_nsec/NANO;
	double diff_sec = now_sec - last_sec;

	int64_t pos = lastpos + (int64_t)(diff_sec * samplerate);
	//printf("AlsaPlayer: current position = %lld\n", pos);

	return pos;
}

void AlsaPlayer::SetVolume(double vol)
{
	PthreadMutexLocker ml(ps->mutex);
	ps->volume = vol;
	ps->signal_volume = true;
	pthread_cond_signal(&ps->cond);
}


double AlsaPlayer::GetVolume()
{
	PthreadMutexLocker ml(ps->mutex);
	return ps->volume;
}

#endif // WITH_ALSA
