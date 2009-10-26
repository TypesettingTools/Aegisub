// Copyright (c) 2008, Niels Martin Hansen
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
// Website: http://www.aegisub.net/
// Contact: mailto:jiifurusu@gmail.com
//


///////////
// Headers
#include "config.h"

#ifdef WITH_DIRECTSOUND

#include <wx/wxprec.h>
#include <mmsystem.h>
#include <dsound.h>
#include <process.h>
#include "include/aegisub/audio_provider.h"
#include "utils.h"
#include "main.h"
#include "frame_main.h"
#include "options.h"
#include "audio_player_dsound2.h"


struct COMInitialization {
	bool inited;

	COMInitialization()
	{
		inited = false;
	}

	~COMInitialization()
	{
		if (inited) CoUninitialize();
	}

	void Init()
	{
		if (!inited)
		{
			if (FAILED(CoInitialize(NULL)))
				throw std::exception();
			inited = true;
		}
	}
};


template<class T>
struct COMObjectRetainer {
	T *obj;

	COMObjectRetainer()
	{
		obj = 0;
	}

	COMObjectRetainer(T *_obj)
	{
		obj = _obj;
	}

	~COMObjectRetainer()
	{
		if (obj) obj->Release();
	}

	T * operator -> ()
	{
		return obj;
	}
};


class DirectSoundPlayer2Thread {
	static unsigned int __stdcall ThreadProc(void *parameter);
	void Run();

	DWORD FillAndUnlockBuffers(void *buf1, DWORD buf1sz, void *buf2, DWORD buf2sz, int64_t &input_frame, IDirectSoundBuffer8 *bfr);

	void CheckError();

	HANDLE thread_handle;

	// Used to signal state-changes to thread
	HANDLE
		event_start_playback,
		event_stop_playback,
		event_update_end_time,
		event_set_volume,
		event_kill_self;

	// Thread communicating back
	HANDLE
		thread_running,
		is_playing,
		error_happened;

	wxChar *error_message;
	double volume;
	int64_t start_frame;
	int64_t end_frame;

	int wanted_latency;
	int buffer_length;

	DWORD last_playback_restart;

	AudioProvider *provider;

public:
	DirectSoundPlayer2Thread(AudioProvider *provider, int WantedLatency, int BufferLength);
	~DirectSoundPlayer2Thread();

	void Play(int64_t start, int64_t count);
	void Stop();
	void SetEndFrame(int64_t new_end_frame);
	void SetVolume(double new_volume);

	bool IsPlaying();
	int64_t GetStartFrame();
	int64_t GetCurrentFrame();
	int64_t GetEndFrame();
	double GetVolume();
	bool IsDead();
};


unsigned int __stdcall DirectSoundPlayer2Thread::ThreadProc(void *parameter)
{
	static_cast<DirectSoundPlayer2Thread*>(parameter)->Run();
	return 0;
}


