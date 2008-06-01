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
#include <wx/wxprec.h>
#include <wx/intl.h>
#include "subs_edit_ctrl.h"
#include "subs_edit_box.h"
#include "options.h"
#include "subs_grid.h"
#include "utils.h"
#include "ass_dialogue.h"


////////////////////////
// Edit box constructor
SubsTextEditCtrl::SubsTextEditCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& wsize, long style, const wxValidator& validator, const wxString& name)
: ScintillaTextCtrl(parent, id, value, pos, wsize, style, validator, name)
{
	// Set properties
	SetWrapMode(wxSTC_WRAP_WORD);
	SetMarginWidth(1,0);
	UsePopUp(false);
	SetStyles();

	// Set hotkeys
	CmdKeyClear(wxSTC_KEY_RETURN,wxSTC_SCMOD_CTRL);
	CmdKeyClear(wxSTC_KEY_RETURN,wxSTC_SCMOD_NORM);
	CmdKeyClear(wxSTC_KEY_TAB,wxSTC_SCMOD_NORM);
	CmdKeyClear(wxSTC_KEY_TAB,wxSTC_SCMOD_SHIFT);
	CmdKeyClear('D',wxSTC_SCMOD_CTRL);
	CmdKeyClear('L',wxSTC_SCMOD_CTRL);
	CmdKeyClear('L',wxSTC_SCMOD_CTRL | wxSTC_SCMOD_SHIFT);
	CmdKeyClear('T',wxSTC_SCMOD_CTRL);
	CmdKeyClear('T',wxSTC_SCMOD_CTRL | wxSTC_SCMOD_SHIFT);
	CmdKeyClear('U',wxSTC_SCMOD_CTRL);

	// Set spellchecker
	spellchecker = SpellCheckerFactoryManager::GetSpellChecker();

	// Set thesaurus
	thesaurus = Thesaurus::GetThesaurus();
	
	// Prototypes for call tips
	tipProtoN = -1;
	proto.Add(_T("move(x1,y1,x2,y2)"));
	proto.Add(_T("move(x1,y1,x2,y2,startTime,endTime)"));
	proto.Add(_T("fn;FontName"));
	proto.Add(_T("bord;Width"));
	proto.Add(_T("shad;Depth"));
	proto.Add(_T("be;1/0"));
	proto.Add(_T("fscx;Scale"));
	proto.Add(_T("fscy;Scale"));
	proto.Add(_T("fsp;Spacing"));
	proto.Add(_T("fs;FontSize"));
	proto.Add(_T("fe;Encoding"));
	proto.Add(_T("frx;Angle"));
	proto.Add(_T("fry;Angle"));
	proto.Add(_T("frz;Angle"));
	proto.Add(_T("fr;Angle"));
	proto.Add(_T("pbo;Offset"));
	proto.Add(_T("clip(command)"));
	proto.Add(_T("clip(scale,command)"));
	proto.Add(_T("clip(x1,y1,x2,y2)"));
	proto.Add(_T("t(acceleration,tags)"));
	proto.Add(_T("t(startTime,endTime,tags)"));
	proto.Add(_T("t(startTime,endTime,acceleration,tags)"));
	proto.Add(_T("pos(x,y)"));
	proto.Add(_T("p;Exponent"));
	proto.Add(_T("org(x,y)"));
	proto.Add(_T("fade(startAlpha,middleAlpha,endAlpha,startIn,endIn,startOut,endOut)"));
	proto.Add(_T("fad(startTime,endTime)"));
	proto.Add(_T("c;Colour"));
	proto.Add(_T("1c;Colour"));
	proto.Add(_T("2c;Colour"));
	proto.Add(_T("3c;Colour"));
	proto.Add(_T("4c;Colour"));
	proto.Add(_T("alpha;Alpha"));
	proto.Add(_T("1a;Alpha"));
	proto.Add(_T("2a;Alpha"));
	proto.Add(_T("3a;Alpha"));
	proto.Add(_T("4a;Alpha"));
	proto.Add(_T("an;Alignment"));
	proto.Add(_T("a;Alignment"));
	proto.Add(_T("b;Weight"));
	proto.Add(_T("i;1/0"));
	proto.Add(_T("u;1/0"));
	proto.Add(_T("s;1/0"));
	proto.Add(_T("kf;Duration"));
	proto.Add(_T("ko;Duration"));
	proto.Add(_T("k;Duration"));
	proto.Add(_T("K;Duration"));
	proto.Add(_T("q;WarpStyle"));
	proto.Add(_T("r;Style"));
}


