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
#include "options.h"
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>


//////////////
// Prototypes
class PulseAudioPlayer;


//////////
// Thread
class PulseAudioPlayerThread : public wxThread {
private:
	wxMutex play_mutex; // held while playing
	wxSemaphore play_notify; // posted when a playback operation is set up
	wxSemaphore stop_notify; // when set, audio playback should stop asap
	wxSemaphore shutdown_notify; // when set, thread should shutdown asap
	wxMutex parameter_mutex;

	AudioProvider *provider; // provides sample data!

	pa_simple *s; // stream handle
	int paerror;

	volatile double volume; // volume to get audio at
	volatile unsigned long start_frame; // first frame of playback
	volatile unsigned long cur_frame; // last written frame + 1
	volatile unsigned long end_frame; // last frame to play

	void PlaybackLoop();

public:
	PulseAudioPlayerThread(AudioProvider *_provider);
	~PulseAudioPlayerThread();
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


//////////////////////
// Pulse Audio player
class PulseAudioPlayer : public AudioPlayer {
private:
	float volume;

	PulseAudioPlayerThread *thread;

public:
	PulseAudioPlayer();
	~PulseAudioPlayer();

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
class PulseAudioPlayerFactory : public AudioPlayerFactory {
public:
	AudioPlayer *CreatePlayer() { return new PulseAudioPlayer(); }
	PulseAudioPlayerFactory() : AudioPlayerFactory(_T("pulse")) {}
} registerPulsePlayer;



///////////////
// Constructor
PulseAudioPlayer::PulseAudioPlayer() {
	volume = 1.0f;
	thread = NULL;
}


//////////////
// Destructor
PulseAudioPlayer::~PulseAudioPlayer() {
	CloseStream();
}


///////////////
// Open stream
void PulseAudioPlayer::OpenStream() {
	CloseStream();

	// Get provider
	AudioProvider *provider = GetProvider();

	try {
		thread = new PulseAudioPlayerThread(provider);
		thread->Create();
		thread->Run();
	}
	catch (const char *e) {
		wxLogError(_T("Failed to initialise PulseAudio: %s"), wxString(e, wxConvLocal).c_str());
	}
}


////////////////
// Close stream
void PulseAudioPlayer::CloseStream() {
	if (!thread) return;

	thread->Shutdown();
	thread->Wait();
	thread = 0;
}


////////
// Play
void PulseAudioPlayer::Play(__int64 start,__int64 count) {
	// Make sure that it's stopped
	thread->Stop();

	thread->Play(start, count);
}


////////
// Stop
void PulseAudioPlayer::Stop(bool timerToo) {
	if (thread) thread->Stop();

        if (timerToo && displayTimer) {
                displayTimer->Stop();
        }
}


bool PulseAudioPlayer::IsPlaying()
{
	return thread && thread->IsPlaying();
}


///////////
// Set end
void PulseAudioPlayer::SetEndPosition(__int64 pos) {
	if (thread) thread->SetEndPosition(pos);
}


////////////////////////
// Set current position
void PulseAudioPlayer::SetCurrentPosition(__int64 pos) {
	assert(false); // not supported (yet?)
	// I believe this isn't used anywhere. I hope not.
}


__int64 PulseAudioPlayer::GetStartPosition()
{
	if (thread) return thread->GetStartPosition();
	return 0;
}


__int64 PulseAudioPlayer::GetEndPosition()
{
	if (thread) return thread->GetEndPosition();
	return 0;
}


////////////////////////
// Get current position
__int64 PulseAudioPlayer::GetCurrentPosition() {
	if (thread) return thread->GetCurrentPosition();
	return 0;
}



//////////////////////
// Thread constructor
PulseAudioPlayerThread::PulseAudioPlayerThread(AudioProvider *_provider)
: wxThread(wxTHREAD_JOINABLE)
, play_notify(0, 1)
, stop_notify(0, 1)
, shutdown_notify(0, 1)
, provider(_provider)
{
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16LE;
	ss.channels = provider->GetChannels();
	ss.rate = provider->GetSampleRate();

	s = pa_simple_new(
		NULL, // default server
		"Aegisub", // application name
		PA_STREAM_PLAYBACK,
		NULL, // default device
		"Sound", // stream description
		&ss, // sample format
		NULL, // default channel map
		NULL, // default buffering attributes
		&paerror // store error code
		);
	if (!s)	throw pa_strerror(paerror);
}


/////////////////////
// Thread destructor
PulseAudioPlayerThread::~PulseAudioPlayerThread() {
	pa_simple_free(s);
}


//////////////////////
// Thread entry point
wxThread::ExitCode PulseAudioPlayerThread::Entry()
{
	// Loop as long as we aren't told to shutdown
	while (shutdown_notify.TryWait() == wxSEMA_BUSY) {
		// Wait for a playback operation
		if (play_notify.WaitTimeout(100) == wxSEMA_NO_ERROR) {
			// So playback was posted... now play something
			play_mutex.Lock();
			PlaybackLoop();
			play_mutex.Unlock();
		}
		cur_frame = cur_frame / 2;
	}

	return 0;
}

void PulseAudioPlayerThread::PlaybackLoop()
{
	// Bytes per frame
	unsigned long bpf = provider->GetChannels() * provider->GetBytesPerSample();
	// Number of frames in a read
	unsigned long datalen = provider->GetSampleRate()/8;
	// Read buffer
	void *data = malloc(datalen * bpf);
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
			pa_simple_flush(s, &paerror);
			break;
		}