void DirectSoundPlayer2Thread::Run()
{
#define REPORT_ERROR(msg) { error_message = _T("DirectSoundPlayer2Thread: ") _T(msg); SetEvent(error_happened); return; }

	COMInitialization COM_library;
	try	{ COM_library.Init(); }
	catch (std::exception e)
		REPORT_ERROR("Could not initialise COM")


	// Create DirectSound object
	COMObjectRetainer<IDirectSound8> ds;
	if (FAILED(DirectSoundCreate8(&DSDEVID_DefaultPlayback, &ds.obj, NULL)))
		REPORT_ERROR("Cound not create DirectSound object")


	// Ensure we can get interesting wave formats (unless we have PRIORITY we can only use a standard 8 bit format)
	ds->SetCooperativeLevel((HWND)static_cast<AegisubApp*>(wxApp::GetInstance())->frame->GetHandle(), DSSCL_PRIORITY);

	// Describe the wave format
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = provider->GetSampleRate();
	waveFormat.nChannels = provider->GetChannels();
	waveFormat.wBitsPerSample = provider->GetBytesPerSample() * 8;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = sizeof(waveFormat);

	// And the buffer itself
	int aim = waveFormat.nAvgBytesPerSec * (wanted_latency*buffer_length)/1000;
	int min = DSBSIZE_MIN;
	int max = DSBSIZE_MAX;
	DWORD bufSize = MIN(MAX(min,aim),max); // size of entier playback buffer
	DSBUFFERDESC desc;
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	desc.dwBufferBytes = bufSize;
	desc.dwReserved = 0;
	desc.lpwfxFormat = &waveFormat;
	desc.guid3DAlgorithm = GUID_NULL;

	// And then create the buffer
	IDirectSoundBuffer *bfr7 = 0;
	if FAILED(ds->CreateSoundBuffer(&desc, &bfr7, 0))
		REPORT_ERROR("Could not create buffer")

	// But it's an old version interface we get, query it for the DSound8 interface
	COMObjectRetainer<IDirectSoundBuffer8> bfr;
	if (FAILED(bfr7->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&bfr.obj)))
		REPORT_ERROR("Buffer doesn't support version 8 interface")
	bfr7->Release();
	bfr7 = 0;

	//wxLogDebug(_T("DirectSoundPlayer2: Created buffer of %d bytes, supposed to be %d milliseconds or %d frames"), bufSize, WANTED_LATENCY*BUFFER_LENGTH, bufSize/provider->GetBytesPerSample());


	// Now we're ready to roll!
	SetEvent(thread_running);
	bool running = true;

	HANDLE events_to_wait[] = {
		event_start_playback,
		event_stop_playback,
		event_update_end_time,
		event_set_volume,
		event_kill_self
	};

	int64_t next_input_frame = 0;
	DWORD buffer_offset = 0;
	bool playback_should_be_running = false;
	int current_latency = wanted_latency;
	const DWORD wanted_latency_bytes = wanted_latency*waveFormat.nSamplesPerSec*provider->GetBytesPerSample()/1000;

	while (running)
	{
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait)/sizeof(HANDLE), events_to_wait, FALSE, current_latency);

		switch (wait_result)
		{
		case WAIT_OBJECT_0+0:
			{
				// Start or restart playback
				bfr->Stop();
				ResetEvent(is_playing);

				next_input_frame = start_frame;

				DWORD buf_size; // size of buffer locked for filling
				void *buf;
				buffer_offset = 0;

				if (FAILED(bfr->SetCurrentPosition(0)))
					REPORT_ERROR("Could not reset playback buffer cursor before filling first buffer.")

				HRESULT res = bfr->Lock(buffer_offset, 0, &buf, &buf_size, 0, 0, DSBLOCK_ENTIREBUFFER);
				while (FAILED(res)) // yes, while, so I can break out of it without a goto!
				{
					if (res == DSERR_BUFFERLOST)
					{
						// Try to regain the buffer
						if (SUCCEEDED(bfr->Restore()) &&
							SUCCEEDED(bfr->Lock(buffer_offset, 0, &buf, &buf_size, 0, 0, DSBLOCK_ENTIREBUFFER)))
						{
							//wxLogDebug(_T("DirectSoundPlayer2: Lost and restored buffer"));
							break;
						}

						REPORT_ERROR("Lost buffer and could not restore it.")
					}

					REPORT_ERROR("Could not lock buffer for playback.")
				}

				// Clear the buffer in case we can't fill it completely
				memset(buf, 0, buf_size);

				DWORD bytes_filled = FillAndUnlockBuffers(buf, buf_size, 0, 0, next_input_frame, bfr.obj);
				buffer_offset += bytes_filled;
				if (buffer_offset >= bufSize) buffer_offset -= bufSize;

				if (FAILED(bfr->SetCurrentPosition(0)))
					REPORT_ERROR("Could not reset playback buffer cursor before playback.")

				if (bytes_filled < wanted_latency_bytes)
				{
					// Very short playback length, do without streaming playback
					current_latency = (bytes_filled*1000) / (waveFormat.nSamplesPerSec*provider->GetBytesPerSample());
					if (FAILED(bfr->Play(0, 0, 0)))
						REPORT_ERROR("Could not start single-buffer playback.")
				}
				else
				{
					// We filled the entire buffer so there's reason to do streaming playback
					current_latency = wanted_latency;
					if (FAILED(bfr->Play(0, 0, DSBPLAY_LOOPING)))
						REPORT_ERROR("Could not start looping playback.")
				}

				SetEvent(is_playing);
				playback_should_be_running = true;

				break;
			}

		case WAIT_OBJECT_0+1:
			{
				// Stop playing
				bfr->Stop();
				ResetEvent(is_playing);
				playback_should_be_running = false;
				break;
			}

		case WAIT_OBJECT_0+2:
			{
				// Set end frame
				if (end_frame <= next_input_frame)
				{
					bfr->Stop();
					ResetEvent(is_playing);
					playback_should_be_running = false;
				}
				else
				{
					// If the user is dragging the start or end point in the audio display
					// the set end frame events might come in faster than the timeouts happen
					// and then new data never get filled into the buffer. See bug #915.
					goto do_fill_buffer;
				}
				break;
			}

		case WAIT_OBJECT_0+3:
			{
				// Change volume
				// We aren't thread safe right now, filling the buffers grabs volume directly
				// from the field set by the controlling thread, but it shouldn't be a major
				// problem if race conditions do occur, just some momentary distortion.
				goto do_fill_buffer;
			}

		case WAIT_OBJECT_0+4:
			{
				// Perform suicide
				bfr->Stop();
				ResetEvent(is_playing);
				playback_should_be_running = false;
				running = false;
				break;
			}

		case WAIT_TIMEOUT:
do_fill_buffer:
			{
				// Time to fill more into buffer

				if (!playback_should_be_running)
					break;

				DWORD status;
				if (FAILED(bfr->GetStatus(&status)))
					REPORT_ERROR("Could not get playback buffer status")

				if (!(status & DSBSTATUS_LOOPING))
				{
					// Not looping playback...
					// hopefully we only triggered timeout after being done with the buffer
					bfr->Stop();
					ResetEvent(is_playing);
					playback_should_be_running = false;
					break;
				}

				DWORD play_cursor;
				if (FAILED(bfr->GetCurrentPosition(&play_cursor, 0)))
					REPORT_ERROR("Could not get play cursor position for filling buffer.")

				int bytes_needed = (int)play_cursor - (int)buffer_offset;
				if (bytes_needed < 0) bytes_needed += (int)bufSize;

				DWORD buf1sz, buf2sz;
				void *buf1, *buf2;

				HRESULT res = bfr->Lock(buffer_offset, bytes_needed, &buf1, &buf1sz, &buf2, &buf2sz, 0);
				while (FAILED(res)) // yes, while, so I can break out of it without a goto!
				{
					if (res == DSERR_BUFFERLOST)
					{
						// Try to regain the buffer
						// When the buffer was lost the entire contents was lost too, so we have to start over
						if (SUCCEEDED(bfr->Restore()) &&
						    SUCCEEDED(bfr->Lock(0, bufSize, &buf1, &buf1sz, &buf2, &buf2sz, 0)) &&
						    SUCCEEDED(bfr->Play(0, 0, DSBPLAY_LOOPING)))
						{
							wxLogDebug(_T("DirectSoundPlayer2: Lost and restored buffer"));
							break;
						}

						REPORT_ERROR("Lost buffer and could not restore it.")
					}

					REPORT_ERROR("Could not lock buffer for filling.")
				}

				DWORD bytes_filled = FillAndUnlockBuffers(buf1, buf1sz, buf2, buf2sz, next_input_frame, bfr.obj);
				buffer_offset += bytes_filled;
				if (buffer_offset >= bufSize) buffer_offset -= bufSize;

				if (bytes_filled < 1024)
				{
					// Arbitrary low number, we filled in very little so better get back to filling in the rest with silence
					// really fast... set latency to zero in this case.
					current_latency = 0;
				}
				else if (bytes_filled < wanted_latency_bytes)
				{
					// Didn't fill as much as we wanted to, let's get back to filling sooner than normal
					current_latency = (bytes_filled*1000) / (waveFormat.nSamplesPerSec*provider->GetBytesPerSample());
				}
				else
				{
					// Plenty filled in, do regular latency
					current_latency = wanted_latency;
				}

				break;
			}

		default:
			REPORT_ERROR("Something bad happened while waiting on events in playback loop, either the wait failed or an event object was abandoned.")
			break;
		}
	}

