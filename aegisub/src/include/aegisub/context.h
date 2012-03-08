class AssFile;
class AudioBox;
class AudioController;
class AssDialogue;
class AudioKaraoke;
class DialogDetachedVideo;
class DialogStyling;
class DialogStyleManager;
class DialogTranslation;
template<class T> class SelectionController;
class SubsTextEditCtrl;
class SubtitlesGrid;
class VideoContext;
class VideoDisplay;
class wxWindow;
namespace Automation4 { class ScriptManager; }

namespace agi {

struct Context {
	// Models
	AssFile *ass;
	Automation4::ScriptManager *local_scripts;

	// Controllers
	AudioController *audioController;
	SelectionController<AssDialogue> *selectionController;
	VideoContext *videoController;

	// Things that should probably be in some sort of UI-context-model
	wxWindow *parent;
	wxWindow *previousFocus;
	wxWindow *videoSlider;

	// Views (i.e. things that should eventually not be here at all)
	AudioBox *audioBox;
	AudioKaraoke *karaoke;
	DialogDetachedVideo *detachedVideo;
	DialogStyling *stylingAssistant;
	DialogStyleManager *stylesManager;
	DialogTranslation *translationAssistant;
	SubsTextEditCtrl *editBox;
	SubtitlesGrid *subsGrid;
	VideoDisplay *videoDisplay;
};

}
