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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_player_pulse.cpp
/// @brief PulseAudio-based audio output
/// @ingroup audio_output
///


#include "config.h"

#ifdef WITH_PULSEAUDIO


///////////
// Headers
#ifndef AGI_PRE
#include <stdio.h>
#endif

#include "audio_player_pulse.h"
#include "audio_provider_manager.h"
#include "utils.h"


/// @brief Constructor 
///
PulseAudioPlayer::PulseAudioPlayer()
: context_notify(0, 1)
, context_success(0, 1)
, stream_notify(0, 1)
, stream_success(0, 1)
{
	volume = 1.0f;
	paerror = 0;
	open = false;
	is_playing = false;
}



/// @brief Destructor 
///
PulseAudioPlayer::~PulseAudioPlayer()
{
	if (open) CloseStream();
}



/// @brief Open stream 
///
void PulseAudioPlayer::OpenStream()
{
	//printf("Opening PulseAudio stream\n");
	if (open) CloseStream();

	// Get provider
	AudioProvider *provider = GetProvider();

	// Initialise a mainloop
	//printf("Initialising threaded main loop\n");
	mainloop = pa_threaded_mainloop_new();
	if (!mainloop) {
		throw _T("Failed to initialise PulseAudio threaded mainloop object");
	}
	//printf("Starting main loop\n");
	pa_threaded_mainloop_start(mainloop);

	// Create context
	//printf("Creating context\n");
	context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "Aegisub");
	if (!context) {
		pa_threaded_mainloop_free(mainloop);
		throw _T("Failed to create PulseAudio context");
	}
	pa_context_set_state_callback(context, (pa_context_notify_cb_t)pa_context_notify, this);

	// Connect the context
	//printf("Connecting context\n");
	pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
	// Wait for connection
	while (true) {
		context_notify.Wait();
		if (cstate == PA_CONTEXT_READY) {
			break;
		} else if (cstate == PA_CONTEXT_FAILED) {
			// eww
			paerror = pa_context_errno(context);
			pa_context_unref(context);
			pa_threaded_mainloop_stop(mainloop);
			pa_threaded_mainloop_free(mainloop);
			wxString s(pa_strerror(paerror), wxConvUTF8);
			s.Prepend(_T("PulseAudio reported error: "));
			throw s.c_str();
		}
		// otherwise loop once more
	}
	//printf("Context connected\n");
	
	// Set up stream
	bpf = provider->GetChannels() * provider->GetBytesPerSample();
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16LE; // FIXME
	ss.rate = provider->GetSampleRate();
	ss.channels = provider->GetChannels();
	pa_channel_map map;
	pa_channel_map_init_auto(&map, ss.channels, PA_CHANNEL_MAP_DEFAULT);
	//printf("Creating stream\n");
	stream = pa_stream_new(context, "Sound", &ss, &map);
	if (!stream) {
		// argh!
		pa_context_disconnect(context);
		pa_context_unref(context);
		pa_threaded_mainloop_stop(mainloop);
		pa_threaded_mainloop_free(mainloop);
		throw _T("PulseAudio could not create stream");
	}
	pa_stream_set_state_callback(stream, (pa_stream_notify_cb_t)pa_stream_notify, this);
	pa_stream_set_write_callback(stream, (pa_stream_request_cb_t)pa_stream_write, this);

	// Connect stream
	//printf("Connecting playback stream\n");
	paerror = pa_stream_connect_playback(stream, NULL, NULL, (pa_stream_flags_t)(PA_STREAM_INTERPOLATE_TIMING|PA_STREAM_NOT_MONOTONOUS|PA_STREAM_AUTO_TIMING_UPDATE), NULL, NULL);
	if (paerror) {
		printf("PulseAudio reported error: %s (%d)\n", pa_strerror(paerror), paerror);
		wxString s(pa_strerror(paerror), wxConvUTF8);
		s.Prepend(_T("PulseAudio reported error: "));
		throw s.c_str();
	}
	while (true) {
		stream_notify.Wait();
		if (sstate == PA_STREAM_READY) {
			break;
		} else if (sstate == PA_STREAM_FAILED) {
			paerror = pa_context_errno(context);
			printf("PulseAudio player: Stream connection failed: %s (%d)\n", pa_strerror(paerror), paerror);
			throw _T("PulseAudio player: Something went wrong connecting the stream");
		}
	}
	//printf("Connected playback stream, now playing\n\n");

	// Hopefully this marks success
	//printf("Finished opening PulseAudio\n\n");
	open = true;
}



