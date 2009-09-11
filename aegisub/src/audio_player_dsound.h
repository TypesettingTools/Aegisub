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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_player_dsound.h
/// @see audio_player_dsound.cpp
/// @ingroup audio_output
///


///////////
// Headers
#ifdef WITH_DIRECTSOUND

#ifndef AGI_PRE
#include <mmsystem.h>
#endif

#include <dsound.h>

#include "frame_main.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "utils.h"


//////////////
// Prototypes
class DirectSoundPlayer;



/// DOCME
/// @class DirectSoundPlayerThread
/// @brief DOCME
///
/// DOCME
class DirectSoundPlayerThread : public wxThread {
private:

	/// DOCME
	DirectSoundPlayer *parent;

	/// DOCME
	HANDLE stopnotify;

public:
	void Stop(); // Notify thread to stop audio playback. Thread safe.
	DirectSoundPlayerThread(DirectSoundPlayer *parent);
	~DirectSoundPlayerThread();

	wxThread::ExitCode Entry();
};
/*
TODO: Rewrite playback thread to manage all of the buffer, and properly marshal the IDirectSound8
object into the thread for creating the buffer there.
The thread should own the buffer and manage all of the playback.
It must be created with start and duration set, and begins playback at the given position.
New functions:
* Seek(pos) : Restart playback from the given position
* SetEnd(pos) : Set new end point
* GetPosition() : Get the current sample number being played
* Stop() : Stop playback immediately

Instead of using a stop event, use a playback parameters changed event. When that one's fired,
detect which were actually changed and act accordingly.
All but GetPosition() set appropriate fields and then raise the parameters changed event.
*/



/// DOCME
/// @class DirectSoundPlayer
/// @brief DOCME
///
/// DOCME
class DirectSoundPlayer : public AudioPlayer {
	friend class DirectSoundPlayerThread;

private:

	/// DOCME
	volatile bool playing;

	/// DOCME
	float volume;

	/// DOCME
	int offset;

	/// DOCME
	DWORD bufSize;


	/// DOCME
	volatile int64_t playPos;

	/// DOCME
	int64_t startPos;

	/// DOCME
	volatile int64_t endPos;

	/// DOCME
	DWORD startTime;


	/// DOCME
	IDirectSound8 *directSound;

	/// DOCME
	IDirectSoundBuffer8 *buffer;

	bool FillBuffer(bool fill);


	/// DOCME
	DirectSoundPlayerThread *thread;

public:
	DirectSoundPlayer();
	~DirectSoundPlayer();

	void OpenStream();
	void CloseStream();

	void Play(int64_t start,int64_t count);
	void Stop(bool timerToo=true);

	/// @brief DOCME
	/// @return 
	///
	bool IsPlaying() { return playing; }


	/// @brief DOCME
	/// @return 
	///
	int64_t GetStartPosition() { return startPos; }

	/// @brief DOCME
	/// @return 
	///
	int64_t GetEndPosition() { return endPos; }
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);
	void SetCurrentPosition(int64_t pos);


	/// @brief DOCME
	/// @param vol 
	/// @return 
	///
	void SetVolume(double vol) { volume = vol; }

	/// @brief DOCME
	/// @return 
	///
	double GetVolume() { return volume; }

	//wxMutex *GetMutex() { return &DSMutex; }
};



/// DOCME
/// @class DirectSoundPlayerFactory
/// @brief DOCME
///
/// DOCME
class DirectSoundPlayerFactory : public AudioPlayerFactory {
public:

	/// @brief DOCME
	///
	AudioPlayer *CreatePlayer() { return new DirectSoundPlayer(); }
};

#endif


