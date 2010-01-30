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
//
// $Id$

/// @file audio_player_oss.cpp
/// @brief Open Sound System audio output
/// @ingroup audio_output
///


#include "config.h"

#ifdef WITH_OSS

///////////
// Headers
#include "audio_player_manager.h"
#include "audio_player_oss.h"
#include "audio_provider_manager.h"
#include "frame_main.h"
#include "options.h"
#include "utils.h"


/// @brief Constructor 
///
OSSPlayer::OSSPlayer()
{
    volume = 1.0f;
    open = false;
    playing = false;
    start_frame = cur_frame = end_frame = bpf = 0;
    provider = 0;
    thread = 0;
}



/// @brief Destructor 
///
OSSPlayer::~OSSPlayer()
{
    CloseStream();
}



/// @brief Open stream 
///
void OSSPlayer::OpenStream()
{
    CloseStream();

    // Get provider
    provider = GetProvider();
    bpf = provider->GetChannels() * provider->GetBytesPerSample();

    // Open device
    wxString device = Options.AsText(_T("Audio OSS Device"));
    dspdev = ::open(device.mb_str(wxConvUTF8), O_WRONLY, 0);
    if (dspdev < 0) {
        throw _T("OSS player: opening device failed");
    }

    // Use a reasonable buffer policy for low latency (OSS4)
#ifdef SNDCTL_DSP_POLICY
    int policy = 3;
    ioctl(dspdev, SNDCTL_DSP_POLICY, &policy);
#endif

    // Set number of channels
    int channels = provider->GetChannels();
    if (ioctl(dspdev, SNDCTL_DSP_CHANNELS, &channels) < 0) {
        throw _T("OSS player: setting channels failed");
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
            throw _T("OSS player: can only handle 8 and 16 bit sound");
    }

    if (ioctl(dspdev, SNDCTL_DSP_SETFMT, &sample_format) < 0) {
        throw _T("OSS player: setting sample format failed");
    }

    // Set sample rate
    rate = provider->GetSampleRate();
    if (ioctl(dspdev, SNDCTL_DSP_SPEED, &rate) < 0) {
        throw("OSS player: setting samplerate failed");
    }

    // Now ready
    open = true;
}



/// @brief Close stream 
/// @return 
///
void OSSPlayer::CloseStream()
{
    if (!open) return;

    Stop();
    ::close(dspdev);

    // No longer working
    open = false;
}



/// @brief Play 
/// @param start 
/// @param count 
///
void OSSPlayer::Play(int64_t start, int64_t count)
{
    Stop();

    start_frame = cur_frame = start;
    end_frame = start + count;

    thread = new OSSPlayerThread(this);
    thread->Create();
    thread->Run();

    // Update timer
    if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(15);
    playing = true;
}



/// @brief Stop 
/// @param timerToo 
/// @return 
///
void OSSPlayer::Stop(bool timerToo)
{
    if (!open) return;
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
    ioctl(dspdev, SNDCTL_DSP_RESET, NULL);

    // Reset data
    playing = false;
    start_frame = 0;
    cur_frame = 0;
    end_frame = 0;

    // Stop timer
    if (timerToo && displayTimer) {
        displayTimer->Stop();
    }
}



/// @brief DOCME 
/// @return 
///
bool OSSPlayer::IsPlaying()
{
    return playing;
}



/// @brief Set end 
/// @param pos 
///
void OSSPlayer::SetEndPosition(int64_t pos)
{
    end_frame = pos;

    if (end_frame <= GetCurrentPosition()) {
        ioctl(dspdev, SNDCTL_DSP_RESET, NULL);
        if (thread && thread->IsAlive())
            thread->Delete();
    }

}



/// @brief Set current position 
/// @param pos 
///
void OSSPlayer::SetCurrentPosition(int64_t pos)
{
    cur_frame = start_frame = pos;
}



/// @brief DOCME 
/// @return 
///
int64_t OSSPlayer::GetStartPosition()
{
    return start_frame;
}



/// @brief DOCME 
/// @return 
///
int64_t OSSPlayer::GetEndPosition()
{
    return end_frame;
}



/// @brief Get current position 
/// @return 
///
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
        wxLogDebug("OSS player: played_frames %d fifo %d", played_frames,
                   pos.fifo_samples);
        if (start_frame + played_frames >= end_frame) {
            if (displayTimer)
                displayTimer->Stop();
        }
        return start_frame + played_frames;
    }
#endif

    // Fallback for old OSS versions
    int delay = 0;
    if (ioctl(dspdev, SNDCTL_DSP_GETODELAY, &delay) >= 0) {
        delay /= bpf;
        wxLogDebug("OSS player: cur_frame %d delay %d", cur_frame, delay);
        // delay can jitter a bit at the end, detect that
        if (cur_frame == end_frame && delay < rate / 20) {
            if (displayTimer)
                displayTimer->Stop();
            return cur_frame;
        }
        return MAX(0, (long) cur_frame - delay);
    }

    // Maybe this still didn't work...
    // Return the last written frame, timing will suffer
    return cur_frame;
}



/// @brief Thread constructor 
/// @param par 
///
OSSPlayerThread::OSSPlayerThread(OSSPlayer *par) : wxThread(wxTHREAD_JOINABLE)
{
    parent = par;
}

/// @brief Thread entry point
/// @return
///
wxThread::ExitCode OSSPlayerThread::Entry() {
    // Use small enough writes for good timing accuracy with all
    // timing methods.
    const int wsize = parent->rate / 25;
    void *buf = malloc(wsize * parent->bpf);

    while (!TestDestroy() && parent->cur_frame < parent->end_frame) {
        int rsize = MIN(wsize, parent->end_frame - parent->cur_frame);
        parent->provider->GetAudioWithVolume(buf, parent->cur_frame,
                                             rsize, parent->volume);
        int written = ::write(parent->dspdev, buf, rsize * parent->bpf);
        parent->cur_frame += written / parent->bpf;
    }
    free(buf);
    parent->cur_frame = parent->end_frame;

    wxLogDebug(_T("OSS player thread dead"));
    return 0;
}



#endif // WITH_OSS
