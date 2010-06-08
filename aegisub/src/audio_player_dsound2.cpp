// Copyright (c) 2008, 2010, Niels Martin Hansen
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

/// @file audio_player_dsound2.cpp
/// @brief New DirectSound-based audio output
/// @ingroup audio_output
///


///////////
// Headers
#include "config.h"

#ifdef WITH_DIRECTSOUND

#ifndef AGI_PRE
#include <mmsystem.h>
#endif
#include <process.h>
#include <dsound.h>

#include <libaegisub/log.h>

#include "audio_player_dsound2.h"
#include "frame_main.h"
#include "include/aegisub/audio_provider.h"
#include "main.h"
#include "options.h"
#include "utils.h"



/// @brief RAII support class to init and de-init the COM library
struct COMInitialization {

	/// Flag set if an inited COM library is managed
	bool inited;


	/// @brief Constructor, sets inited false
	COMInitialization()
	{
		inited = false;
	}


	/// @brief Destructor, de-inits COM if it is inited
	~COMInitialization()
	{
		if (inited) CoUninitialize();
	}


	/// @brief Initialise the COM library as single-threaded apartment if isn't already inited by us
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



/// @class COMObjectRetainer
/// @brief Simple auto_ptr-like class for COM objects
template<class T>
struct COMObjectRetainer {

	/// Managed object
	T *obj;


	/// @brief Constructor for null object
	COMObjectRetainer()
	{
		obj = 0;
	}


	/// @brief Constructor to take object immediately
	/// @param _obj Object to manage
	COMObjectRetainer(T *_obj)
	{
		obj = _obj;
	}


	/// @brief Destructor, releases object if there is one
	~COMObjectRetainer()
	{
		if (obj) obj->Release();
	}


	/// @brief Dereference the managed object
	/// @return The managed object
	T * operator -> ()
	{
		return obj;
	}
};



/// @brief RAII wrapper around Win32 HANDLE type
struct Win32KernelHandle {
	/// HANDLE value being managed
	HANDLE handle;

	/// @brief Create with a managed handle
	/// @param handle Win32 handle to manage
	Win32KernelHandle(HANDLE handle = 0)
		: handle(handle)
	{
	}

	/// @brief Destructor, closes the managed handle
	~Win32KernelHandle()
	{
		if (handle) CloseHandle(handle);
	}

	/// @brief Returns the managed handle
	operator HANDLE () const { return handle; }
};



/// @class DirectSoundPlayer2Thread
/// @brief Playback thread class for DirectSoundPlayer2
///
/// Not based on wxThread, but uses Win32 threads directly
class DirectSoundPlayer2Thread {
	static unsigned int __stdcall ThreadProc(void *parameter);
	void Run();

	DWORD FillAndUnlockBuffers(void *buf1, DWORD buf1sz, void *buf2, DWORD buf2sz, int64_t &input_frame, IDirectSoundBuffer8 *bfr);

	void CheckError();


	/// Win32 handle to the thread
	Win32KernelHandle thread_handle;

	/// Event object, world to thread, set to start playback
	Win32KernelHandle event_start_playback;

	/// Event object, world to thread, set to stop playback
	Win32KernelHandle event_stop_playback;

	/// Event object, world to thread, set if playback end time was updated
	Win32KernelHandle event_update_end_time;

	/// Event object, world to thread, set if the volume was changed
	Win32KernelHandle event_set_volume;

	/// Event object, world to thread, set if the thread should end as soon as possible
	Win32KernelHandle event_kill_self;

	/// Event object, thread to world, set when the thread has entered its main loop
	Win32KernelHandle thread_running;

	/// Event object, thread to world, set when playback is ongoing
	Win32KernelHandle is_playing;

	/// Event object, thread to world, set if an error state has occurred (implies thread is dying)
	Win32KernelHandle error_happened;

	/// Statically allocated error message text describing reason for error_happened being set
	const wxChar *error_message;

	/// Playback volume, 1.0 is "unchanged"
	double volume;

	/// Audio frame to start playback at
	int64_t start_frame;

