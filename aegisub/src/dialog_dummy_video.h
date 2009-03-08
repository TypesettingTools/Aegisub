// Copyright (c) 2007, Niels Martin Hansen
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
// Contact: mailto:jiifurusu@gmail.com
//

#ifndef _DIALOG_DUMMY_VIDEO_H
#define _DIALOG_DUMMY_VIDEO_H

#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include "video_provider_dummy.h"
#include "colour_button.h"

class DialogDummyVideo : public wxDialog {
private:
	DialogDummyVideo(wxWindow *parent);
	virtual ~DialogDummyVideo();

	wxComboBox *resolution_shortcuts;
	wxTextCtrl *width;
	wxTextCtrl *height;
	ColourButton *colour;
	wxCheckBox *pattern;
	//wxComboBox *fps;
	wxTextCtrl *fps;
	wxSpinCtrl *length;
	wxStaticText *length_display;
	wxButton *ok_button;
	wxButton *cancel_button;

	void OnResolutionShortcut(wxCommandEvent &evt);
	void OnFpsChange(wxCommandEvent &evt);
	void OnLengthSpin(wxSpinEvent &evt);
	void OnLengthChange(wxCommandEvent &evt);

	void UpdateLengthDisplay();

public:
	static bool CreateDummyVideo(wxWindow *parent, wxString &out_filename);

	DECLARE_EVENT_TABLE()
};

enum {
	Dummy_Video_Resolution_Shortcut = 1700,
	Dummy_Video_FPS,
	Dummy_Video_Length,
};


#endif