//////////////
// Destructor
SubsTextEditCtrl::~SubsTextEditCtrl() {
	delete spellchecker;
	spellchecker = NULL;
	delete thesaurus;
	thesaurus = NULL;
}


///////////////////////
// Control event table
BEGIN_EVENT_TABLE(SubsTextEditCtrl,wxStyledTextCtrl)
	EVT_MOUSE_EVENTS(SubsTextEditCtrl::OnMouseEvent)
	EVT_KILL_FOCUS(SubsTextEditCtrl::OnLoseFocus)

	EVT_MENU(EDIT_MENU_SPLIT_PRESERVE,SubsTextEditCtrl::OnSplitLinePreserve)
	EVT_MENU(EDIT_MENU_SPLIT_ESTIMATE,SubsTextEditCtrl::OnSplitLineEstimate)
	EVT_MENU(EDIT_MENU_CUT,SubsTextEditCtrl::OnCut)
	EVT_MENU(EDIT_MENU_COPY,SubsTextEditCtrl::OnCopy)
	EVT_MENU(EDIT_MENU_PASTE,SubsTextEditCtrl::OnPaste)
	EVT_MENU(EDIT_MENU_UNDO,SubsTextEditCtrl::OnUndo)
	EVT_MENU(EDIT_MENU_SELECT_ALL,SubsTextEditCtrl::OnSelectAll)
	EVT_MENU(EDIT_MENU_ADD_TO_DICT,SubsTextEditCtrl::OnAddToDictionary)
	EVT_MENU_RANGE(EDIT_MENU_SUGGESTIONS,EDIT_MENU_THESAURUS-1,SubsTextEditCtrl::OnUseSuggestion)
	EVT_MENU_RANGE(EDIT_MENU_THESAURUS_SUGS,EDIT_MENU_DIC_LANGUAGE-1,SubsTextEditCtrl::OnUseThesaurusSuggestion)
	EVT_MENU_RANGE(EDIT_MENU_DIC_LANGS,EDIT_MENU_THES_LANGUAGE-1,SubsTextEditCtrl::OnSetDicLanguage)
	EVT_MENU_RANGE(EDIT_MENU_THES_LANGS,EDIT_MENU_THES_LANGS+100,SubsTextEditCtrl::OnSetThesLanguage)
END_EVENT_TABLE()


//////////////
// Lose focus
void SubsTextEditCtrl::OnLoseFocus(wxFocusEvent &event) {
	CallTipCancel();
	event.Skip();
}


