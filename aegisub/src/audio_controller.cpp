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

#include <libaegisub/io.h>

#include "ass_file.h"
#include "audio_controller.h"
#include "audio_provider_dummy.h"
#include "audio_timing.h"
#include "compat.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "include/aegisub/context.h"
#include "pen.h"
#include "main.h"
#include "selection_controller.h"
#include "standard_paths.h"
#include "video_context.h"

class VideoPositionMarker : public AudioMarker {
	Pen style;
	int64_t position;

public:
	VideoPositionMarker()
	: style("Colour/Audio Display/Play Cursor")
	, position(-1)
	{
	}

	void SetPosition(int64_t new_pos)
	{
		position = new_pos;
	}

	int64_t GetPosition() const { return position; }
	FeetStyle GetFeet() const { return Feet_None; }
	bool CanSnap() const { return true; }
	wxPen GetStyle() const { return style; }
	operator int64_t() const { return position; }
};

class VideoPositionMarkerProvider : public AudioMarkerProvider {
	AudioController *ac;
	VideoContext *vc;

	VideoPositionMarker marker;

	agi::signal::Connection video_seek_slot;
	agi::signal::Connection enable_opt_changed_slot;

	void Update(int frame_number)
	{
		if (frame_number == -1)
		{
			marker.SetPosition(-1);
		}
		else
		{
			marker.SetPosition(ac->SamplesFromMilliseconds(vc->TimeAtFrame(frame_number)));
		}
		AnnounceMarkerMoved();
	}

	void OptChanged(agi::OptionValue const& opt)
	{
		if (opt.GetBool())
		{
			video_seek_slot.Unblock();
			Update(vc->GetFrameN());
		}
		else
		{
			video_seek_slot.Block();
			Update(-1);
		}
	}

public:
	VideoPositionMarkerProvider(agi::Context *c)
	: ac(c->audioController)
	, vc(c->videoController)
	, video_seek_slot(vc->AddSeekListener(&VideoPositionMarkerProvider::Update, this))
	, enable_opt_changed_slot(OPT_SUB("Audio/Display/Draw/Video Position", &VideoPositionMarkerProvider::OptChanged, this))
	{
		OptChanged(*OPT_GET("Audio/Display/Draw/Video Position"));
	}

	void GetMarkers(const SampleRange &range, AudioMarkerVector &out) const
	{
		if (range.contains(marker))
		{
			out.push_back(&marker);
		}
	}
};

AudioController::AudioController(agi::Context *context)
: context(context)
, subtitle_save_slot(context->ass->AddFileSaveListener(&AudioController::OnSubtitlesSave, this))
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
		// The +200 is to allow the player to end the sound output cleanly,
		// otherwise a popping artifact can sometimes be heard.
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

void AudioController::OnAudioPlayerChanged()
{
	if (!IsAudioOpen()) return;

	Stop();

	delete player;

	try
	{
		player = AudioPlayerFactory::GetAudioPlayer();
		player->SetProvider(provider);
		player->OpenStream();
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
		OpenAudio(audio_url.Clone());
}


void AudioController::OpenAudio(const wxString &url)
{
	CloseAudio();

	if (!url)
		throw agi::InternalError("AudioController::OpenAudio() was passed an empty string. This must not happen.", 0);

	wxString path_part;

	if (url.StartsWith("dummy-audio:", &path_part))
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
		 provider = new DummyAudioProvider(5*30*60*1000, path_part.StartsWith("noise"));
	}
	else if (url.StartsWith("video-audio:", &path_part))
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
	else if (url.StartsWith("file:", &path_part))
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
			config::mru->Remove("Audio", STD_STR(url));
			agi::FileNotFoundError fnf(STD_STR(url));
			throw agi::AudioOpenError(
				"Failed opening audio file (parsing as plain filename)",
				&fnf);
		}
		provider = AudioProviderFactory::GetProvider(url);

		StandardPaths::SetPathValue("?audio", fn.GetPath());
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

	config::mru->Add("Audio", STD_STR(url));

	if (!video_position_marker_provider.get())
	{
		video_position_marker_provider.reset(new VideoPositionMarkerProvider(context));
		video_position_marker_provider->AddMarkerMovedListener(std::tr1::bind(std::tr1::ref(AnnounceMarkerMoved)));
	}

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

	StandardPaths::SetPathValue("?audio", "");

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
		if (timing_controller)
		{
			timing_controller->AddMarkerMovedListener(bind(std::tr1::ref(AnnounceMarkerMoved)));
			timing_controller->AddLabelChangedListener(bind(std::tr1::ref(AnnounceLabelChanged)));
			timing_controller->AddUpdatedPrimaryRangeListener(&AudioController::OnTimingControllerUpdatedPrimaryRange, this);
			timing_controller->AddUpdatedStyleRangesListener(bind(std::tr1::ref(AnnounceStyleRangesChanged)));
		}
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

void AudioController::OnSubtitlesSave()
{
	if (IsAudioOpen())
	{
		context->ass->SetScriptInfo("Audio URI", audio_url);
	}
	else
	{
		context->ass->SetScriptInfo("Audio URI", "");
	}
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
	if (timing_controller)
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
	if (keyframes_marker_provider.get()) keyframes_marker_provider->GetMarkers(range, markers);
	if (video_position_marker_provider.get()) video_position_marker_provider->GetMarkers(range, markers);
	if (timing_controller) timing_controller->GetMarkers(range, markers);
}

void AudioController::GetLabels(const SampleRange &range, std::vector<AudioLabel> &labels) const
{
	if (timing_controller) timing_controller->GetLabels(range, labels);
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

void AudioController::SaveClip(wxString const& filename, SampleRange const& range) const
{
	if (filename.empty() || range.begin() > provider->GetNumSamples() || range.length() == 0) return;

	agi::io::Save outfile(STD_STR(filename), true);
	std::ofstream& out(outfile.Get());

	size_t bytes_per_sample = provider->GetBytesPerSample() * provider->GetChannels();
	size_t bufsize = range.length() * bytes_per_sample;

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
	for(int64_t i = range.begin(); i < range.end(); i += spr) {
		size_t len = std::min<size_t>(spr, range.end() - i);
		provider->GetAudio(&buf[0], i, len);
		out.write(&buf[0], len * bytes_per_sample);
	}
}
