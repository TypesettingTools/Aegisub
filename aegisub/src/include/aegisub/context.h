class wxWindow;
class DialogStyling;
class AudioBox;
class AudioController;
class DialogDetachedVideo;
class AssFile;
namespace Automation4 { class ScriptManager; }
class SubsEditBox;
class SubtitlesGrid;
class VideoBox;
class VideoContext;

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
	VideoContext *videoContext;
};

}
