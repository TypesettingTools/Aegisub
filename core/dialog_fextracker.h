// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#ifndef DIALOG_FEXTRACKER_H
#define DIALOG_FEXTRACKER_H


///////////
// Headers
#include <wx/wxprec.h>


//////////////
// Prototypes
class FexTrackerConfig;


/////////
// Class
class DialogFexTracker : public wxDialog {
private:
	FexTrackerConfig * cfg;

	wxTextCtrl *FeatureNumber;
	wxTextCtrl *EdgeDetectSigma;
	wxTextCtrl *WindowX, *WindowY;
	wxTextCtrl *SearchRange;
	wxTextCtrl *MaxIterations;
	wxTextCtrl *MinDeterminant;
	wxTextCtrl *MinDisplacement;
	wxTextCtrl *MaxResidue;
	wxTextCtrl *MinDistanceSquare;

	void OnStart (wxCommandEvent &event);

public:
	DialogFexTracker(wxWindow *parent, FexTrackerConfig * cfg);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	BUTTON_START = 1520,
};


#endif