#undef REPORT_ERROR
}


DWORD DirectSoundPlayer2Thread::FillAndUnlockBuffers(void *buf1, DWORD buf1sz, void *buf2, DWORD buf2sz, int64_t &input_frame, IDirectSoundBuffer8 *bfr)
{
	// Assume buffers have been locked and are ready to be filled

	DWORD bytes_per_frame = provider->GetChannels() * provider->GetBytesPerSample();
	DWORD buf1szf = buf1sz / bytes_per_frame;
	DWORD buf2szf = buf2sz / bytes_per_frame;

	if (input_frame >= end_frame)
	{
		// Silence

		if (buf1)
			memset(buf1, 0, buf1sz);

		if (buf2)
			memset(buf2, 0, buf2sz);

		input_frame += buf1szf + buf2szf;

		bfr->Unlock(buf1, buf1sz, buf2, buf2sz); // should be checking for success

		return buf1sz + buf2sz;
	}

	if (buf1 && buf1sz)
	{
		if (buf1szf + input_frame > end_frame)
		{
			buf1szf = end_frame - input_frame;
			buf1sz = buf1szf * bytes_per_frame;
			buf2szf = 0;
			buf2sz = 0;
		}

		provider->GetAudioWithVolume(buf1, input_frame, buf1szf, volume);

		input_frame += buf1szf;
	}

	if (buf2 && buf2sz)
	{
		if (buf2szf + input_frame > end_frame)
		{
			buf2szf = end_frame - input_frame;
			buf2sz = buf2szf * bytes_per_frame;
		}

		provider->GetAudioWithVolume(buf2, input_frame, buf2szf, volume);

		input_frame += buf2szf;
	}

	bfr->Unlock(buf1, buf1sz, buf2, buf2sz); // bad? should check for success

	return buf1sz + buf2sz;
}


