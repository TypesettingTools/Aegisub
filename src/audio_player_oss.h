// Copyright (c) 2009, Grigori Goronzy
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

/// @file audio_player_oss.h
/// @see audio_player_oss.cpp
/// @ingroup audio_output
///

#ifdef WITH_OSS
#include <sys/ioctl.h>

#include <wx/thread.h>

#include <fcntl.h>
#ifdef HAVE_SOUNDCARD_H
#   include <soundcard.h>
#else
#   ifdef HAVE_SYS_SOUNDCARD_H
#       include <sys/soundcard.h>
#   endif
#endif

#include "include/aegisub/audio_player.h"

class AudioProvider;
class OSSPlayer;

/// Worker thread to asynchronously write audio data to the output device
class OSSPlayerThread final : public wxThread {
    /// Parent player
    OSSPlayer *parent;

public:
    /// Constructor
    /// @param parent Player to get audio data and playback state from
    OSSPlayerThread(OSSPlayer *parent);

    /// Main thread entry point
    wxThread::ExitCode Entry();
};

class OSSPlayer final : public AudioPlayer {
    friend class OSSPlayerThread;

    /// sample rate of audio
    unsigned int rate = 0;

    /// Worker thread that does the actual writing
    OSSPlayerThread *thread = nullptr;

    /// Is the player currently playing?
    volatile bool playing = false;

    /// Current volume level
    volatile float volume = 1.f;

    /// first frame of playback
    volatile unsigned long start_frame = 0;

    /// last written frame + 1
    volatile unsigned long cur_frame = 0;

    /// last frame to play
    volatile unsigned long end_frame = 0;

    /// bytes per frame
    unsigned long bpf = 0;

    /// OSS audio device handle
    volatile int dspdev = 0;

    void OpenStream();

public:
    OSSPlayer(AudioProvider *provider);
    ~OSSPlayer();

    void Play(int64_t start, int64_t count);
    void Stop();
    bool IsPlaying() { return playing; }

    int64_t GetEndPosition() { return end_frame; }
    void SetEndPosition(int64_t pos);

    int64_t GetCurrentPosition();

    void SetVolume(double vol) { volume = vol; }
};
#endif
