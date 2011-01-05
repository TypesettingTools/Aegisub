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

#include "../audio_controller.h"
#include "../compat.h"
#include "../include/aegisub/context.h"
#include "../main.h"

namespace cmd {
/// @defgroup cmd-audio Audio commands.
/// @{


/// Closes the currently open audio file.
class audio_close: public Command {
public:
	CMD_NAME("audio/close")
	STR_MENU("&Close Audio")
	STR_DISP("Close Audio")
	STR_HELP("Closes the currently open audio file.")

	void operator()(agi::Context *c) {
		c->audioController->CloseAudio();
	}
};


/// Opens an audio file.
class audio_open: public Command {
public:
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
class audio_open_blank: public Command {
public:
	CMD_NAME("audio/open/blank")
	STR_MENU("Open 2h30 Blank Audio")
	STR_DISP("Open 2h30 Blank Audio")
	STR_HELP("Open a 150 minutes blank audio clip, for debugging.")

	void operator()(agi::Context *c) {
		c->audioController->OpenAudio(_T("dummy-audio:silence?sr=44100&bd=16&ch=1&ln=396900000"));
	}
};


/// Open a 150 minutes noise-filled audio clip, for debugging.
class audio_open_noise: public Command {
public:
	CMD_NAME("audio/open/noise")
	STR_MENU("Open 2h30 Noise Audio")
	STR_DISP("Open 2h30 Noise Audio")
	STR_HELP("Open a 150 minutes noise-filled audio clip, for debugging.")

	void operator()(agi::Context *c) {
		c->audioController->OpenAudio(_T("dummy-audio:noise?sr=44100&bd=16&ch=1&ln=396900000"));
	}
};


/// Opens the audio from the current video file.
class audio_open_video: public Command {
public:
	CMD_NAME("audio/open/video")
	STR_MENU("Open Audio from &Video")
	STR_DISP("Open Audio from Video")
	STR_HELP("Opens the audio from the current video file.")

	void operator()(agi::Context *c) {
		c->audioController->OpenAudio(_T("audio-video:cache"));
	}
};


/// Display audio as a frequency-power spectrograph.
class audio_view_spectrum: public Command {
public:
	CMD_NAME("audio/view/spectrum")
	STR_MENU("Spectrum Display")
	STR_DISP("Spectrum Display")
	STR_HELP("Display audio as a frequency-power spectrograph.")

	void operator()(agi::Context *c) {
		printf("XXX: fixme\n");
	}
};


/// Display audio as a linear amplitude graph.
class audio_view_waveform: public Command {
public:
	CMD_NAME("audio/view/waveform")
	STR_MENU("Waveform Display")
	STR_DISP("Waveform Display")
	STR_HELP("Display audio as a linear amplitude graph.")

	void operator()(agi::Context *c) {
		printf("XXX: fixme\n");
	}
};

/// @}

/// Init audio/ commands
void init_audio(CommandManager *cm) {
	cm->reg(new audio_close());
	cm->reg(new audio_open());
	cm->reg(new audio_open_blank());
	cm->reg(new audio_open_noise());
	cm->reg(new audio_open_video());
	cm->reg(new audio_view_spectrum());
	cm->reg(new audio_view_waveform());
}

} // namespace cmd