/// @brief Close stream 
/// @return 
///
void PulseAudioPlayer::CloseStream()
{
	if (!open) return;
	//printf("Closing PuseAudio\n");

	if (is_playing) Stop();

	// Hope for the best and just do things as quickly as possible
	pa_stream_disconnect(stream);
	pa_stream_unref(stream);
	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_threaded_mainloop_stop(mainloop);
	pa_threaded_mainloop_free(mainloop);

	//printf("Closed PulseAudio\n");
	open = false;
}



/// @brief Play 
/// @param start 
/// @param count 
///
void PulseAudioPlayer::Play(int64_t start,int64_t count)
{
	//printf("Starting PulseAudio playback\n");
	if (!open) OpenStream();

	if (is_playing) {
		// If we're already playing, do a quick "reset"
		is_playing = false;

		pa_threaded_mainloop_lock(mainloop);
		pa_operation *op = pa_stream_flush(stream, (pa_stream_success_cb_t)pa_stream_success, this);
		pa_threaded_mainloop_unlock(mainloop);
		stream_success.Wait();
		pa_operation_unref(op);
		if (!stream_success_val) {
			paerror = pa_context_errno(context);
			printf("PulseAudio player: Error flushing stream: %s (%d)\n", pa_strerror(paerror), paerror);
		}
	}

	start_frame = start;
	cur_frame = start;
	end_frame = start + count;
	//printf("start=%lu end=%lu\n", start_frame, end_frame);

	is_playing = true;

	play_start_time = 0;
	pa_threaded_mainloop_lock(mainloop);
	paerror = pa_stream_get_time(stream, (pa_usec_t*) &play_start_time);
	pa_threaded_mainloop_unlock(mainloop);
	if (paerror) {
		printf("PulseAudio player: Error getting stream time: %s (%d)\n", pa_strerror(paerror), paerror);
	}

	PulseAudioPlayer::pa_stream_write(stream, pa_stream_writable_size(stream), this);

	pa_threaded_mainloop_lock(mainloop);
	pa_operation *op = pa_stream_trigger(stream, (pa_stream_success_cb_t)pa_stream_success, this);
	pa_threaded_mainloop_unlock(mainloop);
	stream_success.Wait();
	pa_operation_unref(op);
	if (!stream_success_val) {
		paerror = pa_context_errno(context);
		printf("PulseAudio player: Error triggering stream: %s (%d)\n", pa_strerror(paerror), paerror);
	}

	// Update timer
	if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(15);
}



/// @brief Stop 
/// @param timerToo 
/// @return 
///
void PulseAudioPlayer::Stop(bool timerToo)
{
	if (!is_playing) return;
	//printf("Stopping PulseAudio\n");

	is_playing = false;

	start_frame = 0;
	cur_frame = 0;
	end_frame = 0;

	// Flush the stream of data
	//printf("Flushing stream\n");
	pa_threaded_mainloop_lock(mainloop);
	pa_operation *op = pa_stream_flush(stream, (pa_stream_success_cb_t)pa_stream_success, this);
	pa_threaded_mainloop_unlock(mainloop);
	stream_success.Wait();
	pa_operation_unref(op);
	if (!stream_success_val) {
		paerror = pa_context_errno(context);
		printf("PulseAudio player: Error flushing stream: %s (%d)\n", pa_strerror(paerror), paerror);
	}

	// And unref it
	//printf("Stopped stream\n\n");

        if (timerToo && displayTimer) {
                displayTimer->Stop();
        }
}



/// @brief DOCME
/// @return 
///
bool PulseAudioPlayer::IsPlaying()
{
	return is_playing;
}



