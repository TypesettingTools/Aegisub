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
#include "kana_table.h"


///////////////
// Constructor
KanaTable::KanaTable() {
	level = 0;
	groups[0] = 0;
	groups[1] = 0;

	BeginGroup();
	Insert(L"あ",L"ア",L"a");
	Insert(L"い",L"イ",L"i");
	Insert(L"う",L"ウ",L"u");
	Insert(L"え",L"エ",L"e");
	Insert(L"お",L"オ",L"o");

	BeginGroup();
	Insert(L"か",L"カ",L"ka");
	Insert(L"き",L"キ",L"ki");
	Insert(L"く",L"ク",L"ku");
	Insert(L"け",L"ケ",L"ke");
	Insert(L"こ",L"コ",L"ko");

	BeginGroup();
	Insert(L"さ",L"サ",L"sa");
	Insert(L"し",L"シ",L"shi");
	Insert(L"す",L"ス",L"su");
	Insert(L"せ",L"セ",L"se");
	Insert(L"そ",L"ソ",L"so");

	BeginGroup();
	Insert(L"た",L"タ",L"ta");
	Insert(L"ち",L"チ",L"chi");
	Insert(L"つ",L"ツ",L"tsu");
	Insert(L"て",L"テ",L"te");
	Insert(L"と",L"ト",L"to");

	BeginGroup();
	Insert(L"な",L"ナ",L"na");
	Insert(L"に",L"ニ",L"ni");
	Insert(L"ぬ",L"ヌ",L"nu");
	Insert(L"ね",L"ネ",L"ne");
	Insert(L"の",L"ノ",L"no");

	BeginGroup();
	Insert(L"は",L"ハ",L"ha");
	Insert(L"ひ",L"ヒ",L"hi");
	Insert(L"ふ",L"フ",L"fu");
	Insert(L"へ",L"ヘ",L"he");
	Insert(L"ほ",L"ホ",L"ho");

	BeginGroup();
	Insert(L"ま",L"マ",L"ma");
	Insert(L"み",L"ミ",L"mi");
	Insert(L"む",L"ム",L"mu");
	Insert(L"め",L"メ",L"me");
	Insert(L"も",L"モ",L"mo");

	BeginGroup();
	Insert(L"や",L"ヤ",L"ya");
	Insert(L"ゆ",L"ユ",L"yu");
	Insert(L"よ",L"ヨ",L"yo");

	BeginGroup();
	Insert(L"ら",L"ラ",L"ra");
	Insert(L"り",L"リ",L"ri");
	Insert(L"る",L"ル",L"ru");
	Insert(L"れ",L"レ",L"re");
	Insert(L"ろ",L"ロ",L"ro");

	BeginGroup();
	Insert(L"わ",L"ワ",L"wa");
	Insert(L"ゐ",L"ヰ",L"wi");
	Insert(L"ゑ",L"ヱ",L"we");
	Insert(L"を",L"ヲ",L"wo");

	BeginGroup();
	level--;
	Insert(L"ん",L"ン",L"n");

	BeginGroup();
	Insert(L"が",L"ガ",L"ga");
	Insert(L"ぎ",L"ギ",L"gi");
	Insert(L"ぐ",L"グ",L"gu");
	Insert(L"げ",L"ゲ",L"ge");
	Insert(L"ご",L"ゴ",L"go");

	BeginGroup();
	Insert(L"ざ",L"ザ",L"za");
	Insert(L"じ",L"ジ",L"ji");
	Insert(L"ず",L"ズ",L"zu");
	Insert(L"ぜ",L"ゼ",L"ze");
	Insert(L"ぞ",L"ゾ",L"zo");

	BeginGroup();
	Insert(L"だ",L"ダ",L"da");
	Insert(L"ぢ",L"ヂ",L"ji");
	Insert(L"づ",L"ヅ",L"zu");
	Insert(L"で",L"デ",L"de");
	Insert(L"ど",L"ド",L"do");

	BeginGroup();
	Insert(L"ば",L"バ",L"ba");
	Insert(L"び",L"ビ",L"bi");
	Insert(L"ぶ",L"ブ",L"bu");
	Insert(L"べ",L"ベ",L"be");
	Insert(L"ぼ",L"ボ",L"bo");

	BeginGroup();
	Insert(L"ぱ",L"パ",L"pa");
	Insert(L"ぴ",L"ピ",L"pi");
	Insert(L"ぷ",L"プ",L"pu");
	Insert(L"ぺ",L"ペ",L"pe");
	Insert(L"ぽ",L"ポ",L"po");

	BeginGroup();
	Insert(L"きゃ",L"キャ",L"kya");
	Insert(L"きゅ",L"キュ",L"kyu");
	Insert(L"きょ",L"キョ",L"kyo");

	BeginGroup();
	Insert(L"しゃ",L"シャ",L"sha");
	Insert(L"しゅ",L"シュ",L"shu");
	Insert(L"しょ",L"ショ",L"sho");

	BeginGroup();
	Insert(L"ちゃ",L"チャ",L"cha");
	Insert(L"ちゅ",L"チュ",L"chu");
	Insert(L"ちょ",L"チョ",L"cho");

	BeginGroup();
	Insert(L"にゃ",L"ニャ",L"nya");
	Insert(L"にゅ",L"ニュ",L"nyu");
	Insert(L"にょ",L"ニョ",L"nyo");

	BeginGroup();
	Insert(L"ひゃ",L"ヒャ",L"hya");
	Insert(L"ひゅ",L"ヒュ",L"hyu");
	Insert(L"ひょ",L"ヒョ",L"hyo");

	BeginGroup();
	Insert(L"みゃ",L"ミャ",L"mya");
	Insert(L"みゅ",L"ミュ",L"myu");
	Insert(L"みょ",L"ミョ",L"myo");

	BeginGroup();
	Insert(L"りゃ",L"リャ",L"rya");
	Insert(L"りゅ",L"リュ",L"ryu");
	Insert(L"りょ",L"リョ",L"ryo");

	BeginGroup();
	Insert(L"ぎゃ",L"ギャ",L"gya");
	Insert(L"ぎゅ",L"ギュ",L"gyu");
	Insert(L"ぎょ",L"ギョ",L"gyo");

	BeginGroup();
	Insert(L"じゃ",L"ジャ",L"ja");
	Insert(L"じゅ",L"ジュ",L"ju");
	Insert(L"じょ",L"ジョ",L"jo");

	BeginGroup();
	Insert(L"ぢゃ",L"ヂャ",L"ja");
	Insert(L"ぢゅ",L"ヂュ",L"ju");
	Insert(L"ぢょ",L"ヂョ",L"jo");

	BeginGroup();
	Insert(L"びゃ",L"ビャ",L"bya");
	Insert(L"びゅ",L"ビュ",L"byu");
	Insert(L"びょ",L"ビョ",L"byo");

	BeginGroup();
	Insert(L"ぴゃ",L"ピャ",L"pya");
	Insert(L"ぴゅ",L"ピュ",L"pyu");
	Insert(L"ぴょ",L"ピョ",L"pyo");

	BeginGroup();
	Insert(L"",L"ファ",L"fa");
	Insert(L"",L"フィ",L"fi");
	Insert(L"",L"フェ",L"fe");
	Insert(L"",L"フォ",L"fo");

	BeginGroup();
	Insert(L"",L"ヴァ",L"va");
	Insert(L"",L"ヴィ",L"vi");
	Insert(L"",L"ヴ",L"vu");
	Insert(L"",L"ヴェ",L"ve");
	Insert(L"",L"ヴォ",L"vo");
	Insert(L"",L"フュ",L"fyu");

	BeginGroup();
	Insert(L"",L"イェ",L"ye");
	Insert(L"",L"ウィ",L"wi");
	Insert(L"",L"ウェ",L"we");
	Insert(L"",L"ウォ",L"wo");

	BeginGroup();
	Insert(L"",L"ヴャ",L"vya");
	Insert(L"",L"ヴュ",L"vyu");
	Insert(L"",L"ヴョ",L"vyo");

	BeginGroup();
	Insert(L"",L"シェ",L"she");
	Insert(L"",L"ジェ",L"je");
	Insert(L"",L"チェ",L"che");

	BeginGroup();
	Insert(L"",L"ティ",L"ti");
	Insert(L"",L"テゥ",L"tu");
	Insert(L"",L"テュ",L"tyu");

	BeginGroup();
	Insert(L"",L"ディ",L"di");
	Insert(L"",L"デゥ",L"du");
	Insert(L"",L"デゥ",L"dyu");

	BeginGroup();
	Insert(L"",L"ツァ",L"tsa");
	Insert(L"",L"ツィ",L"tsi");
	Insert(L"",L"ツェ",L"tse");
	Insert(L"",L"ツォ",L"tso");
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
			if (kana.Mid(i,1) == _T("っ") || kana.Mid(i,1) == _T("ｯ") || kana.Mid(i,1) == _T("ッ")) {
				ltsu = true;
				continue;
			}

			if (kana.Mid(i,1) == _T("ー") || kana.Mid(i,1) == _T("-")) {
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
						if (fl == _T("n") && cur->hiragana == _T("っ")) add = true;
						if (add) final += _T("'");
					}

					// Check if it needs to add a macron
					wxString last = lastSyl.Right(1);
					wxString curV = cur->hepburn;
					if ((last == _T("o") && curV == _T("u")) || (last == curV && last != lastSyl && last != _T("n"))) {
						final += 0x304;
						vetoAdd = true;
					}
				}
			}

			// Wapura
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
