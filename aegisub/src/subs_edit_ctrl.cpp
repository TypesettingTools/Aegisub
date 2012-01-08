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
#include <wx/clipbrd.h>
#include <wx/intl.h>
#endif

#include "ass_dialogue.h"
#include "compat.h"
#include "main.h"
#include "include/aegisub/spellchecker.h"
#include "selection_controller.h"
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

SubsTextEditCtrl::SubsTextEditCtrl(wxWindow* parent, wxSize wsize, long style, SubtitlesGrid *grid)
: ScintillaTextCtrl(parent, -1, "", wxDefaultPosition, wsize, style)
, spellchecker(SpellCheckerFactory::GetSpellChecker())
, thesaurus(new Thesaurus)
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
	proto.Add("move(x1,y1,x2,y2)");
	proto.Add("move(x1,y1,x2,y2,startTime,endTime)");
	proto.Add("fn;FontName");
	proto.Add("bord;Width");
	proto.Add("xbord;Width");
	proto.Add("ybord;Width");
	proto.Add("shad;Depth");
	proto.Add("xshad;Depth");
	proto.Add("yshad;Depth");
	proto.Add("be;Strength");
	proto.Add("blur;Strength");
	proto.Add("fscx;Scale");
	proto.Add("fscy;Scale");
	proto.Add("fsp;Spacing");
	proto.Add("fs;FontSize");
	proto.Add("fe;Encoding");
	proto.Add("frx;Angle");
	proto.Add("fry;Angle");
	proto.Add("frz;Angle");
	proto.Add("fr;Angle");
	proto.Add("pbo;Offset");
	proto.Add("clip(command)");
	proto.Add("clip(scale,command)");
	proto.Add("clip(x1,y1,x2,y2)");
	proto.Add("iclip(command)");
	proto.Add("iclip(scale,command)");
	proto.Add("iclip(x1,y1,x2,y2)");
	proto.Add("t(acceleration,tags)");
	proto.Add("t(startTime,endTime,tags)");
	proto.Add("t(startTime,endTime,acceleration,tags)");
	proto.Add("pos(x,y)");
	proto.Add("p;Exponent");
	proto.Add("org(x,y)");
	proto.Add("fade(startAlpha,middleAlpha,endAlpha,startIn,endIn,startOut,endOut)");
	proto.Add("fad(startTime,endTime)");
	proto.Add("c;Colour");
	proto.Add("1c;Colour");
	proto.Add("2c;Colour");
	proto.Add("3c;Colour");
	proto.Add("4c;Colour");
	proto.Add("alpha;Alpha");
	proto.Add("1a;Alpha");
	proto.Add("2a;Alpha");
	proto.Add("3a;Alpha");
	proto.Add("4a;Alpha");
	proto.Add("an;Alignment");
	proto.Add("a;Alignment");
	proto.Add("b;Weight");
	proto.Add("i;1/0");
	proto.Add("u;1/0");
	proto.Add("s;1/0");
	proto.Add("kf;Duration");
	proto.Add("ko;Duration");
	proto.Add("k;Duration");
	proto.Add("K;Duration");
	proto.Add("q;WrapStyle");
	proto.Add("r;Style");
	proto.Add("fax;Factor");
	proto.Add("fay;Factor");

	using namespace std::tr1;

	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::Cut, this)), EDIT_MENU_CUT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::Copy, this)), EDIT_MENU_COPY);
	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::Paste, this)), EDIT_MENU_PASTE);
	Bind(wxEVT_COMMAND_MENU_SELECTED, function<void (wxCommandEvent &)>(bind(&SubsTextEditCtrl::SelectAll, this)), EDIT_MENU_SELECT_ALL);

	Bind(wxEVT_STC_STYLENEEDED, &SubsTextEditCtrl::UpdateCallTip, this);

	Bind(wxEVT_CONTEXT_MENU, &SubsTextEditCtrl::OnContextMenu, this);

	OPT_SUB("Subtitle/Edit Box/Font Face", &SubsTextEditCtrl::SetStyles, this);
	OPT_SUB("Subtitle/Edit Box/Font Size", &SubsTextEditCtrl::SetStyles, this);
	Subscribe("Normal");
	Subscribe("Comment");
	Subscribe("Drawing");
	Subscribe("Brackets");
	Subscribe("Slashes");
	Subscribe("Tags");
	Subscribe("Error");
	Subscribe("Parameters");
	Subscribe("Line Break");
	Subscribe("Karaoke Template");
	Subscribe("Karaoke Variable");

	OPT_SUB("Subtitle/Highlight/Syntax", &SubsTextEditCtrl::UpdateStyle, this);
	static wxStyledTextEvent evt;
	OPT_SUB("App/Call Tips", &SubsTextEditCtrl::UpdateCallTip, this, ref(evt));
}


