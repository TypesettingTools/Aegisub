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

#include "audio_controller.h"

#include "audio_timing.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "project.h"
#include "video_controller.h"

#include <libaegisub/audio/playback.h>
#include <libaegisub/audio/provider.h>

#include <algorithm>

AudioController::AudioController(agi::Context *context)
: context(context)
, playback_timer(this)
, provider_connection(context->project->AddAudioProviderListener(&AudioController::OnAudioProvider, this))
, playback_rate(agi::audio::ClampPlaybackRate(OPT_GET("Audio/Playback Rate")->GetDouble()))
{
	Bind(wxEVT_TIMER, &AudioController::OnPlaybackTimer, this, playback_timer.GetId());

#ifdef wxHAS_POWER_EVENTS
	Bind(wxEVT_POWER_SUSPENDED, &AudioController::OnComputerSuspending, this);
	Bind(wxEVT_POWER_RESUME, &AudioController::OnComputerResuming, this);
#endif

	BindConnection(OPT_SUB("Audio/Player", &AudioController::OnAudioPlayerChanged, this));
	BindConnection(OPT_SUB("Audio/Playback Rate", &AudioController::OnPlaybackRateChanged, this));
}

AudioController::~AudioController()
{
	Stop();
}

agi::AudioProvider *AudioController::GetPlayerProvider() const {
	return playback_provider ? playback_provider.get() : provider;
}

void AudioController::RecreatePlaybackProvider() {
	playback_provider.reset();
	if (!provider || playback_rate == 1.0) return;
	playback_provider = agi::CreatePlaybackAudioProvider(provider, playback_rate);
}

int64_t AudioController::PlaybackSamplesFromSourceSamples(int64_t samples, bool end) const {
	if (!playback_provider) return samples;
	return end
		? agi::audio::PlaybackSamplesFromSourceSamplesCeil(samples, playback_rate)
		: agi::audio::PlaybackSamplesFromSourceSamplesFloor(samples, playback_rate);
}

int64_t AudioController::SourceSamplesFromPlaybackSamples(int64_t samples) const {
	if (!playback_provider) return samples;
	return agi::audio::SourceSamplesFromPlaybackSamplesFloor(samples, playback_rate);
}

void AudioController::OnPlaybackTimer(wxTimerEvent &)
{
	if (!player) return;

	int64_t pos = player->GetCurrentPosition();
	if (!player->IsPlaying() ||
		(playback_mode != PM_ToEnd && pos >= player->GetEndPosition()+200))
	{
		// The +200 is to allow the player to end the sound output cleanly,
		// otherwise a popping artifact can sometimes be heard.
		Stop();
	}
	else
	{
		AnnouncePlaybackPosition(MillisecondsFromSamples(SourceSamplesFromPlaybackSamples(pos)));
	}
}

#ifdef wxHAS_POWER_EVENTS
void AudioController::OnComputerSuspending(wxPowerEvent &)
{
	Stop();
	player.reset();
}

void AudioController::OnComputerResuming(wxPowerEvent &)
{
	OnAudioPlayerChanged();
}
#endif

void AudioController::OnAudioPlayerChanged()
{
	if (!provider) return;

	Stop();
	player.reset();

	try
	{
		player = AudioPlayerFactory::GetAudioPlayer(GetPlayerProvider(), context->parent);
	}
	catch (...)
	{
		/// @todo This really shouldn't be just swallowing all audio player open errors
		context->project->CloseAudio();
	}
	AnnounceAudioPlayerOpened();
}

void AudioController::OnAudioProvider(agi::AudioProvider *new_provider)
{
	provider = new_provider;
	Stop();
	RecreatePlaybackProvider();
	player.reset();
	OnAudioPlayerChanged();
}