		// Check for end of stream
		if (cur_pos >= end_pos) {
			pa_simple_drain(s, &paerror);
			break;
		}

		// Write some frames
		unsigned long to_read = datalen;
		if (cur_pos + to_read > end_pos)
			to_read = end_pos - cur_pos;
		provider->GetAudioWithVolume(data, cur_pos, to_read, vol);
		if (pa_simple_write(s, data, to_read*bpf, &paerror) < 0) break;
		cur_pos += to_read;
	}

	free(data);
}


void PulseAudioPlayerThread::Play(unsigned long _start_frame, unsigned long _num_frames)
{
	StopWait();

	cur_frame = start_frame = _start_frame;
	end_frame = start_frame + _num_frames;
	
	play_notify.Post();
}


void PulseAudioPlayerThread::SetEndPosition(unsigned long _end_frame)
{
	parameter_mutex.Lock();
	end_frame = _end_frame;
	parameter_mutex.Unlock();
}


////////////////////////
// Stop playback thread
void PulseAudioPlayerThread::Stop()
{
	stop_notify.Post();	
}


void PulseAudioPlayerThread::StopWait()
{
	stop_notify.Post();
	// Now wait for the play mutex to have been freed
	play_mutex.Lock();
	play_mutex.Unlock();
}


void PulseAudioPlayerThread::Shutdown()
{
	Stop();
	shutdown_notify.Post();
}


void PulseAudioPlayerThread::SetVolume(double new_volume)
{
	parameter_mutex.Lock();
	volume = new_volume;
	parameter_mutex.Unlock();
}


unsigned long PulseAudioPlayerThread::GetStartPosition()
{
	wxMutexLocker lock(parameter_mutex);
	return start_frame;
}


unsigned long PulseAudioPlayerThread::GetEndPosition()
{
	wxMutexLocker lock(parameter_mutex);
	return end_frame;
}


unsigned long PulseAudioPlayerThread::GetCurrentPosition()
{
	// Step 1: Get last written frame number
	parameter_mutex.Lock();
	unsigned long pos = cur_frame;
	parameter_mutex.Unlock();

	// Step 2: ???

	// Step 3: Profit!
	return pos;
}


bool PulseAudioPlayerThread::IsPlaying()
{
	return play_mutex.TryLock() == wxMUTEX_BUSY;
}