SubsTextEditCtrl::~SubsTextEditCtrl() {
}

void SubsTextEditCtrl::Subscribe(std::string const& name) {
	OPT_SUB("Colour/Subtitle/Syntax/" + name, &SubsTextEditCtrl::SetStyles, this);
	OPT_SUB("Colour/Subtitle/Syntax/Background/" + name, &SubsTextEditCtrl::SetStyles, this);
	OPT_SUB("Colour/Subtitle/Syntax/Bold/" + name, &SubsTextEditCtrl::SetStyles, this);
}

BEGIN_EVENT_TABLE(SubsTextEditCtrl,wxStyledTextCtrl)
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

enum {
	STYLE_NORMAL = 0,
	STYLE_COMMENT,
	STYLE_DRAWING,
	STYLE_OVERRIDE,
	STYLE_PUNCTUATION,
	STYLE_TAG,
	STYLE_ERROR,
	STYLE_PARAMETER,
	STYLE_LINE_BREAK,
	STYLE_KARAOKE_TEMPLATE,
	STYLE_KARAOKE_VARIABLE
};

void SubsTextEditCtrl::SetSyntaxStyle(int id, wxFont &font, std::string const& name) {
	StyleSetFont(id, font);
	StyleSetBold(id, OPT_GET("Colour/Subtitle/Syntax/Bold/" + name)->GetBool());
	StyleSetForeground(id, lagi_wxColour(OPT_GET("Colour/Subtitle/Syntax/" + name)->GetColour()));
	const agi::OptionValue *background = OPT_GET("Colour/Subtitle/Syntax/Background/" + name);
	if (background->GetType() == agi::OptionValue::Type_Colour)
		StyleSetBackground(id, lagi_wxColour(background->GetColour()));
}

