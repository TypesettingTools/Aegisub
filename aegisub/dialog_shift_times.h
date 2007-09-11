// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#ifndef DIALOG_SHIFT_TIMES_H
#define DIALOG_SHIFT_TIMES_H


///////////
// Headers
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/radiobox.h>
#include <wx/listbox.h>
#include "timeedit_ctrl.h"


//////////////
// Prototypes
class SubtitlesGrid;


/////////
// Class
class DialogShiftTimes : public wxDialog {
private:
	bool ready;
	SubtitlesGrid *grid;
	int shiftframe;
	wxString HistoryFile;

	TimeEdit *ShiftTime;
	wxTextCtrl *ShiftFrame;
	wxRadioButton *RadioTime;
	wxRadioButton *RadioFrames;
	wxRadioButton *DirectionForward;
	wxRadioButton *DirectionBackward;
	wxRadioBox *SelChoice;
	wxRadioBox *TimesChoice;
	wxListBox *History;

	void AppendToHistory(wxString text);
	void LoadHistory(wxString filename);
	void OnClear(wxCommandEvent &event);

public:
	DialogShiftTimes (wxWindow *parent,SubtitlesGrid *grid);

	void OnKey(wxKeyEvent &event);
	void OnClose(wxCommandEvent &event);
	void OnOK(wxCommandEvent &event);
	void OnEditTime(wxCommandEvent &event);
	void OnEditFrame(wxCommandEvent &event);
	void OnRadioTime(wxCommandEvent &event);
	void OnRadioFrame(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	TEXT_SHIFT_TIME = 1100,
	TEXT_SHIFT_FRAME,
	RADIO_BACKWARD,
	RADIO_FORWARD,
	RADIO_TIME,
	RADIO_FRAME,
	SHIFT_CLEAR_HISTORY
};


#endif
