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

const KanaEntry KanaTable[] =
{
	// Regular kana usage and combinations
	{ L"\u3042", L"\u30a2", L"a" },
	{ L"\u3044", L"\u30a4", L"i" },
	{ L"\u3046", L"\u30a6", L"u" },
	{ L"\u3048", L"\u30a8", L"e" },
	{ L"\u304a", L"\u30aa", L"o" },

	{ L"\u304b", L"\u30ab", L"ka" },
	{ L"\u304d", L"\u30ad", L"ki" },
	{ L"\u304f", L"\u30af", L"ku" },
	{ L"\u3051", L"\u30b1", L"ke" },
	{ L"\u3053", L"\u30b3", L"ko" },

	{ L"\u3055", L"\u30b5", L"sa" },
	{ L"\u3057", L"\u30b7", L"shi" },
	{ L"\u3059", L"\u30b9", L"su" },
	{ L"\u305b", L"\u30bb", L"se" },
	{ L"\u305d", L"\u30bd", L"so" },

	{ L"\u305f", L"\u30bf", L"ta" },
	{ L"\u3061", L"\u30c1", L"chi" },
	{ L"\u3064", L"\u30c4", L"tsu" },
	{ L"\u3066", L"\u30c6", L"te" },
	{ L"\u3068", L"\u30c8", L"to" },

	{ L"\u306a", L"\u30ca", L"na" },
	{ L"\u306b", L"\u30cb", L"ni" },
	{ L"\u306c", L"\u30cc", L"nu" },
	{ L"\u306d", L"\u30cd", L"ne" },
	{ L"\u306e", L"\u30ce", L"no" },

	{ L"\u306f", L"\u30cf", L"ha" },
	{ L"\u3072", L"\u30d2", L"hi" },
	{ L"\u3075", L"\u30d5", L"fu" },
	{ L"\u3078", L"\u30d8", L"he" },
	{ L"\u307b", L"\u30db", L"ho" },

	{ L"\u307e", L"\u30de", L"ma" },
	{ L"\u307f", L"\u30df", L"mi" },
	{ L"\u3080", L"\u30e0", L"mu" },
	{ L"\u3081", L"\u30e1", L"me" },
	{ L"\u3082", L"\u30e2", L"mo" },

	{ L"\u3084", L"\u30e4", L"ya" },
	{ L"\u3086", L"\u30e6", L"yu" },
	{ L"\u3088", L"\u30e8", L"yo" },

	{ L"\u3089", L"\u30e9", L"ra" },
	{ L"\u308a", L"\u30ea", L"ri" },
	{ L"\u308b", L"\u30eb", L"ru" },
	{ L"\u308c", L"\u30ec", L"re" },
	{ L"\u308d", L"\u30ed", L"ro" },

	{ L"\u308f", L"\u30ef", L"wa" },
	{ L"\u3090", L"\u30f0", L"wi" },
	{ L"\u3091", L"\u30f1", L"we" },
	{ L"\u3092", L"\u30f2", L"wo" },

	{ L"\u304c", L"\u30ac", L"ga" },
	{ L"\u304e", L"\u30ae", L"gi" },
	{ L"\u3050", L"\u30b0", L"gu" },
	{ L"\u3052", L"\u30b2", L"ge" },
	{ L"\u3054", L"\u30b4", L"go" },

	{ L"\u3056", L"\u30b6", L"za" },
	{ L"\u3058", L"\u30b8", L"ji" },
	{ L"\u305a", L"\u30ba", L"zu" },
	{ L"\u305c", L"\u30bc", L"ze" },
	{ L"\u305e", L"\u30be", L"zo" },

	{ L"\u3060", L"\u30c0", L"da" },
	{ L"\u3062", L"\u30c2", L"ji" },
	{ L"\u3065", L"\u30c5", L"zu" },
	{ L"\u3067", L"\u30c7", L"de" },
	{ L"\u3069", L"\u30c9", L"do" },

	{ L"\u3070", L"\u30d0", L"ba" },
	{ L"\u3073", L"\u30d3", L"bi" },
	{ L"\u3076", L"\u30d6", L"bu" },
	{ L"\u3079", L"\u30d9", L"be" },
	{ L"\u307c", L"\u30dc", L"bo" },

	{ L"\u3071", L"\u30d1", L"pa" },
	{ L"\u3074", L"\u30d4", L"pi" },
	{ L"\u3077", L"\u30d7", L"pu" },
	{ L"\u307a", L"\u30da", L"pe" },
	{ L"\u307d", L"\u30dd", L"po" },

	{ L"\u304d\u3083", L"\u30ad\u30e3", L"kya" },
	{ L"\u304d\u3085", L"\u30ad\u30e5", L"kyu" },
	{ L"\u304d\u3087", L"\u30ad\u30e7", L"kyo" },

	{ L"\u3057\u3083", L"\u30b7\u30e3", L"sha" },
	{ L"\u3057\u3085", L"\u30b7\u30e5", L"shu" },
	{ L"\u3057\u3087", L"\u30b7\u30e7", L"sho" },

	{ L"\u3061\u3083", L"\u30c1\u30e3", L"cha" },
	{ L"\u3061\u3085", L"\u30c1\u30e5", L"chu" },
	{ L"\u3061\u3087", L"\u30c1\u30e7", L"cho" },

	{ L"\u306b\u3083", L"\u30cb\u30e3", L"nya" },
	{ L"\u306b\u3085", L"\u30cb\u30e5", L"nyu" },
	{ L"\u306b\u3087", L"\u30cb\u30e7", L"nyo" },

	{ L"\u3072\u3083", L"\u30d2\u30e3", L"hya" },
	{ L"\u3072\u3085", L"\u30d2\u30e5", L"hyu" },
	{ L"\u3072\u3087", L"\u30d2\u30e7", L"hyo" },

	{ L"\u307f\u3083", L"\u30df\u30e3", L"mya" },
	{ L"\u307f\u3085", L"\u30df\u30e5", L"myu" },
	{ L"\u307f\u3087", L"\u30df\u30e7", L"myo" },

	{ L"\u308a\u3083", L"\u30ea\u30e3", L"rya" },
	{ L"\u308a\u3085", L"\u30ea\u30e5", L"ryu" },
	{ L"\u308a\u3087", L"\u30ea\u30e7", L"ryo" },

	{ L"\u304e\u3083", L"\u30ae\u30e3", L"gya" },
	{ L"\u304e\u3085", L"\u30ae\u30e5", L"gyu" },
	{ L"\u304e\u3087", L"\u30ae\u30e7", L"gyo" },

	{ L"\u3058\u3083", L"\u30b8\u30e3", L"ja" },
	{ L"\u3058\u3085", L"\u30b8\u30e5", L"ju" },
	{ L"\u3058\u3087", L"\u30b8\u30e7", L"jo" },

	{ L"\u3062\u3083", L"\u30c2\u30e3", L"ja" },
	{ L"\u3062\u3085", L"\u30c2\u30e5", L"ju" },
	{ L"\u3062\u3087", L"\u30c2\u30e7", L"jo" },

	{ L"\u3073\u3083", L"\u30d3\u30e3", L"bya" },
	{ L"\u3073\u3085", L"\u30d3\u30e5", L"byu" },
	{ L"\u3073\u3087", L"\u30d3\u30e7", L"byo" },

	{ L"\u3074\u3083", L"\u30d4\u30e3", L"pya" },
	{ L"\u3074\u3085", L"\u30d4\u30e5", L"pyu" },
	{ L"\u3074\u3087", L"\u30d4\u30e7", L"pyo" },


	// Specialty katakana usage for loan words

	// Katakana fu + small vowel
	{ L"", L"\u30d5\u30a1", L"fa" },
	{ L"", L"\u30d5\u30a3", L"fi" },
	{ L"", L"\u30d5\u30a7", L"fe" },
	{ L"", L"\u30d5\u30a9", L"fo" },

	// Katakana vu + small vowel
	{ L"", L"\u30f4\u30a1", L"va" },
	{ L"", L"\u30f4\u30a3", L"vi" },
	{ L"", L"\u30f4", L"vu" },
	{ L"", L"\u30f4\u30a7", L"ve" },
	{ L"", L"\u30f4\u30a9", L"vo" },

	// Katakana fu + small yu
	{ L"", L"\u30d5\u30e5", L"fyu" },

	// Katakana i + little e
	{ L"", L"\u30a4\u30a7", L"ye" },

	// Katakana u + little vowels
	{ L"", L"\u30a6\u30a3", L"wi" },
	{ L"", L"\u30a6\u30a7", L"we" },
	{ L"", L"\u30a6\u30a9", L"wo" },

	// Katakana vu + small ya-yu-yo
	{ L"", L"\u30f4\u30e3", L"vya" },
	{ L"", L"\u30f4\u30e5", L"vyu" },
	{ L"", L"\u30f4\u30e7", L"vyo" },

	// Katakana shi-ji-chi + small e
	{ L"", L"\u30b7\u30a7", L"she" },
	{ L"", L"\u30b8\u30a7", L"je" },
	{ L"", L"\u30c1\u30a7", L"che" },

	// Katakana de + small i-u-yu
	{ L"", L"\u30c6\u30a3", L"ti" },
	{ L"", L"\u30c6\u30a5", L"tu" },
	{ L"", L"\u30c6\u30e5", L"tyu" },

	// Katakana de + small i-u-yu
	{ L"", L"\u30c7\u30a3", L"di" },
	{ L"", L"\u30c7\u30a5", L"du" },
	{ L"", L"\u30c7\u30a5", L"dyu" },

	// Katakana tsu + small vowels
	{ L"", L"\u30c4\u30a1", L"tsa" },
	{ L"", L"\u30c4\u30a3", L"tsi" },
	{ L"", L"\u30c4\u30a7", L"tse" },
	{ L"", L"\u30c4\u30a9", L"tso" },


	// Syllablic consonants

	// Small tsu
	{ L"\u3063", L"\u30c3", L"t" },
	{ L"\u3063", L"\u30c3", L"c" },
	{ L"\u3063", L"\u30c3", L"s" },
	{ L"\u3063", L"\u30c3", L"k" },
	{ L"\u3063", L"\u30c3", L"p" },

	// Syllabic n
	{ L"\u3093", L"\u30f3", L"n" },
	{ L"\u3093", L"\u30f3", L"m" },


	// Other special usage

	// Small vowels
	{ L"\u3041", L"\u30a1", L"a" },
	{ L"\u3043", L"\u30a3", L"i" },
	{ L"\u3045", L"\u30a5", L"u" },
	{ L"\u3047", L"\u30a7", L"e" },
	{ L"\u3049", L"\u30a9", L"o" },

	// Long vowel mark (dash)
	{ L"", L"\u30fc", L"a" },
	{ L"", L"\u30fc", L"i" },
	{ L"", L"\u30fc", L"u" },
	{ L"", L"\u30fc", L"e" },
	{ L"", L"\u30fc", L"o" },
	{ 0, 0, 0 }
};