void SubsTextEditCtrl::SetStyles() {
	wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	font.SetEncoding(wxFONTENCODING_DEFAULT); // this solves problems with some fonts not working properly
	wxString fontname = lagi_wxString(OPT_GET("Subtitle/Edit Box/Font Face")->GetString());
	if (!fontname.empty()) font.SetFaceName(fontname);
	font.SetPointSize(OPT_GET("Subtitle/Edit Box/Font Size")->GetInt());

	SetSyntaxStyle(STYLE_NORMAL, font, "Normal");
	SetSyntaxStyle(STYLE_COMMENT, font, "Comment");
	SetSyntaxStyle(STYLE_DRAWING, font, "Drawing");
	SetSyntaxStyle(STYLE_OVERRIDE, font, "Brackets");
	SetSyntaxStyle(STYLE_PUNCTUATION, font, "Slashes");
	SetSyntaxStyle(STYLE_TAG, font, "Tags");
	SetSyntaxStyle(STYLE_ERROR, font, "Error");
	SetSyntaxStyle(STYLE_PARAMETER, font, "Parameters");
	SetSyntaxStyle(STYLE_LINE_BREAK, font, "Line Break");
	SetSyntaxStyle(STYLE_KARAOKE_TEMPLATE, font, "Karaoke Template");
	SetSyntaxStyle(STYLE_KARAOKE_VARIABLE, font, "Karaoke Variable");

	// Misspelling indicator
	IndicatorSetStyle(0,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetForeground(0,wxColour(255,0,0));
}

void SubsTextEditCtrl::UpdateStyle() {
	StartStyling(0,255);

	std::string text = STD_STR(GetText());

	if (!OPT_GET("Subtitle/Highlight/Syntax")->GetBool()) {
		SetStyling(text.size(), 0);
		return;
	}

	if (text.empty()) return;

	// Check if it's a template line
	AssDialogue *diag = grid->GetActiveLine();
	bool templateLine = diag && diag->Comment && diag->Effect.Lower().StartsWith("template");

	bool in_parens = false;
	bool in_unparened_arg = false;
	bool in_draw_mode = false;
	bool in_ovr = false;
	bool in_karaoke_template = false;

	int range_len = 0;
	int style = STYLE_NORMAL;

	char cur_char = 0;
	char next_char = text[0];

	size_t eat_chars = 0;

	for (size_t i = 0; i < text.size(); ++i) {
		// Current/previous characters
		char prev_char = cur_char;
		cur_char = next_char;
		next_char = i + 1 < text.size() ? text[i + 1] : 0;

		if (eat_chars > 0) {
			++range_len;
			--eat_chars;
			continue;
		}

		int new_style = style;

		// Start override block
		if (cur_char == '{') {
			new_style = in_ovr ? STYLE_ERROR : STYLE_OVERRIDE;
			in_ovr = true;
			in_parens = false;
		}
		// End override block
		else if (cur_char == '}') {
			new_style = in_ovr ? STYLE_OVERRIDE : STYLE_ERROR;
			in_ovr = false;
		}
		// Start karaoke template
		else if (templateLine && style != STYLE_KARAOKE_TEMPLATE && cur_char == '!') {
			new_style = STYLE_KARAOKE_TEMPLATE;
			in_karaoke_template = true;
		}
		// End karaoke template
		else if (style == STYLE_KARAOKE_TEMPLATE && cur_char == '!') {
			in_karaoke_template = false;
		}
		// Continue karaoke template
		else if (in_karaoke_template) {
			// Do nothing and just continue the karaoke template style
		}
		// Start karaoke template variable
		else if (templateLine && style != STYLE_KARAOKE_TEMPLATE && cur_char == '$') {
			new_style = STYLE_KARAOKE_VARIABLE;
		}
		// Continue karaoke template variable
		else if (style == STYLE_KARAOKE_VARIABLE && ((cur_char >= 'A' && cur_char <= 'Z') || (cur_char >= 'a' && cur_char <= 'z') || cur_char == '_')) {
			// Do nothing and just continue the karaoke variable style
		}
		// Plain text
		else if (!in_ovr) {
			// Is \n, \N or \h?
			if (cur_char == '\\' && (next_char == 'n' || next_char == 'N' || next_char == 'h')) {
				new_style = STYLE_LINE_BREAK;
				eat_chars = 1;
			}
			else if (in_draw_mode)
				new_style = STYLE_DRAWING;
			else
				new_style = STYLE_NORMAL;
		}
		// Inside override tag
		else {
			// Special character
			if (cur_char == '\\' || cur_char == '(' || cur_char == ')' || cur_char == ',') {
				new_style = STYLE_PUNCTUATION;
				in_unparened_arg = false;

				if (style == STYLE_TAG && cur_char == '(')
					in_parens = true;
				// This is technically wrong for nested tags, but it doesn't
				// matter as \t doesn't have any arguments after the subtag(s)
				else if (cur_char == ')')
					in_parens = false;
			}
			// Beginning of a tag
			else if (prev_char == '\\') {
				new_style = STYLE_TAG;
				// \r and \fn are special as their argument is an
				// unparenthesized string
				if (cur_char == 'r')
					in_unparened_arg = true;
				else if (cur_char == 'f' && next_char == 'n') {
					eat_chars = 1;
					in_unparened_arg = true;
				}
				// For \p we need to check if it's entering or leaving draw mode
				// Luckily draw mode can't be set in the style, so no argument
				// always means leave draw mode
				else if (cur_char == 'p' && (next_char < 'a' || next_char > 'z')) {
					in_draw_mode = false;
					for (size_t idx = i + 1; idx < text.size(); ++idx) {
						char c = text[idx];
						// I have no idea why one would use leading zeros for
						// the scale, but vsfilter allows it
						if (c >= '1' && c <= '9')
							in_draw_mode = true;
						else if (c != '0')
							break;
					}
				}
				// All tags start with letters or numbers
				else if (cur_char < '0' || (cur_char > '9' && cur_char < 'A') || (cur_char > 'Z' && cur_char < 'a') || cur_char > 'z')
					new_style = STYLE_ERROR;
			}
			else if ((in_parens && style != STYLE_TAG) || in_unparened_arg || (style == STYLE_TAG && ((cur_char < 'A' || cur_char > 'z') || (cur_char > 'Z' && cur_char < 'a')))) {
				new_style = STYLE_PARAMETER;
			}
			else if (style != STYLE_TAG && style != STYLE_PARAMETER && style != STYLE_ERROR) {
				new_style = STYLE_COMMENT;
			}
		}

		if (new_style != style) {
			SetStyling(range_len, style);
			style = new_style;
			range_len = 0;
		}

		++range_len;
	}
	SetStyling(range_len, style);
	StyleSpellCheck();
}

/// @brief Update call tip 
void SubsTextEditCtrl::UpdateCallTip(wxStyledTextEvent &) {
	UpdateStyle();

	if (!OPT_GET("App/Call Tips")->GetBool()) return;

	// Get position and text
	const unsigned int pos = GetReverseUnicodePosition(GetCurrentPos());
	wxString text = GetText();

	// Find the start and end of current tag
	wxChar prevChar = 0;
	int depth = 0;
	int inDepth = 0;
	int tagStart = -1;
	int tagEnd = -1;
	for (unsigned int i=0;i<text.Length()+1;i++) {
		wxChar curChar = i < text.size() ? text[i] : 0;

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
				if (prevChar == '\\') {
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
			cleanProto.Replace(";","");
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

void SubsTextEditCtrl::StyleSpellCheck() {
	if (!spellchecker.get()) return;

	// Results
	wxString text = GetText();
	IntPairVector results;
	GetWordBoundaries(text,results);

	// Style
	int count = results.size();
	for (int i=0;i<count;i++) {
		// Get current word
		int s = results[i].first;
		int e = results[i].second;
		wxString curWord = text.Mid(s,e-s);

		// Check if it's valid
		if (!spellchecker->CheckWord(curWord)) {
			StartUnicodeStyling(s,32);
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

	int from=0,to=0;
	GetSelection(&from,&to);

	SetText(text);
	UpdateStyle();

	// Restore selection
	SetSelectionU(GetReverseUnicodePosition(from), GetReverseUnicodePosition(to));

	SetEvtHandlerEnabled(true);
	Thaw();
}


void SubsTextEditCtrl::Paste() {
	wxString data;
	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject rawdata;
			wxTheClipboard->GetData(rawdata);
			data = rawdata.GetText();
		}
		wxTheClipboard->Close();
	}

	data.Replace("\r\n", "\\N");
	data.Replace("\n", "\\N");
	data.Replace("\r", "\\N");

	int from = GetReverseUnicodePosition(GetSelectionStart());
	int to = GetReverseUnicodePosition(GetSelectionEnd());

	wxString old = GetText();
	SetText(old.Left(from) + data + old.Mid(to));

	int sel_start = GetUnicodePosition(from + data.size());
	SetSelectionStart(sel_start);
	SetSelectionEnd(sel_start);
	UpdateStyle();
}

void SubsTextEditCtrl::OnContextMenu(wxContextMenuEvent &event) {
	if (!grid->GetActiveLine())
		return;

	wxPoint pos = event.GetPosition();
	int activePos;
	if (pos == wxDefaultPosition) {
		activePos = GetCurrentPos();
	}
	else {
		activePos = PositionFromPoint(ScreenToClient(pos));
	}

	wxMenu menu;
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
			add_to_dict_text.Replace("%s", currentWord);
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
				menu.Append(-1,wxString::Format(_("Spell checker suggestions for \"%s\""),currentWord), subMenu);
			}
		}

		wxArrayString langs = spellchecker->GetLanguageList();
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
		std::vector<Thesaurus::Entry> result;
		thesaurus->Lookup(currentWord,&result);

		// Compile list
		thesSugs.clear();
		thesSugs.reserve(result.size() * 5);
		for (size_t i = 0; i < result.size(); ++i) {
			for (size_t j = 0; j < result[i].second.size(); ++j) {
				thesSugs.push_back(result[i].second[j]);
			}
		}

		if (result.size()) {
			wxFont font;
			font.SetStyle(wxFONTSTYLE_ITALIC);

			// Create thesaurus menu
			wxMenu *thesMenu = new wxMenu();

			// Build menu
			int curThesEntry = 0;
			for (size_t i=0;i<result.size();i++) {
				// Single word, insert directly
				if (result[i].second.size() == 1) {
					thesMenu->Append(EDIT_MENU_THESAURUS_SUGS+curThesEntry,lagi_wxString(result[i].first));
					curThesEntry++;
				}

				// Multiple, create submenu
				else {
					// Insert entries
					wxMenu *subMenu = new wxMenu();
					for (size_t j=0;j<result[i].second.size();j++) {
						subMenu->Append(EDIT_MENU_THESAURUS_SUGS+curThesEntry,lagi_wxString(result[i].second[j]));
						curThesEntry++;
					}

					// Insert submenu
					thesMenu->Append(-1, lagi_wxString(result[i].first), subMenu);
				}
			}

			// Thesaurus menu
			wxString thes_suggestion_text(_("Thesaurus suggestions for \"%s\""));
			thes_suggestion_text.Replace("%s", currentWord);
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
		suggestion = lagi_wxString(thesSugs[sugIdx]);
	}
	else {
		suggestion = sugs[event.GetId() - EDIT_MENU_SUGGESTIONS];
	}

	// Stripe suggestion of parenthesis
	int pos = suggestion.Find("(");
	if (pos != wxNOT_FOUND) {
		suggestion = suggestion.Left(pos-1);
	}

	// Get boundaries of text being replaced
	int start,end;
	GetBoundsOfWordAtPosition(currentWordPos,start,end);

	wxString text = GetText();
	SetText(text.Left(std::max(0,start)) + suggestion + text.Mid(end+1));

	// Set selection
	SetSelectionU(start,start+suggestion.Length());
	SetFocus();
}

void SubsTextEditCtrl::OnSetDicLanguage(wxCommandEvent &event) {
	wxArrayString langs = spellchecker->GetLanguageList();

	// Set dictionary
	int index = event.GetId() - EDIT_MENU_DIC_LANGS - 1;
	if (index >= 0) {
		spellchecker->SetLanguage(langs[index]);
		OPT_SET("Tool/Spell Checker/Language")->SetString(STD_STR(langs[index]));
	}

	UpdateStyle();
}

void SubsTextEditCtrl::OnSetThesLanguage(wxCommandEvent &event) {
	wxArrayString langs = thesaurus->GetLanguageList();

	// Set language
	int index = event.GetId() - EDIT_MENU_THES_LANGS - 1;
	wxString lang;
	if (index >= 0) lang = langs[index];
	OPT_SET("Tool/Thesaurus/Language")->SetString(STD_STR(lang));

	UpdateStyle();
}
