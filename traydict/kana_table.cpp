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
//   * Neither the name of the TrayDict Group nor the names of its contributors
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
// TRAYDICT
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "kana_table.h"


///////////////
// Constructor
KanaTable::KanaTable() {
	level = 0;
	groups[0] = 0;
	groups[1] = 0;

	BeginGroup();
	Insert(L"\u3042",L"\u30a2",L"a");
	Insert(L"\u3044",L"\u30a4",L"i");
	Insert(L"\u3046",L"\u30a6",L"u");
	Insert(L"\u3048",L"\u30a8",L"e");
	Insert(L"\u304a",L"\u30aa",L"o");

	BeginGroup();
	Insert(L"\u304b",L"\u30ab",L"ka");
	Insert(L"\u304d",L"\u30ad",L"ki");
	Insert(L"\u304f",L"\u30af",L"ku");
	Insert(L"\u3051",L"\u30b1",L"ke");
	Insert(L"\u3053",L"\u30b3",L"ko");

	BeginGroup();
	Insert(L"\u3055",L"\u30b5",L"sa");
	Insert(L"\u3057",L"\u30b7",L"shi");
	Insert(L"\u3059",L"\u30b9",L"su");
	Insert(L"\u305b",L"\u30bb",L"se");
	Insert(L"\u305d",L"\u30bd",L"so");

	BeginGroup();
	Insert(L"\u305f",L"\u30bf",L"ta");
	Insert(L"\u3061",L"\u30c1",L"chi");
	Insert(L"\u3064",L"\u30c4",L"tsu");
	Insert(L"\u3066",L"\u30c6",L"te");
	Insert(L"\u3068",L"\u30c8",L"to");

	BeginGroup();
	Insert(L"\u306a",L"\u30ca",L"na");
	Insert(L"\u306b",L"\u30cb",L"ni");
	Insert(L"\u306c",L"\u30cc",L"nu");
	Insert(L"\u306d",L"\u30cd",L"ne");
	Insert(L"\u306e",L"\u30ce",L"no");

	BeginGroup();
	Insert(L"\u306f",L"\u30cf",L"ha");
	Insert(L"\u3072",L"\u30d2",L"hi");
	Insert(L"\u3075",L"\u30d5",L"fu");
	Insert(L"\u3078",L"\u30d8",L"he");
	Insert(L"\u307b",L"\u30db",L"ho");

	BeginGroup();
	Insert(L"\u307e",L"\u30de",L"ma");
	Insert(L"\u307f",L"\u30df",L"mi");
	Insert(L"\u3080",L"\u30e0",L"mu");
	Insert(L"\u3081",L"\u30e1",L"me");
	Insert(L"\u3082",L"\u30e2",L"mo");

	BeginGroup();
	Insert(L"\u3084",L"\u30e4",L"ya");
	Insert(L"\u3086",L"\u30e6",L"yu");
	Insert(L"\u3088",L"\u30e8",L"yo");

	BeginGroup();
	Insert(L"\u3089",L"\u30e9",L"ra");
	Insert(L"\u308a",L"\u30ea",L"ri");
	Insert(L"\u308b",L"\u30eb",L"ru");
	Insert(L"\u308c",L"\u30ec",L"re");
	Insert(L"\u308d",L"\u30ed",L"ro");

	BeginGroup();
	Insert(L"\u308f",L"\u30ef",L"wa");
	Insert(L"\u3090",L"\u30f0",L"wi");
	Insert(L"\u3091",L"\u30f1",L"we");
	Insert(L"\u3092",L"\u30f2",L"wo");

	BeginGroup();
	level--;
	Insert(L"\u3093",L"\u30f3",L"n");

	BeginGroup();
	Insert(L"\u304c",L"\u30ac",L"ga");
	Insert(L"\u304e",L"\u30ae",L"gi");
	Insert(L"\u3050",L"\u30b0",L"gu");
	Insert(L"\u3052",L"\u30b2",L"ge");
	Insert(L"\u3054",L"\u30b4",L"go");

	BeginGroup();
	Insert(L"\u3056",L"\u30b6",L"za");
	Insert(L"\u3058",L"\u30b8",L"ji");
	Insert(L"\u305a",L"\u30ba",L"zu");
	Insert(L"\u305c",L"\u30bc",L"ze");
	Insert(L"\u305e",L"\u30be",L"zo");

	BeginGroup();
	Insert(L"\u3060",L"\u30c0",L"da");
	Insert(L"\u3062",L"\u30c2",L"ji");
	Insert(L"\u3065",L"\u30c5",L"zu");
	Insert(L"\u3067",L"\u30c7",L"de");
	Insert(L"\u3069",L"\u30c9",L"do");

	BeginGroup();
	Insert(L"\u3070",L"\u30d0",L"ba");
	Insert(L"\u3073",L"\u30d3",L"bi");
	Insert(L"\u3076",L"\u30d6",L"bu");
	Insert(L"\u3079",L"\u30d9",L"be");
	Insert(L"\u307c",L"\u30dc",L"bo");

	BeginGroup();
	Insert(L"\u3071",L"\u30d1",L"pa");
	Insert(L"\u3074",L"\u30d4",L"pi");
	Insert(L"\u3077",L"\u30d7",L"pu");
	Insert(L"\u307a",L"\u30da",L"pe");
	Insert(L"\u307d",L"\u30dd",L"po");

	BeginGroup();
	Insert(L"\u304d\u3083",L"\u30ad\u30e3",L"kya");
	Insert(L"\u304d\u3085",L"\u30ad\u30e5",L"kyu");
	Insert(L"\u304d\u3087",L"\u30ad\u30e7",L"kyo");

	BeginGroup();
	Insert(L"\u3057\u3083",L"\u30b7\u30e3",L"sha");
	Insert(L"\u3057\u3085",L"\u30b7\u30e5",L"shu");
	Insert(L"\u3057\u3087",L"\u30b7\u30e7",L"sho");

	BeginGroup();
	Insert(L"\u3061\u3083",L"\u30c1\u30e3",L"cha");
	Insert(L"\u3061\u3085",L"\u30c1\u30e5",L"chu");
	Insert(L"\u3061\u3087",L"\u30c1\u30e7",L"cho");

	BeginGroup();
	Insert(L"\u306b\u3083",L"\u30cb\u30e3",L"nya");
	Insert(L"\u306b\u3085",L"\u30cb\u30e5",L"nyu");
	Insert(L"\u306b\u3087",L"\u30cb\u30e7",L"nyo");

	BeginGroup();
	Insert(L"\u3072\u3083",L"\u30d2\u30e3",L"hya");
	Insert(L"\u3072\u3085",L"\u30d2\u30e5",L"hyu");
	Insert(L"\u3072\u3087",L"\u30d2\u30e7",L"hyo");

	BeginGroup();
	Insert(L"\u307f\u3083",L"\u30df\u30e3",L"mya");
	Insert(L"\u307f\u3085",L"\u30df\u30e5",L"myu");
	Insert(L"\u307f\u3087",L"\u30df\u30e7",L"myo");

	BeginGroup();
	Insert(L"\u308a\u3083",L"\u30ea\u30e3",L"rya");
	Insert(L"\u308a\u3085",L"\u30ea\u30e5",L"ryu");
	Insert(L"\u308a\u3087",L"\u30ea\u30e7",L"ryo");

	BeginGroup();
	Insert(L"\u304e\u3083",L"\u30ae\u30e3",L"gya");
	Insert(L"\u304e\u3085",L"\u30ae\u30e5",L"gyu");
	Insert(L"\u304e\u3087",L"\u30ae\u30e7",L"gyo");

	BeginGroup();
	Insert(L"\u3058\u3083",L"\u30b8\u30e3",L"ja");
	Insert(L"\u3058\u3085",L"\u30b8\u30e5",L"ju");
	Insert(L"\u3058\u3087",L"\u30b8\u30e7",L"jo");

	BeginGroup();
	Insert(L"\u3062\u3083",L"\u30c2\u30e3",L"ja");
	Insert(L"\u3062\u3085",L"\u30c2\u30e5",L"ju");
	Insert(L"\u3062\u3087",L"\u30c2\u30e7",L"jo");

	BeginGroup();
	Insert(L"\u3073\u3083",L"\u30d3\u30e3",L"bya");
	Insert(L"\u3073\u3085",L"\u30d3\u30e5",L"byu");
	Insert(L"\u3073\u3087",L"\u30d3\u30e7",L"byo");

	BeginGroup();
	Insert(L"\u3074\u3083",L"\u30d4\u30e3",L"pya");
	Insert(L"\u3074\u3085",L"\u30d4\u30e5",L"pyu");
	Insert(L"\u3074\u3087",L"\u30d4\u30e7",L"pyo");

	BeginGroup();
	Insert(L"",L"\u30d5\u30a1",L"fa");
	Insert(L"",L"\u30d5\u30a3",L"fi");
	Insert(L"",L"\u30d5\u30a7",L"fe");
	Insert(L"",L"\u30d5\u30a9",L"fo");

	BeginGroup();
	Insert(L"",L"\u30f4\u30a1",L"va");
	Insert(L"",L"\u30f4\u30a3",L"vi");
	Insert(L"",L"\u30f4",L"vu");
	Insert(L"",L"\u30f4\u30a7",L"ve");
	Insert(L"",L"\u30f4\u30a9",L"vo");
	Insert(L"",L"\u30d5\u30e5",L"fyu");

	BeginGroup();
	Insert(L"",L"\u30a4\u30a7",L"ye");
	Insert(L"",L"\u30a6\u30a3",L"wi");
	Insert(L"",L"\u30a6\u30a7",L"we");
	Insert(L"",L"\u30a6\u30a9",L"wo");

	BeginGroup();
	Insert(L"",L"\u30f4\u30e3",L"vya");
	Insert(L"",L"\u30f4\u30e5",L"vyu");
	Insert(L"",L"\u30f4\u30e7",L"vyo");

	BeginGroup();
	Insert(L"",L"\u30b7\u30a7",L"she");
	Insert(L"",L"\u30b8\u30a7",L"je");
	Insert(L"",L"\u30c1\u30a7",L"che");

	BeginGroup();
	Insert(L"",L"\u30c6\u30a3",L"ti");
	Insert(L"",L"\u30c6\u30a5",L"tu");
	Insert(L"",L"\u30c6\u30e5",L"tyu");

	BeginGroup();
	Insert(L"",L"\u30c7\u30a3",L"di");
	Insert(L"",L"\u30c7\u30a5",L"du");
	Insert(L"",L"\u30c7\u30a5",L"dyu");

	BeginGroup();
	Insert(L"",L"\u30c4\u30a1",L"tsa");
	Insert(L"",L"\u30c4\u30a3",L"tsi");
	Insert(L"",L"\u30c4\u30a7",L"tse");
	Insert(L"",L"\u30c4\u30a9",L"tso");
}


