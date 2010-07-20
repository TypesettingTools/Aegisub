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

/// @file subs_edit_ctrl.cpp
/// @brief Main subtitle editing text control
/// @ingroup main_ui
///

#include "config.h"

#ifndef AGI_PRE
#ifdef _WIN32
#include <functional>
#else
#include <tr1/functional>
#endif
#include <wx/intl.h>
#endif

#include "ass_dialogue.h"
#include "compat.h"
#include "main.h"
#include "spellchecker_manager.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "thesaurus.h"
#include "utils.h"

/// Event ids
enum {
	EDIT_MENU_SPLIT_PRESERVE = 1400,
	EDIT_MENU_SPLIT_ESTIMATE,
	EDIT_MENU_CUT,
	EDIT_MENU_COPY,
	EDIT_MENU_PASTE,
	EDIT_MENU_SELECT_ALL,
	EDIT_MENU_ADD_TO_DICT,
	EDIT_MENU_SUGGESTION,
	EDIT_MENU_SUGGESTIONS,
	EDIT_MENU_THESAURUS = 1450,
	EDIT_MENU_THESAURUS_SUGS,
	EDIT_MENU_DIC_LANGUAGE = 1600,
	EDIT_MENU_DIC_LANGS,
	EDIT_MENU_THES_LANGUAGE = 1700,
	EDIT_MENU_THES_LANGS
};


/// @brief Edit box constructor 
/// @param parent    
/// @param id        
/// @param value     
/// @param pos       
/// @param wsize     
/// @param style     
/// @param validator 
/// @param name      
/// @return 
///
SubsTextEditCtrl::SubsTextEditCtrl(wxWindow* parent, wxSize wsize, long style, SubtitlesGrid *grid)
: ScintillaTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wsize, style)
, spellchecker(SpellCheckerFactoryManager::GetSpellChecker())
, thesaurus(Thesaurus::GetThesaurus())
, grid(grid)
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

	// Prototypes for call tips
	tipProtoN = -1;
	proto.Add(L"move(x1,y1,x2,y2)");
	proto.Add(L"move(x1,y1,x2,y2,startTime,endTime)");
	proto.Add(L"fn;FontName");
	proto.Add(L"bord;Width");
	proto.Add(L"xbord;Width");
	proto.Add(L"ybord;Width");
	proto.Add(L"shad;Depth");
	proto.Add(L"xshad;Depth");
	proto.Add(L"yshad;Depth");
	proto.Add(L"be;Strength");
	proto.Add(L"blur;Strength");
	proto.Add(L"fscx;Scale");
	proto.Add(L"fscy;Scale");
	proto.Add(L"fsp;Spacing");
	proto.Add(L"fs;FontSize");
	proto.Add(L"fe;Encoding");
	proto.Add(L"frx;Angle");
	proto.Add(L"fry;Angle");
	proto.Add(L"frz;Angle");
	proto.Add(L"fr;Angle");
	proto.Add(L"pbo;Offset");
	proto.Add(L"clip(command)");
	proto.Add(L"clip(scale,command)");
	proto.Add(L"clip(x1,y1,x2,y2)");
	proto.Add(L"iclip(command)");
	proto.Add(L"iclip(scale,command)");
	proto.Add(L"iclip(x1,y1,x2,y2)");
	proto.Add(L"t(acceleration,tags)");
	proto.Add(L"t(startTime,endTime,tags)");
	proto.Add(L"t(startTime,endTime,acceleration,tags)");
	proto.Add(L"pos(x,y)");
	proto.Add(L"p;Exponent");
	proto.Add(L"org(x,y)");
	proto.Add(L"fade(startAlpha,middleAlpha,endAlpha,startIn,endIn,startOut,endOut)");
	proto.Add(L"fad(startTime,endTime)");
	proto.Add(L"c;Colour");
	proto.Add(L"1c;Colour");
	proto.Add(L"2c;Colour");
	proto.Add(L"3c;Colour");
	proto.Add(L"4c;Colour");
	proto.Add(L"alpha;Alpha");
	proto.Add(L"1a;Alpha");
	proto.Add(L"2a;Alpha");
	proto.Add(L"3a;Alpha");
	proto.Add(L"4a;Alpha");
	proto.Add(L"an;Alignment");
	proto.Add(L"a;Alignment");
	proto.Add(L"b;Weight");
	proto.Add(L"i;1/0");
	proto.Add(L"u;1/0");
	proto.Add(L"s;1/0");
	proto.Add(L"kf;Duration");
	proto.Add(L"ko;Duration");
	proto.Add(L"k;Duration");
	proto.Add(L"K;Duration");
	proto.Add(L"q;WrapStyle");
	proto.Add(L"r;Style");
	proto.Add(L"fax;Factor");
	proto.Add(L"fay;Factor");

	using namespace std::tr1;

	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::Cut, this)), EDIT_MENU_CUT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::Copy, this)), EDIT_MENU_COPY);
	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::Paste, this)), EDIT_MENU_PASTE);
	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::SelectAll, this)), EDIT_MENU_SELECT_ALL);

	Bind(wxEVT_STC_STYLENEEDED, &SubsTextEditCtrl::UpdateCallTip, this);
}

