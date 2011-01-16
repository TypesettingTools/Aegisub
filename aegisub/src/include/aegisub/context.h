class AssFile;
class AudioBox;
class AudioController;
class DialogDetachedVideo;
class DialogStyling;
class SubsEditBox;
class SubtitlesGrid;
class VideoBox;
class VideoContext;
class wxWindow;
namespace Automation4 { class ScriptManager; }

namespace agi {

struct Context {
	// Models
	AssFile *ass;
	Automation4::ScriptManager *local_scripts;

	// Controllers
	AudioController *audioController;
	VideoContext *videoController;

	// Things that should probably be in some sort of UI-context-model
	wxWindow *parent;
	wxWindow *previousFocus;

	// Views (i.e. things that should eventually not be here at all)
	AudioBox *audioBox;
	DialogDetachedVideo *detachedVideo;
	DialogStyling *stylingAssistant;
	SubsEditBox *editBox;
	SubtitlesGrid *subsGrid;
	VideoBox *videoBox;
};

}
