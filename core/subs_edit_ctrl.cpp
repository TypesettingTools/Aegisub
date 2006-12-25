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
#include "subs_grid.h"
#include "utils.h"


////////////////////////
// Edit box constructor
SubsTextEditCtrl::SubsTextEditCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& wsize, long style, const wxValidator& validator, const wxString& name)
: wxScintilla(parent, id, pos, wsize, 0, value)
{
	// Set properties
	SetWrapMode(wxSCI_WRAP_WORD);
	SetMarginWidth(1,0);

	// Set hotkeys
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

	// Line breaks style
	StyleSetFont(6,font);
	StyleSetSize(6,size);
	StyleSetBold(6,true);
	StyleSetForeground(6,Options.AsColour(_T("Syntax Highlight Line Break")));

	// Misspelling indicator
	IndicatorSetStyle(0,wxSCI_INDIC_SQUIGGLE);
	IndicatorSetForeground(0,wxColour(255,0,0));

	// Set spellchecker
	spellchecker = SpellChecker::GetSpellChecker();
	
	// Delimiters
	delim = _T(" .,;:!?¿¡(){}[]\"/\\");
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
	int end = start + _length;
	if (_length < 0) end = text.Length();

	// Begin styling
	StartStyling(0,255);
	int ran = 0;
	int depth = 0;
	int curStyle = 0;
	int curPos = 0;
	wxChar curChar = 0;
	wxChar prevChar = 0;
	wxChar nextChar = 0;

	// Loop through
	for (int i=start;i<end;i++) {
		// Current/previous characters
		prevChar = curChar;
		curChar = text[i];
		if (i<end-1) nextChar = text[i+1];
		else nextChar = 0;

		// Erroneous
		if (depth < 0 || depth > 1) {
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = 0;
			curStyle = 4;
		}

		// Start override block
		if (curChar == _T('{') && depth >= 0) {
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = 0;
			depth++;
			if (depth == 1) curStyle = 1;
			else curStyle = 4;
		}

		// End override block
		else if (curChar == _T('}') && depth <= 1) {
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = 0;
			depth--;
			if (depth == 0) curStyle = 1;
			else curStyle = 4;
		}

		// Outside
		else if (depth == 0) {
			// Is \n, \N or \h?
			if (curChar == _T('\\') && (nextChar == 'n' || nextChar == 'N' || nextChar == 'h')) {
				SetUnicodeStyling(curPos,ran,curStyle);
				curPos += ran + 1;
				ran = 1;
				curStyle = 6;
				i++;
			}

			// Normal text
			else if (curStyle != 0) {
				SetUnicodeStyling(curPos,ran,curStyle);
				curPos += ran;
				ran = 0;
				curStyle = 0;
			}
		}

		// Inside
		else if (depth == 1) {
			// Special character
			if (curChar == _T('\\') || curChar == _T('(') || curChar == _T(')') || curChar == _T(',')) {
				if (curStyle != 2) {
					SetUnicodeStyling(curPos,ran,curStyle);
					curPos += ran;
					ran = 0;
					curStyle = 2;
				}
			}

			// Number
			else if ((curChar >= '0' && curChar <= '9') || curChar == '.' || curChar == '&' || curChar == '+' || curChar == '-' || (curChar == 'H' && prevChar == '&')) {
				if (curStyle != 5) {
					SetUnicodeStyling(curPos,ran,curStyle);
					curPos += ran;
					ran = 0;
					curStyle = 5;
				}
			}

			// Tag name
			else if (curStyle != 3) {
				SetUnicodeStyling(curPos,ran,curStyle);
				curPos += ran;
				ran = 0;
				curStyle = 3;
			}
		}

		// Increase ran length
		ran++;
	}
	SetUnicodeStyling(curPos,ran,curStyle);

	// Spell check
	StyleSpellCheck(start,_length);
}


////////////////////////
// Unicode-safe styling
void SubsTextEditCtrl::SetUnicodeStyling(int start,int length,int style) {
	// Get the real length
	wxString string = GetText().Mid(start,length);
	wxCharBuffer buffer = string.mb_str(wxConvUTF8);
	const char* utf8str = buffer;
	int len = strlen(utf8str);

	// Set styling
	SetStyling(len,style);
}


