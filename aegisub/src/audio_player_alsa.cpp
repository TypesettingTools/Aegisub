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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://www.aegisub.org
// Contact: mailto:nielsm@indvikleren.dk
//


#include "config.h"

#ifdef WITH_ALSA


///////////
// Headers
#include "audio_player_alsa.h"
#include "include/aegisub/audio_provider.h"

#include <alsa/asoundlib.h>

#include <wx/wxprec.h>
#include "utils.h"
#include "frame_main.h"
#include "options.h"
#include <algorithm>

class PthreadMutexLocker {
	pthread_mutex_t &mutex;

	PthreadMutexLocker(const PthreadMutexLocker &); // uncopyable
	PthreadMutexLocker(); // no default

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
		abstime.tv_sec += abstime.tv_nsec / 1000000000;
		abstime.tv_nsec = abstime.tv_nsec % 1000000000;
		return pthread_cond_timedwait(&cond, &mutex, &abstime);
	}
};


class ScopedAliveFlag {
	volatile bool &flag;

	ScopedAliveFlag(const ScopedAliveFlag &); // uncopyable
	ScopedAliveFlag(); // no default

public:
	explicit ScopedAliveFlag(volatile bool &var) : flag(var) { flag = true; }
	~ScopedAliveFlag() { flag = false; }
};


struct PlaybackState {
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	volatile bool playing;
	volatile bool alive;

	volatile bool signal_start;
	volatile bool signal_stop;
	volatile bool signal_close;
	volatile bool signal_volume;

	volatile double volume;
	volatile int64_t start_position;
	volatile int64_t end_position;

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
		provider = 0;
	}
};


class AlsaPlayer : public AudioPlayer {
private:
	PlaybackState ps;
	pthread_t thread;

	bool IsAlive();

public:
	AlsaPlayer();
	~AlsaPlayer();

	void OpenStream();
	void CloseStream();

	void Play(int64_t start, int64_t count);
	void Stop(bool timerToo=true);
	bool IsPlaying();

	int64_t GetStartPosition();
	int64_t GetEndPosition();
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);
	void SetCurrentPosition(int64_t pos);

	void SetVolume(double vol);
	double GetVolume();
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
		return "snd_pcm_open";
	//printf("alsa_player: opened pcm\n");

do_setup:
	snd_pcm_format_t pcm_format;
	switch (ps.provider->GetBytesPerSample())
	{
	case 1:
		//printf("alsa_player: format U8\n");
		pcm_format = SND_PCM_FORMAT_U8;
		break;
	case 2:
		//printf("alsa_player: format S16_LE\n");
		pcm_format = SND_PCM_FORMAT_S16_LE;
		break;
	default:
		snd_pcm_close(pcm);
		return "snd_pcm_format_t";
	}
	if (snd_pcm_set_params(pcm,
	                       pcm_format,
	                       SND_PCM_ACCESS_RW_INTERLEAVED,
	                       ps.provider->GetChannels(),
	                       ps.provider->GetSampleRate(),
	                       1, // allow resample
	                       100*1000 // 100 milliseconds latency
	                      ) != 0)
		return "snd_pcm_set_params";
	//printf("alsa_player: set pcm params\n");

	size_t framesize = ps.provider->GetChannels() * ps.provider->GetBytesPerSample();

	ps.signal_close = false;
	while (ps.signal_close == false)
	{
		// Wait for condition to trigger
		if (!ps.signal_start)
			ml.WaitCondition(ps.cond);
		//printf("alsa_player: outer loop, condition happened\n");

		if (ps.signal_start == false || ps.end_position <= ps.start_position)
		{
			continue;
		}

		//printf("alsa_player: starting playback\n");
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
				//printf("alsa_player: error filling buffer\n");
				return "snd_pcm_writei";
			}
		}
		delete[] buf;
		position += written;

		// Start playback
		//printf("alsa_player: initial buffer filled, hitting start\n");
		snd_pcm_start(pcm);

		ps.signal_start = false;
		ps.signal_stop = false;

		while (ps.signal_stop == false)
		{
			ScopedAliveFlag playing_flag(ps.playing);

			// Sleep a bit, or until an event
			ml.WaitConditionTimeout(ps.cond, 50);
			//printf("alsa_player: playback loop, out of wait\n");

			// Check for stop signal
			if (ps.signal_stop == true)
			{
				//printf("alsa_player: playback loop, stop signal\n");
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
			avail = std::min(snd_pcm_avail(pcm), (snd_pcm_sframes_t)(ps.end_position-position));
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
					//printf("alsa_player: error filling buffer, written=%d\n", written);
					return "snd_pcm_writei";
				}
			}
			delete[] buf;
			position += written;
			//printf("alsa_player: playback loop, filled buffer\n");

			// Check for end of playback
			if (position >= ps.end_position)
			{
				//printf("alsa_player: playback loop, past end, draining\n");
				snd_pcm_drain(pcm);
				break;
			}
		}
		ps.signal_stop = false;
		//printf("alsa_player: out of playback loop\n");

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
			return "SND_PCM_STATE_DISCONNECTED";

		default:
			// everything else should either be fine or impossible (here)
			break;
		}
	}
	ps.signal_close = false;
	//printf("alsa_player: out of outer loop\n");

	snd_pcm_close(pcm);

	return 0;
}



