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

#include <wx/dialog.h>
#endif

namespace agi { struct Context; }
class AssDialogue;
class wxButton;
class wxCheckBox;
class wxCheckListBox;
class wxSlider;

/// @class DialogTimingProcessor
/// @brief Automatic postprocessor for correcting common timing issues
class DialogTimingProcessor : public wxDialog {
	agi::Context *c; ///< Project context

	int leadIn;      ///< Lead-in to add in milliseconds
	int leadOut;     ///< Lead-out to add in milliseconds
	int beforeStart; ///< Maximum time in milliseconds to move start time of line backwards to land on a keyframe
	int afterStart;  ///< Maximum time in milliseconds to move start time of line forwards to land on a keyframe
	int beforeEnd;   ///< Maximum time in milliseconds to move end time of line backwards to land on a keyframe
	int afterEnd;    ///< Maximum time in milliseconds to move end time of line forwards to land on a keyframe
	int adjGap;      ///< Maximum gap in milliseconds to snap adjacent lines to each other
	int adjOverlap;  ///< Maximum overlap in milliseconds to snap adjacent lines to each other

	wxCheckBox *onlySelection; ///< Only process selected lines of the selected styles
	wxCheckBox *hasLeadIn;     ///< Enable adding lead-in
	wxCheckBox *hasLeadOut;    ///< Enable adding lead-out
	wxCheckBox *keysEnable;    ///< Enable snapping to keyframes
	wxCheckBox *adjsEnable;    ///< Enable snapping adjacent lines to each other
	wxSlider *adjacentBias;    ///< Bias between shifting start and end times when snapping adjacent lines
	wxCheckListBox *StyleList; ///< List of styles to process
	wxButton *ApplyButton;     ///< Button to apply the processing

	void OnApply(wxCommandEvent &event);

	/// Check or uncheck all styles
	void CheckAll(bool check);

	/// Enable and disable text boxes based on which checkboxes are checked
	void UpdateControls();

	/// Process the file
	void Process();

	/// Get a list of dialogue lines in the file sorted by start time
	std::vector<AssDialogue*> SortDialogues();

public:
	/// Constructor
	/// @param c Project context
	DialogTimingProcessor(agi::Context *c);
};