SubsTextEditCtrl::~SubsTextEditCtrl() {
}

BEGIN_EVENT_TABLE(SubsTextEditCtrl,wxStyledTextCtrl)
	EVT_MOUSE_EVENTS(SubsTextEditCtrl::OnMouseEvent)
	EVT_KILL_FOCUS(SubsTextEditCtrl::OnLoseFocus)

	EVT_MENU(EDIT_MENU_SPLIT_PRESERVE,SubsTextEditCtrl::OnSplitLinePreserve)
	EVT_MENU(EDIT_MENU_SPLIT_ESTIMATE,SubsTextEditCtrl::OnSplitLineEstimate)
	EVT_MENU(EDIT_MENU_ADD_TO_DICT,SubsTextEditCtrl::OnAddToDictionary)
	EVT_MENU_RANGE(EDIT_MENU_SUGGESTIONS,EDIT_MENU_THESAURUS-1,SubsTextEditCtrl::OnUseSuggestion)
	EVT_MENU_RANGE(EDIT_MENU_THESAURUS_SUGS,EDIT_MENU_DIC_LANGUAGE-1,SubsTextEditCtrl::OnUseSuggestion)
	EVT_MENU_RANGE(EDIT_MENU_DIC_LANGS,EDIT_MENU_THES_LANGUAGE-1,SubsTextEditCtrl::OnSetDicLanguage)
	EVT_MENU_RANGE(EDIT_MENU_THES_LANGS,EDIT_MENU_THES_LANGS+100,SubsTextEditCtrl::OnSetThesLanguage)
END_EVENT_TABLE()

void SubsTextEditCtrl::OnLoseFocus(wxFocusEvent &event) {
	CallTipCancel();
	event.Skip();
}