//////////////
// Set styles
void SubsTextEditCtrl::SetStyles() {
	// Styles
	wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	wxString fontname = Options.AsText(_T("Edit Font Face"));
	if (fontname != _T("")) font.SetFaceName(fontname);
	int size = Options.AsInt(_T("Edit Font Size"));

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
	StyleSetBackground(4,Options.AsColour(_T("Syntax Highlight Error Background")));

	// Tag Parameters style
	StyleSetFont(5,font);
	StyleSetSize(5,size);
	StyleSetForeground(5,Options.AsColour(_T("Syntax Highlight Parameters")));

	// Line breaks style
	StyleSetFont(6,font);
	StyleSetSize(6,size);
	StyleSetBold(6,true);
	StyleSetForeground(6,Options.AsColour(_T("Syntax Highlight Line Break")));

	// Karaoke template code block style
	StyleSetFont(7,font);
	StyleSetSize(7,size);
	StyleSetBold(7,true);
	//StyleSetItalic(7,true);
	StyleSetForeground(7,Options.AsColour(_T("Syntax Highlight Karaoke Template")));

	// Misspelling indicator
	IndicatorSetStyle(0,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetForeground(0,wxColour(255,0,0));
}


/////////////////
// Style a range
void SubsTextEditCtrl::UpdateStyle(int start, int _length) {
	// Styling enabled?
	if (Options.AsBool(_T("Syntax Highlight Enabled")) == 0) return;

	// Check if it's a template line
	AssDialogue *diag = control->grid->GetDialogue(control->linen);
	bool templateLine = diag && diag->Comment && diag->Effect.Lower().StartsWith(_T("template"));
	//bool templateCodeLine = diag && diag->Comment && diag->Effect.Lower().StartsWith(_T("code"));

	// Template code lines get Lua highlighting instead of ASS highlighting
	// This is broken and needs some extra work
	/*if (templateCodeLine) {
		SetLexer(wxSTC_LEX_LUA);
		Colourise(start, start+_length);
		return;
	}
	else {
		SetLexer(wxSTC_LEX_CONTAINER);
	}*/

	// Set variables
	wxString text = GetText();
	int end = start + _length;
	if (_length < 0) end = text.Length();

	// Flags
	bool numMode = false;			// everything is considered a number/parameter until this is unset
	bool drawingMode = false;		// for \p1 -> \p0 stuff

	// Begin styling
	StartStyling(0,255);
	int ran = 0;		// length of current range
	int depth = 0;		// brace nesting depth
	int curStyle = 0;	// style to apply to current range
	int curPos = 0;		// start of current range?
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
			numMode = false;
		}

		// Start override block
		if (curChar == _T('{') && depth >= 0) {
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = 0;
			depth++;
			if (depth == 1) curStyle = 1;
			else curStyle = 4;
			numMode = false;
		}

		// End override block
		else if (curChar == _T('}') && depth <= 1) {
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = 0;
			depth--;
			if (depth == 0) curStyle = 1;
			else curStyle = 4;
			numMode = false;
		}

		// Karaoke template block
		else if (templateLine && curChar == _T('!')) {
			// Apply previous style
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = -1; // such that ran++ later on resets it to 0 !
			// Eat entire template block
			int endPos = i+1;
			while (endPos < end && text[endPos] != _T('!'))
				endPos++;
			SetUnicodeStyling(curPos,endPos-curPos+1,7);
			curPos = endPos+1;
			i = endPos+0;
		}
		// Karaoke template variable
		else if (templateLine && curChar == _T('$')) {
			// Apply previous style
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = -1; // such that ran++ later on resets it to 0 !
			// Eat entire variable
			int endPos = i+1;
			while (endPos < end) {
				wxChar ch = text[endPos];
				if ((ch >= _T('A') && ch <= _T('Z')) || (ch >= _T('a') && ch <= _T('z')) || ch == _T('_'))
					endPos++;
				else
					break;
			}
			SetUnicodeStyling(curPos,endPos-curPos,7);
			curPos = endPos;
			i = curPos-1;
		}

		// Outside
		else if (depth == 0) {
			// Reset number mode
			numMode = false;

			// Is \n, \N or \h?
			if (curChar == _T('\\') && (nextChar == 'n' || nextChar == 'N' || nextChar == 'h')) {
				SetUnicodeStyling(curPos,ran,curStyle);
				curPos += ran;
				ran = 1;
				curStyle = 6;
				i++;
			}

			// Normal text
			else if (curStyle != 0) {
				SetUnicodeStyling(curPos,ran,curStyle);
				curPos += ran;
				ran = 0;
				if (drawingMode) curStyle = 6;
				else curStyle = 0;
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
					numMode = false;
				}
			}

			else {
				// Number
				if (prevChar != _T('\\') && (numMode || (curChar >= '0' && curChar <= '9') || curChar == '.' || curChar == '&' || curChar == '+' || curChar == '-' || (curChar == 'H' && prevChar == '&'))) {
					if (curStyle != 5) {
						SetUnicodeStyling(curPos,ran,curStyle);
						curPos += ran;
						ran = 0;
						curStyle = 5;
						numMode = true;
					}
				}

				// Tag name
				else if (curStyle != 3) {
					SetUnicodeStyling(curPos,ran,curStyle);
					curPos += ran;
					ran = 0;
					curStyle = 3;

					// Set parameter if it's \fn or \r
					int tagLen = 0;
					if (text.Mid(curPos,2) == _T("fn")) tagLen = 2;
					else if (text.Mid(curPos,1) == _T("r")) tagLen = 1;
					if (tagLen) {
						numMode = true;
						ran = tagLen-1;
						i+=ran;
					}

					// Set drawing mode if it's \p
					if (text.Mid(curPos,1) == _T("p")) {
						if (curPos+2 < (signed) text.Length()) {
							// Disable
							wxChar nextNext = text[curPos+2];
							if ((nextNext == _T('\\') || nextNext == _T('}')) && nextChar == _T('0')) drawingMode = false;

							// Enable
							if (nextChar >= _T('1') && nextChar <= _T('9')) {
								for(int testPos = curPos+2;testPos < (signed) text.Length();testPos++) {
									nextNext = text[testPos];
									if (nextNext == _T('\\') || nextNext == _T('}')) {
										drawingMode = true;
										break;
									}
									if (nextNext < _T('0') || nextNext > _T('9')) break;
								}
							}
						}
					}
				}
			}
		}

		// Increase ran length
		ran++;
	}
	SetUnicodeStyling(curPos,ran,curStyle);

	// Spell check
	StyleSpellCheck(start,_length);

	// Call tip
	UpdateCallTip();
}


