// Copyright (c) 2009, Grigori Goronzy
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

/// @file audio_player_oss.cpp
/// @brief Open Sound System audio output
/// @ingroup audio_output
///

#include "config.h"

#ifdef WITH_OSS
#include "include/aegisub/audio_player.h"

#include "audio_controller.h"
#include "compat.h"
#include "include/aegisub/audio_provider.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/log.h>
#include <libaegisub/util.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <wx/thread.h>

#ifdef HAVE_SOUNDCARD_H
#   include <soundcard.h>
#elif defined(HAVE_SYS_SOUNDCARD_H)
#   include <sys/soundcard.h>
#endif

namespace {
DEFINE_SIMPLE_EXCEPTION(OSSError, agi::AudioPlayerOpenError, "audio/player/open/oss")
class OSSPlayerThread;

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
    OSSPlayer(AudioProvider *provider)
    : AudioPlayer(provider)
    {
        OpenStream();
    }

    ~OSSPlayer() {
        Stop();
        ::close(dspdev);
    }


    void Play(int64_t start, int64_t count);
    void Stop();
    bool IsPlaying() { return playing; }

    int64_t GetEndPosition() { return end_frame; }
    void SetEndPosition(int64_t pos);

    int64_t GetCurrentPosition();

    void SetVolume(double vol) { volume = vol; }
};

/// Worker thread to asynchronously write audio data to the output device
class OSSPlayerThread final : public wxThread {
    /// Parent player
    OSSPlayer *parent;

public:
    /// Constructor
    /// @param parent Player to get audio data and playback state from
    OSSPlayerThread(OSSPlayer *parent) : wxThread(wxTHREAD_JOINABLE) , parent(parent) { }

    /// Main thread entry point
    wxThread::ExitCode Entry() {
        // Use small enough writes for good timing accuracy with all
        // timing methods.
        const unsigned long wsize = parent->rate / 25;
        void *buf = malloc(wsize * parent->bpf);

        while (!TestDestroy() && parent->cur_frame < parent->end_frame) {
            int rsize = std::min(wsize, parent->end_frame - parent->cur_frame);
            parent->provider->GetAudioWithVolume(buf, parent->cur_frame,
                                                 rsize, parent->volume);
            int written = ::write(parent->dspdev, buf, rsize * parent->bpf);
            parent->cur_frame += written / parent->bpf;
        }
        free(buf);
        parent->cur_frame = parent->end_frame;

        LOG_D("player/audio/oss") << "Thread dead";
        return 0;
    }
};

void OSSPlayer::OpenStream()
{
    bpf = provider->GetChannels() * provider->GetBytesPerSample();

    // Open device
    wxString device = to_wx(OPT_GET("Player/Audio/OSS/Device")->GetString());
    dspdev = ::open(device.utf8_str(), O_WRONLY, 0);
    if (dspdev < 0) {
        throw OSSError("OSS player: opening device failed", 0);
    }

    // Use a reasonable buffer policy for low latency (OSS4)
#ifdef SNDCTL_DSP_POLICY
    int policy = 3;
    ioctl(dspdev, SNDCTL_DSP_POLICY, &policy);
#endif

    // Set number of channels
    int channels = provider->GetChannels();
    if (ioctl(dspdev, SNDCTL_DSP_CHANNELS, &channels) < 0) {
        throw OSSError("OSS player: setting channels failed", 0);
    }

    // Set sample format
    int sample_format;
    switch (provider->GetBytesPerSample()) {
        case 1:
            sample_format = AFMT_S8;
            break;
        case 2:
            sample_format = AFMT_S16_LE;
            break;
        default:
            throw OSSError("OSS player: can only handle 8 and 16 bit sound", 0);
    }

    if (ioctl(dspdev, SNDCTL_DSP_SETFMT, &sample_format) < 0) {
        throw OSSError("OSS player: setting sample format failed", 0);
    }

    // Set sample rate
    rate = provider->GetSampleRate();
    if (ioctl(dspdev, SNDCTL_DSP_SPEED, &rate) < 0) {
        throw OSSError("OSS player: setting samplerate failed", 0);
    }
}

void OSSPlayer::Play(int64_t start, int64_t count)
{
    Stop();

    start_frame = cur_frame = start;
    end_frame = start + count;

    thread = new OSSPlayerThread(this);
    thread->Create();
    thread->Run();

    playing = true;
}

void OSSPlayer::Stop()
{
    if (!playing) return;

    // Stop the thread
    if (thread) {
        if (thread->IsAlive()) {
            thread->Delete();
        }
        thread->Wait();
        delete thread;
    }

    // errors can be ignored here
    ioctl(dspdev, SNDCTL_DSP_RESET, nullptr);

    // Reset data
    playing = false;
    start_frame = 0;
    cur_frame = 0;
    end_frame = 0;
}

void OSSPlayer::SetEndPosition(int64_t pos)
{
    end_frame = pos;

    if (pos <= GetCurrentPosition()) {
        ioctl(dspdev, SNDCTL_DSP_RESET, nullptr);
        if (thread && thread->IsAlive())
            thread->Delete();
    }
}

int64_t OSSPlayer::GetCurrentPosition()
{
    if (!playing)
        return 0;

#ifdef SNDCTL_DSP_CURRENT_OPTR
    // OSS4
    long played_frames = 0;
    oss_count_t pos;
    if (ioctl(dspdev, SNDCTL_DSP_CURRENT_OPTR, &pos) >= 0) {
        // XXX: Apparently the semantics are different on FreeBSD...
#ifdef __FREEBSD__
        played_frames = MAX(0, pos.samples - pos.fifo_samples);
#else
        played_frames = pos.samples + pos.fifo_samples;
#endif
        LOG_D("player/audio/oss") << "played_frames: " << played_frames << " fifo " << pos.fifo_samples;
        return start_frame + played_frames;
    }
#endif

    // Fallback for old OSS versions
    int delay = 0;
    if (ioctl(dspdev, SNDCTL_DSP_GETODELAY, &delay) >= 0) {
        delay /= bpf;

        LOG_D("player/audio/oss") << "cur_frame: " << cur_frame << " delay " << delay;
        // delay can jitter a bit at the end, detect that
        if (cur_frame == end_frame && delay < rate / 20) {
            return cur_frame;
        }
        return MAX(0, (long) cur_frame - delay);
    }

    // Maybe this still didn't work...
    // Return the last written frame, timing will suffer
    return cur_frame;
}
}

std::unique_ptr<AudioPlayer> CreateOSSPlayer(AudioProvider *provider) {
    return agi::util::make_unique<OSSPlayer>(provider);
}

#endif // WITH_OSS
