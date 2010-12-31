// Copyright (c) 2009-2010, Niels Martin Hansen
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

/// @file audio_controller.cpp
/// @brief Manage open audio and abstract state away from display
/// @ingroup audio_ui
///


#include "config.h"

#ifndef AGI_PRE
#include <algorithm>

#include <wx/filename.h>
#endif

#include "selection_controller.h"
#include "audio_controller.h"
#include "include/aegisub/audio_provider.h"
#include "include/aegisub/audio_player.h"
#include "audio_provider_dummy.h"
#include "audio_timing.h"
#include "compat.h"
#include "video_context.h"

class AudioMarkerKeyframe : public AudioMarker {
	int64_t position;
	static wxPen style;
public:
	AudioMarkerKeyframe(int64_t position) : position(position) { }
	int64_t GetPosition() const { return position; }
	FeetStyle GetFeet() const { return Feet_None; }
	bool CanSnap() const { return true; }
	wxPen GetStyle() const
	{
		if (!style.IsOk())
			/// @todo Make this colour configurable
			style = wxPen(wxColour(255,0,255), 1);
		return style;
	}
	bool operator < (const AudioMarkerKeyframe &other) const { return position < other.position; }
	operator int64_t() const { return position; }
};
bool operator < (int64_t a, const AudioMarkerKeyframe &b) { return a < b.GetPosition(); }
bool operator < (const AudioMarkerKeyframe &a, int64_t b) { return a.GetPosition() < b; }
wxPen AudioMarkerKeyframe::style;

class AudioMarkerProviderKeyframes : public AudioMarkerProvider {
	VideoContext *vc;

	agi::signal::Connection keyframe_slot;
	agi::signal::Connection audio_open_slot;

	std::vector<AudioMarkerKeyframe> keyframe_samples;
	AudioController *controller;

	void OnKeyframesOpen(std::vector<int> const& raw_keyframes)
	{
		keyframe_samples.clear();
		keyframe_samples.reserve(raw_keyframes.size());
		for (size_t i = 0; i < raw_keyframes.size(); ++i)
		{
			keyframe_samples.push_back(AudioMarkerKeyframe(
				controller->SamplesFromMilliseconds(vc->TimeAtFrame(raw_keyframes[i]))));
		}
		std::sort(keyframe_samples.begin(), keyframe_samples.end());
		AnnounceMarkerMoved();
	}

private:
	// AudioControllerAudioEventListener implementation
	void OnAudioOpen(AudioProvider *)
	{
		OnKeyframesOpen(vc->GetKeyFrames());
	}

public:
	AudioMarkerProviderKeyframes(AudioController *controller)
		: vc(VideoContext::Get())
		, keyframe_slot(vc->AddKeyframesListener(&AudioMarkerProviderKeyframes::OnKeyframesOpen, this))
		, audio_open_slot(controller->AddAudioOpenListener(&AudioMarkerProviderKeyframes::OnAudioOpen, this))
		, controller(controller)
	{
		OnKeyframesOpen(vc->GetKeyFrames());
	}

	void GetMarkers(const SampleRange &range, AudioMarkerVector &out) const
	{
		// Find first and last keyframes inside the range
		std::vector<AudioMarkerKeyframe>::const_iterator a = std::lower_bound(
			keyframe_samples.begin(), keyframe_samples.end(), range.begin());
		std::vector<AudioMarkerKeyframe>::const_iterator b = std::upper_bound(
			keyframe_samples.begin(), keyframe_samples.end(), range.end());

		// Place pointers to the markers in the output vector
		for (; a != b; ++a)
			out.push_back(&*a);
	}
};

AudioController::AudioController()
: player(0)
, provider(0)
, keyframes_marker_provider(new AudioMarkerProviderKeyframes(this))
, playback_mode(PM_NotPlaying)
, playback_timer(this)
{
	Connect(playback_timer.GetId(), wxEVT_TIMER, (wxObjectEventFunction)&AudioController::OnPlaybackTimer);

#ifdef wxHAS_POWER_EVENTS
	Connect(wxEVT_POWER_SUSPENDED, (wxObjectEventFunction)&AudioController::OnComputerSuspending);
	Connect(wxEVT_POWER_RESUME, (wxObjectEventFunction)&AudioController::OnComputerResuming);
#endif
}


