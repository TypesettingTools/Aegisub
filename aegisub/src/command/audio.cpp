// Copyright (c) 2005-2010, Niels Martin Hansen
// Copyright (c) 2005-2010, Rodrigo Braz Monteiro
// Copyright (c) 2010, Amar Takhar
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

/// @file audio.cpp
/// @brief audio/ commands.
/// @ingroup command
///

#include "../config.h"

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

#include "command.h"

#include "../ass_dialogue.h"
#include "../audio_box.h"
#include "../audio_controller.h"
#include "../audio_karaoke.h"
#include "../audio_timing.h"
#include "../compat.h"
#include "../include/aegisub/context.h"
#include "../main.h"
#include "../selection_controller.h"
#include "../video_context.h"

namespace {
	using cmd::Command;

	struct validate_audio_open : public Command {
		CMD_TYPE(COMMAND_VALIDATE)
		bool Validate(const agi::Context *c) {
			return c->audioController->IsAudioOpen();
		}
	};

/// @defgroup cmd-audio Audio commands.
/// @{

/// Closes the currently open audio file.
struct audio_close : public validate_audio_open {
	CMD_NAME("audio/close")
	STR_MENU("&Close Audio")
	STR_DISP("Close Audio")
	STR_HELP("Closes the currently open audio file")

	void operator()(agi::Context *c) {
		c->audioController->CloseAudio();
	}
};


/// Opens an audio file.
struct audio_open : public Command {
	CMD_NAME("audio/open")
	STR_MENU("&Open Audio File...")
	STR_DISP("Open Audio File")
	STR_HELP("Opens an audio file")

	void operator()(agi::Context *c) {
		try {
			wxString path = to_wx(OPT_GET("Path/Last/Audio")->GetString());
			wxString str = _("Audio Formats") + " (*.aac,*.ac3,*.ape,*.dts,*.flac,*.m4a,*.mka,*.mp3,*.mp4,*.ogg,*.w64,*.wav,*.wma)|*.aac;*.ac3;*.ape;*.dts;*.flac;*.m4a;*.mka;*.mp3;*.mp4;*.ogg;*.w64;*.wav;*.wma|"
						+ _("Video Formats") + " (*.asf,*.avi,*.avs,*.d2v,*.m2ts,*.m4v,*.mkv,*.mov,*.mp4,*.mpeg,*.mpg,*.ogm,*.webm,*.wmv,*.ts)|*.asf;*.avi;*.avs;*.d2v;*.m2ts;*.m4v;*.mkv;*.mov;*.mp4;*.mpeg;*.mpg;*.ogm;*.webm;*.wmv;*.ts|"
						+ _("All Files") + " (*.*)|*.*";
			wxString filename = wxFileSelector(_("Open Audio File"),path,"","",str,wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (!filename.empty()) {
				c->audioController->OpenAudio(filename);
				OPT_SET("Path/Last/Audio")->SetString(from_wx(wxFileName(filename).GetPath()));
			}
		}
		catch (agi::UserCancelException const&) { }
		catch (agi::Exception const& e) {
			wxMessageBox(to_wx(e.GetChainedMessage()), "Error loading file", wxOK | wxICON_ERROR | wxCENTER, c->parent);
		}
	}
};


/// Open a 150 minutes blank audio clip, for debugging.
struct audio_open_blank : public Command {
	CMD_NAME("audio/open/blank")
	STR_MENU("Open 2h30 Blank Audio")
	STR_DISP("Open 2h30 Blank Audio")
	STR_HELP("Open a 150 minutes blank audio clip, for debugging")

	void operator()(agi::Context *c) {
		try {
			c->audioController->OpenAudio("dummy-audio:silence?sr=44100&bd=16&ch=1&ln=396900000");
		}
		catch (agi::Exception const& e) {
			wxMessageBox(to_wx(e.GetChainedMessage()), "Error loading file", wxOK | wxICON_ERROR | wxCENTER, c->parent);
		}
	}
};


/// Open a 150 minutes noise-filled audio clip, for debugging.
struct audio_open_noise : public Command {
	CMD_NAME("audio/open/noise")
	STR_MENU("Open 2h30 Noise Audio")
	STR_DISP("Open 2h30 Noise Audio")
	STR_HELP("Open a 150 minutes noise-filled audio clip, for debugging")

