// Copyright (c) 2006, Rodrigo Braz Monteiro
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
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once


///////////
// Headers
#include "setup.h"
#if USE_DIRECTSOUND == 1
#include <wx/wxprec.h>
#include "audio_provider.h"
#include "audio_player_dsound.h"
#include "utils.h"
#include "main.h"
#include "frame_main.h"


///////////////
// Constructor
DirectSoundPlayer::DirectSoundPlayer() {
	playing = false;
	volume = 1.0f;
	playPos = 0;
	startPos = 0;
	endPos = 0;
	offset = 0;

	buffer = NULL;
	directSound = NULL;
}


//////////////
// Destructor
DirectSoundPlayer::~DirectSoundPlayer() {
	CloseStream();
}


///////////////
// Open stream
void DirectSoundPlayer::OpenStream() {
	// Get provider
	AudioProvider *provider = GetProvider();

	// Initialize the DirectSound object
	HRESULT res;
	res = DirectSoundCreate8(NULL,&directSound,NULL);
	if (res != DS_OK) throw _T("Failed initializing DirectSound");

	// Set DirectSound parameters
	AegisubApp *app = (AegisubApp*) wxTheApp;
	directSound->SetCooperativeLevel((HWND)app->frame->GetHandle(),DSSCL_NORMAL);

	// Create the wave format structure
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = provider->GetSampleRate();
	waveFormat.nChannels = provider->GetChannels();
	waveFormat.wBitsPerSample = provider->GetBytesPerSample() * 8;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Create the buffer initializer
	int aim = 0x20000;
	int min = DSBSIZE_MIN;
	int max = DSBSIZE_MAX;
	bufSize = MIN(MAX(min,aim),max);
	DSBUFFERDESC desc;
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
	desc.dwBufferBytes = bufSize;
	desc.dwReserved = 0;
	desc.lpwfxFormat = &waveFormat;
	desc.guid3DAlgorithm = GUID_NULL;

	// Create the buffer
	IDirectSoundBuffer *buf;
	res = directSound->CreateSoundBuffer(&desc,&buf,NULL);
	if (res != DS_OK) throw _T("Failed creating DirectSound buffer");

	// Copy interface to buffer
	res = buf->QueryInterface(IID_IDirectSoundBuffer8,(LPVOID*) &buffer);
	if (res != S_OK) throw _T("Failed casting interface to IDirectSoundBuffer8");

	// Set data
	offset = 0;
}


////////////////
// Close stream
void DirectSoundPlayer::CloseStream() {
	// Stop it
	Stop();

	// Delete the DirectSound buffer
//	delete buffer;
	buffer = NULL;

	// Delete the DirectSound object
//	delete directSound;
	directSound = NULL;
}


///////////////
// Fill buffer
void DirectSoundPlayer::FillBuffer(bool fill) {
	// Variables
	void *ptr1, *ptr2;
	unsigned long int size1, size2;
	AudioProvider *provider = GetProvider();
	int bytesps = provider->GetBytesPerSample();

	// To write length
	//int max = bufSize/4;
	//if (fill) max = bufSize;
	//int toWrite = MIN(max,(endPos-playPos)*bytesps);
	int toWrite = bufSize/4;

	// Lock buffer
	HRESULT res;
	res = buffer->Lock(offset,toWrite,&ptr1,&size1,&ptr2,&size2,0);

	// Buffer lost?
	if (res == DSERR_BUFFERLOST) {
		buffer->Restore();
		res = buffer->Lock(offset,toWrite,&ptr1,&size1,&ptr2,&size2,0);
	}

	// Error
	if (!SUCCEEDED(res)) return;

	// Set offset
	offset = (offset + toWrite) % bufSize;

	// Convert size to number of samples
	unsigned long int count1 = size1 / bytesps;
	unsigned long int count2 = size2 / bytesps;

	// Check if it's writing over play
	unsigned long int totalCount = count1+count2;
	unsigned long int left = 0;
	if (endPos > playPos) left = endPos - playPos;
	unsigned long int delta = 0;
	if (totalCount > left) delta = totalCount - left;

	// If so, don't allow it
	if (delta) {
		// Zero at start
		memset(ptr1,0,size1);
		memset(ptr2,0,size2);

		// Lower counts
		int temp = MIN(delta,count2);
		count2 -= temp;
		delta -= temp;
		temp = MIN(delta,count1);
		count1 -= temp;
		delta -= temp;
	}

	// Get source wave
	if (count1) provider->GetAudio(ptr1,playPos,count1);
	if (count2) provider->GetAudio(ptr2,playPos+count1,count2);
	playPos += totalCount;

	// Unlock
	buffer->Unlock(ptr1,size1,ptr2,size2);
}


