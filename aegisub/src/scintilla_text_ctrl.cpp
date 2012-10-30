// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file scintilla_text_ctrl.cpp
/// @brief Customised version of wxStyledTextControl used for main edit box
/// @ingroup custom_control
///

#include "config.h"

#include "scintilla_text_ctrl.h"
#include "utils.h"

ScintillaTextCtrl::ScintillaTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style)
: wxStyledTextCtrl(parent, id, pos, size, style, value)
{
	Bind(wxEVT_MOUSEWHEEL, &ScintillaTextCtrl::OnMouseWheel, this);
}

/// @brief Get unicode-compatible position
int ScintillaTextCtrl::GetUnicodePosition(int pos) {
	return GetText().Left(pos).utf8_str().length();
}

/// @brief Reverse unicode-compatible position
int ScintillaTextCtrl::GetReverseUnicodePosition(int pos) {
	wxCharBuffer buffer = GetTextRaw();

	// Limit position to it
	if (pos > (signed)buffer.length()) pos = buffer.length();

	// Get UTF8 substring
	char *buf2 = new char[pos+1];
	memcpy(buf2,buffer,pos);
	buf2[pos] = 0;

	// Convert back and return its length
	wxString buf3(buf2,wxConvUTF8);
	delete[] buf2;
	return buf3.Length();
}

/// @brief Start unicode-safe styling
void ScintillaTextCtrl::StartUnicodeStyling(int start,int mask) {
	StartStyling(GetUnicodePosition(start),mask);
	// Cache the text for styling as GetText is hideously slow
	text = GetText();
}

/// @brief Unicode-safe styling
void ScintillaTextCtrl::SetUnicodeStyling(int start,int length,int style) {
	// Get the real length
	int len = text.Mid(start, length).utf8_str().length();

	// Set styling
	SetStyling(len,style);
}

/// @brief Get boundaries of word at position
void ScintillaTextCtrl::GetBoundsOfWordAtPosition(int pos,int &start,int &end) {
	IntPairVector results;
	GetWordBoundaries(GetText(), results);

	// Get boundaries
	for (size_t i = 0; i < results.size(); i++) {
		if (results[i].first <= pos && results[i].second >= pos) {
			start = results[i].first;
			end = results[i].second;
			return;
		}
	}

	// Word not found
	start = 0;
	end = 0;
}

/// @brief Get word at specified position
wxString ScintillaTextCtrl::GetWordAtPosition(int pos) {
	int start,end;
	GetBoundsOfWordAtPosition(pos, start, end);
	return GetText().Mid(start, end - start);
}

/// @brief Set selection, unicode-aware
void ScintillaTextCtrl::SetSelectionU(int start, int end) {
	SetSelection(GetUnicodePosition(start),GetUnicodePosition(end));
}

void ScintillaTextCtrl::OnMouseWheel(wxMouseEvent& evt) {
	if (ForwardMouseWheelEvent(this, evt)) {
		// Skip the event so that wxSTC's default mouse wheel handler is hit
		evt.Skip();
	}
}