	void operator()(agi::Context *c) {
		try {
			c->audioController->OpenAudio("dummy-audio:noise?sr=44100&bd=16&ch=1&ln=396900000");
		}
		catch (agi::Exception const& e) {
			wxMessageBox(to_wx(e.GetChainedMessage()), "Error loading file", wxOK | wxICON_ERROR | wxCENTER, c->parent);
		}
	}
};


/// Opens the audio from the current video file.
struct audio_open_video : public Command {
	CMD_NAME("audio/open/video")
	STR_MENU("Open Audio from &Video")
	STR_DISP("Open Audio from Video")
	STR_HELP("Opens the audio from the current video file")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		return c->videoController->IsLoaded();
	}

	void operator()(agi::Context *c) {
		try {
			c->audioController->OpenAudio(c->videoController->GetVideoName());
		}
		catch (agi::UserCancelException const&) { }
		catch (agi::Exception const& e) {
			wxMessageBox(to_wx(e.GetChainedMessage()), "Error loading file", wxOK | wxICON_ERROR | wxCENTER, c->parent);
		}
	}
};


/// Display audio as a frequency-power spectrograph.
struct audio_view_spectrum : public Command {
	CMD_NAME("audio/view/spectrum")
	STR_MENU("&Spectrum Display")
	STR_DISP("Spectrum Display")
	STR_HELP("Display audio as a frequency-power spectrograph")
	CMD_TYPE(COMMAND_RADIO)

	bool IsActive(const agi::Context *) {
		return OPT_GET("Audio/Spectrum")->GetBool();
	}

	void operator()(agi::Context *) {
		OPT_SET("Audio/Spectrum")->SetBool(true);
	}
};


/// Display audio as a linear amplitude graph.
struct audio_view_waveform : public Command {
	CMD_NAME("audio/view/waveform")
	STR_MENU("&Waveform Display")
	STR_DISP("Waveform Display")
	STR_HELP("Display audio as a linear amplitude graph")
	CMD_TYPE(COMMAND_RADIO)

	bool IsActive(const agi::Context *) {
		return !OPT_GET("Audio/Spectrum")->GetBool();
	}

	void operator()(agi::Context *) {
		OPT_SET("Audio/Spectrum")->SetBool(false);
	}
};

/// Save the audio for the selected lines.
struct audio_save_clip : public Command {
	CMD_NAME("audio/save/clip")
	STR_MENU("Create audio clip")
	STR_DISP("Create audio clip")
	STR_HELP("Create an audio clip of the selected line")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		return c->audioController->IsAudioOpen() && !c->selectionController->GetSelectedSet().empty();
	}

	void operator()(agi::Context *c) {
		SubtitleSelection sel = c->selectionController->GetSelectedSet();
		if (sel.empty()) return;

		AssTime start = INT_MAX, end = 0;
		for (auto line : sel) {
			start = std::min(start, line->Start);
			end = std::max(end, line->End);
		}

		c->audioController->SaveClip(
			wxFileSelector(_("Save audio clip"), "", "", "wav", "", wxFD_SAVE|wxFD_OVERWRITE_PROMPT, c->parent),
			TimeRange(start, end));
	}
};

/// Play the current audio selection
struct audio_play_current_selection : public validate_audio_open {
	CMD_NAME("audio/play/current")
	STR_MENU("Play current audio selection")
	STR_DISP("Play current audio selection")
	STR_HELP("Play the current audio selection, ignoring changes made while playing")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->audioController->PlayRange(c->audioController->GetPrimaryPlaybackRange());
	}
};

