// Copyright (c) 2006, Rodrigo Braz Monteiro
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


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/zstream.h>
#include <wx/wfstream.h>
#include <stdio.h>
#include "dictionary.h"
#include "../aegisub/text_file_reader.h"
#include "main.h"


///////////////
// Constructor
Dictionary::Dictionary(wxString _name,wxCheckBox *_check) {
	name = _name;
	check = _check;

	// Set file
	wxString filename = TrayDict::folderName + _name + _T(".dic");
	Load(filename);

	// Set checkbox
	wxFileName file(filename);
	check->Enable(file.FileExists());
}


//////////////
// Destructor
Dictionary::~Dictionary() {
}


//////////
// Static
KanaTable Dictionary::kanatable;


////////
// Load
void Dictionary::Convert(wxString source,wxString dest) {
	// Variables
	const wchar_t *str = NULL;
	short len;
	DictEntry curEntry;
	
	try {
		// Open source file
		TextFileReader file(source,_T("EUC-JP"));

		// Open destination file
		wxFileOutputStream out(dest);
		wxBufferedOutputStream buf(out);
		wxZlibOutputStream fp(buf,9,wxZLIB_GZIP);

		// Skip first line
		if (file.HasMoreLines()) file.ReadLineFromFile();

		// Read lines
		while (file.HasMoreLines()) {
			// Get string
			wxString string = file.ReadLineFromFile();

			// Process string to account for lack of kana
			if (!string.Contains(_T("["))) {
				if (!string.Contains(_T("]"))) {
					int pos = string.Find(_T(' '));
					if (pos == -1) continue;
					wxString temp = string;
					string = temp.Left(pos) + _T(" []") + temp.Right(temp.Length() - pos);
				}
				else continue;
			}

			// Tokenize
			wxStringTokenizer token(string,_T("[]"),wxTOKEN_RET_EMPTY);

			// Kanji
			if (token.HasMoreTokens()) {
				curEntry.kanji = token.GetNextToken().Trim(false).Trim(true);
			}
			else continue;

			// Kana & romaji
			if (token.HasMoreTokens()) {
				curEntry.kana = token.GetNextToken().Trim(false).Trim(true);
				if (curEntry.kana.IsEmpty()) curEntry.kana = curEntry.kanji;
				curEntry.romaji = kanatable.KanaToRomaji(curEntry.kana,0);
			}
			else continue;

			// English
			if (token.HasMoreTokens()) {
				curEntry.english = token.GetNextToken().Trim(false).Trim(true);
				curEntry.english = curEntry.english.Mid(1,curEntry.english.Length()-2);
			}
			else continue;

			// Write kanji
			str = curEntry.kanji.c_str();
			len = wcslen(str);
			fp.Write(&len,2);
			fp.Write(str,len*2);

			// Write kana
			str = curEntry.kana.c_str();
			len = wcslen(str);
			fp.Write(&len,2);
			fp.Write(str,len*2);

			// Write romaji
			str = curEntry.romaji.c_str();
			len = wcslen(str);
			fp.Write(&len,2);
			fp.Write(str,len*2);

			// Write english
			str = curEntry.english.c_str();
			len = wcslen(str);
			fp.Write(&len,2);
			fp.Write(str,len*2);
		}
	}

	catch (...) {
		wxMessageBox(_T("Could not find dictionary file: ") + source,_T("File not found!"),wxICON_ERROR);
	}
}


////////
// Load
void Dictionary::Load(wxString filename) {
	dictFile = filename;
}


//////////
// Search
void Dictionary::Search(ResultSet &results,wxString query) {
	// Prepare results
	int resCount = 0;
	results.ownData = true;
	results.dicName = name;
	results.query = query;
	query.Trim(true);
	query.Trim(false);

	// Determine query type
	bool isJapanese = false;
	for (size_t i=0;i<query.Length();i++) {
		if (query[i] > 255) {
			isJapanese = true;
			break;
		}
	}

	// Stopwatch
	wxStopWatch stopwatch;

	// Open file
	wxInputStream *file;
	wxFileInputStream filestream(dictFile);
	wxBufferedInputStream buf(filestream);
	wxZlibInputStream zstream(buf);
	wxBufferedInputStream buf2(zstream);
	file = &zstream;

	// Buffer
	wchar_t buffer[16384];
	short len;
	DictEntry *cur = NULL;

	// Search for matches
	while (!file->Eof()) {
		// Prepare
		bool addThis = false;
		int rel = 0;

		// Create data
		if (!cur) cur = new DictEntry;

		// Read kanji
		file->Read(&len,2);
		if (len < 0) return;
		file->Read(buffer,2*len);
		buffer[len] = 0;
		cur->kanji = buffer;

		// Read kana
		file->Read(&len,2);
		if (len < 0) return;
		file->Read(buffer,2*len);
		buffer[len] = 0;
		cur->kana = buffer;

		// Read romaji
		file->Read(&len,2);
		if (len < 0) return;
		file->Read(buffer,2*len);
		buffer[len] = 0;
		cur->romaji = buffer;

		// Read english
		file->Read(&len,2);
		if (len < 0) return;
		file->Read(buffer,2*len);
		buffer[len] = 0;
		cur->english = buffer;

		// Japanese query
		if (isJapanese) {
			// Matches kanji?
			if (cur->kanji.Contains(query)) {
				addThis = true;
				rel = GetRelevancy(query,cur->kanji,cur->english.Contains(_T("(P)")));
			}

			// Matches kana?
			else if (cur->kana.Contains(query)) {
				addThis = true;
				rel = GetRelevancy(query,cur->kana,cur->english.Contains(_T("(P)")));
			}
		}

		// English/romaji query
		else {
			// Lowercase query
			wxString lowQuery = query.Lower();

			// Matches english?
			if (cur->english.Lower().Contains(lowQuery)) {
				addThis = true;
				rel = GetRelevancy(lowQuery,cur->english,cur->english.Contains(_T("(P)")),true);
			}

			// Matches wapuro romaji?
			if (cur->romaji.Contains(lowQuery)) {
				addThis = true;
				rel = GetRelevancy(lowQuery,cur->romaji,cur->english.Contains(_T("(P)")));
			}
		}

		// Add entry
		if (addThis) {
			SearchResult res;
			res.relevancy = rel;
			res.entry = cur;
			results.results.push_back(res);
			cur = NULL;
		}
	}

	// Delete cur
	if (cur) delete cur;

	// Time
	stopwatch.Pause();
	results.time = stopwatch.Time();

	// Close file
}


