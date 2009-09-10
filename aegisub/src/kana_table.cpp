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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file kana_table.cpp
/// @brief Data about the Japanese kana syllabary used by kanji karaoke timing copying
/// @ingroup kara_timing_copy
///


#include "config.h"

#include "kana_table.h"


/// @brief Constructor
KanaTable::KanaTable()
{

	// Regular kana usage and combinations
	Insert(L"\u3042",L"\u30a2",L"a");
	Insert(L"\u3044",L"\u30a4",L"i");
	Insert(L"\u3046",L"\u30a6",L"u");
	Insert(L"\u3048",L"\u30a8",L"e");
	Insert(L"\u304a",L"\u30aa",L"o");

	Insert(L"\u304b",L"\u30ab",L"ka");
	Insert(L"\u304d",L"\u30ad",L"ki");
	Insert(L"\u304f",L"\u30af",L"ku");
	Insert(L"\u3051",L"\u30b1",L"ke");
	Insert(L"\u3053",L"\u30b3",L"ko");

	Insert(L"\u3055",L"\u30b5",L"sa");
	Insert(L"\u3057",L"\u30b7",L"shi");
	Insert(L"\u3059",L"\u30b9",L"su");
	Insert(L"\u305b",L"\u30bb",L"se");
	Insert(L"\u305d",L"\u30bd",L"so");

	Insert(L"\u305f",L"\u30bf",L"ta");
	Insert(L"\u3061",L"\u30c1",L"chi");
	Insert(L"\u3064",L"\u30c4",L"tsu");
	Insert(L"\u3066",L"\u30c6",L"te");
	Insert(L"\u3068",L"\u30c8",L"to");

	Insert(L"\u306a",L"\u30ca",L"na");
	Insert(L"\u306b",L"\u30cb",L"ni");
	Insert(L"\u306c",L"\u30cc",L"nu");
	Insert(L"\u306d",L"\u30cd",L"ne");
	Insert(L"\u306e",L"\u30ce",L"no");

	Insert(L"\u306f",L"\u30cf",L"ha");
	Insert(L"\u3072",L"\u30d2",L"hi");
	Insert(L"\u3075",L"\u30d5",L"fu");
	Insert(L"\u3078",L"\u30d8",L"he");
	Insert(L"\u307b",L"\u30db",L"ho");

	Insert(L"\u307e",L"\u30de",L"ma");
	Insert(L"\u307f",L"\u30df",L"mi");
	Insert(L"\u3080",L"\u30e0",L"mu");
	Insert(L"\u3081",L"\u30e1",L"me");
	Insert(L"\u3082",L"\u30e2",L"mo");

	Insert(L"\u3084",L"\u30e4",L"ya");
	Insert(L"\u3086",L"\u30e6",L"yu");
	Insert(L"\u3088",L"\u30e8",L"yo");

	Insert(L"\u3089",L"\u30e9",L"ra");
	Insert(L"\u308a",L"\u30ea",L"ri");
	Insert(L"\u308b",L"\u30eb",L"ru");
	Insert(L"\u308c",L"\u30ec",L"re");
	Insert(L"\u308d",L"\u30ed",L"ro");

	Insert(L"\u308f",L"\u30ef",L"wa");
	Insert(L"\u3090",L"\u30f0",L"wi");
	Insert(L"\u3091",L"\u30f1",L"we");
	Insert(L"\u3092",L"\u30f2",L"wo");

	Insert(L"\u304c",L"\u30ac",L"ga");
	Insert(L"\u304e",L"\u30ae",L"gi");
	Insert(L"\u3050",L"\u30b0",L"gu");
	Insert(L"\u3052",L"\u30b2",L"ge");
	Insert(L"\u3054",L"\u30b4",L"go");

	Insert(L"\u3056",L"\u30b6",L"za");
	Insert(L"\u3058",L"\u30b8",L"ji");
	Insert(L"\u305a",L"\u30ba",L"zu");
	Insert(L"\u305c",L"\u30bc",L"ze");
	Insert(L"\u305e",L"\u30be",L"zo");

	Insert(L"\u3060",L"\u30c0",L"da");
	Insert(L"\u3062",L"\u30c2",L"ji");
	Insert(L"\u3065",L"\u30c5",L"zu");
	Insert(L"\u3067",L"\u30c7",L"de");
	Insert(L"\u3069",L"\u30c9",L"do");

	Insert(L"\u3070",L"\u30d0",L"ba");
	Insert(L"\u3073",L"\u30d3",L"bi");
	Insert(L"\u3076",L"\u30d6",L"bu");
	Insert(L"\u3079",L"\u30d9",L"be");
	Insert(L"\u307c",L"\u30dc",L"bo");

	Insert(L"\u3071",L"\u30d1",L"pa");
	Insert(L"\u3074",L"\u30d4",L"pi");
	Insert(L"\u3077",L"\u30d7",L"pu");
	Insert(L"\u307a",L"\u30da",L"pe");
	Insert(L"\u307d",L"\u30dd",L"po");

	Insert(L"\u304d\u3083",L"\u30ad\u30e3",L"kya");
	Insert(L"\u304d\u3085",L"\u30ad\u30e5",L"kyu");
	Insert(L"\u304d\u3087",L"\u30ad\u30e7",L"kyo");

	Insert(L"\u3057\u3083",L"\u30b7\u30e3",L"sha");
	Insert(L"\u3057\u3085",L"\u30b7\u30e5",L"shu");
	Insert(L"\u3057\u3087",L"\u30b7\u30e7",L"sho");

	Insert(L"\u3061\u3083",L"\u30c1\u30e3",L"cha");
	Insert(L"\u3061\u3085",L"\u30c1\u30e5",L"chu");
	Insert(L"\u3061\u3087",L"\u30c1\u30e7",L"cho");

	Insert(L"\u306b\u3083",L"\u30cb\u30e3",L"nya");
	Insert(L"\u306b\u3085",L"\u30cb\u30e5",L"nyu");
	Insert(L"\u306b\u3087",L"\u30cb\u30e7",L"nyo");

	Insert(L"\u3072\u3083",L"\u30d2\u30e3",L"hya");
	Insert(L"\u3072\u3085",L"\u30d2\u30e5",L"hyu");
	Insert(L"\u3072\u3087",L"\u30d2\u30e7",L"hyo");

	Insert(L"\u307f\u3083",L"\u30df\u30e3",L"mya");
	Insert(L"\u307f\u3085",L"\u30df\u30e5",L"myu");
	Insert(L"\u307f\u3087",L"\u30df\u30e7",L"myo");

	Insert(L"\u308a\u3083",L"\u30ea\u30e3",L"rya");
	Insert(L"\u308a\u3085",L"\u30ea\u30e5",L"ryu");
	Insert(L"\u308a\u3087",L"\u30ea\u30e7",L"ryo");

	Insert(L"\u304e\u3083",L"\u30ae\u30e3",L"gya");
	Insert(L"\u304e\u3085",L"\u30ae\u30e5",L"gyu");
	Insert(L"\u304e\u3087",L"\u30ae\u30e7",L"gyo");

	Insert(L"\u3058\u3083",L"\u30b8\u30e3",L"ja");
	Insert(L"\u3058\u3085",L"\u30b8\u30e5",L"ju");
	Insert(L"\u3058\u3087",L"\u30b8\u30e7",L"jo");

	Insert(L"\u3062\u3083",L"\u30c2\u30e3",L"ja");
	Insert(L"\u3062\u3085",L"\u30c2\u30e5",L"ju");
	Insert(L"\u3062\u3087",L"\u30c2\u30e7",L"jo");

	Insert(L"\u3073\u3083",L"\u30d3\u30e3",L"bya");
	Insert(L"\u3073\u3085",L"\u30d3\u30e5",L"byu");
	Insert(L"\u3073\u3087",L"\u30d3\u30e7",L"byo");

	Insert(L"\u3074\u3083",L"\u30d4\u30e3",L"pya");
	Insert(L"\u3074\u3085",L"\u30d4\u30e5",L"pyu");
	Insert(L"\u3074\u3087",L"\u30d4\u30e7",L"pyo");


	// Specialty katakana usage for loan words

	// Katakana fu + small vowel
	Insert(L"",L"\u30d5\u30a1",L"fa");
	Insert(L"",L"\u30d5\u30a3",L"fi");
	Insert(L"",L"\u30d5\u30a7",L"fe");
	Insert(L"",L"\u30d5\u30a9",L"fo");

	// Katakana vu + small vowel
	Insert(L"",L"\u30f4\u30a1",L"va");
	Insert(L"",L"\u30f4\u30a3",L"vi");
	Insert(L"",L"\u30f4",L"vu");
	Insert(L"",L"\u30f4\u30a7",L"ve");
	Insert(L"",L"\u30f4\u30a9",L"vo");

	// Katakana fu + small yu
	Insert(L"",L"\u30d5\u30e5",L"fyu");

	// Katakana i + little e
	Insert(L"",L"\u30a4\u30a7",L"ye");

	// Katakana u + little vowels
	Insert(L"",L"\u30a6\u30a3",L"wi");
	Insert(L"",L"\u30a6\u30a7",L"we");
	Insert(L"",L"\u30a6\u30a9",L"wo");

	// Katakana vu + small ya-yu-yo
	Insert(L"",L"\u30f4\u30e3",L"vya");
	Insert(L"",L"\u30f4\u30e5",L"vyu");
	Insert(L"",L"\u30f4\u30e7",L"vyo");

	// Katakana shi-ji-chi + small e
	Insert(L"",L"\u30b7\u30a7",L"she");
	Insert(L"",L"\u30b8\u30a7",L"je");
	Insert(L"",L"\u30c1\u30a7",L"che");

	// Katakana de + small i-u-yu
	Insert(L"",L"\u30c6\u30a3",L"ti");
	Insert(L"",L"\u30c6\u30a5",L"tu");
	Insert(L"",L"\u30c6\u30e5",L"tyu");

	// Katakana de + small i-u-yu
	Insert(L"",L"\u30c7\u30a3",L"di");
	Insert(L"",L"\u30c7\u30a5",L"du");
	Insert(L"",L"\u30c7\u30a5",L"dyu");

	// Katakana tsu + small vowels
	Insert(L"",L"\u30c4\u30a1",L"tsa");
	Insert(L"",L"\u30c4\u30a3",L"tsi");
	Insert(L"",L"\u30c4\u30a7",L"tse");
	Insert(L"",L"\u30c4\u30a9",L"tso");


	// Syllablic consonants

	// Small tsu
	Insert(L"\u3063",L"\u30c3",L"t");
	Insert(L"\u3063",L"\u30c3",L"c");
	Insert(L"\u3063",L"\u30c3",L"s");
	Insert(L"\u3063",L"\u30c3",L"k");
	Insert(L"\u3063",L"\u30c3",L"p");

	// Syllabic n
	Insert(L"\u3093",L"\u30f3",L"n");
	Insert(L"\u3093",L"\u30f3",L"m");


	// Other special usage

	// Small vowels
	Insert(L"\u3041",L"\u30a1",L"a");
	Insert(L"\u3043",L"\u30a3",L"i");
	Insert(L"\u3045",L"\u30a5",L"u");
	Insert(L"\u3047",L"\u30a7",L"e");
	Insert(L"\u3049",L"\u30a9",L"o");

	// Long vowel mark (dash)
	Insert(L"",L"\u30fc",L"a");
	Insert(L"",L"\u30fc",L"i");
	Insert(L"",L"\u30fc",L"u");
	Insert(L"",L"\u30fc",L"e");
	Insert(L"",L"\u30fc",L"o");
}


/// @brief Destructor
KanaTable::~KanaTable()
{
	// Do nothing
}



/// @brief Add Hiragana, Katakana and hepburn romaji tuple.
/// @param hira Hiragana to add.
/// @param kata Katakana to add.
/// @param hep Hepburn romaji to add.
///
void KanaTable::Insert(const wchar_t *hira, const wchar_t *kata, const wchar_t *hep)
{
	entries.push_back(KanaEntry(hira,kata,hep));
}