	/// Audio frame to end playback at
	int64_t end_frame;


	/// Desired length in milliseconds to write ahead of the playback cursor
	int wanted_latency;

	/// Multiplier for WantedLatency to get total buffer length
	int buffer_length;


	/// System millisecond timestamp of last playback start, used to calculate playback position
	DWORD last_playback_restart;


	/// Audio provider to take sample data from
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



/// @brief Win32 thread entry point
/// @param parameter Pointer to our thread object
/// @return Thread return value, always 0 here
///
unsigned int __stdcall DirectSoundPlayer2Thread::ThreadProc(void *parameter)
{
	static_cast<DirectSoundPlayer2Thread*>(parameter)->Run();
	return 0;
}



/// @brief Thread entry point
void DirectSoundPlayer2Thread::Run()
{

/// Macro used to set error_message, error_happened and end the thread
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

	//wx Log Debug(_T("DirectSoundPlayer2: Created buffer of %d bytes, supposed to be %d milliseconds or %d frames"), bufSize, WANTED_LATENCY*BUFFER_LENGTH, bufSize/provider->GetBytesPerSample());


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
				if (FAILED(res))
				{
					if (res == DSERR_BUFFERLOST)
					{
						// Try to regain the buffer
						if (FAILED(bfr->Restore()) ||
							FAILED(bfr->Lock(buffer_offset, 0, &buf, &buf_size, 0, 0, DSBLOCK_ENTIREBUFFER)))
						{
							REPORT_ERROR("Lost buffer and could not restore it.")
						}
					}
					else
					{
						REPORT_ERROR("Could not lock buffer for playback.")
					}
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

				// Requesting zero buffer makes Windows cry, and zero buffer seemed to be
				// a common request on Windows 7. (Maybe related to the new timer coalescing?)
				// We'll probably get non-zero bytes requested on the next iteration.
				if (bytes_needed == 0)
					break;

				DWORD buf1sz, buf2sz;
				void *buf1, *buf2;

				assert(bytes_needed > 0);
				assert(buffer_offset < bufSize);
				assert((DWORD)bytes_needed <= bufSize);

				HRESULT res = bfr->Lock(buffer_offset, bytes_needed, &buf1, &buf1sz, &buf2, &buf2sz, 0);
				switch (res)
				{
				case DSERR_BUFFERLOST:
					// Try to regain the buffer
					// When the buffer was lost the entire contents was lost too, so we have to start over
					if (SUCCEEDED(bfr->Restore()) &&
					    SUCCEEDED(bfr->Lock(0, bufSize, &buf1, &buf1sz, &buf2, &buf2sz, 0)) &&
					    SUCCEEDED(bfr->Play(0, 0, DSBPLAY_LOOPING)))
					{
						LOG_D("audio/player/dsound") << "Lost and restored buffer";
						break;
					}
					REPORT_ERROR("Lost buffer and could not restore it.")

				case DSERR_INVALIDPARAM:
					REPORT_ERROR("Invalid parameters to IDirectSoundBuffer8::Lock().")

				case DSERR_INVALIDCALL:
					REPORT_ERROR("Invalid call to IDirectSoundBuffer8::Lock().")

				case DSERR_PRIOLEVELNEEDED:
					REPORT_ERROR("Incorrect priority level set on DirectSoundBuffer8 object.")

				default:
					if (FAILED(res))
						REPORT_ERROR("Could not lock audio buffer, unknown error.")
					break;
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



/// @brief Fill audio data into a locked buffer-pair and unlock the buffers
/// @param buf1        First buffer in pair
/// @param buf1sz      Byte-size of first buffer in pair
/// @param buf2        Second buffer in pair, or null
/// @param buf2sz      Byte-size of second buffer in pair
/// @param input_frame First audio frame to fill into buffers
/// @param bfr         DirectSound buffer object owning the buffer pair
/// @return Number of bytes written
///
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



/// @brief Check for error state and throw exception if one occurred
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



/// @brief Constructor, creates and starts playback thread
/// @param provider       Audio provider to take sample data from
/// @param _WantedLatency Desired length in milliseconds to write ahead of the playback cursor
/// @param _BufferLength  Multiplier for WantedLatency to get total buffer length
DirectSoundPlayer2Thread::DirectSoundPlayer2Thread(AudioProvider *provider, int _WantedLatency, int _BufferLength)
: event_start_playback  (CreateEvent(0, FALSE, FALSE, 0))
, event_stop_playback   (CreateEvent(0, FALSE, FALSE, 0))
, event_update_end_time (CreateEvent(0, FALSE, FALSE, 0))
, event_set_volume      (CreateEvent(0, FALSE, FALSE, 0))
, event_kill_self       (CreateEvent(0, FALSE, FALSE, 0))
, thread_running        (CreateEvent(0,  TRUE, FALSE, 0))
, is_playing            (CreateEvent(0,  TRUE, FALSE, 0))
, error_happened        (CreateEvent(0, FALSE, FALSE, 0))
{
	error_message = 0;
	volume = 1.0;
	start_frame = 0;
	end_frame = 0;

	wanted_latency	= _WantedLatency;
	buffer_length	= _BufferLength;

	this->provider = provider;

	thread_handle.handle = (HANDLE)_beginthreadex(0, 0, ThreadProc, this, 0, 0);

	if (!thread_handle)
		throw _T("Failed creating playback thread in DirectSoundPlayer2. This is bad.");

	HANDLE running_or_error[] = { thread_running, error_happened };
	switch (WaitForMultipleObjects(2, running_or_error, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
		// running, all good
		return;

	case WAIT_OBJECT_0 + 1:
		// error happened, we fail
		throw error_message;

	default:
		throw _T("Failed wait for thread start or thread error in DirectSoundPlayer2. This is bad.");
	}
}



/// @brief Destructor, waits for thread to have died
DirectSoundPlayer2Thread::~DirectSoundPlayer2Thread()
{
	SetEvent(event_kill_self);
	WaitForSingleObject(thread_handle, INFINITE);
}



/// @brief Start audio playback
/// @param start Audio frame to start playback at
/// @param count Number of audio frames to play
void DirectSoundPlayer2Thread::Play(int64_t start, int64_t count)
{
	CheckError();

	start_frame = start;
	end_frame = start+count;
	SetEvent(event_start_playback);

	last_playback_restart = GetTickCount();
}



/// @brief Stop audio playback
void DirectSoundPlayer2Thread::Stop()
{
	CheckError();

	SetEvent(event_stop_playback);
}



/// @brief Change audio playback end point
/// @param new_end_frame New last audio frame to play
///
/// Playback stops instantly if new_end_frame is before the current playback position
void DirectSoundPlayer2Thread::SetEndFrame(int64_t new_end_frame)
{
	CheckError();

	end_frame = new_end_frame;
	SetEvent(event_update_end_time);
}



/// @brief Change audio playback volume
/// @param new_volume New playback amplification factor, 1.0 is "unchanged"
void DirectSoundPlayer2Thread::SetVolume(double new_volume)
{
	CheckError();

	volume = new_volume;
	SetEvent(event_set_volume);
}



/// @brief Tell whether audio playback is active
/// @return True if audio is being played back, false if it is not
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



/// @brief Get first audio frame in current playback range
/// @return Audio frame index
int64_t DirectSoundPlayer2Thread::GetStartFrame()
{
	CheckError();

	return start_frame;
}



/// @brief Get approximate current audio frame being heard by the user
/// @return Audio frame index
///
/// Returns 0 if not playing
int64_t DirectSoundPlayer2Thread::GetCurrentFrame()
{
	CheckError();

	if (!IsPlaying()) return 0;

	DWORD milliseconds_elapsed = GetTickCount() - last_playback_restart;

	return start_frame + milliseconds_elapsed * provider->GetSampleRate() / 1000;
}



/// @brief Get audio playback end point
/// @return Audio frame index
int64_t DirectSoundPlayer2Thread::GetEndFrame()
{
	CheckError();

	return end_frame;
}



/// @brief Get current playback volume
/// @return Audio amplification factor
double DirectSoundPlayer2Thread::GetVolume()
{
	CheckError();

	return volume;
}



/// @brief Tell whether playback thread has died
/// @return True if thread is no longer running
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





/// @brief Constructor
DirectSoundPlayer2::DirectSoundPlayer2()
{
	thread = 0;

	// The buffer will hold BufferLength times WantedLatency milliseconds of audio
	WantedLatency = OPT_GET("Player/Audio/DirectSound/Buffer Latency")->GetInt();
	BufferLength = OPT_GET("Player/Audio/DirectSound/Buffer Length")->GetInt();

	// sanity checking
	if (WantedLatency <= 0)
		WantedLatency = 100;
	if (BufferLength <= 0)
		BufferLength = 5;
}



/// @brief Destructor
DirectSoundPlayer2::~DirectSoundPlayer2()
{
	CloseStream();
}



/// @brief Tell whether playback thread is alive
/// @return True if there is a playback thread and it's ready
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



/// @brief Prepare for playback
///
/// This means creating the playback thread
void DirectSoundPlayer2::OpenStream()
{
	if (IsThreadAlive()) return;

	try
	{
		thread = new DirectSoundPlayer2Thread(GetProvider(), WantedLatency, BufferLength);
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
		thread = 0;
	}
}



/// @brief Shutdown playback
void DirectSoundPlayer2::CloseStream()
{
	if (!IsThreadAlive()) return;

	try
	{
		delete thread;
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
	}
	thread = 0;
}



/// @brief Change audio provider used
/// @param provider New audio provider to use
///
/// Will re-create the playback thread if the provider changed and playback was open
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
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
	}
}



/// @brief Start playback
/// @param start First audio frame to play
/// @param count Number of audio frames to play
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
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
	}
}



/// @brief Stop audio playback
/// @param timerToo Whether to also stop the playback update timer
///
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
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
	}
}



