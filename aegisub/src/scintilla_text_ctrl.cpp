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
//
// $Id$

/// @file scintilla_text_ctrl.cpp
/// @brief Customised version of wxStyledTextControl used for main edit box
/// @ingroup custom_control
///


////////////
// Includes
#include "config.h"

#include "scintilla_text_ctrl.h"
#include "utils.h"



/// @brief Constructor 
/// @param parent    
/// @param id        
/// @param value     
/// @param pos       
/// @param size      
/// @param style     
/// @param validator 
/// @param name      
///
ScintillaTextCtrl::ScintillaTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
: wxStyledTextCtrl(parent, id, pos, size, 0, value)
{
	//SetWindowStyle(style);
}



/// @brief Destructor 
///
ScintillaTextCtrl::~ScintillaTextCtrl() {
}



/// @brief Get unicode-compatible position 
/// @param pos 
/// @return 
///
int ScintillaTextCtrl::GetUnicodePosition(int pos) {
	wxString string = GetText().Left(pos);
	wxCharBuffer buffer = string.mb_str(wxConvUTF8);
	return strlen(buffer);
}



/// @brief Reverse unicode-compatible position 
/// @param pos 
/// @return 
///
int ScintillaTextCtrl::GetReverseUnicodePosition(int pos) {
	// Get UTF8
	wxCharBuffer buffer = GetText().mb_str(wxConvUTF8);

	// Limit position to it
	if (pos > (signed)strlen(buffer)) pos = strlen(buffer);

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
/// @param start 
/// @param mask  
///
void ScintillaTextCtrl::StartUnicodeStyling(int start,int mask) {
	StartStyling(GetUnicodePosition(start),mask);
}



/// @brief Unicode-safe styling 
/// @param start  
/// @param length 
/// @param style  
///
void ScintillaTextCtrl::SetUnicodeStyling(int start,int length,int style) {
	// Get the real length
	wxString string = GetText().Mid(start,length);
	wxCharBuffer buffer = string.mb_str(wxConvUTF8);
	int len = strlen(buffer);

	// Set styling
	SetStyling(len,style);
}



/// @brief Get boundaries of word at position 
/// @param pos    
/// @param _start 
/// @param _end   
/// @return 
///
void ScintillaTextCtrl::GetBoundsOfWordAtPosition(int pos,int &_start,int &_end) {
	// Results
	IntPairVector results;
	GetWordBoundaries(GetText(),results);

	// Get boundaries
	int count = results.size();
	for (int i=0;i<count;i++) {
		if (results[i].first <= pos && results[i].second >= pos) {
			_start = results[i].first;
			_end = results[i].second-1;
			return;
		}
	}
}



/// @brief Get word at specified position 
/// @param pos 
/// @return 
///
wxString ScintillaTextCtrl::GetWordAtPosition(int pos) {
	int start,end;
	GetBoundsOfWordAtPosition(pos,start,end);
	return GetText().Mid(start,end-start+1);
}



/// @brief Set selection, unicode-aware 
/// @param start 
/// @param end   
///
void ScintillaTextCtrl::SetSelectionU(int start, int end) {
	SetSelection(GetUnicodePosition(start),GetUnicodePosition(end));
}