///////////////
// Spell check
void SubsTextEditCtrl::StyleSpellCheck(int start, int len) {
	// See if it has a spellchecker
	if (!spellchecker) return;

	// Variables
	wxChar cur;
	wxString text = GetText();
	int curPos;
	int lastpos = -1;
	int end = start+len;
	int depth = 0;
	if (len < 0) end = text.Length();
	wxArrayInt startPos;
	wxArrayInt endPos;
	bool isDelim;

	// Scan
	for (int i=start;i<end+1;i++) {
		// Current character
		curPos = i;
		if (i < end) cur = text[i];
		else cur = '.';
		isDelim = false;

		// Increase depth
		if (cur == '{') {
			depth++;
			if (depth == 1) {
				if (lastpos+1 != curPos) {
					startPos.Add(lastpos+1);
					endPos.Add(curPos);
				}
				continue;
			}
		}

		// Decrease depth
		if (cur == '}') {
			depth--;
			if (depth == 0) {
				lastpos = i;
				continue;
			}
		}

		// Wrong depth
		if (depth != 0) continue;

		// Check if it is \n or \N
		if (cur == '\\' && i < end-1 && (text[i+1] == 'N' || text[i+1] == 'n' || text[i+1] == 'h')) {
			isDelim = true;
			i++;
		}

		// Check for standard delimiters
		if (delim.Find(cur) != wxNOT_FOUND) {
			isDelim = true;
		}

		// Is delimiter?
		if (isDelim) {
			if (lastpos+1 != curPos) {
				startPos.Add(lastpos+1);
				endPos.Add(curPos);
			}
			lastpos = i;
		}
	}

	// Style
	int count = startPos.Count();
	for (int i=0;i<count;i++) {
		// Get current word
		wxString curWord = text.Mid(startPos[i],endPos[i]-startPos[i]);

		// Check if it's valid
		if (!spellchecker->CheckWord(curWord)) {
			// Get length before it
			wxString string = GetText().Left(startPos[i]);
			wxCharBuffer buffer = string.mb_str(wxConvUTF8);
			const char* utf8str = buffer;
			int utf8len = strlen(utf8str);

			// Set styling
			StartStyling(utf8len,32);
			SetUnicodeStyling(startPos[i],endPos[i]-startPos[i],32);
		}
	}
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

	EVT_MENU(EDIT_MENU_SPLIT_PRESERVE,SubsTextEditCtrl::OnSplitLinePreserve)
	EVT_MENU(EDIT_MENU_SPLIT_ESTIMATE,SubsTextEditCtrl::OnSplitLineEstimate)
	EVT_MENU(EDIT_MENU_CUT,SubsTextEditCtrl::OnCut)
	EVT_MENU(EDIT_MENU_COPY,SubsTextEditCtrl::OnCopy)
	EVT_MENU(EDIT_MENU_PASTE,SubsTextEditCtrl::OnPaste)
	EVT_MENU(EDIT_MENU_UNDO,SubsTextEditCtrl::OnUndo)
	EVT_MENU(EDIT_MENU_SELECT_ALL,SubsTextEditCtrl::OnSelectAll)
	EVT_MENU(EDIT_MENU_ADD_TO_DICT,SubsTextEditCtrl::OnAddToDictionary)
	EVT_MENU_RANGE(EDIT_MENU_SUGGESTIONS,EDIT_MENU_SUGGESTIONS+16,SubsTextEditCtrl::OnUseSuggestion)
END_EVENT_TABLE()


///////////////
// Mouse event
void SubsTextEditCtrl::OnMouseEvent(wxMouseEvent &event) {
	// Right click
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		if (control->linen >= 0) {
			int pos = PositionFromPoint(event.GetPosition());
			ShowPopupMenu(pos);
			return;
		}
	}

	event.Skip();
}


///////////////////
// Show popup menu
void SubsTextEditCtrl::ShowPopupMenu(int activePos) {
	// Menu
	wxMenu menu;

	// Position
	if (activePos == -1) activePos = GetCurrentPos();

	// Spell check
	int style = GetStyleAt(activePos);
	if (style & 32 && spellchecker) {
		// Get word
		currentWord = GetWordAtPosition(activePos);
		currentWordPos = activePos;
		sugs.Clear();

		// Set font
		wxFont font;
		font.SetWeight(wxFONTWEIGHT_BOLD);

		// Word is really a typo
		if (!spellchecker->CheckWord(currentWord)) {
			// Get suggestions
			sugs = spellchecker->GetSuggestions(currentWord);

			// Build menu
			int nSugs = sugs.Count();
			for (int i=0;i<nSugs;i++) menu.Append(EDIT_MENU_SUGGESTIONS+i,sugs[i])->SetFont(font);

			// No suggestions
			if (!nSugs) menu.Append(EDIT_MENU_SUGGESTION,_("No correction suggestions"))->Enable(false);

			// Append "add word"
			menu.Append(EDIT_MENU_ADD_TO_DICT,wxString::Format(_("Add \"%s\" to dictionary"),currentWord.c_str()));
			menu.AppendSeparator();
		}
	}

	// Standard actions
	menu.Append(EDIT_MENU_UNDO,_("&Undo"))->Enable(CanUndo());
	menu.AppendSeparator();
	menu.Append(EDIT_MENU_CUT,_("Cu&t"))->Enable(GetSelectionStart()-GetSelectionEnd() != 0);
	menu.Append(EDIT_MENU_COPY,_("&Copy"))->Enable(GetSelectionStart()-GetSelectionEnd() != 0);
	menu.Append(EDIT_MENU_PASTE,_("&Paste"))->Enable(CanPaste());
	menu.AppendSeparator();
	menu.Append(EDIT_MENU_SELECT_ALL,_("Select &All"));

	// Split
	menu.AppendSeparator();
	menu.Append(EDIT_MENU_SPLIT_PRESERVE,_("Split at cursor (preserve times)"));
	menu.Append(EDIT_MENU_SPLIT_ESTIMATE,_("Split at cursor (estimate times)"));

	// Pop the menu
	PopupMenu(&menu);
}


