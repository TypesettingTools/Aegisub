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
//
// $Id$

/// @file audio.cpp
/// @brief audio/ commands.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <wx/filedlg.h>
#endif

#include "command.h"

#include "../selection_controller.h"
#include "../ass_dialogue.h"
#include "../audio_controller.h"
#include "../audio_timing.h"
#include "../compat.h"
#include "../include/aegisub/context.h"
#include "../selection_controller.h"
#include "../main.h"

typedef SelectionController<AssDialogue>::Selection Selection;

namespace cmd {
/// @defgroup cmd-audio Audio commands.
/// @{


/// Closes the currently open audio file.
struct audio_close : public Command {
	CMD_NAME("audio/close")
	STR_MENU("&Close Audio")
	STR_DISP("Close Audio")
	STR_HELP("Closes the currently open audio file.")

	void operator()(agi::Context *c) {
		c->audioController->CloseAudio();
	}
};


/// Opens an audio file.
struct audio_open : public Command {
	CMD_NAME("audio/open")
	STR_MENU("&Open Audio File..")
	STR_DISP("Open Audio File")
	STR_HELP("Opens an audio file.")

	void operator()(agi::Context *c) {
		wxString path = lagi_wxString(OPT_GET("Path/Last/Audio")->GetString());  
		wxString str = wxString(_("Audio Formats")) + _T(" (*.wav,*.mp3,*.ogg,*.flac,*.mp4,*.ac3,*.aac,*.mka,*.m4a,*.w64)|*.wav;*.mp3;*.ogg;*.flac;*.mp4;*.ac3;*.aac;*.mka;*.m4a;*.w64|")
					+ _("Video Formats") + _T(" (*.avi,*.mkv,*.ogm,*.mpg,*.mpeg)|*.avi;*.mkv;*.ogm;*.mp4;*.mpeg;*.mpg|")
					+ _("All files") + _T(" (*.*)|*.*");
		wxString filename = wxFileSelector(_("Open audio file"),path,_T(""),_T(""),str,wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (!filename.empty()) {
			c->audioController->OpenAudio(filename);
			OPT_SET("Path/Last/Audio")->SetString(STD_STR(filename));
		}
	}
};


/// Open a 150 minutes blank audio clip, for debugging.
struct audio_open_blank : public Command {
	CMD_NAME("audio/open/blank")
	STR_MENU("Open 2h30 Blank Audio")
	STR_DISP("Open 2h30 Blank Audio")
	STR_HELP("Open a 150 minutes blank audio clip, for debugging.")

	void operator()(agi::Context *c) {
		c->audioController->OpenAudio(_T("dummy-audio:silence?sr=44100&bd=16&ch=1&ln=396900000"));
	}
};


/// Open a 150 minutes noise-filled audio clip, for debugging.
struct audio_open_noise : public Command {
	CMD_NAME("audio/open/noise")
	STR_MENU("Open 2h30 Noise Audio")
	STR_DISP("Open 2h30 Noise Audio")
	STR_HELP("Open a 150 minutes noise-filled audio clip, for debugging.")

	void operator()(agi::Context *c) {
		c->audioController->OpenAudio(_T("dummy-audio:noise?sr=44100&bd=16&ch=1&ln=396900000"));
	}
};


/// Opens the audio from the current video file.
struct audio_open_video : public Command {
	CMD_NAME("audio/open/video")
	STR_MENU("Open Audio from &Video")
	STR_DISP("Open Audio from Video")
	STR_HELP("Opens the audio from the current video file.")

	void operator()(agi::Context *c) {
		c->audioController->OpenAudio(_T("audio-video:cache"));
	}
};


/// Display audio as a frequency-power spectrograph.
struct audio_view_spectrum : public Command {
	CMD_NAME("audio/view/spectrum")
	STR_MENU("Spectrum Display")
	STR_DISP("Spectrum Display")
	STR_HELP("Display audio as a frequency-power spectrograph.")