void AudioController::ApplyPlaybackRate(double rate)
{
	double clamped = agi::audio::ClampPlaybackRate(rate);
	if (playback_rate == clamped) return;

	double old_rate = playback_rate;
	bool was_playing = IsPlaying();
	bool video_was_playing = context->videoController && context->videoController->IsPlaying();
	PlaybackMode old_mode = playback_mode;
	int current_ms = was_playing ? GetPlaybackPosition() : 0;
	int end_ms = 0;
	if (was_playing && player) {
		if (old_mode == PM_ToEnd)
			end_ms = GetDuration();
		else if (old_mode == PM_PrimaryRange)
			end_ms = GetPrimaryPlaybackRange().end();
		else
			end_ms = MillisecondsFromSamples(SourceSamplesFromPlaybackSamples(player->GetEndPosition()));
	}

	Stop();
	playback_rate = clamped;
	RecreatePlaybackProvider();
	player.reset();
	OnAudioPlayerChanged();

	if (was_playing && !video_was_playing) {
		switch (old_mode) {
			case PM_PrimaryRange:
				if (current_ms < end_ms) {
					PlayRange(TimeRange(current_ms, end_ms));
					playback_mode = PM_PrimaryRange;
				}
				break;
			case PM_Range:
				if (current_ms < end_ms)
					PlayRange(TimeRange(current_ms, end_ms));
				break;
			case PM_ToEnd:
				if (current_ms < GetDuration())
					PlayToEnd(current_ms);
				break;
			case PM_NotPlaying:
				break;
		}
	}

	AnnouncePlaybackRateChanged(old_rate, playback_rate, current_ms);
}

void AudioController::OnPlaybackRateChanged(agi::OptionValue const& option)
{
	double clamped = agi::audio::ClampPlaybackRate(option.GetDouble());
	if (clamped != option.GetDouble()) {
		OPT_SET("Audio/Playback Rate")->SetDouble(clamped);
		return;
	}
	ApplyPlaybackRate(clamped);
}

void AudioController::SetTimingController(std::unique_ptr<AudioTimingController> new_controller)
{
	timing_controller = std::move(new_controller);
	if (timing_controller)
		timing_controller_connection = timing_controller->AddUpdatedPrimaryRangeListener(&AudioController::OnTimingControllerUpdatedPrimaryRange, this);
	else
		timing_controller_connection = agi::signal::Connection();

	AnnounceTimingControllerChanged();
}

void AudioController::OnTimingControllerUpdatedPrimaryRange()
{
	if (playback_mode == PM_PrimaryRange)
		player->SetEndPosition(PlaybackSamplesFromSourceSamples(SamplesFromMilliseconds(timing_controller->GetPrimaryPlaybackRange().end()), true));
}

void AudioController::PlayRange(const TimeRange &range)
{
	if (!player) return;

	int64_t start = PlaybackSamplesFromSourceSamples(SamplesFromMilliseconds(range.begin()), false);
	int64_t end = PlaybackSamplesFromSourceSamples(SamplesFromMilliseconds(range.end()), true);
	player->Play(start, end - start);
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

void AudioController::PlayToEndOfPrimary(int start_ms)
{
	PlayRange(TimeRange(start_ms, GetPrimaryPlaybackRange().end()));
	if (playback_mode == PM_Range)
		playback_mode = PM_PrimaryRange;
}

void AudioController::PlayToEnd(int start_ms)
{
	if (!player) return;

	int64_t start_sample = PlaybackSamplesFromSourceSamples(SamplesFromMilliseconds(start_ms), false);
	player->Play(start_sample, GetPlayerProvider()->GetNumSamples() - start_sample);
	playback_mode = PM_ToEnd;
	playback_timer.Start(20);

	AnnouncePlaybackPosition(start_ms);
}

void AudioController::Stop()
{
	if (!player) return;

	player->Stop();
	playback_mode = PM_NotPlaying;
	playback_timer.Stop();

	AnnouncePlaybackStop();
}

bool AudioController::IsPlaying()
{
	return player && playback_mode != PM_NotPlaying;
}

int AudioController::GetPlaybackPosition()
{
	if (!IsPlaying()) return 0;

	return MillisecondsFromSamples(SourceSamplesFromPlaybackSamples(player->GetCurrentPosition()));
}

int AudioController::GetDuration() const
{
	if (!provider) return 0;
	return (provider->GetNumSamples() * 1000 + provider->GetSampleRate() - 1) / provider->GetSampleRate();
}

TimeRange AudioController::GetPrimaryPlaybackRange() const
{
	if (timing_controller)
		return timing_controller->GetPrimaryPlaybackRange();
	else
		return TimeRange{0, 0};
}

void AudioController::SetVolume(double volume)
{
	if (!player) return;
	player->SetVolume(volume);
}

void AudioController::SetPlaybackRate(double rate)
{
	ApplyPlaybackRate(rate);
}

int64_t AudioController::SamplesFromMilliseconds(int64_t ms) const
{
	if (!provider) return 0;
	return agi::audio::SourceSamplesFromMilliseconds(ms, provider->GetSampleRate());
}

int64_t AudioController::MillisecondsFromSamples(int64_t samples) const
{
	if (!provider) return 0;
	return agi::audio::MillisecondsFromSourceSamples(samples, provider->GetSampleRate());
}
