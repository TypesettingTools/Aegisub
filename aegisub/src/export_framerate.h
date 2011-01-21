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

/// @file export_framerate.h
/// @see export_framerate.cpp
/// @ingroup export
///

#include "ass_export_filter.h"
#include <libaegisub/vfr.h>

class AssDialogue;
class AssOverrideParameter;
class wxCheckBox;
class wxRadioButton;
class wxTextCtrl;

/// DOCME
/// @class AssTransformFramerateFilter
/// @brief Transform subtitle times, including those in override tags, from an input framerate to an output framerate
class AssTransformFramerateFilter : public AssExportFilter {
	AssDialogue *line;
	int newStart;
	int newEnd;
	int newK;
	int oldK;

	// Yes, these are backwards
	const agi::vfr::Framerate *Input;  ///< Destination frame rate
	const agi::vfr::Framerate *Output; ///< Source frame rate

	agi::vfr::Framerate t1,t2;

	wxTextCtrl *InputFramerate; ///< Input frame rate text box
	wxTextCtrl *OutputFramerate; ///< Output frame rate text box

	wxRadioButton *RadioOutputCFR; ///< CFR radio control
	wxRadioButton *RadioOutputVFR; ///< VFR radio control

	wxCheckBox *Reverse; ///< Switch input and output

	/// @brief Apply the transformation to a file
	/// @param subs File to process
	void TransformFrameRate(AssFile *subs);
	/// @brief Transform a single tag
	/// @param name Name of the tag
	/// @param curParam Current parameter being processed
	/// @param userdata Filter instance
	static void TransformTimeTags(wxString name,int,AssOverrideParameter *curParam,void *userdata);

	/// @brief Convert a time from the input frame rate to the output frame rate
	/// @param time Time in ms to convert
	/// @return Time in ms
	///
	/// This preserves two things:
	///   1. The frame number
	///   2. The relative distance between the beginning of the frame which time
	///      is in and the beginning of the next frame
	int ConvertTime(int time);

	/// From Video click handler
	void OnFpsFromVideo(wxCommandEvent &);
public:
	/// Constructor
	AssTransformFramerateFilter();
	void ProcessSubs(AssFile *subs, wxWindow *export_dialog);
	wxWindow *GetConfigDialogWindow(wxWindow *parent);
	void LoadSettings(bool IsDefault);
};