/// Play the current line
struct audio_play_current_line : public validate_audio_open {
	CMD_NAME("audio/play/line")
	STR_MENU("Play current line")
	STR_DISP("Play current line")
	STR_HELP("Play current line")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		AudioTimingController *tc = c->audioController->GetTimingController();
		if (tc)
			c->audioController->PlayRange(tc->GetActiveLineRange());
	}
};

/// Play the current audio selection
struct audio_play_selection : public validate_audio_open {
	CMD_NAME("audio/play/selection")
	STR_MENU("Play audio selection")
	STR_DISP("Play audio selection")
	STR_HELP("Play audio until the end of the selection is reached")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->audioController->PlayPrimaryRange();
	}
};

/// Play the current audio selection or stop audio playback
struct audio_play_toggle : public validate_audio_open {
	CMD_NAME("audio/play/toggle")
	STR_MENU("Play audio selection or stop")
	STR_DISP("Play audio selection or stop")
	STR_HELP("Play selection or stop playback if it's already playing")

	void operator()(agi::Context *c) {
		if (c->audioController->IsPlaying())
			c->audioController->Stop();
		else {
			c->videoController->Stop();
			c->audioController->PlayPrimaryRange();
		}
	}
};

/// Stop playing audio
struct audio_stop : public Command {
	CMD_NAME("audio/stop")
	STR_MENU("Stop playing")
	STR_DISP("Stop playing")
	STR_HELP("Stop")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		return c->audioController->IsPlaying();
	}

	void operator()(agi::Context *c) {
		c->audioController->Stop();
		c->videoController->Stop();
	}
};

/// Play 500 ms before the selected audio range
struct audio_play_before : public validate_audio_open {
	CMD_NAME("audio/play/selection/before")
	STR_MENU("Play 500 ms before selection")
	STR_DISP("Play 500 ms before selection")
	STR_HELP("Play 500 ms before selection")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		int begin = c->audioController->GetPrimaryPlaybackRange().begin();
		c->audioController->PlayRange(TimeRange(begin - 500, begin));
	}
};

/// Play 500 ms after the selected audio range
struct audio_play_after : public validate_audio_open {
	CMD_NAME("audio/play/selection/after")
	STR_MENU("Play 500 ms after selection")
	STR_DISP("Play 500 ms after selection")
	STR_HELP("Play 500 ms after selection")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		int end = c->audioController->GetPrimaryPlaybackRange().end();
		c->audioController->PlayRange(TimeRange(end, end + 500));
	}
};

/// Play the last 500 ms of the audio range
struct audio_play_end : public validate_audio_open {
	CMD_NAME("audio/play/selection/end")
	STR_MENU("Play last 500 ms of selection")
	STR_DISP("Play last 500 ms of selection")
	STR_HELP("Play last 500 ms of selection")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		TimeRange times(c->audioController->GetPrimaryPlaybackRange());
		c->audioController->PlayToEndOfPrimary(times.end() - std::min(500, times.length()));
	}
};

/// Play the first 500 ms of the audio range
struct audio_play_begin : public validate_audio_open {
	CMD_NAME("audio/play/selection/begin")
	STR_MENU("Play first 500 ms of selection")
	STR_DISP("Play first 500 ms of selection")
	STR_HELP("Play first 500 ms of selection")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		TimeRange times(c->audioController->GetPrimaryPlaybackRange());
		c->audioController->PlayRange(TimeRange(
			times.begin(),
			times.begin() + std::min(500, times.length())));
	}
};

/// Play from the beginning of the audio range to the end of the file
struct audio_play_to_end : public validate_audio_open {
	CMD_NAME("audio/play/to_end")
	STR_MENU("Play from selection start to end of file")
	STR_DISP("Play from selection start to end of file")
	STR_HELP("Play from selection start to end of file")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->audioController->PlayToEnd(c->audioController->GetPrimaryPlaybackRange().begin());
	}
};

/// Commit any pending audio timing changes
/// @todo maybe move to time?
struct audio_commit : public validate_audio_open {
	CMD_NAME("audio/commit")
	STR_MENU("Commit")
	STR_DISP("Commit")
	STR_HELP("Commit any pending audio timing changes")

