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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_timing_processor.h
/// @see dialog_timing_processor.cpp
/// @ingroup tools_ui
///

#ifndef AGI_PRE
#include <vector>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#endif

namespace agi { struct Context; }
class AssDialogue;
class SubtitlesGrid;

/// DOCME
/// @class DialogTimingProcessor
/// @brief DOCME
///
/// DOCME
class DialogTimingProcessor : public wxDialog {
	agi::Context *c;

	/// DOCME
	wxStaticBoxSizer *KeyframesSizer;

	/// DOCME
	wxCheckBox *onlySelection;

	/// DOCME
	wxTextCtrl *leadIn;

	/// DOCME
	wxTextCtrl *leadOut;

	/// DOCME
	wxCheckBox *hasLeadIn;

	/// DOCME
	wxCheckBox *hasLeadOut;

	/// DOCME
	wxCheckBox *keysEnable;

	/// DOCME
	wxTextCtrl *keysStartBefore;

	/// DOCME
	wxTextCtrl *keysStartAfter;

	/// DOCME
	wxTextCtrl *keysEndBefore;

	/// DOCME
	wxTextCtrl *keysEndAfter;

	/// DOCME
	wxCheckBox *adjsEnable;

	/// DOCME
	wxTextCtrl *adjacentThres;

	/// DOCME
	wxSlider *adjacentBias;

	/// DOCME
	wxCheckListBox *StyleList;

	/// DOCME
	wxButton *ApplyButton;

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME
	wxString leadInTime,leadOutTime,thresStartBefore,thresStartAfter,thresEndBefore,thresEndAfter,adjsThresTime;

	/// DOCME
	std::vector<int> KeyFrames;

	void OnCheckBox(wxCommandEvent &event);
	void OnSelectAll(wxCommandEvent &event);
	void OnSelectNone(wxCommandEvent &event);
	void OnApply(wxCommandEvent &event);

	void UpdateControls();
	void Process();
	int GetClosestKeyFrame(int frame);

	/// DOCME
	std::vector<AssDialogue*> Sorted;
	void SortDialogues();

public:
	DialogTimingProcessor(agi::Context *c);

	DECLARE_EVENT_TABLE()
};