/// @brief Tell whether playback is active
/// @return True if audio is playing back
bool DirectSoundPlayer2::IsPlaying()
{
	try
	{
		if (!IsThreadAlive()) return false;
		return thread->IsPlaying();
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
		return false;
	}
}



/// @brief Get first audio frame in playback range
/// @return Audio frame index
///
/// Returns 0 if playback is stopped or there is no playback thread
int64_t DirectSoundPlayer2::GetStartPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetStartFrame();
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
		return 0;
	}
}



/// @brief Get playback end position
/// @return Audio frame index
///
/// Returns 0 if playback is stopped or there is no playback thread
int64_t DirectSoundPlayer2::GetEndPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetEndFrame();
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
		return 0;
	}
}



/// @brief Get approximate playback position
/// @return Index of audio frame user is currently hearing
///
/// Returns 0 if playback is stopped or there is no playback thread
int64_t DirectSoundPlayer2::GetCurrentPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetCurrentFrame();
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
		return 0;
	}
}



/// @brief Change playback end position
/// @param pos New end position
void DirectSoundPlayer2::SetEndPosition(int64_t pos)
{
	try
	{
		if (IsThreadAlive()) thread->SetEndFrame(pos);
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
	}
}



/// @brief Seek playback to new position
/// @param pos New position to seek to
///
/// This is done by simply restarting playback
void DirectSoundPlayer2::SetCurrentPosition(int64_t pos)
{
	try
	{
		if (IsThreadAlive()) thread->Play(pos, thread->GetEndFrame()-pos);
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
	}
}



/// @brief Change playback volume
/// @param vol Amplification factor
void DirectSoundPlayer2::SetVolume(double vol)
{
	try
	{
		if (IsThreadAlive()) thread->SetVolume(vol);
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
	}
}



/// @brief Get playback volume
/// @return Amplification factor
double DirectSoundPlayer2::GetVolume()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetVolume();
	}
	catch (const wxChar *msg)
	{
		LOG_E("audio/player/dsound") << msg;
		wxLogError(msg);
		return 0;
	}
}


#endif // WITH_DIRECTSOUND