	void operator()(agi::Context *c) {
		AudioTimingController *tc = c->audioController->GetTimingController();
		if (tc) {
			tc->Commit();
			if(OPT_GET("Audio/Next Line on Commit")->GetBool())
				tc->Next(AudioTimingController::LINE);
		}
	}
};

/// Commit any pending audio timing changes
/// @todo maybe move to time?
struct audio_commit_default : public validate_audio_open {
	CMD_NAME("audio/commit/default")
	STR_MENU("Commit and use default timing for next line")
	STR_DISP("Commit and use default timing for next line")
	STR_HELP("Commit any pending audio timing changes and reset the next line's times to the default")

	void operator()(agi::Context *c) {
		AudioTimingController *tc = c->audioController->GetTimingController();
		if (tc) {
			tc->Commit();
			tc->Next(AudioTimingController::LINE_RESET_DEFAULT);
		}
	}
};

/// Commit any pending audio timing changes and move to the next line
/// @todo maybe move to time?
struct audio_commit_next : public validate_audio_open {
	CMD_NAME("audio/commit/next")
	STR_MENU("Commit and move to next line")
	STR_DISP("Commit and move to next line")
	STR_HELP("Commit any pending audio timing changes and move to the next line")

	void operator()(agi::Context *c) {
		AudioTimingController *tc = c->audioController->GetTimingController();
		if (tc) {
			tc->Commit();
			tc->Next(AudioTimingController::LINE);
		}
	}
};

/// Commit any pending audio timing changes and stay on the current line
/// @todo maybe move to time?
struct audio_commit_stay : public validate_audio_open {
	CMD_NAME("audio/commit/stay")
	STR_MENU("Commit and stay on current line")
	STR_DISP("Commit and stay on current line")
	STR_HELP("Commit any pending audio timing changes and stay on the current line")

	void operator()(agi::Context *c) {
		AudioTimingController *tc = c->audioController->GetTimingController();
		if (tc) tc->Commit();
	}
};

/// Scroll the audio display to the current selection
struct audio_go_to : public validate_audio_open {
	CMD_NAME("audio/go_to")
	STR_MENU("Go to selection")
	STR_DISP("Go to selection")
	STR_HELP("Go to selection")

	void operator()(agi::Context *c) {
		c->audioBox->ScrollToActiveLine();
	}
};

/// Scroll the audio display left
struct audio_scroll_left : public validate_audio_open {
	CMD_NAME("audio/scroll/left")
		STR_MENU("Scroll left")
		STR_DISP("Scroll left")
		STR_HELP("Scroll the audio display left")

		void operator()(agi::Context *c) {
			c->audioBox->ScrollAudioBy(-128);
	}
};


/// Scroll the audio display right
struct audio_scroll_right : public validate_audio_open {
	CMD_NAME("audio/scroll/right")
		STR_MENU("Scroll right")
		STR_DISP("Scroll right")
		STR_HELP("Scroll the audio display right")

		void operator()(agi::Context *c) {
			c->audioBox->ScrollAudioBy(128);
	}
};

static inline void toggle(const char *opt) {
	OPT_SET(opt)->SetBool(!OPT_GET(opt)->GetBool());
}

/// Toggle autoscrolling the audio display to the selected line when switch lines
struct audio_autoscroll : public Command {
	CMD_NAME("audio/opt/autoscroll")
	STR_MENU("Auto scrolls audio display to selected line")
	STR_DISP("Auto scrolls audio display to selected line")
	STR_HELP("Auto scrolls audio display to selected line")
	CMD_TYPE(COMMAND_TOGGLE)

	bool IsActive(const agi::Context *) {
		return OPT_GET("Audio/Auto/Scroll")->GetBool();
	}

	void operator()(agi::Context *) {
		toggle("Audio/Auto/Scroll");
	}
};

