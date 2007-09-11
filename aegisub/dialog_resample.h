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


#ifndef DIALOG_RESAMPLE_H
#define DIALOG_RESAMPLE_H


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>


//////////////
// Prototypes
class SubtitlesGrid;
class AssOverrideParameter;


/////////
// Class
class DialogResample : public wxDialog {
private:
	SubtitlesGrid *grid;
	wxString ResXValue,ResYValue;
	wxTextCtrl *ResX;
	wxTextCtrl *ResY;
	wxCheckBox *Anamorphic;

	static double rx,ry,r,ar;

	void OnResample (wxCommandEvent &event);
	void OnGetDestRes (wxCommandEvent &event);
	static void ResampleTags (wxString name,int n,AssOverrideParameter *curParam,void *_curDiag);

public:
	DialogResample(wxWindow *parent, SubtitlesGrid *grid);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	BUTTON_DEST_FROM_VIDEO = 1520,
	BUTTON_RESAMPLE,
	CHECK_ANAMORPHIC
};


#endif