	void operator()(agi::Context *c) {
		OPT_SET("Audio/Spectrum")->SetBool(true);
	}
};


/// Display audio as a linear amplitude graph.
struct audio_view_waveform : public Command {
	CMD_NAME("audio/view/waveform")
	STR_MENU("Waveform Display")
	STR_DISP("Waveform Display")
	STR_HELP("Display audio as a linear amplitude graph.")

	void operator()(agi::Context *c) {
		OPT_SET("Audio/Spectrum")->SetBool(false);
	}
};

/// Save the audio for the selected lines.
struct audio_save_clip : public Command {
	CMD_NAME("audio/save/clip")
	STR_MENU("Create audio clip")
	STR_DISP("Create audio clip")
	STR_HELP("Create an audio clip of the selected line")

	void operator()(agi::Context *c) {
		Selection sel = c->selectionController->GetSelectedSet();
		for (Selection::iterator it = sel.begin(); it != sel.end(); ++it) {
			c->audioController->SaveClip(
				wxFileSelector(_("Save audio clip"), "", "", "wav", "", wxFD_SAVE|wxFD_OVERWRITE_PROMPT, c->parent),
				SampleRange(c->audioController->SamplesFromMilliseconds((*it)->Start.GetMS()),
							c->audioController->SamplesFromMilliseconds((*it)->End.GetMS())));
		}
	}
};

/// Play the current audio selection
struct audio_play_selection : public Command {
	CMD_NAME("audio/play/selection")
	STR_MENU("Play audio selection")
	STR_DISP("Play audio selection")
	STR_HELP("Play selection")
	void operator()(agi::Context *c) {
		c->audioController->PlayPrimaryRange();
	}
};

/// Stop playing audio
struct audio_stop : public Command {
	CMD_NAME("audio/stop")
	STR_MENU("Stop playing")
	STR_DISP("Stop playing")
	STR_HELP("Stop")
	void operator()(agi::Context *c) {
		c->audioController->Stop();
	}
};

/// Play 500 ms before the selected audio range
struct audio_play_before : public Command {
	CMD_NAME("audio/play/selection/before")
	STR_MENU("Play 500 ms before selection")
	STR_DISP("Play 500 ms before selection")
	STR_HELP("Play 500 ms before selection")
	void operator()(agi::Context *c) {
		SampleRange times(c->audioController->GetPrimaryPlaybackRange());
		c->audioController->PlayRange(SampleRange(
			times.begin() - c->audioController->SamplesFromMilliseconds(500),
			times.begin()));
	}
};

/// Play 500 ms after the selected audio range
struct audio_play_after : public Command {
	CMD_NAME("audio/play/selection/after")
	STR_MENU("Play 500 ms after selection")
	STR_DISP("Play 500 ms after selection")
	STR_HELP("Play 500 ms after selection")
	void operator()(agi::Context *c) {
		SampleRange times(c->audioController->GetPrimaryPlaybackRange());
		c->audioController->PlayRange(SampleRange(
			times.end(),
			times.end() + c->audioController->SamplesFromMilliseconds(500)));
	}
};

/// Play from the beginning of the audio range to the end of the file
struct audio_play_end : public Command {
	CMD_NAME("audio/play/selection/end")
	STR_MENU("Play last 500 ms of selection")
	STR_DISP("Play last 500 ms of selection")
	STR_HELP("Play last 500 ms of selection")
	void operator()(agi::Context *c) {
		SampleRange times(c->audioController->GetPrimaryPlaybackRange());
		c->audioController->PlayRange(SampleRange(
			times.begin(),
			times.begin() + std::min(
				c->audioController->SamplesFromMilliseconds(500),
				times.length())));
	}
};

/// Play the first 500 ms of the audio range
struct audio_play_begin : public Command {
	CMD_NAME("audio/play/selection/begin")
	STR_MENU("Play first 500 ms of selection")
	STR_DISP("Play first 500 ms of selection")
	STR_HELP("Play first 500 ms of selection")
	void operator()(agi::Context *c) {
		SampleRange times(c->audioController->GetPrimaryPlaybackRange());
		c->audioController->PlayRange(SampleRange(
			times.end() - std::min(
				c->audioController->SamplesFromMilliseconds(500),
				times.length()),
			times.end()));
	}
};

/// Play the last 500 ms of the audio range
struct audio_play_to_end : public Command {
	CMD_NAME("audio/play/to_end")
	STR_MENU("Play from selection start to end of file")
	STR_DISP("Play from selection start to end of file")
	STR_HELP("Play from selection start to end of file")
	void operator()(agi::Context *c) {
		c->audioController->PlayToEnd(c->audioController->GetPrimaryPlaybackRange().begin());
	}
};

/// Commit any pending audio timing changes
/// @todo maybe move to time?
struct audio_commit : public Command {
	CMD_NAME("audio/commit")
	STR_MENU("Commit")
	STR_DISP("Commit")
	STR_HELP("Commit")
	void operator()(agi::Context *c) {
		c->audioController->GetTimingController()->Commit();
	}
};

/// Scroll the audio display to the current selection
struct audio_go_to : public Command {
	CMD_NAME("audio/?")
	STR_MENU("Go to selection")
	STR_DISP("Go to selection")
	STR_HELP("Go to selection")
	void operator()(agi::Context *c) {
		//if (c->audioController->GetTimingController())
			//audioDisplay->ScrollSampleRangeInView(c->audioController->GetTimingController()->GetIdealVisibleSampleRange());
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
	void operator()(agi::Context *c) {
		toggle("Audio/Auto/Scroll");
	}
};

/// Toggle automatically committing changes made in the audio display
struct audio_autocommit : public Command {
	CMD_NAME("audio/opt/autocommit")
	STR_MENU("Automatically commit all changes")
	STR_DISP("Automatically commit all changes")
	STR_HELP("Automatically commit all changes")
	void operator()(agi::Context *c) {
		toggle("Audio/Auto/Commit");
	}
};

/// Toggle automatically advancing to the next line after a commit
struct audio_autonext : public Command {
	CMD_NAME("audio/opt/autonext")
	STR_MENU("Auto goes to next line on commit")
	STR_DISP("Auto goes to next line on commit")
	STR_HELP("Auto goes to next line on commit")
	void operator()(agi::Context *c) {
		toggle("Audio/Next Line on Commit");
	}
};

/// Toggle linked vertical zoom and volume
struct audio_vertical_link : public Command {
	CMD_NAME("audio/opt/vertical_link")
	STR_MENU("Link vertical zoom and volume sliders")
	STR_DISP("Link vertical zoom and volume sliders")
	STR_HELP("Link vertical zoom and volume sliders")
	void operator()(agi::Context *c) {
		toggle("Audio/Link");
	}
};

/// @}

/// Init audio/ commands
void init_audio(CommandManager *cm) {
	cm->reg(new audio_autocommit);
	cm->reg(new audio_autonext);
	cm->reg(new audio_autoscroll);
	cm->reg(new audio_close);
	cm->reg(new audio_commit);
	cm->reg(new audio_go_to);
	cm->reg(new audio_open);
	cm->reg(new audio_open_blank);
	cm->reg(new audio_open_noise);
	cm->reg(new audio_open_video);
	cm->reg(new audio_play_after);
	cm->reg(new audio_play_before);
	cm->reg(new audio_play_begin);
	cm->reg(new audio_play_end);
	cm->reg(new audio_play_selection);
	cm->reg(new audio_play_to_end);
	cm->reg(new audio_save_clip);
	cm->reg(new audio_stop);
	cm->reg(new audio_vertical_link);
	cm->reg(new audio_view_spectrum);
	cm->reg(new audio_view_waveform);
}

} // namespace cmd