////////
// Play
void DirectSoundPlayer::Play(__int64 start,__int64 count) {
	// Make sure that it's stopped
	Stop();

	// Lock
	wxMutexLocker locker(DSMutex);

	// Set variables
	HRESULT res;
	startPos = start;
	endPos = start+count;
	playPos = start;
	offset = 0;

	// Check if buffer is loaded
	if (!buffer) return;

	// Create notification event
	notificationEvent = CreateEvent(NULL,false,false,NULL);

	// Create notification interface
	IDirectSoundNotify8 *notify;
	res = buffer->QueryInterface(IID_IDirectSoundNotify8,(LPVOID*)&notify);
	if (!SUCCEEDED(res)) return;

	// Set notification
	DSBPOSITIONNOTIFY positionNotify[4];
	positionNotify[0].dwOffset = bufSize / 8;
	positionNotify[0].hEventNotify = notificationEvent;
	positionNotify[1].dwOffset = 3 * bufSize / 8;
	positionNotify[1].hEventNotify = notificationEvent;
	positionNotify[2].dwOffset = 5 * bufSize / 8;
	positionNotify[2].hEventNotify = notificationEvent;
	positionNotify[3].dwOffset = 7 * bufSize / 8;
	positionNotify[3].hEventNotify = notificationEvent;
	res = notify->SetNotificationPositions(4,positionNotify);
	notify->Release();

	// Fill buffer
	FillBuffer(false);

	// Play
	buffer->SetCurrentPosition(0);
	res = buffer->Play(0,0,DSBPLAY_LOOPING);
	if (SUCCEEDED(res)) playing = true;

	// Start thread
	thread = new DirectSoundPlayerThread(this);
	thread->Create();
	thread->Run();

	// Update timer
	if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(15);
}


////////
// Stop
void DirectSoundPlayer::Stop(bool timerToo) {
	// Lock
	wxMutexLocker locker(DSMutex);

	// Check if buffer is loaded and playing
	if (!buffer || !playing) return;

	// Stop
	buffer->Stop();

	// Stop the thread
	thread = NULL;

	// Close event handle
	CloseHandle(notificationEvent);
	
	// Stop timer
	if (timerToo && displayTimer) {
		displayTimer->Stop();
	}
	playing = false;
}


///////////
// Set end
void DirectSoundPlayer::SetEndPosition(__int64 pos) {
	endPos = pos;
}


////////////////////////
// Set current position
void DirectSoundPlayer::SetCurrentPosition(__int64 pos) {
	playPos = pos;
}


////////////////////////
// Get current position
__int64 DirectSoundPlayer::GetCurrentPosition() {
	// Check if buffer is loaded
	if (!buffer || !playing) return 0;

	// Read position
	unsigned long int play,write;
	HRESULT res = buffer->GetCurrentPosition(&play,NULL);
	if (SUCCEEDED(res)) {
		int bytesps = provider->GetBytesPerSample();
		write = offset;
		if (write < play) write += bufSize;
		return playPos + play/bytesps - write/bytesps;
	}

	// Failed, just return playPos
	return playPos;
}


//////////////////////
// Thread constructor
DirectSoundPlayerThread::DirectSoundPlayerThread(DirectSoundPlayer *par) : wxThread(wxTHREAD_DETACHED) {
	parent = par;
}


/////////////////////
// Thread destructor
DirectSoundPlayerThread::~DirectSoundPlayerThread() {
}


//////////////////////
// Thread entry point
wxThread::ExitCode DirectSoundPlayerThread::Entry() {
	// Variables
	unsigned long int playPos,endPos,bufSize;

	// Wait for notification
	while (parent->playing) {
		// Get variables
		parent->DSMutex.Lock();
		playPos = parent->GetCurrentPosition();
		endPos = parent->endPos;
		bufSize = parent->bufSize;
		parent->DSMutex.Unlock();

		// Still playing?
		if (playPos < endPos + bufSize/8) {
			// Wait for signal
			WaitForSingleObject(parent->notificationEvent,1000);

			// Fill buffer
			//parent->DSMutex.Lock();
			parent->FillBuffer(false);
			//parent->DSMutex.Unlock();
		}

		// Over, stop it
		else {
			parent->buffer->Stop();
			break;
		}
	}

	Delete();
	return 0;
}

#endif
