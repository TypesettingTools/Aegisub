// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "include/aegisub/context.h"

#include "ass_file.h"
#include "audio_controller.h"
#include "auto4_base.h"
#include "dialog_manager.h"
#include "initial_line_state.h"
#include "search_replace_engine.h"
#include "selection_controller.h"
#include "subs_controller.h"
#include "video_context.h"

#include <libaegisub/util.h>

namespace agi {
Context::Context()
: ass(util::make_unique<AssFile>())
, subsController(util::make_unique<SubsController>(this))
, local_scripts(util::make_unique<Automation4::LocalScriptManager>(this))
, videoController(util::make_unique<VideoContext>(this))
, audioController(util::make_unique<AudioController>(this))
, selectionController(util::make_unique<SelectionController>(this))
, initialLineState(util::make_unique<InitialLineState>(this))
, search(util::make_unique<SearchReplaceEngine>(this))
, dialog(util::make_unique<DialogManager>())
{
	subsController->SetSelectionController(selectionController.get());
}

Context::~Context() {}
}