AudioController::~AudioController()
{
	CloseAudio();
}


void AudioController::OnPlaybackTimer(wxTimerEvent &event)
{
	int64_t pos = player->GetCurrentPosition();

	if (!player->IsPlaying() ||
		(playback_mode != PM_ToEnd && pos >= player->GetEndPosition()+200))
	{
		// The +200 is to allow the player to end the sound output cleanly, otherwise a popping
		// artifact can sometimes be heard.
		Stop();
	}
	else
	{
		AnnouncePlaybackPosition(pos);
	}
}


#ifdef wxHAS_POWER_EVENTS
void AudioController::OnComputerSuspending(wxPowerEvent &event)
{
	Stop();
	player->CloseStream();
}


void AudioController::OnComputerResuming(wxPowerEvent &event)
{
	if (provider)
		player->OpenStream();
}
#endif


void AudioController::OpenAudio(const wxString &url)
{
	CloseAudio();

	if (!url)
		throw agi::InternalError("AudioController::OpenAudio() was passed an empty string. This must not happen.", 0);

	wxString path_part;

	if (url.StartsWith(_T("dummy-audio:"), &path_part))
	{
		/*
		 * scheme            ::= "dummy-audio" ":" signal-specifier "?" signal-parameters
		 * signal-specifier  ::= "silence" | "noise" | "sine" "/" frequency
		 * frequency         ::= integer
		 * signal-parameters ::= signal-parameter [ "&" signal-parameters ]
		 * signal-parameter  ::= signal-parameter-name "=" integer
		 * signal-parameter-name ::= "sr" | "bd" | "ch" | "ln"
		 *
		 * Signal types:
		 * "silence", a silent signal is generated.
		 * "noise", a white noise signal is generated.
		 * "sine", a sine wave is generated at the specified frequency.
		 *
		 * Signal parameters:
		 * "sr", sample rate to generate signal at.
		 * "bd", bit depth to generate signal at (usually 16).
		 * "ch", number of channels to generate, usually 1 or 2. The same signal is generated
		 *       in every channel even if one would be LFE.
		 * "ln", length of signal in samples. ln/sr gives signal length in seconds.
		 */
		 provider = new DummyAudioProvider(5*30*60*1000, path_part.StartsWith(L"noise"));
	}
	else if (url.StartsWith(_T("video-audio:"), &path_part))
	{
		/*
		 * scheme      ::= "video-audio" ":" stream-type
		 * stream-type ::= "stream" | "cache"
		 *
		 * Stream types:
		 *
		 * "stream", the audio is streamed as required directly from the video provider,
		 * and cannot be used to drive an audio display. Seeking is unreliable.
		 *
		 * "cache", the entire audio is cached to memory or disk. Audio displays can be
		 * driven and seeking is reliable. Opening takes longer because the entire audio
		 * stream has to be decoded and stored.
		 */
	}
	else if (url.StartsWith(_T("file:"), &path_part))
	{
		/*
		 * scheme    ::= "file" ":" "//" file-system-path
		 *
		 * On Unix-like systems, the file system path is regular. On Windows-systems, the
		 * path uses forward slashes instead of back-slashes and the drive letter is
		 * preceded by a slash.
		 *
		 * URL-encoding??
		 */
	}
	else
	{
		/*
		 * Assume it's not a URI but instead a filename in the platform's native format.
		 */
		wxFileName fn(url);
		if (!fn.FileExists())
		{
			agi::FileNotFoundError fnf(STD_STR(url));
			throw agi::AudioOpenError(
				"Failed opening audio file (parsing as plain filename)",
				&fnf);
		}
		provider = AudioProviderFactory::GetProvider(url);
	}

	try
	{
		player = AudioPlayerFactory::GetAudioPlayer();
		player->SetProvider(provider);
		player->OpenStream();
	}
	catch (...)
	{
		delete player;
		delete provider;
		player = 0;
		provider = 0;
		throw;
	}

	audio_url = url;

	// Tell listeners about this.
	AnnounceAudioOpen(provider);
}