/// Toggle automatically committing changes made in the audio display
struct audio_autocommit : public Command {
	CMD_NAME("audio/opt/autocommit")
	STR_MENU("Automatically commit all changes")
	STR_DISP("Automatically commit all changes")
	STR_HELP("Automatically commit all changes")
	CMD_TYPE(COMMAND_TOGGLE)

	bool IsActive(const agi::Context *) {
		return OPT_GET("Audio/Auto/Commit")->GetBool();
	}

	void operator()(agi::Context *) {
		toggle("Audio/Auto/Commit");
	}
};

/// Toggle automatically advancing to the next line after a commit
struct audio_autonext : public Command {
	CMD_NAME("audio/opt/autonext")
	STR_MENU("Auto goes to next line on commit")
	STR_DISP("Auto goes to next line on commit")
	STR_HELP("Auto goes to next line on commit")
	CMD_TYPE(COMMAND_TOGGLE)

	bool IsActive(const agi::Context *) {
		return OPT_GET("Audio/Next Line on Commit")->GetBool();
	}

	void operator()(agi::Context *) {
		toggle("Audio/Next Line on Commit");
	}
};

/// Toggle spectrum analyzer mode
struct audio_toggle_spectrum : public Command {
	CMD_NAME("audio/opt/spectrum")
	STR_MENU("Spectrum analyzer mode")
	STR_DISP("Spectrum analyzer mode")
	STR_HELP("Spectrum analyzer mode")
	CMD_TYPE(COMMAND_TOGGLE)

	bool IsActive(const agi::Context *) {
		return OPT_GET("Audio/Spectrum")->GetBool();
	}

	void operator()(agi::Context *) {
		toggle("Audio/Spectrum");
	}
};

/// Toggle linked vertical zoom and volume
struct audio_vertical_link : public Command {
	CMD_NAME("audio/opt/vertical_link")
	STR_MENU("Link vertical zoom and volume sliders")
	STR_DISP("Link vertical zoom and volume sliders")
	STR_HELP("Link vertical zoom and volume sliders")
	CMD_TYPE(COMMAND_TOGGLE)

	bool IsActive(const agi::Context *) {
		return OPT_GET("Audio/Link")->GetBool();
	}

	void operator()(agi::Context *) {
		toggle("Audio/Link");
	}
};

/// Toggle karaoke mode
struct audio_karaoke : public Command {
	CMD_NAME("audio/karaoke")
	STR_MENU("Toggle karaoke mode")
	STR_DISP("Toggle karaoke mode")
	STR_HELP("Toggle karaoke mode")
	CMD_TYPE(COMMAND_TOGGLE)

	bool IsActive(const agi::Context *c) {
		return c->karaoke->IsEnabled();
	}
	void operator()(agi::Context *c) {
		c->karaoke->SetEnabled(!c->karaoke->IsEnabled());
	}
};

/// @}

}

namespace cmd {
	void init_audio() {
		reg(new audio_autocommit);
		reg(new audio_autonext);
		reg(new audio_autoscroll);
		reg(new audio_close);
		reg(new audio_commit);
		reg(new audio_commit_default);
		reg(new audio_commit_next);
		reg(new audio_commit_stay);
		reg(new audio_go_to);
		reg(new audio_karaoke);
		reg(new audio_open);
		reg(new audio_open_blank);
		reg(new audio_open_noise);
		reg(new audio_open_video);
		reg(new audio_play_after);
		reg(new audio_play_before);
		reg(new audio_play_begin);
		reg(new audio_play_end);
		reg(new audio_play_current_selection);
		reg(new audio_play_current_line);
		reg(new audio_play_selection);
		reg(new audio_play_to_end);
		reg(new audio_play_toggle);
		reg(new audio_save_clip);
		reg(new audio_scroll_left);
		reg(new audio_scroll_right);
		reg(new audio_stop);
		reg(new audio_toggle_spectrum);
		reg(new audio_vertical_link);
		reg(new audio_view_spectrum);
		reg(new audio_view_waveform);
	}
}