///////////////////
// Update call tip
void SubsTextEditCtrl::UpdateCallTip() {
	// Enabled?
	if (!Options.AsBool(_T("Call tips enabled"))) return;

	// Get position and text
	const unsigned int pos = GetReverseUnicodePosition(GetCurrentPos());
	wxString text = GetText();

	// Find the start and end of current tag
	wxChar curChar = 0;
	wxChar prevChar = 0;
	int depth = 0;
	int inDepth = 0;
	int tagStart = -1;
	int tagEnd = -1;
	for (unsigned int i=0;i<text.Length()+1;i++) {
		// Get character
		if (i<text.Length()) curChar = text[i];
		else curChar = 0;

		// Change depth
		if (curChar == _T('{')) {
			depth++;
			continue;
		}
		if (curChar == _T('}')) {
			depth--;
			if (i >= pos && depth == 0) {
				tagEnd = i-1;
				break;
			}
			continue;
		}

		// Outside
		if (depth == 0) {
			tagStart = -1;
			if (i == pos) break;
			continue;
		}
		
		// Inside overrides
		if (depth == 1) {
			// Inner depth
			if (tagStart != -1) {
				if (curChar == _T('(')) inDepth++;
				else if (curChar == _T(')')) inDepth--;
			}

			// Not inside parenthesis
			if (inDepth == 0) {
				if (prevChar == _T('\\')) {
					// Found start
					if (i <= pos) tagStart = i;

					// Found end
					else {
						tagEnd = i-2;
						break;
					}
				}
			}
		}

		// Previous character
		prevChar = curChar;
	}

	// Calculate length
	int len;
	if (tagEnd != -1) len = tagEnd - tagStart + 1;
	else len = text.Length() - tagStart;

	// No tag available
	int textLen = text.Length();
	unsigned int posInTag = pos - tagStart;
	if (tagStart+len > textLen || len <= 0 || tagStart < 0) {
		CallTipCancel();
		return;
	}

	// Current tag
	wxString tag = text.Mid(tagStart,len);

	// Metrics in tag
	int tagCommas = 0;
	int tagParenthesis = 0;
	int parN = 0;
	int parPos = -1;
	bool gotName = false;
	wxString tagName = tag;
	for (unsigned int i=0;i<tag.Length();i++) {
		wxChar curChar = tag[i];
		bool isEnd = false;

		// Commas
		if (curChar == _T(',')) {
			tagCommas++;
			parN++;
		}

		// Parenthesis
		else if (curChar == _T('(')) {
			tagParenthesis++;
			parN++;
		}
		else if (curChar == _T(')')) {
			tagParenthesis++;
			parN++;
			isEnd = true;
		}

		// Tag name
		if (parN == 1 && !gotName) {
			tagName = tag.Left(i); 
			gotName = true;
		}

		// Parameter it's on
		if (i == posInTag) {
			parPos = parN;
			if (curChar == _T(',') || curChar == _T('(') || curChar == _T(')')) {
				parPos--;
			}
		}
		else if (isEnd) {
			parN = 1000;
			break;
		}
	}
	if (parPos == -1) parPos = parN;

	// Tag name
	if (tagName.IsEmpty()) {
		CallTipCancel();
		return;
	}

	// Find matching prototype
	wxString useProto;
	wxString cleanProto;
	wxString protoName;
	int protoN = 0;
	bool semiProto = false;
	for (unsigned int i=0;i<proto.Count();i++) {
		// Get prototype name
		int div = proto[i].Find(_T(';'));
		if (div != wxNOT_FOUND) protoName = proto[i].Left(div);
		else {
			div = proto[i].Find(_T('('));
			protoName = proto[i].Left(div);
		}
		
		// Fix name
		semiProto = false;
		cleanProto = proto[i];
		if (cleanProto.Freq(_T(';')) > 0) {
			cleanProto.Replace(_T(";"),_T(""));
			semiProto = true;
		}

		// Prototype match
		wxString temp;
		if (semiProto) temp = tagName.Left(protoName.Length());
		else temp = tagName;
		if (protoName == temp) {
			// Parameter count match
			if (proto[i].Freq(_T(',')) >= tagCommas) {
				// Found
				useProto = proto[i];
				protoN = i;
				break;
			}
		}
	}

	// No matching prototype
	if (useProto.IsEmpty()) {
		CallTipCancel();
		return;
	}

	// Parameter number for tags without "(),"
	if (semiProto && parPos == 0 && posInTag >= protoName.Length()) parPos = 1;

	// Highlight start/end
	int highStart = useProto.Length();
	int highEnd = -1;
	parN = 0;
	int delta = 0;
	for (unsigned int i=0;i<useProto.Length();i++) {
		wxChar curChar = useProto[i];
		if (i == 0 || curChar == _T(',') || curChar == _T(';') || curChar == _T('(') || curChar == _T(')')) {
			if (curChar == _T(';')) delta++;
			if (parN == parPos) highStart = i+1-delta;
			else if (parN == parPos+1) highEnd = i;
			parN++;
		}
	}
	if (highStart <= 1) highStart = 0;
	if (highEnd == -1) highEnd = useProto.Length();

	// Calltip is over
	if (highStart == (signed) useProto.Length()) {
		CallTipCancel();
		return;
	}

	// Show calltip
	if (!CallTipActive() || tipProtoN != protoN) CallTipShow(GetUnicodePosition(tagStart),cleanProto);
	tipProtoN = protoN;
	CallTipSetHighlight(highStart,highEnd);
}