void DirectSoundPlayer2Thread::CheckError()
{
	try
	{
		switch (WaitForSingleObject(error_happened, 0))
		{
		case WAIT_OBJECT_0:
			throw error_message;

		case WAIT_ABANDONED:
			throw _T("The DirectShowPlayer2Thread error signal event was abandoned, somehow. This should not happen.");

		case WAIT_FAILED:
			throw _T("Failed checking state of DirectShowPlayer2Thread error signal event.");

		case WAIT_TIMEOUT:
		default:
			return;
		}
	}
	catch (...)
	{
		ResetEvent(is_playing);
		ResetEvent(thread_running);
		throw;
	}
}


DirectSoundPlayer2Thread::DirectSoundPlayer2Thread(AudioProvider *provider, int _WantedLatency, int _BufferLength)
{
	event_start_playback  = CreateEvent(0, FALSE, FALSE, 0);
	event_stop_playback   = CreateEvent(0, FALSE, FALSE, 0);
	event_update_end_time = CreateEvent(0, FALSE, FALSE, 0);
	event_set_volume      = CreateEvent(0, FALSE, FALSE, 0);
	event_kill_self       = CreateEvent(0, FALSE, FALSE, 0);

	thread_running        = CreateEvent(0,  TRUE, FALSE, 0);
	is_playing            = CreateEvent(0,  TRUE, FALSE, 0);
	error_happened        = CreateEvent(0, FALSE, FALSE, 0);

	error_message = 0;
	volume = 1.0;
	start_frame = 0;
	end_frame = 0;

	wanted_latency	= _WantedLatency;
	buffer_length	= _BufferLength;

	this->provider = provider;

	thread_handle = (HANDLE)_beginthreadex(0, 0, ThreadProc, this, 0, 0);

	if (!thread_handle)
		throw _T("Failed creating playback thread in DirectSoundPlayer2. This is bad.");

	CheckError();

	WaitForSingleObject(thread_running, INFINITE);
}


DirectSoundPlayer2Thread::~DirectSoundPlayer2Thread()
{
	SetEvent(event_kill_self);
	WaitForSingleObject(thread_handle, INFINITE);
}


void DirectSoundPlayer2Thread::Play(int64_t start, int64_t count)
{
	CheckError();

	start_frame = start;
	end_frame = start+count;
	SetEvent(event_start_playback);

	last_playback_restart = GetTickCount();
}


void DirectSoundPlayer2Thread::Stop()
{
	CheckError();

	SetEvent(event_stop_playback);
}


void DirectSoundPlayer2Thread::SetEndFrame(int64_t new_end_frame)
{
	CheckError();

	end_frame = new_end_frame;
	SetEvent(event_update_end_time);
}


void DirectSoundPlayer2Thread::SetVolume(double new_volume)
{
	CheckError();

	volume = new_volume;
	SetEvent(event_set_volume);
}


bool DirectSoundPlayer2Thread::IsPlaying()
{
	CheckError();

	switch (WaitForSingleObject(is_playing, 0))
	{
	case WAIT_ABANDONED:
		throw _T("The DirectShowPlayer2Thread playback state event was abandoned, somehow. This should not happen.");

	case WAIT_FAILED:
		throw _T("Failed checking state of DirectShowPlayer2Thread playback state event.");

	case WAIT_OBJECT_0:
		return true;

	case WAIT_TIMEOUT:
	default:
		return false;
	}
}