//////////////
// Destructor
KanaTable::~KanaTable() {
}


///////////////
// Begin group
void KanaTable::BeginGroup() {
	curGroup = _T("");
	level++;
}


//////////
// Insert
void KanaTable::Insert(wchar_t *hira,wchar_t *kata,wchar_t *hep) {
#ifdef _UNICODE
	KanaEntry entry(hira,kata,hep);
	if (curGroup.IsEmpty()) curGroup = hep;
	entry.group = curGroup;
	entry.level = level;
	if (!entry.hiragana.IsEmpty() && level > groups[0]) groups[0] = level;
	if (!entry.katakana.IsEmpty() && level > groups[1]) groups[1] = level;
	entries.push_back(entry);
#endif
}


/////////////////////
// Number of entries
int KanaTable::GetNumberEntries(int level) const {
	if (level == -1) return entries.size();
	else {
		int count = 0;
		int n = entries.size();
		for (int i=0;i<n;i++) {
			if (entries[i].level <= level) count++;
		}
		return count;
	}
}


////////////////////////
// Get a specific entry
const KanaEntry &KanaTable::GetEntry(int i) const {
	return entries.at(i);
}


//////////////////////////
// Find a specific romaji
const KanaEntry *KanaTable::FindByRomaji(wxString romaji) const {
	int n = entries.size();
	for (int i=0;i<n;i++) {
		if (entries[i].hepburn == romaji) return &entries[i];
	}
	return NULL;
}