/////////////////
// Get relevancy
int Dictionary::GetRelevancy(wxString substr,wxString _str,bool isPop,bool english) {
	// Best score
	int bestScore = 0;

	// Generate list of strings
	wxArrayString strings;
	if (!english) strings.Add(_str.Lower());
	else {
		wxStringTokenizer tkn(_str.Lower(),_T("/"));
		while (tkn.HasMoreTokens()) {
			// Get token
			wxString token = tkn.GetNextToken();

			// Remove parenthesis
			wxString temp;
			bool inside = false;
			bool gotOne = false;
			for (size_t i=0;i<token.Length();i++) {
				if (token[i] == _T('(')) {
					inside = true;
					gotOne = true;
				}
				if (!inside) temp += token[i];
				if (token[i] == _T(')')) inside = false;
			}

			// Add a copy with parenthesis
			if (gotOne) strings.Add(token);

			// Trim & add
			temp.Trim(true);
			temp.Trim(false);
			strings.Add(temp);
		}
	}

	// Search in each match
	for (size_t i=0;i<strings.Count();i++) {
		// Get string
		wxString str = strings[i];
		if (!str.Contains(substr)) continue;

		// Score
		int score = 0;
		if (isPop) score += 5000;

		// Exact match, can't get better
		if (substr == str) {
			score += 10000;
		}

		else {
			// Semi-exact match (to e.g. match "car shed" higher than "card" when looking for "car")
			if (english) {
				wxString temp1 = _T(" ") + str + _T(" ");
				wxString temp2 = _T(" ") + substr + _T(" ");
				if (temp1.Contains(temp2)) score += 5000;
			}

			// Calculate how much of a partial match it was
			score += 1000 - (str.Length() - substr.Length())*1000/str.Length();

			// Find match position
			int start = str.Find(substr);
			if (start == -1) throw 0;
			int temp1 = (str.length() - start)*500/str.Length() + 1;
			int temp2 = (start + substr.Length())*500/str.Length();
			if (temp1 > temp2) score += temp1;
			else score += temp2;
		}

		// Best score?
		if (score > bestScore) bestScore = score;
	}

	return bestScore;
}


//////////////////////////
// Comparison for sorting
bool operator < (const SearchResult &a,const SearchResult &b) {
	return (a.relevancy > b.relevancy);
}


/////////////////
// Compact entry
void DictEntry::Compact() {
	kanji.Trim(true).Trim(false).Shrink();
	kana.Trim(true).Trim(false).Shrink();
	english.Trim(true).Trim(false).Shrink();
}


///////////////
// Constructor
ResultSet::ResultSet() {
	ownData = false;
}


//////////////
// Destructor
ResultSet::~ResultSet() {
	if (ownData) {
		std::list<SearchResult>::iterator cur;
		for (cur = results.begin(); cur != results.end();cur++) {
			delete (*cur).entry;
		}
	}
}


