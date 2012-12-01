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

/// @file audio_player_dsound.h
/// @see audio_player_dsound.cpp
/// @ingroup audio_output
///

#ifdef WITH_DIRECTSOUND

#include <mmsystem.h>

#include <dsound.h>

#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"

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
	DirectSoundPlayer(AudioProvider *provider);
	~DirectSoundPlayer();

	void Play(int64_t start,int64_t count);
	void Stop();

	bool IsPlaying() { return playing; }

	int64_t GetStartPosition() { return startPos; }
	int64_t GetEndPosition() { return endPos; }
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);
	void SetCurrentPosition(int64_t pos);

	void SetVolume(double vol) { volume = vol; }
	double GetVolume() { return volume; }
};
#endif
