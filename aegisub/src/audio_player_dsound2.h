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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_player_dsound2.h
/// @see audio_player_dsound2.cpp
/// @ingroup audio_output
///

#ifdef WITH_DIRECTSOUND

#include "include/aegisub/audio_player.h"

class DirectSoundPlayer2Thread;

/// @class DirectSoundPlayer2
/// @brief New implementation of DirectSound-based audio player
///
/// The core design idea is to have a playback thread that owns the DirectSound COM objects
/// and performs all playback operations, and use the player object as a proxy to
/// send commands to the playback thread.
class DirectSoundPlayer2 : public AudioPlayer {
	/// The playback thread
	DirectSoundPlayer2Thread *thread;

protected:

	/// Desired length in milliseconds to write ahead of the playback cursor
	int WantedLatency;

	/// Multiplier for WantedLatency to get total buffer length
	int BufferLength;

	bool IsThreadAlive();

public:
	DirectSoundPlayer2();
	~DirectSoundPlayer2();

	void OpenStream();
	void CloseStream();

	void SetProvider(AudioProvider *provider);

	void Play(int64_t start,int64_t count);
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
#endif