AlsaPlayer::AlsaPlayer()
{
	ps.Reset();
	thread = 0;
}


AlsaPlayer::~AlsaPlayer()
{
	CloseStream();
}


void AlsaPlayer::OpenStream()
{
	// Don't re-open if it's already alive
	// Assume this means it isn't about to shut down
	if (IsAlive()) return;

	// Catch any errors in case the thread died unexpectedly
	CloseStream();

	ps.Reset();
	ps.provider = GetProvider();

	wxString device_name = Options.AsText(_T("Audio ALSA Device"));
	ps.device_name = std::string(device_name.utf8_str());

	int err = pthread_create(&thread, 0, &playback_thread, &ps);

	if (err == EAGAIN)
	{
		throw _T("AlsaPlayer: Failed creating playback thread, too little resources");
	}
	else if (err == EPERM)
	{
		throw _T("AlsaPlayer: Failed creating playback thread, permissions error");
	}
	else if (err != 0)
	{
		throw _T("AlsaPlayer: Failed creating playback thread, unexpected error, report this to the developers");
	}
}


void AlsaPlayer::CloseStream()
{
	// Make the thread go away if it's alive
	if (IsAlive())
	{
		PthreadMutexLocker ml(ps.mutex);
		ps.signal_stop = true;
		ps.signal_close = true;
		//printf("AlsaPlayer: close stream, stop+close signal\n");
		pthread_cond_signal(&ps.cond);
	}

	if (thread == 0) return;

	void *thread_result;
	int err = pthread_join(thread, &thread_result);
	thread = 0;

	if (err == 0 && thread_result != 0)
	{
		// Thread errored
		wxString errstr((const char *)thread_result, wxConvUTF8);
		throw wxString::Format(_T("AlsaPlayer: Error in thread: %s"), errstr.c_str());
	}
	else if (err == 0 && thread_result == 0)
	{
		// Successful exit
	}
	else if (err == EINVAL || err == ESRCH)
	{
		// Thread was already joined to or was never created, ignore this
	}
	else
	{
		// EDEADLK but the playback thread shouldn't be trying to join us
		throw _T("AlsaPlayer: Unexpected thread error, report this to the developers");
	}
}


void AlsaPlayer::Play(int64_t start, int64_t count)
{
	OpenStream();

	{
		PthreadMutexLocker ml(ps.mutex);
		ps.signal_start = true;
		ps.signal_stop = true; // make sure to stop any ongoing playback first
		ps.start_position = start;
		ps.end_position = start + count;
		pthread_cond_signal(&ps.cond);
	}

	if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(15);
}


void AlsaPlayer::Stop(bool timerToo)
{
	if (!IsPlaying()) return;

	{
		PthreadMutexLocker ml(ps.mutex);
		ps.signal_stop = true;
		//printf("AlsaPlayer: stop stream, stop signal\n");
		pthread_cond_signal(&ps.cond);
	}

	if (timerToo && displayTimer) {
		    displayTimer->Stop();
	}
}


bool AlsaPlayer::IsAlive()
{
	return ps.alive;
}


bool AlsaPlayer::IsPlaying()
{
	return ps.playing; // should always be false if ps.alive is false
}


void AlsaPlayer::SetEndPosition(int64_t pos)
{
	if (!IsPlaying()) return;
	PthreadMutexLocker ml(ps.mutex);
	ps.end_position = pos;
}


void AlsaPlayer::SetCurrentPosition(int64_t pos)
{
	if (!IsPlaying()) return;

	PthreadMutexLocker ml(ps.mutex);

	ps.start_position = pos;
	ps.signal_start = true;
	ps.signal_stop = true;
	//printf("AlsaPlayer: set position, stop+start signal\n");
	pthread_cond_signal(&ps.cond);
}


int64_t AlsaPlayer::GetStartPosition()
{
	if (!IsPlaying()) return 0;
	return ps.start_position;
}


int64_t AlsaPlayer::GetEndPosition()
{
	if (!IsPlaying()) return 0;
	return ps.end_position;
}


int64_t AlsaPlayer::GetCurrentPosition()
{
	if (!IsPlaying()) return 0;

	int64_t lastpos;
	timespec lasttime;
	int64_t samplerate;

	{
		// Try to do this without synchronisation
		// Might give unreliable results but shouldn't be catastrophic
		//PthreadMutexLocker ml(ps.mutex);
		lastpos = ps.last_position;
		lasttime = ps.last_position_time;
		samplerate = ps.provider->GetSampleRate();
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
	if (!IsAlive()) return;
	PthreadMutexLocker ml(ps.mutex);
	ps.volume = vol;
	ps.signal_volume = true;
	pthread_cond_signal(&ps.cond);
}


double AlsaPlayer::GetVolume()
{
	if (!IsAlive()) return 1.0;
	PthreadMutexLocker ml(ps.mutex);
	return ps.volume;
}


// The factory method
AudioPlayer * AlsaPlayerFactory::CreatePlayer()
{
	return new AlsaPlayer();
}


#endif // WITH_ALSA

