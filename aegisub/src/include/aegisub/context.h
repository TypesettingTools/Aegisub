#pragma once // sigh.

#include <wx/window.h>

#include "ass_file.h"
#include "subs_grid.h"
#include "audio_box.h"
#include "video_box.h"
#include "dialog_detached_video.h"
#include "auto4_base.h"
#include "dialog_styling_assistant.h"
#include "audio_controller.h"

namespace agi {

struct Context {
	// Frames
	wxWindow *parent;

	DialogStyling *stylingAssistant;

	AudioBox *audioBox;
	AudioController *audioController;
	DialogDetachedVideo *detachedVideo;
	AssFile *ass;
	Automation4::ScriptManager *local_scripts;
	wxWindow *PreviousFocus;
	SubsEditBox *EditBox;
	SubtitlesGrid *SubsGrid;
	VideoBox *videoBox;
};

}
