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
#include "subs_edit_ctrl.h"
#include "subs_edit_box.h"
#include "options.h"


////////////////////////
// Edit box constructor
SubsTextEditCtrl::SubsTextEditCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& wsize, long style, const wxValidator& validator, const wxString& name)
: wxScintilla(parent, id, pos, wsize, 0, value)
{
	// Set properties
	SetWrapMode(wxSCI_WRAP_WORD);
	SetMarginWidth(1,0);
	CmdKeyClear(wxSCI_KEY_RETURN,wxSCI_SCMOD_CTRL);
	CmdKeyClear(wxSCI_KEY_RETURN,wxSCI_SCMOD_NULL);
	CmdKeyClear(wxSCI_KEY_TAB,wxSCI_SCMOD_NULL);
	CmdKeyClear(wxSCI_KEY_TAB,wxSCI_SCMOD_SHIFT);
	CmdKeyClear('D',wxSCI_SCMOD_CTRL);
	CmdKeyClear('L',wxSCI_SCMOD_CTRL);
	CmdKeyClear('L',wxSCI_SCMOD_CTRL | wxSCI_SCMOD_SHIFT);
	CmdKeyClear('T',wxSCI_SCMOD_CTRL);
	CmdKeyClear('T',wxSCI_SCMOD_CTRL | wxSCI_SCMOD_SHIFT);
	CmdKeyClear('U',wxSCI_SCMOD_CTRL);

	// Styles
	wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	wxString fontname = Options.AsText(_T("Font Face"));
	if (fontname != _T("")) font.SetFaceName(fontname);
	int size = Options.AsInt(_T("Font Size"));

	// Normal style
	StyleSetFont(0,font);
	StyleSetSize(0,size);
	StyleSetForeground(0,Options.AsColour(_T("Syntax Highlight Normal")));

	// Brackets style
	StyleSetFont(1,font);
	StyleSetSize(1,size);
	StyleSetForeground(1,Options.AsColour(_T("Syntax Highlight Brackets")));

	// Slashes/Parenthesis/Comma style
	StyleSetFont(2,font);
	StyleSetSize(2,size);
	StyleSetForeground(2,Options.AsColour(_T("Syntax Highlight Slashes")));

	// Tags style
	StyleSetFont(3,font);
	StyleSetSize(3,size);
	StyleSetBold(3,true);
	StyleSetForeground(3,Options.AsColour(_T("Syntax Highlight Tags")));

	// Error style
	StyleSetFont(4,font);
	StyleSetSize(4,size);
	StyleSetForeground(4,Options.AsColour(_T("Syntax Highlight Error")));

	// Tag Number Parameters style
	StyleSetFont(5,font);
	StyleSetSize(5,size);
	StyleSetForeground(5,Options.AsColour(_T("Syntax Highlight Numbers")));

	// Set spellchecker
	spellchecker = SpellChecker::GetSpellChecker();
}


//////////////
// Destructor
SubsTextEditCtrl::~SubsTextEditCtrl() {
	delete spellchecker;
	spellchecker = NULL;
}


/////////////////
// Style a range
void SubsTextEditCtrl::UpdateStyle(int start, int _length) {
	// Styling enabled?
	if (Options.AsBool(_T("Syntax Highlight Enabled")) == 0) return;

	// Set variables
	wxString text = GetText();
	int len = _length;
	if (len < 0) len = text.Length();

	// Begin styling
	StartStyling(0,31);
	int ran = 0;
	int depth = 0;
	int curStyle = 0;
	wxChar curChar = 0;
	wxChar prevChar = 0;

	// Loop through
	for (int i=start;i<len;i++) {
		// Current/previous characters
		prevChar = curChar;
		curChar = text[i];

		// Erroneous
		if (depth < 0 || depth > 1) {
			SetStyling(ran,curStyle);
			ran = 0;
			curStyle = 4;
		}

		// Start override block
		if (curChar == _T('{') && depth >= 0) {
			SetStyling(ran,curStyle);
			ran = 0;
			depth++;
			if (depth == 1) curStyle = 1;
			else curStyle = 4;
		}

		// End override block
		else if (curChar == _T('}') && depth <= 1) {
			SetStyling(ran,curStyle);
			ran = 0;
			depth--;
			if (depth == 0) curStyle = 1;
			else curStyle = 4;
		}

		// Outside
		else if (depth == 0 && curStyle != 0) {
			SetStyling(ran,curStyle);
			ran = 0;
			curStyle = 0;
		}

		// Inside
		else if (depth == 1) {
			// Special character
			if (curChar == _T('\\') || curChar == _T('(') || curChar == _T(')') || curChar == _T(',')) {
				if (curStyle != 2) {
					SetStyling(ran,curStyle);
					ran = 0;
					curStyle = 2;
				}
			}

			// Number
			else if ((curChar >= '0' && curChar <= '9') || curChar == '.' || curChar == '&' || curChar == '+' || curChar == '-' || (curChar == 'H' && prevChar == '&')) {
				if (curStyle != 5) {
					SetStyling(ran,curStyle);
					ran = 0;
					curStyle = 5;
				}
			}

			// Tag name
			else if (curStyle != 3) {
				SetStyling(ran,curStyle);
				ran = 0;
				curStyle = 3;
			}
		}

		// Increase ran length
		ran++;
	}
	SetStyling(ran,curStyle);
}


///////////////////////////
// Set text to a new value
void SubsTextEditCtrl::SetTextTo(const wxString _text) {
	// Setup
	control->textEditReady = false;
	Freeze();
	wxString text = _text;
	text.Replace(_T("\r\n"),_T("\\N"));
	text.Replace(_T("\n\r"),_T("\\N"));
	text.Replace(_T("\r"),_T("\\N"));
	text.Replace(_T("\n"),_T("\\N"));

	// Prepare
	int from=0,to=0;
	GetSelection(&from,&to);
	Clear();

	// Set text
	SetText(text);

	// Style
	UpdateStyle();

	// Restore selection
	SetSelection(from,to);

	// Finish
	Thaw();
	control->textEditReady = true;
}


///////////////////////
// Control event table
BEGIN_EVENT_TABLE(SubsTextEditCtrl,wxScintilla)
	EVT_MOUSE_EVENTS(SubsTextEditCtrl::OnMouseEvent)
END_EVENT_TABLE()


///////////////
// Mouse event
void SubsTextEditCtrl::OnMouseEvent(wxMouseEvent &event) {
	// Right click
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		if (control->linen >= 0) {
			// Popup
			wxMenu menu;
			menu.Append(EDIT_MENU_UNDO,_("&Undo"))->Enable(CanUndo());
			menu.AppendSeparator();
			menu.Append(EDIT_MENU_CUT,_("Cu&t"))->Enable(GetSelectionStart()-GetSelectionEnd() != 0);
			menu.Append(EDIT_MENU_COPY,_("&Copy"))->Enable(GetSelectionStart()-GetSelectionEnd() != 0);
			menu.Append(EDIT_MENU_PASTE,_("&Paste"))->Enable(CanPaste());
			menu.AppendSeparator();
			menu.Append(EDIT_MENU_SELECT_ALL,_("Select &All"));
			menu.AppendSeparator();
			menu.Append(EDIT_MENU_SPLIT_PRESERVE,_("Split at cursor (preserve times)"));
			menu.Append(EDIT_MENU_SPLIT_ESTIMATE,_("Split at cursor (estimate times)"));
			PopupMenu(&menu);
			return;
		}
	}

	event.Skip();
}