///////////////
// Spell check
void SubsTextEditCtrl::StyleSpellCheck(int start, int len) {
	// See if it has a spellchecker
	if (!spellchecker) return;

	// Results
	wxString text = GetText();
	IntPairVector results;
	GetWordBoundaries(text,results,start,(len == -1) ? len : start+len);

	// Style
	int count = results.size();
	for (int i=0;i<count;i++) {
		// Get current word
		int s = results[i].first;
		int e = results[i].second;
		wxString curWord = text.Mid(s,e-s);

		// Check if it's valid
		if (!spellchecker->CheckWord(curWord)) {
			// Get length before it
			int utf8len = GetUnicodePosition(s);

			// Set styling
			StartStyling(utf8len,32);
			SetUnicodeStyling(s,e-s,32);
		}
	}
	
	// It seems like wxStyledTextCtrl wants you to finish styling at the end of the text.
	// I don't really understand why, it's not documented anywhere I can find, but this fixes bug #595.
	StartUnicodeStyling(text.Length(), 0);
	SetUnicodeStyling(text.Length(), 0, 0);
}


///////////////////////////
// Set text to a new value
void SubsTextEditCtrl::SetTextTo(const wxString _text) {
	// Setup
	control->textEditReady = false;
	Freeze();
	wxString text = _text;
	text.Replace(_T("\r\n"),_T("\\N"));
	//text.Replace(_T("\n\r"),_T("\\N")); // never a valid linebreak
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
	SetSelectionU(GetReverseUnicodePosition(from),GetReverseUnicodePosition(to));

	// Finish
	Thaw();
	control->textEditReady = true;
}


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
	GetParent()->AddPendingEvent(event);
}