//////////////////////////////////////
// Get boundaries of word at position
void SubsTextEditCtrl::GetBoundsOfWordAtPosition(int pos,int &_start,int &_end) {
	// Variables
	wxString text = GetText();
	int len = text.Length();
	int lastDelimBefore = -1;
	int firstDelimAfter = len;
	wxChar cur,next;
	int depth=0;

	// Scan for delimiters
	for (int i=0;i<len;i++) {
		// Current char
		cur = text[i];
		if (i<len-1) next = text[i+1];
		else next = 0;

		// Depth
		if (cur == '{') depth++;
		if (cur == '}') depth--;
		if (depth != 0) continue;

		// Line breaks
		if (cur == '\\' && (next == 'N' || next == 'n' || next == 'h')) {
			// Before
			if (i < pos) {
				i++;
				lastDelimBefore = i;
				continue;
			}
		}

		// Check for delimiters
		if (delim.Find(cur) != wxNOT_FOUND) {
			// Before
			if (i < pos) lastDelimBefore = i;
			
			// After
			else {
				firstDelimAfter = i;
				break;
			}
		}
	}

	// Set start and end
	_start = lastDelimBefore+1;
	_end = firstDelimAfter-1;
}


//////////////////////////////////
// Get word at specified position
wxString SubsTextEditCtrl::GetWordAtPosition(int pos) {
	int start,end;
	GetBoundsOfWordAtPosition(pos,start,end);
	return GetText().Mid(start,end-start+1);
}


///////////////////////////////
// Split line preserving times
void SubsTextEditCtrl::OnSplitLinePreserve (wxCommandEvent &event) {
	int from,to;
	GetSelection(&from, &to);
	control->grid->SplitLine(control->linen,from,0);
}


///////////////////////////////
// Split line estimating times
void SubsTextEditCtrl::OnSplitLineEstimate (wxCommandEvent &event) {
	int from,to;
	GetSelection(&from, &to);
	control->grid->SplitLine(control->linen,from,1);
}


///////
// Cut
void SubsTextEditCtrl::OnCut(wxCommandEvent &event) {
	Cut();
}


////////
// Copy
void SubsTextEditCtrl::OnCopy(wxCommandEvent &event) {
	Copy();
}


/////////
// Paste
void SubsTextEditCtrl::OnPaste(wxCommandEvent &event) {
	Paste();
}


////////
// Undo
void SubsTextEditCtrl::OnUndo(wxCommandEvent &event) {
	Undo();
}


//////////////
// Select All
void SubsTextEditCtrl::OnSelectAll(wxCommandEvent &event) {
	SelectAll();
}


//////////////////////////
// Add word to dictionary
void SubsTextEditCtrl::OnAddToDictionary(wxCommandEvent &event) {
	if (spellchecker) spellchecker->AddWord(currentWord);
	SetFocus();
}


//////////////////
// Use suggestion
void SubsTextEditCtrl::OnUseSuggestion(wxCommandEvent &event) {
	// Get suggestion
	wxString suggestion = sugs[event.GetId()-EDIT_MENU_SUGGESTIONS];
	
	// Get boundaries of text being replaced
	int start,end;
	GetBoundsOfWordAtPosition(currentWordPos,start,end);

	// Replace
	wxString text = GetText();
	SetText(text.Left(MAX(0,start)) + suggestion + text.Mid(end+1));

	// Set selection
	SetSelection(start,start+suggestion.Length());
	SetFocus();
}
