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
	Insert(L"あ",L"ア",L"a");
	Insert(L"い",L"イ",L"i");
	Insert(L"う",L"ウ",L"u");
	Insert(L"え",L"エ",L"e");
	Insert(L"お",L"オ",L"o");

	Insert(L"さ",L"サ",L"sa");
	Insert(L"し",L"シ",L"shi");
	Insert(L"す",L"ス",L"su");
	Insert(L"せ",L"セ",L"se");
	Insert(L"そ",L"ソ",L"so");

	Insert(L"た",L"タ",L"ta");
	Insert(L"ち",L"チ",L"chi");
	Insert(L"つ",L"ツ",L"tsu");
	Insert(L"て",L"テ",L"te");
	Insert(L"と",L"ト",L"to");

	Insert(L"な",L"ナ",L"na");
	Insert(L"に",L"ニ",L"ni");
	Insert(L"ぬ",L"ヌ",L"nu");
	Insert(L"ね",L"ネ",L"ne");
	Insert(L"の",L"ノ",L"no");

	Insert(L"は",L"ハ",L"ha");
	Insert(L"ひ",L"ヒ",L"hi");
	Insert(L"ふ",L"フ",L"fu");
	Insert(L"へ",L"ヘ",L"he");
	Insert(L"ほ",L"ホ",L"ho");

	Insert(L"ま",L"マ",L"ma");
	Insert(L"み",L"ミ",L"mi");
	Insert(L"む",L"ム",L"mu");
	Insert(L"め",L"メ",L"me");
	Insert(L"も",L"モ",L"mo");

	Insert(L"や",L"ヤ",L"ya");
	Insert(L"ゆ",L"ユ",L"yu");
	Insert(L"よ",L"ヨ",L"yo");

	Insert(L"ら",L"ラ",L"ra");
	Insert(L"り",L"リ",L"ri");
	Insert(L"る",L"ル",L"ru");
	Insert(L"れ",L"レ",L"re");
	Insert(L"り",L"ロ",L"ro");

	Insert(L"わ",L"ワ",L"wa");
	Insert(L"を",L"ヲ",L"wo");
	Insert(L"ん",L"ン",L"n");

	Insert(L"が",L"ガ",L"ga");
	Insert(L"ぎ",L"ギ",L"gi");
	Insert(L"ぐ",L"グ",L"gu");
	Insert(L"げ",L"ゲ",L"ge");
	Insert(L"ご",L"ゴ",L"go");

	Insert(L"ざ",L"ザ",L"za");
	Insert(L"じ",L"ジ",L"ji");
	Insert(L"ず",L"ズ",L"zu");
	Insert(L"ぜ",L"ゼ",L"ze");
	Insert(L"ぞ",L"ゾ",L"zo");

	Insert(L"だ",L"ダ",L"da");
	Insert(L"ぢ",L"ヂ",L"di");
	Insert(L"づ",L"ヅ",L"du");
	Insert(L"で",L"デ",L"de");
	Insert(L"ど",L"ド",L"do");

	Insert(L"ば",L"バ",L"ba");
	Insert(L"び",L"ビ",L"bi");
	Insert(L"ぶ",L"ブ",L"bu");
	Insert(L"べ",L"ベ",L"be");
	Insert(L"ぼ",L"ボ",L"bo");

	Insert(L"ぱ",L"パ",L"pa");
	Insert(L"ぴ",L"ピ",L"pi");
	Insert(L"ぷ",L"プ",L"pu");
	Insert(L"ぺ",L"ペ",L"pe");
	Insert(L"ぽ",L"ポ",L"po");
}


//////////
// Insert
void KanaTable::Insert(wchar_t *hira,wchar_t *kata,wchar_t *hep) {
#ifdef _UNICODE
	entries.push_back(KanaEntry(hira,kata,hep));
#endif
}