///////////////////
// Show popup menu
void SubsTextEditCtrl::ShowPopupMenu(int activePos) {
	// Menu
	wxMenu menu;

	// Position
	if (activePos == -1) activePos = GetCurrentPos();
	activePos = GetReverseUnicodePosition(activePos);

	// Get current word under cursor
	currentWord = GetWordAtPosition(activePos);
	currentWordPos = activePos;

	// Spell check
	//int style = GetStyleAt(activePos);
	if (spellchecker && currentWord.Length()) {
		// Spelled right?
		bool rightSpelling = spellchecker->CheckWord(currentWord);

		// Set font
		wxFont font;
		font.SetWeight(wxFONTWEIGHT_BOLD);

		// Get suggestions
		sugs.Clear();
		sugs = spellchecker->GetSuggestions(currentWord);
		int nSugs = sugs.Count();

		// Spelled wrong
		if (!rightSpelling) {
			// No suggestions
			if (!nSugs) menu.Append(EDIT_MENU_SUGGESTION,_("No correction suggestions"))->Enable(false);

			// Build menu
			for (int i=0;i<nSugs;i++) {
				wxMenuItem *itm;
				itm = menu.Append(EDIT_MENU_SUGGESTIONS+i,sugs[i]);
#if wxCHECK_VERSION(2, 8, 0) && defined(__WINDOWS__)
				itm->SetFont(font);
#endif
			}

			// Append "add word"
			menu.Append(EDIT_MENU_ADD_TO_DICT,wxString::Format(_("Add \"%s\" to dictionary"),currentWord.c_str()))->Enable(spellchecker->CanAddWord(currentWord));
		}

		// Spelled right
		else {
			// No suggestions
			if (!nSugs) menu.Append(EDIT_MENU_SUGGESTION,_("No spell checker suggestions"))->Enable(false);

			// Has suggestions
			else {
				// Build list
				wxMenu *subMenu = new wxMenu();
				for (int i=0;i<nSugs;i++) subMenu->Append(EDIT_MENU_SUGGESTIONS+i,sugs[i]);
				menu.Append(-1,wxString::Format(_("Spell checker suggestions for \"%s\""),currentWord.c_str()), subMenu);
			}

			// Separator
			//if (!thesaurus) menu.AppendSeparator();
		}
		
		// Language list
		wxArrayString langs = spellchecker->GetLanguageList();	// This probably should be cached...

		// Current language
		wxString curLang = Options.AsText(_T("Spell checker language"));

		// Languages
		wxMenu *languageMenu = new wxMenu();
		wxMenuItem *cur;
		wxString name;
		const wxLanguageInfo *info;

		// Insert "Disable"
		cur = languageMenu->AppendCheckItem(EDIT_MENU_DIC_LANGS,_("Disable"));
		if (curLang.IsEmpty()) cur->Check();

		// Each language found
		for (unsigned int i=0;i<langs.Count();i++) {
			info = wxLocale::FindLanguageInfo(langs[i]);
			if (info) name = info->Description;
			else name = langs[i];
			cur = languageMenu->AppendCheckItem(EDIT_MENU_DIC_LANGS+i+1,name);
			if (langs[i] == curLang) cur->Check();
		}

		// Append language list
		menu.Append(-1,_("Spell checker language"), languageMenu);
		menu.AppendSeparator();
	}

	// Thesaurus
	if (thesaurus && currentWord.Length()) {
		// Get results
		ThesaurusEntryArray result;
		thesaurus->Lookup(currentWord,result);

		// Compile list
		thesSugs.Clear();
		for (unsigned int i=0;i<result.size();i++) {
			for (unsigned int j=0;j<result[i].words.Count();j++) {
				thesSugs.Add(result[i].words[j]);
			}
		}

		// Has suggestions
		if (result.size()) {
			// Set font
			wxFont font;
			font.SetStyle(wxFONTSTYLE_ITALIC);

			// Create thesaurus menu
			wxMenu *thesMenu = new wxMenu();

			// Build menu
			int curThesEntry = 0;
			for (unsigned int i=0;i<result.size();i++) {
				// Single word, insert directly
				if (result[i].words.Count() == 1) {
					thesMenu->Append(EDIT_MENU_THESAURUS_SUGS+curThesEntry,result[i].name);
					curThesEntry++;
				}

				// Multiple, create submenu
				else {
					// Insert entries
					wxMenu *subMenu = new wxMenu();
					for (unsigned int j=0;j<result[i].words.Count();j++) {
						subMenu->Append(EDIT_MENU_THESAURUS_SUGS+curThesEntry,result[i].words[j]);
						curThesEntry++;
					}

					// Insert submenu
					thesMenu->Append(-1, result[i].name, subMenu);
				}
			}

			// Thesaurus menu
			menu.Append(-1,wxString::Format(_("Thesaurus suggestions for \"%s\""),currentWord.c_str()), thesMenu);
		}

		// No suggestions
		if (!result.size()) menu.Append(EDIT_MENU_THESAURUS,_("No thesaurus suggestions"))->Enable(false);

		// Language list
		wxArrayString langs = thesaurus->GetLanguageList();	// This probably should be cached...

		// Current language
		wxString curLang = Options.AsText(_T("Thesaurus language"));

		// Languages
		wxMenu *languageMenu = new wxMenu();
		wxMenuItem *cur;
		wxString name;
		const wxLanguageInfo *info;

		// Insert "Disable"
		cur = languageMenu->AppendCheckItem(EDIT_MENU_THES_LANGS,_("Disable"));
		if (curLang.IsEmpty()) cur->Check();

		// Each language found
		for (unsigned int i=0;i<langs.Count();i++) {
			info = wxLocale::FindLanguageInfo(langs[i]);
			if (info) name = info->Description;
			else name = langs[i];
			cur = languageMenu->AppendCheckItem(EDIT_MENU_THES_LANGS+i+1,name);
			if (langs[i] == curLang) cur->Check();
		}

		// Append language list
		menu.Append(-1, _("Thesaurus language"), languageMenu);
		menu.AppendSeparator();
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


///////////////////////////////
// Split line preserving times
void SubsTextEditCtrl::OnSplitLinePreserve (wxCommandEvent &event) {
	int from,to;
	GetSelection(&from, &to);
	from = GetReverseUnicodePosition(from);
	to = GetReverseUnicodePosition(to);
	// Call SplitLine() with the text currently in the editbox.
	// This makes sure we split what the user sees, not the committed line.
	control->grid->SplitLine(control->linen,from,0,GetText());
}


///////////////////////////////
// Split line estimating times
void SubsTextEditCtrl::OnSplitLineEstimate (wxCommandEvent &event) {
	int from,to;
	GetSelection(&from, &to);
	from = GetReverseUnicodePosition(from);
	to = GetReverseUnicodePosition(to);
	// Call SplitLine() with the text currently in the editbox.
	// This makes sure we split what the user sees, not the committed line.
	control->grid->SplitLine(control->linen,from,1,GetText());
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
	UpdateStyle();
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
	SetSelectionU(start,start+suggestion.Length());
	SetFocus();
}



////////////////////////////
// Use thesaurus suggestion
void SubsTextEditCtrl::OnUseThesaurusSuggestion(wxCommandEvent &event) {
	// Get suggestion
	wxString suggestion = thesSugs[event.GetId()-EDIT_MENU_THESAURUS_SUGS];

	// Stripe suggestion of parenthesis
	int pos = suggestion.Find(_T("("));
	if (pos != wxNOT_FOUND) {
		suggestion = suggestion.Left(pos-1);
	}
	
	// Get boundaries of text being replaced
	int start,end;
	GetBoundsOfWordAtPosition(currentWordPos,start,end);

	// Replace
	wxString text = GetText();
	SetText(text.Left(MAX(0,start)) + suggestion + text.Mid(end+1));

	// Set selection
	SetSelectionU(start,start+suggestion.Length());
	SetFocus();
}


///////////////////////////
// Set dictionary language
void SubsTextEditCtrl::OnSetDicLanguage(wxCommandEvent &event) {
	// Get language list
	wxArrayString langs = spellchecker->GetLanguageList();

	// Set dictionary
	int index = event.GetId() - EDIT_MENU_DIC_LANGS - 1;
	wxString lang;
	if (index >= 0) lang = langs[index];
	spellchecker->SetLanguage(lang);
	Options.SetText(_T("Spell checker language"),lang);
	Options.Save();

	// Update styling
	UpdateStyle();
}


//////////////////////////
// Set thesaurus language
void SubsTextEditCtrl::OnSetThesLanguage(wxCommandEvent &event) {
	// Get language list
	wxArrayString langs = thesaurus->GetLanguageList();

	// Set language
	int index = event.GetId() - EDIT_MENU_THES_LANGS - 1;
	wxString lang;
	if (index >= 0) lang = langs[index];
	thesaurus->SetLanguage(lang);
	Options.SetText(_T("Thesaurus language"),lang);
	Options.Save();

	// Update styling
	UpdateStyle();
}
