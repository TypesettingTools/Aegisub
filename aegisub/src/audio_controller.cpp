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

/// @file audio_controller.cpp
/// @brief Manage open audio and abstract state away from display
/// @ingroup audio_ui
///

#include "config.h"

#include "audio_controller.h"

#include "ass_file.h"
#include "audio_provider_dummy.h"
#include "audio_timing.h"
#include "compat.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "include/aegisub/context.h"
#include "pen.h"
#include "options.h"
#include "selection_controller.h"
#include "subs_controller.h"
#include "utils.h"
#include "video_context.h"

#include <libaegisub/io.h>
#include <libaegisub/path.h>

#include <algorithm>

AudioController::AudioController(agi::Context *context)
: context(context)
, subtitle_save_slot(context->subsController->AddFileSaveListener(&AudioController::OnSubtitlesSave, this))
, player(0)
, provider(0)
, playback_mode(PM_NotPlaying)
, playback_timer(this)
{
	Bind(wxEVT_TIMER, &AudioController::OnPlaybackTimer, this, playback_timer.GetId());

#ifdef wxHAS_POWER_EVENTS
	Bind(wxEVT_POWER_SUSPENDED, &AudioController::OnComputerSuspending, this);
	Bind(wxEVT_POWER_RESUME, &AudioController::OnComputerResuming, this);
#endif

	OPT_SUB("Audio/Player", &AudioController::OnAudioPlayerChanged, this);
	OPT_SUB("Audio/Provider", &AudioController::OnAudioProviderChanged, this);
	OPT_SUB("Audio/Cache/Type", &AudioController::OnAudioProviderChanged, this);

#ifdef WITH_FFMS2
	// As with the video ones, it'd be nice to figure out a decent way to move
	// this to the provider itself
	OPT_SUB("Provider/Audio/FFmpegSource/Decode Error Handling", &AudioController::OnAudioProviderChanged, this);
#endif
}

AudioController::~AudioController()
{
	CloseAudio();
}

void AudioController::OnPlaybackTimer(wxTimerEvent &)
{
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
		AnnouncePlaybackPosition(MillisecondsFromSamples(pos));
	}
}

#ifdef wxHAS_POWER_EVENTS
void AudioController::OnComputerSuspending(wxPowerEvent &)
{
	Stop();
	delete player;
	player = 0;
}

void AudioController::OnComputerResuming(wxPowerEvent &)
{
	if (provider)
	{
		try
		{
			player = AudioPlayerFactory::GetAudioPlayer(provider);
		}
		catch (...)
		{
			CloseAudio();
		}
	}
}
#endif

void AudioController::OnAudioPlayerChanged()
{
	if (!IsAudioOpen()) return;

	Stop();

	delete player;

	try
	{
		player = AudioPlayerFactory::GetAudioPlayer(provider);
	}
	catch (...)
	{
		CloseAudio();
		throw;
	}
}

void AudioController::OnAudioProviderChanged()
{
	if (IsAudioOpen())
		// url is cloned because CloseAudio clears it and OpenAudio takes a const reference
		OpenAudio(agi::fs::path(audio_url));
}


void AudioController::OpenAudio(agi::fs::path const& url)
{
	if (url.empty())
		throw agi::InternalError("AudioController::OpenAudio() was passed an empty string. This must not happen.", 0);

	AudioProvider *new_provider = 0;
	try {
		new_provider = AudioProviderFactory::GetProvider(url);
		config::path->SetToken("?audio", url);
	}
	catch (agi::UserCancelException const&) {
		throw;
	}
	catch (...) {
		config::mru->Remove("Audio", url);
		throw;
	}

	CloseAudio();
	provider = new_provider;

	try
	{
		player = AudioPlayerFactory::GetAudioPlayer(provider);
	}
	catch (...)
	{
		delete provider;
		provider = 0;
		throw;
	}

	audio_url = url;

	config::mru->Add("Audio", url);

	try
	{
		AnnounceAudioOpen(provider);
	}
	catch (...)
	{
		CloseAudio();
		throw;
	}
}

void AudioController::CloseAudio()
{
	Stop();

	delete player;
	delete provider;
	player = 0;
	provider = 0;

	audio_url.clear();

	config::path->SetToken("?audio", "");

	AnnounceAudioClose();
}