void SubsTextEditCtrl::SetStyles() {
	wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	font.SetEncoding(wxFONTENCODING_DEFAULT); // this solves problems with some fonts not working properly
	wxString fontname = lagi_wxString(OPT_GET("Subtitle/Edit Box/Font Face")->GetString());
	if (!fontname.empty()) font.SetFaceName(fontname);
	int size = OPT_GET("Subtitle/Edit Box/Font Size")->GetInt();

	// Normal style
	StyleSetFont(0,font);
	StyleSetSize(0,size);
	StyleSetForeground(0,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Normal")->GetColour()));

	// Brackets style
	StyleSetFont(1,font);
	StyleSetSize(1,size);
	StyleSetForeground(1,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Brackets")->GetColour()));

	// Slashes/Parenthesis/Comma style
	StyleSetFont(2,font);
	StyleSetSize(2,size);
	StyleSetForeground(2,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Slashes")->GetColour()));

	// Tags style
	StyleSetFont(3,font);
	StyleSetSize(3,size);
	StyleSetBold(3,true);
	StyleSetForeground(3,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Highlight Tags")->GetColour()));

	// Error style
	StyleSetFont(4,font);
	StyleSetSize(4,size);
	StyleSetForeground(4,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Error")->GetColour()));
	StyleSetBackground(4,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Background/Error")->GetColour()));

	// Tag Parameters style
	StyleSetFont(5,font);
	StyleSetSize(5,size);
	StyleSetForeground(5,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Parameters")->GetColour()));

	// Line breaks style
	StyleSetFont(6,font);
	StyleSetSize(6,size);
	StyleSetBold(6,true);
	StyleSetForeground(6,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Line Break")->GetColour()));

	// Karaoke template code block style
	StyleSetFont(7,font);
	StyleSetSize(7,size);
	StyleSetBold(7,true);
	//StyleSetItalic(7,true);
	StyleSetForeground(7,lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/Karaoke Template")->GetColour()));

	// Misspelling indicator
	IndicatorSetStyle(0,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetForeground(0,wxColour(255,0,0));
}

void SubsTextEditCtrl::UpdateStyle(int start, int _length) {
	if (OPT_GET("Subtitle/Highlight/Syntax")->GetBool() == 0) return;

	// Check if it's a template line
	AssDialogue *diag = grid->GetActiveLine();
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
		if (curChar == '{' && depth >= 0) {
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = 0;
			depth++;
			if (depth == 1) curStyle = 1;
			else curStyle = 4;
			numMode = false;
		}

		// End override block
		else if (curChar == '}' && depth <= 1) {
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = 0;
			depth--;
			if (depth == 0) curStyle = 1;
			else curStyle = 4;
			numMode = false;
		}

		// Karaoke template block
		else if (templateLine && curChar == '!') {
			// Apply previous style
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = -1; // such that ran++ later on resets it to 0 !
			// Eat entire template block
			int endPos = i+1;
			while (endPos < end && text[endPos] != '!')
				endPos++;
			SetUnicodeStyling(curPos,endPos-curPos+1,7);
			curPos = endPos+1;
			i = endPos+0;
		}
		// Karaoke template variable
		else if (templateLine && curChar == '$') {
			// Apply previous style
			SetUnicodeStyling(curPos,ran,curStyle);
			curPos += ran;
			ran = -1; // such that ran++ later on resets it to 0 !
			// Eat entire variable
			int endPos = i+1;
			while (endPos < end) {
				wxChar ch = text[endPos];
				if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_')
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
			if (curChar == L'\\' && (nextChar == 'n' || nextChar == 'N' || nextChar == 'h')) {
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
			if (curChar == L'\\' || curChar == '(' || curChar == ')' || curChar == ',') {
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
				if (prevChar != L'\\' && (numMode || (curChar >= '0' && curChar <= '9') || curChar == '.' || curChar == '&' || curChar == '+' || curChar == '-' || (curChar == 'H' && prevChar == '&'))) {
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
					if (text.Mid(curPos,2) == L"fn") tagLen = 2;
					else if (text.Mid(curPos,1) == L"r") tagLen = 1;
					if (tagLen) {
						numMode = true;
						ran = tagLen-1;
						i+=ran;
					}

					// Set drawing mode if it's \p
					if (text.Mid(curPos,1) == L"p") {
						if (curPos+2 < (signed) text.Length()) {
							// Disable
							wxChar nextNext = text[curPos+2];
							if ((nextNext == L'\\' || nextNext == '}') && nextChar == '0') drawingMode = false;

							// Enable
							if (nextChar >= '1' && nextChar <= '9') {
								for(int testPos = curPos+2;testPos < (signed) text.Length();testPos++) {
									nextNext = text[testPos];
									if (nextNext == L'\\' || nextNext == '}') {
										drawingMode = true;
										break;
									}
									if (nextNext < '0' || nextNext > '9') break;
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
	StyleSpellCheck(start,_length);
	wxStyledTextEvent evt;
	UpdateCallTip(evt);
}



/// @brief Update call tip 
void SubsTextEditCtrl::UpdateCallTip(wxStyledTextEvent &) {
	if (!OPT_GET("App/Call Tips")->GetBool()) return;

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
		if (curChar == '{') {
			depth++;
			continue;
		}
		if (curChar == '}') {
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
				if (curChar == '(') inDepth++;
				else if (curChar == ')') inDepth--;
			}

			// Not inside parenthesis
			if (inDepth == 0) {
				if (prevChar == L'\\') {
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
		if (curChar == ',') {
			tagCommas++;
			parN++;
		}

		// Parenthesis
		else if (curChar == '(') {
			tagParenthesis++;
			parN++;
		}
		else if (curChar == ')') {
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
			if (curChar == ',' || curChar == '(' || curChar == ')') {
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
		int div = proto[i].Find(';');
		if (div != wxNOT_FOUND) protoName = proto[i].Left(div);
		else {
			div = proto[i].Find('(');
			protoName = proto[i].Left(div);
		}
		
		// Fix name
		semiProto = false;
		cleanProto = proto[i];
		if (cleanProto.Freq(';') > 0) {
			cleanProto.Replace(L";","");
			semiProto = true;
		}

		// Prototype match
		wxString temp;
		if (semiProto) temp = tagName.Left(protoName.Length());
		else temp = tagName;
		if (protoName == temp) {
			// Parameter count match
			if (proto[i].Freq(',') >= tagCommas) {
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
		if (i == 0 || curChar == ',' || curChar == ';' || curChar == '(' || curChar == ')') {
			if (curChar == ';') delta++;
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

void SubsTextEditCtrl::StyleSpellCheck(int start, int len) {
	if (!spellchecker.get()) return;

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

void SubsTextEditCtrl::SetTextTo(wxString text) {
	SetEvtHandlerEnabled(false);
	Freeze();

	text.Replace(L"\r\n",L"\\N");
	text.Replace(L"\r",L"\\N");
	text.Replace(L"\n",L"\\N");

	int from=0,to=0;
	GetSelection(&from,&to);
	Clear();

	SetText(text);
	UpdateStyle();

	// Restore selection
	SetSelectionU(GetReverseUnicodePosition(from),GetReverseUnicodePosition(to));

	SetEvtHandlerEnabled(true);
	Thaw();
}

void SubsTextEditCtrl::OnMouseEvent(wxMouseEvent &event) {
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		if (grid->GetActiveLine() != 0) {
			int pos = PositionFromPoint(event.GetPosition());
			ShowPopupMenu(pos);
			return;
		}
	}

	event.Skip();
	GetParent()->GetEventHandler()->ProcessEvent(event);
}

void SubsTextEditCtrl::ShowPopupMenu(int activePos) {
	wxMenu menu;

	if (activePos == -1) activePos = GetCurrentPos();
	activePos = GetReverseUnicodePosition(activePos);

	currentWord = GetWordAtPosition(activePos);
	currentWordPos = activePos;

	if (spellchecker.get() && currentWord.Length()) {
		bool rightSpelling = spellchecker->CheckWord(currentWord);

		wxFont font;
		font.SetWeight(wxFONTWEIGHT_BOLD);

		sugs.Clear();
		sugs = spellchecker->GetSuggestions(currentWord);
		int nSugs = sugs.Count();

		if (!rightSpelling) {
			if (!nSugs) {
				menu.Append(EDIT_MENU_SUGGESTION,_("No correction suggestions"))->Enable(false);
			}

			for (int i=0;i<nSugs;i++) {
				wxMenuItem *itm = new wxMenuItem(&menu, EDIT_MENU_SUGGESTIONS+i, sugs[i]);
#ifdef __WINDOWS__
				itm->SetFont(font);
#endif
				menu.Append(itm);
			}

			// Append "add word"
			wxString add_to_dict_text(_("Add \"%s\" to dictionary"));
			add_to_dict_text.Replace(L"%s", currentWord);
			menu.Append(EDIT_MENU_ADD_TO_DICT,add_to_dict_text)->Enable(spellchecker->CanAddWord(currentWord));
		}
		// Spelled right
		else {
			if (!nSugs) {
				menu.Append(EDIT_MENU_SUGGESTION,_("No spell checker suggestions"))->Enable(false);
			}
			else {
				// Build list
				wxMenu *subMenu = new wxMenu();
				for (int i=0;i<nSugs;i++) subMenu->Append(EDIT_MENU_SUGGESTIONS+i,sugs[i]);
				menu.Append(-1,wxString::Format(_("Spell checker suggestions for \"%s\""),currentWord.c_str()), subMenu);
			}
		}

		wxArrayString langs = spellchecker->GetLanguageList();	// This probably should be cached...
		wxString curLang = lagi_wxString(OPT_GET("Tool/Spell Checker/Language")->GetString());

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
	if (thesaurus.get() && currentWord.Length()) {
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

		if (result.size()) {
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
			wxString thes_suggestion_text(_("Thesaurus suggestions for \"%s\""));
			thes_suggestion_text.Replace(L"%s", currentWord);
			menu.Append(-1,thes_suggestion_text,thesMenu);

		}

		if (!result.size()) menu.Append(EDIT_MENU_THESAURUS,_("No thesaurus suggestions"))->Enable(false);

		wxArrayString langs = thesaurus->GetLanguageList();	// This probably should be cached...
		wxString curLang = lagi_wxString(OPT_GET("Tool/Thesaurus/Language")->GetString());

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
	menu.Append(EDIT_MENU_CUT,_("Cu&t"))->Enable(GetSelectionStart()-GetSelectionEnd() != 0);
	menu.Append(EDIT_MENU_COPY,_("&Copy"))->Enable(GetSelectionStart()-GetSelectionEnd() != 0);
	menu.Append(EDIT_MENU_PASTE,_("&Paste"))->Enable(CanPaste());
	menu.AppendSeparator();
	menu.Append(EDIT_MENU_SELECT_ALL,_("Select &All"));

	// Split
	menu.AppendSeparator();
	menu.Append(EDIT_MENU_SPLIT_PRESERVE,_("Split at cursor (preserve times)"));
	menu.Append(EDIT_MENU_SPLIT_ESTIMATE,_("Split at cursor (estimate times)"));

	PopupMenu(&menu);
}

void SubsTextEditCtrl::OnSplitLinePreserve (wxCommandEvent &) {
	int from,to;
	GetSelection(&from, &to);
	from = GetReverseUnicodePosition(from);
	grid->SplitLine(grid->GetActiveLine(),from,0);
}

void SubsTextEditCtrl::OnSplitLineEstimate (wxCommandEvent &) {
	int from,to;
	GetSelection(&from, &to);
	from = GetReverseUnicodePosition(from);
	grid->SplitLine(grid->GetActiveLine(),from,1);
}

void SubsTextEditCtrl::OnAddToDictionary(wxCommandEvent &) {
	if (spellchecker.get()) spellchecker->AddWord(currentWord);
	UpdateStyle();
	SetFocus();
}

void SubsTextEditCtrl::OnUseSuggestion(wxCommandEvent &event) {
	wxString suggestion;
	int sugIdx = event.GetId() - EDIT_MENU_THESAURUS_SUGS;
	if (sugIdx >= 0) {
		suggestion = thesSugs[sugIdx];
	}
	else {
		suggestion = sugs[event.GetId() - EDIT_MENU_SUGGESTIONS];
	}

	// Stripe suggestion of parenthesis
	int pos = suggestion.Find(L"(");
	if (pos != wxNOT_FOUND) {
		suggestion = suggestion.Left(pos-1);
	}

	// Get boundaries of text being replaced
	int start,end;
	GetBoundsOfWordAtPosition(currentWordPos,start,end);

	wxString text = GetText();
	SetText(text.Left(MAX(0,start)) + suggestion + text.Mid(end+1));

	// Set selection
	SetSelectionU(start,start+suggestion.Length());
	SetFocus();
}

void SubsTextEditCtrl::OnSetDicLanguage(wxCommandEvent &event) {
	wxArrayString langs = spellchecker->GetLanguageList();

	// Set dictionary
	int index = event.GetId() - EDIT_MENU_DIC_LANGS - 1;
	wxString lang;
	if (index >= 0) lang = langs[index];
	spellchecker->SetLanguage(lang);
	OPT_SET("Tool/Spell Checker/Language")->SetString(STD_STR(lang));

	UpdateStyle();
}

void SubsTextEditCtrl::OnSetThesLanguage(wxCommandEvent &event) {
	wxArrayString langs = thesaurus->GetLanguageList();

	// Set language
	int index = event.GetId() - EDIT_MENU_THES_LANGS - 1;
	wxString lang;
	if (index >= 0) lang = langs[index];
	thesaurus->SetLanguage(lang);
	OPT_SET("Tool/Thesaurus/Language")->SetString(STD_STR(lang));

	UpdateStyle();
}
