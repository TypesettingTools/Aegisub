// Copyright (c) 2007, Niels Martin Hansen
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
// Contact: mailto:jiifurusu@gmail.com
//


///////////
// Headers
#include <wx/wxprec.h>
#include "audio_player.h"
#include "audio_provider.h"
#include "utils.h"
#include "main.h"
#include "frame_main.h"
#include "audio_player.h"
#include <alsa/asoundlib.h>


//////////////
// Prototypes
class AlsaPlayer;


//////////
// Thread
class AlsaPlayerThread : public wxThread {
private:
	wxMutex play_mutex; // held while playing
	wxSemaphore play_notify; // posted when a playback operation is set up
	wxSemaphore stop_notify; // when set, audio playback should stop asap
	wxSemaphore shutdown_notify; // when set, thread should shutdown asap
	wxMutex parameter_mutex;

	AudioProvider *provider; // provides sample data!

	snd_pcm_t *pcm_handle; // device handle
	snd_pcm_stream_t stream; // stream direction
	snd_pcm_hw_params_t *hwparams; // hardware info
	char *pcm_name; // device name

	snd_pcm_format_t sample_format;
	unsigned int rate; // sample rate of audio
	unsigned int real_rate; // actual sample rate played back
	int periods; // number of bytes in a frame
	snd_pcm_uframes_t bufsize; // size of buffer in frames

	volatile double volume; // volume to get audio at
	volatile unsigned long start_frame; // first frame of playback
	volatile unsigned long cur_frame; // last written frame + 1
	volatile unsigned long end_frame; // last frame to play

	void SetUpHardware();
	void PlaybackLoop();

public:
	AlsaPlayerThread(AudioProvider *_provider);
	~AlsaPlayerThread();
	wxThread::ExitCode Entry();

	// The following methods are all thread safe
	void Play(unsigned long _start_frame, unsigned long _num_frames); // Notify thread to stop any playback and instead play specified range
	void SetEndPosition(unsigned long _end_frame); // Notify thread to use new end position
	void Stop(); // Notify thread to stop audio playback
	void StopWait(); // Notify thread to stop, and wait for it to do it
	void Shutdown(); // Notify the thread to stop playback and die
	void SetVolume(double new_volume); // Set volume
	unsigned long GetStartPosition(); // Get first played back frame number
	unsigned long GetEndPosition(); // Get last frame number to be played back
	unsigned long GetCurrentPosition(); // Get currently played back frame number
	bool IsPlaying();
};


///////////////
// Alsa player
class AlsaPlayer : public AudioPlayer {
private:
	float volume;

	AlsaPlayerThread *thread;

public:
	AlsaPlayer();
	~AlsaPlayer();

	void OpenStream();
	void CloseStream();

	void Play(__int64 start,__int64 count);
	void Stop(bool timerToo=true);
	bool IsPlaying();

	__int64 GetStartPosition();
	__int64 GetEndPosition();
	__int64 GetCurrentPosition();
	void SetEndPosition(__int64 pos);
	void SetCurrentPosition(__int64 pos);

	void SetVolume(double vol) { volume = vol; }
	double GetVolume() { return volume; }
};



///////////
// Factory
class AlsaPlayerFactory : public AudioPlayerFactory {
public:
	AudioPlayer *CreatePlayer() { return new AlsaPlayer(); }
	AlsaPlayerFactory() : AudioPlayerFactory(_T("alsa")) {}
} registerAlsaPlayer;



///////////////
// Constructor
AlsaPlayer::AlsaPlayer() {
	volume = 1.0f;
	thread = NULL;
}


//////////////
// Destructor
AlsaPlayer::~AlsaPlayer() {
	CloseStream();
}


///////////////
// Open stream
void AlsaPlayer::OpenStream() {
	CloseStream();

	// Get provider
	AudioProvider *provider = GetProvider();

	thread = new AlsaPlayerThread(provider);
}


////////////////
// Close stream
void AlsaPlayer::CloseStream() {
	if (!thread) return;

	thread->Shutdown();
	thread->Wait();
	thread = 0;
}


////////
// Play
void AlsaPlayer::Play(__int64 start,__int64 count) {
	// Make sure that it's stopped
	thread->Stop();

	thread->Play(start, count);
}


////////
// Stop
void AlsaPlayer::Stop(bool timerToo) {
	if (thread) thread->Stop();

        if (timerToo && displayTimer) {
                displayTimer->Stop();
        }
}


bool AlsaPlayer::IsPlaying()
{
	return thread && thread->IsPlaying();
}


///////////
// Set end
void AlsaPlayer::SetEndPosition(__int64 pos) {
	if (thread) thread->SetEndPosition(pos);
}


////////////////////////
// Set current position
void AlsaPlayer::SetCurrentPosition(__int64 pos) {
	assert(false); // not supported (yet?)
	// I believe this isn't used anywhere. I hope not.
}


__int64 AlsaPlayer::GetStartPosition()
{
	if (thread) return thread->GetStartPosition();
	return 0;
}


__int64 AlsaPlayer::GetEndPosition()
{
	if (thread) return thread->GetEndPosition();
	return 0;
}


////////////////////////
// Get current position
__int64 AlsaPlayer::GetCurrentPosition() {
	if (thread) return thread->GetCurrentPosition();
	return 0;
}