void AudioController::CloseAudio()
{
	Stop();

	delete player;
	delete provider;
	player = 0;
	provider = 0;

	audio_url.clear();

	AnnounceAudioClose();
}


bool AudioController::IsAudioOpen() const
{
	return player && provider;
}


wxString AudioController::GetAudioURL() const
{
	return audio_url;
}

void AudioController::SetTimingController(AudioTimingController *new_controller)
{
	if (timing_controller.get() != new_controller) {
		timing_controller.reset(new_controller);
		timing_controller->AddMarkerMovedListener(std::tr1::bind(std::tr1::ref(AnnounceMarkerMoved)));
		timing_controller->AddUpdatedPrimaryRangeListener(&AudioController::OnTimingControllerUpdatedPrimaryRange, this);
		timing_controller->AddUpdatedStyleRangesListener(&AudioController::OnTimingControllerUpdatedStyleRanges, this);
	}

	AnnounceTimingControllerChanged();
}



void AudioController::OnTimingControllerUpdatedPrimaryRange()
{
	if (playback_mode == PM_PrimaryRange)
	{
		player->SetEndPosition(timing_controller->GetPrimaryPlaybackRange().end());
	}

	AnnounceSelectionChanged();
}


void AudioController::OnTimingControllerUpdatedStyleRanges()
{
	/// @todo redraw and stuff, probably
}

void AudioController::PlayRange(const SampleRange &range)
{
	if (!IsAudioOpen()) return;

	player->Play(range.begin(), range.length());
	playback_mode = PM_Range;
	playback_timer.Start(20);

	AnnouncePlaybackPosition(range.begin());
}


void AudioController::PlayPrimaryRange()
{
	PlayRange(GetPrimaryPlaybackRange());
	if (playback_mode == PM_Range)
		playback_mode = PM_PrimaryRange;
}


void AudioController::PlayToEnd(int64_t start_sample)
{
	if (!IsAudioOpen()) return;

	player->Play(start_sample, provider->GetNumSamples()-start_sample);
	playback_mode = PM_ToEnd;
	playback_timer.Start(20);

	AnnouncePlaybackPosition(start_sample);
}


void AudioController::Stop()
{
	if (!IsAudioOpen()) return;

	player->Stop();
	playback_mode = PM_NotPlaying;
	playback_timer.Stop();

	AnnouncePlaybackStop();
}


bool AudioController::IsPlaying()
{
	return IsAudioOpen() && playback_mode != PM_NotPlaying;
}


int64_t AudioController::GetPlaybackPosition()
{
	if (!IsPlaying()) return 0;

	return player->GetCurrentPosition();
}


void AudioController::ResyncPlaybackPosition(int64_t new_position)
{
	if (!IsPlaying()) return;

	player->SetCurrentPosition(new_position);
}


SampleRange AudioController::GetPrimaryPlaybackRange() const
{
	if (timing_controller.get())
	{
		return timing_controller->GetPrimaryPlaybackRange();
	}
	else
	{
		return SampleRange(0, 0);
	}
}


void AudioController::GetMarkers(const SampleRange &range, AudioMarkerVector &markers) const
{
	/// @todo Find all sources of markers
	keyframes_marker_provider->GetMarkers(range, markers);
	if (timing_controller.get()) timing_controller->GetMarkers(range, markers);
}


double AudioController::GetVolume() const
{
	if (!IsAudioOpen()) return 1.0;
	return player->GetVolume();
}


void AudioController::SetVolume(double volume)
{
	if (!IsAudioOpen()) return;
	player->SetVolume(volume);
}


int64_t AudioController::SamplesFromMilliseconds(int64_t ms) const
{
	/// @todo There might be some subtle rounding errors here.

	if (!provider) return 0;

	int64_t sr = provider->GetSampleRate();

	int64_t millisamples = ms * sr;

	return (millisamples + 999) / 1000;
}


int64_t AudioController::MillisecondsFromSamples(int64_t samples) const
{
	/// @todo There might be some subtle rounding errors here.

	if (!provider) return 0;

	int64_t sr = provider->GetSampleRate();

	int64_t millisamples = samples * 1000;

	return millisamples / sr;
}