int64_t DirectSoundPlayer2Thread::GetStartFrame()
{
	CheckError();

	return start_frame;
}


int64_t DirectSoundPlayer2Thread::GetCurrentFrame()
{
	CheckError();

	if (!IsPlaying()) return 0;

	DWORD milliseconds_elapsed = GetTickCount() - last_playback_restart;

	return start_frame + milliseconds_elapsed * provider->GetSampleRate() / 1000;
}


int64_t DirectSoundPlayer2Thread::GetEndFrame()
{
	CheckError();

	return end_frame;
}


double DirectSoundPlayer2Thread::GetVolume()
{
	CheckError();

	return volume;
}


bool DirectSoundPlayer2Thread::IsDead()
{
	switch (WaitForSingleObject(thread_running, 0))
	{
	case WAIT_OBJECT_0:
		return false;

	default:
		return true;
	}
}




DirectSoundPlayer2::DirectSoundPlayer2()
{
	thread = 0;

	// The buffer will hold BufferLength times WantedLatency milliseconds of audio
	WantedLatency = Options.AsInt(_T("Audio dsound buffer latency"));
	BufferLength = Options.AsInt(_T("Audio dsound buffer length"));

	// sanity checking
	if (WantedLatency <= 0)
		WantedLatency = 100;
	if (BufferLength <= 0)
		BufferLength = 5;
}


DirectSoundPlayer2::~DirectSoundPlayer2()
{
	CloseStream();
}


bool DirectSoundPlayer2::IsThreadAlive()
{
	if (!thread) return false;
	
	if (thread->IsDead())
	{
		delete thread;
		thread = 0;
		return false;
	}

	return true;
}


void DirectSoundPlayer2::OpenStream()
{
	if (IsThreadAlive()) return;

	try
	{
		thread = new DirectSoundPlayer2Thread(GetProvider(), WantedLatency, BufferLength);
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
		thread = 0;
	}
}


void DirectSoundPlayer2::CloseStream()
{
	if (!IsThreadAlive()) return;

	try
	{
		delete thread;
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
	}
	thread = 0;
}


void DirectSoundPlayer2::SetProvider(AudioProvider *provider)
{
	try
	{
		if (IsThreadAlive() && provider != GetProvider())
		{
			delete thread;
			thread = new DirectSoundPlayer2Thread(provider, WantedLatency, BufferLength);
		}

		AudioPlayer::SetProvider(provider);
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
	}
}


void DirectSoundPlayer2::Play(int64_t start,int64_t count)
{
	try
	{
		OpenStream();
		thread->Play(start, count);

		if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(15);
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
	}
}


void DirectSoundPlayer2::Stop(bool timerToo)
{
	try
	{
		if (IsThreadAlive()) thread->Stop();

		if (timerToo && displayTimer) {
			displayTimer->Stop();
		}
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
	}
}


bool DirectSoundPlayer2::IsPlaying()
{
	try
	{
		if (!IsThreadAlive()) return false;
		return thread->IsPlaying();
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
		return false;
	}
}


int64_t DirectSoundPlayer2::GetStartPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetStartFrame();
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
		return 0;
	}
}


int64_t DirectSoundPlayer2::GetEndPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetEndFrame();
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
		return 0;
	}
}


int64_t DirectSoundPlayer2::GetCurrentPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetCurrentFrame();
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
		return 0;
	}
}


void DirectSoundPlayer2::SetEndPosition(int64_t pos)
{
	try
	{
		if (IsThreadAlive()) thread->SetEndFrame(pos);
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
	}
}


void DirectSoundPlayer2::SetCurrentPosition(int64_t pos)
{
	try
	{
		if (IsThreadAlive()) thread->Play(pos, thread->GetEndFrame()-pos);
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
	}
}


void DirectSoundPlayer2::SetVolume(double vol)
{
	try
	{
		if (IsThreadAlive()) thread->SetVolume(vol);
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
	}
}


double DirectSoundPlayer2::GetVolume()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetVolume();
	}
	catch (const wxChar *msg)
	{
		wxLogError(msg);
		return 0;
	}
}


#endif // WITH_DIRECTSOUND