///////////////////
// Print resultset
void ResultSet::Print(wxTextCtrl *target,int bitmask) {
	// Get options
	bool drawKanji = (bitmask & 1) != 0;
	bool drawKana = (bitmask & 2) != 0;
	bool drawRomaji = (bitmask & 4) != 0;
	bool drawEnglish = (bitmask & 8) != 0;

	// Fonts
	wxFont font;
	font.SetFaceName(_T("MS Mincho"));
	font.SetPointSize(9);
	wxFont font2;
	font2.SetFaceName(_T("Tahoma"));

	// Text attributes
	wchar_t space = 0x3000;
	wxString spaceStr = space;
	wxTextAttr fontAttr;
	fontAttr.SetFont(font);
	wxTextAttr kanjiCol;
	kanjiCol.SetFont(font);
	kanjiCol.SetTextColour(wxColour(192,0,0));
	wxTextAttr kanaCol;
	kanaCol.SetTextColour(wxColour(0,0,192));
	wxTextAttr romajiCol;
	romajiCol.SetTextColour(wxColour(0,128,0));
	wxTextAttr engCol;
	engCol.SetFont(font2);
	engCol.SetTextColour(wxColour(0,0,0));
	wxTextAttr commonCol;
	commonCol.SetTextColour(wxColour(0,128,128));
	commonCol.SetFont(font2);
	wxTextAttr sepCol;
	sepCol.SetTextColour(wxColour(128,90,0));
	wxTextAttr boldCol;
	font2.SetWeight(wxBOLD);
	boldCol.SetFont(font2);
	wxTextAttr notBoldCol;
	font2.SetWeight(wxNORMAL);
	notBoldCol.SetFont(font);

	// Find column widths
	int kanjiWidth = 0;
	int kanaWidth = 0;
	int romajiWidth = 0;
	std::list<SearchResult>::iterator cur;
	DictEntry *entry;
	int curLen;
	int resPrinted = 0;
	for (cur=results.begin();cur!=results.end();cur++) {
		entry = cur->entry;
		curLen = entry->kanji.Length();
		if (curLen > kanjiWidth) kanjiWidth = curLen;
		curLen = entry->kana.Length();
		if (curLen > kanaWidth) kanaWidth = curLen;
		wxString temp = Dictionary::kanatable.KanaToRomaji(entry->kana);
		curLen = temp.Length() - temp.Freq(0x304);
		if (curLen > romajiWidth) romajiWidth = curLen;

		// Limit to 1000
		resPrinted++;
		if (resPrinted >= 1000) break;
	}

	// List number of results
	target->SetDefaultStyle(boldCol);
	int maxDisp = results.size();
	if (maxDisp > 1000) maxDisp = 1000;
	target->AppendText(wxString::Format(_T("Searched %s for \"%s\". Displaying %i matches. Search took %i ms.\n"),dicName.Upper().c_str(),query.c_str(),maxDisp,time));
	target->SetDefaultStyle(notBoldCol);
	target->SetDefaultStyle(fontAttr);

	// Append to results
	wxString curText;
	resPrinted = 0;
	for (cur=results.begin();cur!=results.end();cur++) {
		entry = cur->entry;

		// Write kanji
		if (drawKanji) {
			target->SetDefaultStyle(kanjiCol);
			curText = entry->kanji;
			curLen = kanjiWidth - curText.Length() + 1;
			for (int i=0;i<curLen;i++) curText = curText + spaceStr;
			target->AppendText(curText);
		}

		// Write kana
		if (drawKana) {
			target->SetDefaultStyle(kanaCol);
			//curText = _T("[") + entry->kana + _T("]");
			curText = entry->kana;
			curLen = kanaWidth - curText.Length() + 1;
			for (int i=0;i<curLen;i++) curText = curText + spaceStr;
			target->AppendText(curText);
		}

		// Write romaji
		if (drawRomaji) {
			target->SetDefaultStyle(romajiCol);
			curText = Dictionary::kanatable.KanaToRomaji(entry->kana);
			curLen = romajiWidth - curText.Length() + curText.Freq(0x304) + 1;
			for (int i=0;i<curLen;i++) curText = curText + _T(" ");
			target->AppendText(curText);
		}

		// Write english
		if (drawEnglish) {
			// Search for grammatical class
			wxString mainText;
			int pos = entry->english.Find(_T(')'));
			if (entry->english[0] == _T('(') && pos != -1) {
				mainText = entry->english.Mid(pos+1);
				mainText.Trim(false);

				// Draw grammatical class
				target->SetDefaultStyle(sepCol);
				target->AppendText(entry->english.Left(pos+1) + _T(" "));
			}
			else mainText = entry->english;
			target->SetDefaultStyle(engCol);

			// Draw rest
			target->SetDefaultStyle(engCol);
			wxStringTokenizer tkn(mainText,_T("/"));
			bool hadPrev = false;
			while (tkn.HasMoreTokens()) {
				// Popular entry
				wxString token = tkn.GetNextToken();
				if (token == _T("(P)")) {
					target->SetDefaultStyle(commonCol);
					target->AppendText(_T(" [Common]"));
					target->SetDefaultStyle(engCol);
				}

				// Normal entry
				else {
					// Separator
					if (hadPrev) {
						target->SetDefaultStyle(sepCol);
						target->AppendText(_T(" / "));
						target->SetDefaultStyle(engCol);
					}

					// Append text
					target->AppendText(token);
					hadPrev = true;
				}
			}
		}

		// Line break
		target->SetDefaultStyle(fontAttr);
		target->AppendText(_T("\n"));

		// Limit to 1000
		resPrinted++;
		if (resPrinted >= 1000) {
			target->SetDefaultStyle(boldCol);
			target->AppendText(wxString::Format(_T("Too many (%i) matches, stopping.\n"),results.size()));
			break;
		}
	}

	// Print two carriage returns
	target->AppendText(_T("\n\n"));
}
