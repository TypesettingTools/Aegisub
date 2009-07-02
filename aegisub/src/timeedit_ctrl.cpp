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


////////////
// Includes
#include "config.h"

#include <wx/clipbrd.h>
#include <wx/valtext.h>
#include <wx/menu.h>
#include "timeedit_ctrl.h"
#include "ass_time.h"
#include "vfr.h"
#include "options.h"

// Use the multiline style only on wxGTK to workaround some wxGTK bugs with the default singleline style
#ifdef __WXGTK__
#define TimeEditWindowStyle wxTE_MULTILINE | wxTE_CENTRE
#else
#define TimeEditWindowStyle wxTE_CENTRE
#endif

///////////////
// Constructor
TimeEdit::TimeEdit(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
wxTextCtrl(parent,id,value,pos,size,TimeEditWindowStyle | style,validator,name)
{
	// Set validator
	wxTextValidator val(wxFILTER_INCLUDE_CHAR_LIST);
	wxArrayString includes;
	includes.Add(_T("0"));
	includes.Add(_T("1"));
	includes.Add(_T("2"));
	includes.Add(_T("3"));
	includes.Add(_T("4"));
	includes.Add(_T("5"));
	includes.Add(_T("6"));
	includes.Add(_T("7"));
	includes.Add(_T("8"));
	includes.Add(_T("9"));
	includes.Add(_T("."));
	includes.Add(_T(":"));
	val.SetIncludes(includes);
	SetValidator(val);

	// Other stuff
	if (!value) SetValue(time.GetASSFormated());
	// This is a multiline control on wxGTK so we need to size it manually there
#ifdef __WXGTK__ 
	int w, h;
	GetTextExtent(GetValue(),&w,&h);
	w += 20;
	h += 5;
	SetSizeHints(w,h,w,h);
#endif
	ready = true;
	byFrame = false;
	isEnd = false;
	modified = false;
	showModified = false;
	UpdateTime();
	Connect(wxEVT_COMMAND_TEXT_UPDATED,wxCommandEventHandler(TimeEdit::OnModified));
	Connect(wxEVT_KILL_FOCUS,wxFocusEventHandler(TimeEdit::OnKillFocus));
}


///////////////
// Event table
BEGIN_EVENT_TABLE(TimeEdit, wxTextCtrl)
	EVT_MOUSE_EVENTS(TimeEdit::OnMouseEvent)
	EVT_KEY_DOWN(TimeEdit::OnKeyDown)
	EVT_MENU(Time_Edit_Copy,TimeEdit::OnCopy)
	EVT_MENU(Time_Edit_Paste,TimeEdit::OnPaste)
END_EVENT_TABLE()


//////////////////
// Modified event
void TimeEdit::OnModified(wxCommandEvent &event) {
	event.Skip();
	if (!ready) return;
	Modified();
}


/////////////////////
// Modified function
void TimeEdit::Modified(bool byUser) {
	// Lock
	if (!ready) return;
	ready = false;
	
	// Update
	if (byFrame) Update();
	else UpdateTime(byUser);

	// Colour
	if (showModified && !modified) {
		SetBackgroundColour(Options.AsColour(_T("Edit Box Need Enter Background")));
	}
	modified = true;

	// Done
	ready = true;
}


/////////////////////////////
// Set time and update stuff
void TimeEdit::SetTime(int ms,bool setModified) {
	int oldMs = time.GetMS();
	time.SetMS(ms);
	UpdateText();
	if (setModified && oldMs != ms) Modified(false);
}


/////////////////////////////////////////
// Toggles between set by frame and time
void TimeEdit::SetByFrame(bool enable) {
	if (enable == byFrame) return;

	// By frames
	if (enable) {
		if (VFR_Output.IsLoaded()) {
			byFrame = true;
			UpdateText();
		}
	}

	// By actual time
	else {
		byFrame = false;
		UpdateText();
	}
}


/////////////////////////////////////
// Update text to reflect time value
void TimeEdit::UpdateText() {
	ready = false;
	if (byFrame) {
		int frame_n = VFR_Output.GetFrameAtTime(time.GetMS(),!isEnd);
		SetValue(wxString::Format(_T("%i"),frame_n));
	}
	else SetValue(time.GetASSFormated());
	ready = true;
}


//////////
// Update
void TimeEdit::Update() {
	// Update frame
	if (byFrame) {
		long temp;
		GetValue().ToLong(&temp);
		time.SetMS(VFR_Output.GetTimeAtFrame(temp,!isEnd));
	}

	// Update time if not on insertion mode
	else if (!Options.AsBool(_T("Insert Mode on Time Boxes"))) {
		UpdateTime();
		SetValue(time.GetASSFormated());
	}

	// Update modified status
	if (modified && showModified) {
		SetBackgroundColour(wxNullColour);
		Refresh();
	}
	modified = false;
}


/////////////////////////////////////////////////
// Reads value from a text control and update it
void TimeEdit::UpdateTime(bool byUser) {
	bool insertion = Options.AsBool(_T("Insert Mode on Time Boxes"));
	wxString text = GetValue();
	long start=0,end=0;
	if (insertion && byUser) {
		GetSelection(&start,&end);
		if (start == end) {
			wxString nextChar = text.Mid(start,1);
			if (nextChar == _T(":") || nextChar == _T(".")) {
				wxString temp = text;
				text = temp.Left(start-1);
				text += nextChar;
				text += temp.Mid(start-1,1);
				text += temp.Mid(start+2);
				start++;
				end++;
			}
			else if (nextChar.IsEmpty()) text.Remove(start-1,1);
			else text.Remove(start,1);
		}
	}

	// Update time
	time.ParseASS(text);
	if (insertion) {
		SetValue(time.GetASSFormated());
		SetSelection(start,end);
	}
}


///////////////
// Key pressed
void TimeEdit::OnKeyDown(wxKeyEvent &event) {
	// Get key ID
	int key = event.GetKeyCode();
	bool insertMode = Options.AsBool(_T("Insert Mode on Time Boxes"));
	Refresh();

	// Check if it's an acceptable key
#ifdef __APPLE__
	if (!event.CmdDown()) {
#else
	if (!event.ControlDown()) {
#endif
		if (byFrame || !insertMode || (key != WXK_BACK && key != WXK_DELETE)) {
			// Reset selection first, if necessary
			if (!byFrame && insertMode) {
				long from=0,to=0;
				GetSelection(&from,&to);
				if (to != from) SetSelection(from,from);
			}

			// Allow it through
			event.Skip();
		}
	}

	else {
		// Copy
		if (key == 'C' || key == 'X') {
			CopyTime();
		}

		// Paste
		if (key == 'V') {
			PasteTime();
		}
	}
}


//////////////
// Focus lost
void TimeEdit::OnKillFocus(wxFocusEvent &event) {
	if (!byFrame && !Options.AsBool(_T("Insert Mode on Time Boxes"))) {
		if (time.GetASSFormated() != GetValue()) {
			UpdateTime();
			SetValue(time.GetASSFormated());
		}
	}
	event.Skip();
}


///// Mouse/copy/paste events down here /////

///////////////
// Mouse event
void TimeEdit::OnMouseEvent(wxMouseEvent &event) {
	// Right click context menu
	if (event.RightUp()) {
		if (!byFrame && Options.AsBool(_T("Insert Mode on Time Boxes"))) {
			wxMenu menu;
			menu.Append(Time_Edit_Copy,_T("&Copy"));
			menu.Append(Time_Edit_Paste,_T("&Paste"));
			PopupMenu(&menu);
			return;
		}
	}

	// Allow other events through
	event.Skip();
}


/////////////
// Menu Copy
void TimeEdit::OnCopy(wxCommandEvent &event) {
	SetFocus();
	SetSelection(0,GetValue().Length());
	CopyTime();
	Refresh();
}


//////////////
// Menu Paste
void TimeEdit::OnPaste(wxCommandEvent &event) {
	SetFocus();
	PasteTime();
	Refresh();
}


/////////////////////
// Copy to clipboard
void TimeEdit::CopyTime() {
	// Frame
	if (byFrame) {
		Copy();
		return;
	}

	// Time
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(GetStringSelection()));
		wxTheClipboard->Close();
	}
}


////////////////////////
// Paste from clipboard
void TimeEdit::PasteTime() {
	// Frame
	if (byFrame) {
		Paste();
		return;
	}

	// Time
	if (wxTheClipboard->Open()) {
		// Read text
		wxString text;
		if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			text = data.GetText();
			text.Trim(false).Trim(true);
		}
		wxTheClipboard->Close();

		// Paste time
		AssTime tempTime;
		tempTime.ParseASS(text);
		if (tempTime.GetASSFormated() == text) {
			ready = false;
			SetValue(text);
			SetSelection(0,GetValue().Length());
			ready = true;
			Modified();
		}
	}
}
