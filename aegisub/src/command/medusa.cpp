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

/// @file medusa.cpp
/// @brief medusa/ commands.
/// @ingroup command
///

#include "config.h"

#ifndef AGI_PRE
#endif

#include "command.h"

#include "aegisub/context.h"
#include "audio_timing.h"

namespace cmd {

class medusa_enter: public Command {
public:
	CMD_NAME("medusa/enter")
	STR_MENU("Medusa Enter")
	STR_DISP("Medusa Enter")
	STR_HELP("")

	void operator()(agi::Context *c) {
	 /// @todo Figure out how to handle this in the audio controller
	//audioBox->audioDisplay->Prev(false);
	}
};

class medusa_next: public Command {
public:
	CMD_NAME("medusa/next")
	STR_MENU("Medusa Next")
	STR_DISP("Medusa Next")
	STR_HELP("")

	void operator()(agi::Context *c) {
	/// @todo Figure out how to handle this in the audio controller
	//audioBox->audioDisplay->Next(false);
	}
};


class medusa_play: public Command {
public:
	CMD_NAME("medusa/play")
	STR_MENU("Medusa Play")
	STR_DISP("Medusa Play")
	STR_HELP("Medusa play hotkey.")

	void operator()(agi::Context *c) {
		c->audioController->PlayPrimaryRange();
	}
};


class medusa_play_after: public Command {
public:
	CMD_NAME("medusa/play/after")
	STR_MENU("Medusa Play After")
	STR_DISP("Medusa Play After")
	STR_HELP("Medusa play after hotkey.")

	void operator()(agi::Context *c) {
		SampleRange sel(c->audioController->GetPrimaryPlaybackRange());
			c->audioController->PlayRange(SampleRange(
				sel.end(),
				sel.end() + c->audioController->SamplesFromMilliseconds(500)));;
	}
};


class medusa_play_before: public Command {
public:
	CMD_NAME("medusa/play/before")
	STR_MENU("Medusa Play Before")
	STR_DISP("Medusa Play Before")
	STR_HELP("Medusa play before hotkey.")

	void operator()(agi::Context *c) {
		SampleRange sel(c->audioController->GetPrimaryPlaybackRange());
			c->audioController->PlayRange(SampleRange(
				sel.begin() - c->audioController->SamplesFromMilliseconds(500),
				sel.begin()));;
	}
};


class medusa_previous: public Command {
public:
	CMD_NAME("medusa/previous")
	STR_MENU("Medusa Previous")
	STR_DISP("Medusa Previous")
	STR_HELP("Medusa previous hotkey.")

	void operator()(agi::Context *c) {
		/// @todo Figure out how to handle this in the audio controller
		//audioBox->audioDisplay->Prev(false);
	}
};


class medusa_shift_end_back: public Command {
public:
	CMD_NAME("medusa/shift/end/back")
	STR_MENU("Medusa Shift End Back")
	STR_DISP("Medusa Shift End Back")
	STR_HELP("Medusa shift end back hotkey.")

	void operator()(agi::Context *c) {
		SampleRange newsel(
			c->audioController->GetPrimaryPlaybackRange(),
			0,
			-c->audioController->SamplesFromMilliseconds(10));
		/// @todo Make this use the timing controller instead
		//audioController->SetSelection(newsel);
	}
};


class medusa_shift_end_forward: public Command {
public:
	CMD_NAME("medusa/shift/end/forward")
	STR_MENU("Medusa Shift End Forward")
	STR_DISP("Medusa Shift End Forward")
	STR_HELP("Medusa shift end forward hotkey.")

	void operator()(agi::Context *c) {
		SampleRange newsel(
			c->audioController->GetPrimaryPlaybackRange(),
			0,
			c->audioController->SamplesFromMilliseconds(10));
		/// @todo Make this use the timing controller instead
		//audioController->SetSelection(newsel);
	}
};


class medusa_shift_start_back: public Command {
public:
	CMD_NAME("medusa/shift/start/back")
	STR_MENU("Medusa Shift Start Back")
	STR_DISP("Medusa Shift Start Back")
	STR_HELP("Medusa shift start back hotkey.")

	void operator()(agi::Context *c) {
		SampleRange newsel(
			c->audioController->GetPrimaryPlaybackRange(),
			-c->audioController->SamplesFromMilliseconds(10),
			0);
		/// @todo Make this use the timing controller instead
		//audioController->SetSelection(newsel);
	}
};


class medusa_shift_start_forward: public Command {
public:
	CMD_NAME("medusa/shift/start/forward")
	STR_MENU("Medusa Shift Start Forward")
	STR_DISP("Medusa Shift Start Forward")
	STR_HELP("Medusa shift start forward hotkey.")

	void operator()(agi::Context *c) {
		SampleRange newsel(
			c->audioController->GetPrimaryPlaybackRange(),
			c->audioController->SamplesFromMilliseconds(10),
			0);
		/// @todo Make this use the timing controller instead
		//audioController->SetSelection(newsel);
	}
};


class medusa_stop: public Command {
public:
	CMD_NAME("medusa/stop")
	STR_MENU("Medusa Stop")
	STR_DISP("Medusa Stop")
	STR_HELP("Medusa stop hotkey.")

	void operator()(agi::Context *c) {
		// Playing, stop
		if (c->audioController->IsPlaying()) {
			c->audioController->Stop();
		} else {
			// Otherwise, play the last 500 ms
			SampleRange sel(c->audioController->GetPrimaryPlaybackRange());
				c->audioController->PlayRange(SampleRange(
					sel.end() - c->audioController->SamplesFromMilliseconds(500),
					sel.end()));;
		}
	}
};


void init_medusa(CommandManager *cm) {
	cm->reg(new medusa_enter());
	cm->reg(new medusa_next());
	cm->reg(new medusa_play());
	cm->reg(new medusa_play_after());
	cm->reg(new medusa_play_before());
	cm->reg(new medusa_previous());
	cm->reg(new medusa_shift_end_back());
	cm->reg(new medusa_shift_end_forward());
	cm->reg(new medusa_shift_start_back());
	cm->reg(new medusa_shift_start_forward());
	cm->reg(new medusa_stop());
}

} // namespace cmd