//////////////////////
// Thread constructor
AlsaPlayerThread::AlsaPlayerThread(AudioProvider *_provider)
: wxThread(wxTHREAD_JOINABLE)
, play_notify(0, 1)
, stop_notify(0, 1)
, shutdown_notify(0, 1)
, provider(_provider)
{
	// We want playback
	stream = SND_PCM_STREAM_PLAYBACK;
	// Use default device and automatic sample type conversion
	pcm_name = "plughw:0,0";

	// Allocate params structure
	snd_pcm_hw_params_alloca(&hwparams);

	// Open device for blocking access
	if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0) {
		throw _T("Error opening default PCM device");
	}

	SetUpHardware();
}


/////////////////////
// Thread destructor
AlsaPlayerThread::~AlsaPlayerThread() {
}


void AlsaPlayerThread::SetUpHardware()
{
	// Get hardware params
	if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
		throw _T("Error setting up default PCM device");
	}

	// Set stream format
	if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		throw _T("Could not set interleaved stream format");
	}

	// Set sample format
	sample_format = SND_PCM_FORMAT_S16_LE; // TODO: support other formats
	if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, sample_format) < 0) {
		throw _T("Could not set sample format");
	}

	// Set sample rate
	rate = provider->GetSampleRate();
	real_rate = rate;
	if (snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &real_rate, 0) < 0) {
		throw _T("Could not set sample rate");
	}
	if (rate != real_rate) {
		wxLogDebug(_T("Could not set ideal sample rate %d, using %d instead"), rate, real_rate);
	}

	// Set number of channels
	if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, provider->GetChannels()) < 0) {
		throw _T("Could not set number of channels");
	}

	// Set periods (size in bytes of one frame?)
	periods = provider->GetChannels() * provider->GetBytesPerSample();
	if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0) < 0) {
		throw _T("Could not set periods");
	}

	// Set buffer size
	snd_pcm_uframes_t wanted_bufsize = rate;
	bufsize = wanted_bufsize;
	if (snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams, &bufsize) < 0) {
		throw _T("Could not set buffer size");
	}
	if (bufsize != wanted_bufsize) {
		wxLogDebug(_T("Couldn't get wanted buffer size of %u, got %u instead"), wanted_bufsize, bufsize);
	}

	// Apply parameters
	if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
		throw _T("Failed applying sound hardware settings");
	}
}


//////////////////////
// Thread entry point
wxThread::ExitCode AlsaPlayerThread::Entry()
{
	// Loop as long as we aren't told to shutdown
	while (shutdown_notify.TryWait() == wxSEMA_BUSY) {
		// Wait for a playback operation
		while (play_notify.WaitTimeout(100) == wxSEMA_NO_ERROR) {
			// So playback was posted... now play something
			play_mutex.Lock();
			PlaybackLoop();
			play_mutex.Unlock();
		}
	}

	return 0;
}

void AlsaPlayerThread::PlaybackLoop()
{
	unsigned long datalen = rate/2;
	void *data = malloc(datalen * periods);
	unsigned long cur_pos = 0;

	while (true) {
		// Get/set parameters
		parameter_mutex.Lock();
		double vol = volume;
		cur_frame = cur_pos;
		unsigned long end_pos = end_frame;
		parameter_mutex.Unlock();

		// Check for stop
		if (stop_notify.TryWait() != wxSEMA_BUSY) {
			snd_pcm_drop(pcm_handle);
			break;
		}

		// Check for end of stream
		if (cur_pos >= end_pos) {
			snd_pcm_drain(pcm_handle);
			break;
		}

		// Write some frames
		provider->GetAudioWithVolume(data, cur_pos, datalen, vol);
		snd_pcm_sframes_t written = snd_pcm_writei(pcm_handle, data, datalen);
		cur_pos += written;
	}

	free(data);
}


void AlsaPlayerThread::Play(unsigned long _start_frame, unsigned long _num_frames)
{
	StopWait();

	cur_frame = start_frame = _start_frame;
	end_frame = start_frame + _num_frames;
	
	play_notify.Post();
}


void AlsaPlayerThread::SetEndPosition(unsigned long _end_frame)
{
	parameter_mutex.Lock();
	end_frame = _end_frame;
	parameter_mutex.Unlock();
}


////////////////////////
// Stop playback thread
void AlsaPlayerThread::Stop()
{
	stop_notify.Post();	
}


void AlsaPlayerThread::StopWait()
{
	stop_notify.Post();
	// Now wait for the play mutex to have been freed
	play_mutex.Lock();
	play_mutex.Unlock();
}


void AlsaPlayerThread::Shutdown()
{
	Stop();
	shutdown_notify.Post();
}


void AlsaPlayerThread::SetVolume(double new_volume)
{
	parameter_mutex.Lock();
	volume = new_volume;
	parameter_mutex.Unlock();
}


unsigned long AlsaPlayerThread::GetStartPosition()
{
	wxMutexLocker lock(parameter_mutex);
	return start_frame;
}


unsigned long AlsaPlayerThread::GetEndPosition()
{
	wxMutexLocker lock(parameter_mutex);
	return end_frame;
}


unsigned long AlsaPlayerThread::GetCurrentPosition()
{
	// Step 1: Get last written frame number
	parameter_mutex.Lock();
	unsigned long pos = cur_frame;
	parameter_mutex.Unlock();

	// Step 2: ???

	// Step 3: Profit!
	return pos;
}


bool AlsaPlayerThread::IsPlaying()
{
	return play_mutex.TryLock() == wxMUTEX_BUSY;
}