bool AudioController::IsAudioOpen() const
{
	return player && provider;
}

void AudioController::SetTimingController(AudioTimingController *new_controller)
{
	if (timing_controller.get() != new_controller) {
		timing_controller.reset(new_controller);
		if (timing_controller)
		{
			timing_controller->AddUpdatedPrimaryRangeListener(&AudioController::OnTimingControllerUpdatedPrimaryRange, this);
		}
	}

	AnnounceTimingControllerChanged();
}

void AudioController::OnTimingControllerUpdatedPrimaryRange()
{
	if (playback_mode == PM_PrimaryRange)
		player->SetEndPosition(SamplesFromMilliseconds(timing_controller->GetPrimaryPlaybackRange().end()));
}

void AudioController::OnSubtitlesSave()
{
	if (IsAudioOpen())
		context->ass->SetScriptInfo("Audio URI", config::path->MakeRelative(audio_url, "?script").generic_string());
	else
		context->ass->SetScriptInfo("Audio URI", "");
}

void AudioController::PlayRange(const TimeRange &range)
{
	if (!IsAudioOpen()) return;

	player->Play(SamplesFromMilliseconds(range.begin()), SamplesFromMilliseconds(range.length()));
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
	if (!IsAudioOpen()) return;

	PlayRange(TimeRange(start_ms, GetPrimaryPlaybackRange().end()));
	if (playback_mode == PM_Range)
		playback_mode = PM_PrimaryRange;
}

void AudioController::PlayToEnd(int start_ms)
{
	if (!IsAudioOpen()) return;

	int64_t start_sample = SamplesFromMilliseconds(start_ms);
	player->Play(start_sample, provider->GetNumSamples()-start_sample);
	playback_mode = PM_ToEnd;
	playback_timer.Start(20);

	AnnouncePlaybackPosition(start_ms);
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


int AudioController::GetPlaybackPosition()
{
	if (!IsPlaying()) return 0;

	return MillisecondsFromSamples(player->GetCurrentPosition());
}

int AudioController::GetDuration() const
{
	if (!provider) return 0;

	return (provider->GetNumSamples() * 1000 + provider->GetSampleRate() - 1) / provider->GetSampleRate();
}


void AudioController::ResyncPlaybackPosition(int new_position)
{
	if (!IsPlaying()) return;

	player->SetCurrentPosition(SamplesFromMilliseconds(new_position));
}


TimeRange AudioController::GetPrimaryPlaybackRange() const
{
	if (timing_controller)
	{
		return timing_controller->GetPrimaryPlaybackRange();
	}
	else
	{
		return TimeRange(0, 0);
	}
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

void AudioController::SaveClip(agi::fs::path const& filename, TimeRange const& range) const
{
	int64_t start_sample = SamplesFromMilliseconds(range.begin());
	int64_t end_sample = SamplesFromMilliseconds(range.end());
	if (filename.empty() || start_sample > provider->GetNumSamples() || range.length() == 0) return;

	agi::io::Save outfile(filename, true);
	std::ofstream& out(outfile.Get());

	size_t bytes_per_sample = provider->GetBytesPerSample() * provider->GetChannels();
	size_t bufsize = (end_sample - start_sample) * bytes_per_sample;

	int intval;
	short shortval;

	out << "RIFF";
	out.write((char*)&(intval=bufsize+36),4);
	out<< "WAVEfmt ";
	out.write((char*)&(intval=16),4);
	out.write((char*)&(shortval=1),2);
	out.write((char*)&(shortval=provider->GetChannels()),2);
	out.write((char*)&(intval=provider->GetSampleRate()),4);
	out.write((char*)&(intval=provider->GetSampleRate()*provider->GetChannels()*provider->GetBytesPerSample()),4);
	out.write((char*)&(intval=provider->GetChannels()*provider->GetBytesPerSample()),2);
	out.write((char*)&(shortval=provider->GetBytesPerSample()<<3),2);
	out << "data";
	out.write((char*)&bufsize,4);

	//samples per read
	size_t spr = 65536 / bytes_per_sample;
	std::vector<char> buf(bufsize);
	for(int64_t i = start_sample; i < end_sample; i += spr) {
		size_t len = std::min<size_t>(spr, end_sample - i);
		provider->GetAudio(&buf[0], i, len);
		out.write(&buf[0], len * bytes_per_sample);
	}
}