////////////////////////
// Find a specific kana
const KanaEntry *KanaTable::FindByKana(wxString kana) const {
	int n = entries.size();
	for (int i=0;i<n;i++) {
		if (entries[i].hiragana == kana) return &entries[i];
		if (entries[i].katakana == kana) return &entries[i];
	}
	return NULL;
}


/////////////////////////////////////////////
// Get number of levels for a specific table
int KanaTable::GetLevels(int table) const {
	return groups[table];
}


//////////////////////////
// Convert kana to romaji
wxString KanaTable::KanaToRomaji(wxString kana,int type) {
	// Prepare
	wxString lastSyl;
	wxString final;
	bool ltsu = false;
	bool longVowel = false;

	// Look up the entries
	for (size_t i=0;i<kana.Length();i++) {
		// Find syllable
		const KanaEntry *cur;
		cur = FindByKana(kana.Mid(i,2));
		if (cur) i++;
		else cur = FindByKana(kana.Mid(i,1));

		// Check if it's little tsu or long vowel in katakana
		if (!cur) {
			if (kana.Mid(i,1) == _T("\u3063") || kana.Mid(i,1) == _T("\uff6f") || kana.Mid(i,1) == _T("\u30c3")) {
				ltsu = true;
				continue;
			}

			if (kana.Mid(i,1) == _T("\u30fc") || kana.Mid(i,1) == _T("-")) {
				longVowel = true;
			}
		}

		// Append
		if (cur || longVowel) {
			bool vetoAdd = false;

			// Hepburn
			if (type == 1) {
				if (longVowel) {
					longVowel = false;
					final += 0x304;
					vetoAdd = true;
				}

				else {
					// Check for need to add apostrophe
					wxString fl;
					if (lastSyl == _T("n")) {
						fl = cur->hepburn.Left(1);
						bool add = false;
						if (fl == _T("y") || fl == _T("a") || fl == _T("e") || fl == _T("i") || fl == _T("o") || fl == _T("u")) add = true;
						if (fl == _T("n") && cur->hiragana == _T("\u3063")) add = true;
						if (add) final += _T("'");
					}

					// Check if it needs to add a macron
					wxString last = lastSyl.Right(1);
					wxString curV = cur->hepburn;
					if ((last == _T("o") && curV == _T("u")) || (last == curV && last != _T("n") /*&& last != lastSyl */)) {
						wchar_t macron;
						switch(last[0]) {
							case L'a': macron = 257; break;
							case L'e': macron = 275; break;
							case L'i': macron = 299; break;
							case L'o': macron = 333; break;
							case L'u': macron = 363; break;
						}
						final = final.Left(final.Length()-1) + macron;
						vetoAdd = true;
					}
				}
			}

			// Wopura
			else {
				if (longVowel) {
					longVowel = false;
					final += lastSyl.Right(1);
					vetoAdd = true;
				}
			}

			// Add syllable
			if (!vetoAdd) {
				// Little tsu
				if (ltsu) {
					ltsu = false;
					final += cur->hepburn.Left(1);
				}

				// Standard
				final += cur->hepburn;
			}

			// Set last
			if (cur) lastSyl = cur->hepburn;
		}
	}
	return final;
}