/// @brief Set end 
/// @param pos 
///
void PulseAudioPlayer::SetEndPosition(int64_t pos)
{
	end_frame = pos;
}



/// @brief Set current position 
/// @param pos 
///
void PulseAudioPlayer::SetCurrentPosition(int64_t pos)
{
	cur_frame = pos;
}



/// @brief DOCME
/// @return 
///
int64_t PulseAudioPlayer::GetStartPosition()
{
	return start_frame;
}



/// @brief DOCME
/// @return 
///
int64_t PulseAudioPlayer::GetEndPosition()
{
	return end_frame;
}



/// @brief Get current position 
/// @return 
///
int64_t PulseAudioPlayer::GetCurrentPosition()
{
	if (!is_playing) return 0;

	// FIXME: this should be based on not duration played but actual sample being heard
	// (during vidoeo playback, cur_frame might get changed to resync)

	// Calculation duration we have played, in microseconds
	pa_usec_t play_cur_time;
	pa_stream_get_time(stream, &play_cur_time);
	pa_usec_t playtime = play_cur_time - play_start_time;

	return start_frame + playtime * provider->GetSampleRate() / (1000*1000);
}



/// @brief Called by PA to notify about contetxt operation completion
/// @param c       
/// @param success 
/// @param thread  
///
void PulseAudioPlayer::pa_context_success(pa_context *c, int success, PulseAudioPlayer *thread)
{
	thread->context_success_val = success;
	thread->context_success.Post();
}



/// @brief Called by PA to notify about other context-related stuff
/// @param c      
/// @param thread 
///
void PulseAudioPlayer::pa_context_notify(pa_context *c, PulseAudioPlayer *thread)
{
	thread->cstate = pa_context_get_state(thread->context);
	thread->context_notify.Post();
}



/// @brief Called by PA when an operation completes
/// @param p       
/// @param success 
/// @param thread  
///
void PulseAudioPlayer::pa_stream_success(pa_stream *p, int success, PulseAudioPlayer *thread)
{
	thread->stream_success_val = success;
	thread->stream_success.Post();
}



/// @brief Called by PA to request more data (and other things?)
/// @param p      
/// @param length 
/// @param thread 
/// @return 
///
void PulseAudioPlayer::pa_stream_write(pa_stream *p, size_t length, PulseAudioPlayer *thread)
{
	if (!thread->is_playing) return;

	if (thread->cur_frame >= thread->end_frame + thread->provider->GetSampleRate()) {
		// More than a second past end of stream
		thread->is_playing = false;
		pa_operation *op = pa_stream_drain(p, NULL, NULL);
		pa_operation_unref(op);
		//printf("PA requested more buffer, but no more to stream\n");
		return;

	} else if (thread->cur_frame >= thread->end_frame) {
		// Past end of stream, but not a full second, add some silence
		void *buf = calloc(length, 1);
		::pa_stream_write(p, buf, length, free, 0, PA_SEEK_RELATIVE);
		thread->cur_frame += length / thread->bpf;
		return;
	}

	//printf("PA requested more buffer, %lu bytes\n", (unsigned long)length);
	unsigned long bpf = thread->bpf;
	unsigned long frames = length / thread->bpf;
	unsigned long maxframes = thread->end_frame - thread->cur_frame;
	if (frames > maxframes) frames = maxframes;
	//printf("Handing it %lu frames\n", frames);
	void *buf = malloc(frames * bpf);
	thread->provider->GetAudioWithVolume(buf, thread->cur_frame, frames, thread->volume);
	::pa_stream_write(p, buf, frames*bpf, free, 0, PA_SEEK_RELATIVE);
	thread->cur_frame += frames;
}



/// @brief Called by PA to notify about other stuff
/// @param p      
/// @param thread 
///
void PulseAudioPlayer::pa_stream_notify(pa_stream *p, PulseAudioPlayer *thread)
{
	thread->sstate = pa_stream_get_state(thread->stream);
	thread->stream_notify.Post();
}


#endif // WITH_PULSEAUDIO


