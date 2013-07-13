// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "../config.h"

#include "libaegisub/kana_table.h"

#include <boost/range/algorithm.hpp>

namespace {
agi::kana_pair kana_to_romaji[] = {
	{"\xE3\x81\x81", "a"},               // ぁ
	{"\xE3\x81\x82", "a"},               // あ
	{"\xE3\x81\x83", "i"},               // ぃ
	{"\xE3\x81\x84", "i"},               // い
	{"\xE3\x81\x85", "u"},               // ぅ
	{"\xE3\x81\x86", "u"},               // う
	{"\xE3\x81\x87", "e"},               // ぇ
	{"\xE3\x81\x88", "e"},               // え
	{"\xE3\x81\x89", "o"},               // ぉ
	{"\xE3\x81\x8A", "o"},               // お
	{"\xE3\x81\x8B", "ka"},              // か
	{"\xE3\x81\x8C", "ga"},              // が
	{"\xE3\x81\x8D", "ki"},              // き
	{"\xE3\x81\x8D\xE3\x82\x83", "kya"}, // きゃ
	{"\xE3\x81\x8D\xE3\x82\x85", "kyu"}, // きゅ
	{"\xE3\x81\x8D\xE3\x82\x87", "kyo"}, // きょ
	{"\xE3\x81\x8E", "gi"},              // ぎ
	{"\xE3\x81\x8E\xE3\x82\x83", "gya"}, // ぎゃ
	{"\xE3\x81\x8E\xE3\x82\x85", "gyu"}, // ぎゅ
	{"\xE3\x81\x8E\xE3\x82\x87", "gyo"}, // ぎょ
	{"\xE3\x81\x8F", "ku"},              // く
	{"\xE3\x81\x90", "gu"},              // ぐ
	{"\xE3\x81\x91", "ke"},              // け
	{"\xE3\x81\x92", "ge"},              // げ
	{"\xE3\x81\x93", "ko"},              // こ
	{"\xE3\x81\x94", "go"},              // ご
	{"\xE3\x81\x95", "sa"},              // さ
	{"\xE3\x81\x96", "za"},              // ざ
	{"\xE3\x81\x97", "shi"},             // し
	{"\xE3\x81\x97\xE3\x82\x83", "sha"}, // しゃ
	{"\xE3\x81\x97\xE3\x82\x85", "shu"}, // しゅ
	{"\xE3\x81\x97\xE3\x82\x87", "sho"}, // しょ
	{"\xE3\x81\x98", "ji"},              // じ
	{"\xE3\x81\x98\xE3\x82\x83", "ja"},  // じゃ
	{"\xE3\x81\x98\xE3\x82\x85", "ju"},  // じゅ
	{"\xE3\x81\x98\xE3\x82\x87", "jo"},  // じょ
	{"\xE3\x81\x99", "su"},              // す
	{"\xE3\x81\x9A", "zu"},              // ず
	{"\xE3\x81\x9B", "se"},              // せ
	{"\xE3\x81\x9C", "ze"},              // ぜ
	{"\xE3\x81\x9D", "so"},              // そ
	{"\xE3\x81\x9E", "zo"},              // ぞ
	{"\xE3\x81\x9F", "ta"},              // た
	{"\xE3\x81\xA0", "da"},              // だ
	{"\xE3\x81\xA1", "chi"},             // ち
	{"\xE3\x81\xA1\xE3\x82\x83", "cha"}, // ちゃ
	{"\xE3\x81\xA1\xE3\x82\x85", "chu"}, // ちゅ
	{"\xE3\x81\xA1\xE3\x82\x87", "cho"}, // ちょ
	{"\xE3\x81\xA2", "ji"},              // ぢ
	{"\xE3\x81\xA2\xE3\x82\x83", "ja"},  // ぢゃ
	{"\xE3\x81\xA2\xE3\x82\x85", "ju"},  // ぢゅ
	{"\xE3\x81\xA2\xE3\x82\x87", "jo"},  // ぢょ
	{"\xE3\x81\xA3", "c"},               // っ
	{"\xE3\x81\xA3", "k"},               // っ
	{"\xE3\x81\xA3", "p"},               // っ
	{"\xE3\x81\xA3", "s"},               // っ
	{"\xE3\x81\xA3", "t"},               // っ
	{"\xE3\x81\xA4", "tsu"},             // つ
	{"\xE3\x81\xA5", "zu"},              // づ
	{"\xE3\x81\xA6", "te"},              // て
	{"\xE3\x81\xA7", "de"},              // で
	{"\xE3\x81\xA8", "to"},              // と
	{"\xE3\x81\xA9", "do"},              // ど
	{"\xE3\x81\xAA", "na"},              // な
	{"\xE3\x81\xAB", "ni"},              // に
	{"\xE3\x81\xAB\xE3\x82\x83", "nya"}, // にゃ
	{"\xE3\x81\xAB\xE3\x82\x85", "nyu"}, // にゅ
	{"\xE3\x81\xAB\xE3\x82\x87", "nyo"}, // にょ
	{"\xE3\x81\xAC", "nu"},              // ぬ
	{"\xE3\x81\xAD", "ne"},              // ね
	{"\xE3\x81\xAE", "no"},              // の
	{"\xE3\x81\xAF", "ha"},              // は
	{"\xE3\x81\xAF", "wa"},              // は
	{"\xE3\x81\xB0", "ba"},              // ば
	{"\xE3\x81\xB1", "pa"},              // ぱ
	{"\xE3\x81\xB2", "hi"},              // ひ
	{"\xE3\x81\xB2\xE3\x82\x83", "hya"}, // ひゃ
	{"\xE3\x81\xB2\xE3\x82\x85", "hyu"}, // ひゅ
	{"\xE3\x81\xB2\xE3\x82\x87", "hyo"}, // ひょ
	{"\xE3\x81\xB3", "bi"},              // び
	{"\xE3\x81\xB3\xE3\x82\x83", "bya"}, // びゃ
	{"\xE3\x81\xB3\xE3\x82\x85", "byu"}, // びゅ
	{"\xE3\x81\xB3\xE3\x82\x87", "byo"}, // びょ
	{"\xE3\x81\xB4", "pi"},              // ぴ
	{"\xE3\x81\xB4\xE3\x82\x83", "pya"}, // ぴゃ
	{"\xE3\x81\xB4\xE3\x82\x85", "pyu"}, // ぴゅ
	{"\xE3\x81\xB4\xE3\x82\x87", "pyo"}, // ぴょ
	{"\xE3\x81\xB5", "fu"},              // ふ
	{"\xE3\x81\xB6", "bu"},              // ぶ
	{"\xE3\x81\xB7", "pu"},              // ぷ
	{"\xE3\x81\xB8", "he"},              // へ
	{"\xE3\x81\xB8", "e"},               // へ
	{"\xE3\x81\xB9", "be"},              // べ
	{"\xE3\x81\xBA", "pe"},              // ぺ
	{"\xE3\x81\xBB", "ho"},              // ほ
	{"\xE3\x81\xBC", "bo"},              // ぼ
	{"\xE3\x81\xBD", "po"},              // ぽ
	{"\xE3\x81\xBE", "ma"},              // ま
	{"\xE3\x81\xBF", "mi"},              // み
	{"\xE3\x81\xBF\xE3\x82\x83", "mya"}, // みゃ
	{"\xE3\x81\xBF\xE3\x82\x85", "myu"}, // みゅ
	{"\xE3\x81\xBF\xE3\x82\x87", "myo"}, // みょ
	{"\xE3\x82\x80", "mu"},              // む
	{"\xE3\x82\x81", "me"},              // め
	{"\xE3\x82\x82", "mo"},              // も
	{"\xE3\x82\x84", "ya"},              // や
	{"\xE3\x82\x86", "yu"},              // ゆ
	{"\xE3\x82\x88", "yo"},              // よ
	{"\xE3\x82\x89", "ra"},              // ら
	{"\xE3\x82\x8A", "ri"},              // り
	{"\xE3\x82\x8A\xE3\x82\x83", "rya"}, // りゃ
	{"\xE3\x82\x8A\xE3\x82\x85", "ryu"}, // りゅ
	{"\xE3\x82\x8A\xE3\x82\x87", "ryo"}, // りょ
	{"\xE3\x82\x8B", "ru"},              // る
	{"\xE3\x82\x8C", "re"},              // れ
	{"\xE3\x82\x8D", "ro"},              // ろ
	{"\xE3\x82\x8F", "wa"},              // わ
	{"\xE3\x82\x90", "wi"},              // ゐ
	{"\xE3\x82\x91", "we"},              // ゑ
	{"\xE3\x82\x92", "wo"},              // を
	{"\xE3\x82\x93", "m"},               // ん
	{"\xE3\x82\x93", "n"},               // ん
	{"\xE3\x82\xA1", "a"},               // ァ
	{"\xE3\x82\xA2", "a"},               // ア
	{"\xE3\x82\xA3", "i"},               // ィ
	{"\xE3\x82\xA4", "i"},               // イ
	{"\xE3\x82\xA4\xE3\x82\xA7", "ye"},  // イェ
	{"\xE3\x82\xA5", "u"},               // ゥ
	{"\xE3\x82\xA6", "u"},               // ウ
	{"\xE3\x82\xA6\xE3\x82\xA3", "wi"},  // ウィ
	{"\xE3\x82\xA6\xE3\x82\xA7", "we"},  // ウェ
	{"\xE3\x82\xA6\xE3\x82\xA9", "wo"},  // ウォ
	{"\xE3\x82\xA7", "e"},               // ェ
	{"\xE3\x82\xA8", "e"},               // エ
	{"\xE3\x82\xA9", "o"},               // ォ
	{"\xE3\x82\xAA", "o"},               // オ
	{"\xE3\x82\xAB", "ka"},              // カ
	{"\xE3\x82\xAC", "ga"},              // ガ
	{"\xE3\x82\xAD", "ki"},              // キ
	{"\xE3\x82\xAD\xE3\x83\xA3", "kya"}, // キャ
	{"\xE3\x82\xAD\xE3\x83\xA5", "kyu"}, // キュ
	{"\xE3\x82\xAD\xE3\x83\xA7", "kyo"}, // キョ
	{"\xE3\x82\xAE", "gi"},              // ギ
	{"\xE3\x82\xAE\xE3\x83\xA3", "gya"}, // ギャ
	{"\xE3\x82\xAE\xE3\x83\xA5", "gyu"}, // ギュ
	{"\xE3\x82\xAE\xE3\x83\xA7", "gyo"}, // ギョ
	{"\xE3\x82\xAF", "ku"},              // ク
	{"\xE3\x82\xB0", "gu"},              // グ
	{"\xE3\x82\xB1", "ke"},              // ケ
	{"\xE3\x82\xB2", "ge"},              // ゲ
	{"\xE3\x82\xB3", "ko"},              // コ
	{"\xE3\x82\xB4", "go"},              // ゴ
	{"\xE3\x82\xB5", "sa"},              // サ
	{"\xE3\x82\xB6", "za"},              // ザ
	{"\xE3\x82\xB7", "shi"},             // シ
	{"\xE3\x82\xB7\xE3\x82\xA7", "she"}, // シェ
	{"\xE3\x82\xB7\xE3\x83\xA3", "sha"}, // シャ
	{"\xE3\x82\xB7\xE3\x83\xA5", "shu"}, // シュ
	{"\xE3\x82\xB7\xE3\x83\xA7", "sho"}, // ショ
	{"\xE3\x82\xB8", "ji"},              // ジ
	{"\xE3\x82\xB8\xE3\x82\xA7", "je"},  // ジェ
	{"\xE3\x82\xB8\xE3\x83\xA3", "ja"},  // ジャ
	{"\xE3\x82\xB8\xE3\x83\xA5", "ju"},  // ジュ
	{"\xE3\x82\xB8\xE3\x83\xA7", "jo"},  // ジョ
	{"\xE3\x82\xB9", "su"},              // ス
	{"\xE3\x82\xBA", "zu"},              // ズ
	{"\xE3\x82\xBB", "se"},              // セ
	{"\xE3\x82\xBC", "ze"},              // ゼ
	{"\xE3\x82\xBD", "so"},              // ソ
	{"\xE3\x82\xBE", "zo"},              // ゾ
	{"\xE3\x82\xBF", "ta"},              // タ
	{"\xE3\x83\x80", "da"},              // ダ
	{"\xE3\x83\x81", "chi"},             // チ
	{"\xE3\x83\x81\xE3\x82\xA7", "che"}, // チェ
	{"\xE3\x83\x81\xE3\x83\xA3", "cha"}, // チャ
	{"\xE3\x83\x81\xE3\x83\xA5", "chu"}, // チュ
	{"\xE3\x83\x81\xE3\x83\xA7", "cho"}, // チョ
	{"\xE3\x83\x82", "ji"},              // ヂ
	{"\xE3\x83\x82\xE3\x83\xA3", "ja"},  // ヂャ
	{"\xE3\x83\x82\xE3\x83\xA5", "ju"},  // ヂュ
	{"\xE3\x83\x82\xE3\x83\xA7", "jo"},  // ヂョ
	{"\xE3\x83\x83", "c"},               // ッ
	{"\xE3\x83\x83", "k"},               // ッ
	{"\xE3\x83\x83", "p"},               // ッ
	{"\xE3\x83\x83", "s"},               // ッ
	{"\xE3\x83\x83", "t"},               // ッ
	{"\xE3\x83\x84", "tsu"},             // ツ
	{"\xE3\x83\x84\xE3\x82\xA1", "tsa"}, // ツァ
	{"\xE3\x83\x84\xE3\x82\xA3", "tsi"}, // ツィ
	{"\xE3\x83\x84\xE3\x82\xA7", "tse"}, // ツェ
	{"\xE3\x83\x84\xE3\x82\xA9", "tso"}, // ツォ
	{"\xE3\x83\x85", "zu"},              // ヅ
	{"\xE3\x83\x86", "te"},              // テ
	{"\xE3\x83\x86\xE3\x82\xA3", "ti"},  // ティ
	{"\xE3\x83\x86\xE3\x82\xA5", "tu"},  // テゥ
	{"\xE3\x83\x86\xE3\x83\xA5", "tyu"}, // テュ
	{"\xE3\x83\x87", "de"},              // デ
	{"\xE3\x83\x87\xE3\x82\xA3", "di"},  // ディ
	{"\xE3\x83\x87\xE3\x82\xA5", "du"},  // デゥ
	{"\xE3\x83\x87\xE3\x82\xA5", "dyu"}, // デゥ
	{"\xE3\x83\x88", "to"},              // ト
	{"\xE3\x83\x89", "do"},              // ド
	{"\xE3\x83\x8A", "na"},              // ナ
	{"\xE3\x83\x8B", "ni"},              // ニ
	{"\xE3\x83\x8B\xE3\x83\xA3", "nya"}, // ニャ
	{"\xE3\x83\x8B\xE3\x83\xA5", "nyu"}, // ニュ
	{"\xE3\x83\x8B\xE3\x83\xA7", "nyo"}, // ニョ
	{"\xE3\x83\x8C", "nu"},              // ヌ
	{"\xE3\x83\x8D", "ne"},              // ネ
	{"\xE3\x83\x8E", "no"},              // ノ
	{"\xE3\x83\x8F", "ha"},              // ハ
	{"\xE3\x83\x90", "ba"},              // バ
	{"\xE3\x83\x91", "pa"},              // パ
	{"\xE3\x83\x92", "hi"},              // ヒ
	{"\xE3\x83\x92\xE3\x83\xA3", "hya"}, // ヒャ
	{"\xE3\x83\x92\xE3\x83\xA5", "hyu"}, // ヒュ
	{"\xE3\x83\x92\xE3\x83\xA7", "hyo"}, // ヒョ
	{"\xE3\x83\x93", "bi"},              // ビ
	{"\xE3\x83\x93\xE3\x83\xA3", "bya"}, // ビャ
	{"\xE3\x83\x93\xE3\x83\xA5", "byu"}, // ビュ
	{"\xE3\x83\x93\xE3\x83\xA7", "byo"}, // ビョ
	{"\xE3\x83\x94", "pi"},              // ピ
	{"\xE3\x83\x94\xE3\x83\xA3", "pya"}, // ピャ
	{"\xE3\x83\x94\xE3\x83\xA5", "pyu"}, // ピュ
	{"\xE3\x83\x94\xE3\x83\xA7", "pyo"}, // ピョ
	{"\xE3\x83\x95", "fu"},              // フ
	{"\xE3\x83\x95\xE3\x82\xA1", "fa"},  // ファ
	{"\xE3\x83\x95\xE3\x82\xA3", "fi"},  // フィ
	{"\xE3\x83\x95\xE3\x82\xA7", "fe"},  // フェ
	{"\xE3\x83\x95\xE3\x82\xA9", "fo"},  // フォ
	{"\xE3\x83\x95\xE3\x83\xA5", "fyu"}, // フュ
	{"\xE3\x83\x96", "bu"},              // ブ
	{"\xE3\x83\x97", "pu"},              // プ
	{"\xE3\x83\x98", "he"},              // ヘ
	{"\xE3\x83\x99", "be"},              // ベ
	{"\xE3\x83\x9A", "pe"},              // ペ
	{"\xE3\x83\x9B", "ho"},              // ホ
	{"\xE3\x83\x9C", "bo"},              // ボ
	{"\xE3\x83\x9D", "po"},              // ポ
	{"\xE3\x83\x9E", "ma"},              // マ
	{"\xE3\x83\x9F", "mi"},              // ミ
	{"\xE3\x83\x9F\xE3\x83\xA3", "mya"}, // ミャ
	{"\xE3\x83\x9F\xE3\x83\xA5", "myu"}, // ミュ
	{"\xE3\x83\x9F\xE3\x83\xA7", "myo"}, // ミョ
	{"\xE3\x83\xA0", "mu"},              // ム
	{"\xE3\x83\xA1", "me"},              // メ
	{"\xE3\x83\xA2", "mo"},              // モ
	{"\xE3\x83\xA4", "ya"},              // ヤ
	{"\xE3\x83\xA6", "yu"},              // ユ
	{"\xE3\x83\xA8", "yo"},              // ヨ
	{"\xE3\x83\xA9", "ra"},              // ラ
	{"\xE3\x83\xAA", "ri"},              // リ
	{"\xE3\x83\xAA\xE3\x83\xA3", "rya"}, // リャ
	{"\xE3\x83\xAA\xE3\x83\xA5", "ryu"}, // リュ
	{"\xE3\x83\xAA\xE3\x83\xA7", "ryo"}, // リョ
	{"\xE3\x83\xAB", "ru"},              // ル
	{"\xE3\x83\xAC", "re"},              // レ
	{"\xE3\x83\xAD", "ro"},              // ロ
	{"\xE3\x83\xAF", "wa"},              // ワ
	{"\xE3\x83\xB0", "wi"},              // ヰ
	{"\xE3\x83\xB1", "we"},              // ヱ
	{"\xE3\x83\xB2", "wo"},              // ヲ
	{"\xE3\x83\xB3", "m"},               // ン
	{"\xE3\x83\xB3", "n"},               // ン
	{"\xE3\x83\xB4", "vu"},              // ヴ
	{"\xE3\x83\xB4\xE3\x82\xA1", "va"},  // ヴァ
	{"\xE3\x83\xB4\xE3\x82\xA3", "vi"},  // ヴィ
	{"\xE3\x83\xB4\xE3\x82\xA7", "ve"},  // ヴェ
	{"\xE3\x83\xB4\xE3\x82\xA9", "vo"},  // ヴォ
	{"\xE3\x83\xB4\xE3\x83\xA3", "vya"}, // ヴャ
	{"\xE3\x83\xB4\xE3\x83\xA5", "vyu"}, // ヴュ
	{"\xE3\x83\xB4\xE3\x83\xA7", "vyo"}, // ヴョ
	{"\xE3\x83\xBC", "a"},               // ー
	{"\xE3\x83\xBC", "e"},               // ー
	{"\xE3\x83\xBC", "i"},               // ー
	{"\xE3\x83\xBC", "o"},               // ー
	{"\xE3\x83\xBC", "u"},               // ー
};

agi::kana_pair romaji_to_kana[] = {
	{"\xE3\x81\x81", "a"},               // ぁ
	{"\xE3\x81\x82", "a"},               // あ
	{"\xE3\x82\xA1", "a"},               // ァ
	{"\xE3\x82\xA2", "a"},               // ア
	{"\xE3\x83\xBC", "a"},               // ー
	{"\xE3\x81\xB0", "ba"},              // ば
	{"\xE3\x83\x90", "ba"},              // バ
	{"\xE3\x81\xB9", "be"},              // べ
	{"\xE3\x83\x99", "be"},              // ベ
	{"\xE3\x81\xB3", "bi"},              // び
	{"\xE3\x83\x93", "bi"},              // ビ
	{"\xE3\x81\xBC", "bo"},              // ぼ
	{"\xE3\x83\x9C", "bo"},              // ボ
	{"\xE3\x81\xB6", "bu"},              // ぶ
	{"\xE3\x83\x96", "bu"},              // ブ
	{"\xE3\x81\xB3\xE3\x82\x83", "bya"}, // びゃ
	{"\xE3\x83\x93\xE3\x83\xA3", "bya"}, // ビャ
	{"\xE3\x81\xB3\xE3\x82\x87", "byo"}, // びょ
	{"\xE3\x83\x93\xE3\x83\xA7", "byo"}, // ビョ
	{"\xE3\x81\xB3\xE3\x82\x85", "byu"}, // びゅ
	{"\xE3\x83\x93\xE3\x83\xA5", "byu"}, // ビュ
	{"\xE3\x81\xA3", "c"},               // っ
	{"\xE3\x83\x83", "c"},               // ッ
	{"\xE3\x81\xA1\xE3\x82\x83", "cha"}, // ちゃ
	{"\xE3\x83\x81\xE3\x83\xA3", "cha"}, // チャ
	{"\xE3\x83\x81\xE3\x82\xA7", "che"}, // チェ
	{"\xE3\x81\xA1", "chi"},             // ち
	{"\xE3\x83\x81", "chi"},             // チ
	{"\xE3\x81\xA1\xE3\x82\x87", "cho"}, // ちょ
	{"\xE3\x83\x81\xE3\x83\xA7", "cho"}, // チョ
	{"\xE3\x81\xA1\xE3\x82\x85", "chu"}, // ちゅ
	{"\xE3\x83\x81\xE3\x83\xA5", "chu"}, // チュ
	{"\xE3\x81\xA0", "da"},              // だ
	{"\xE3\x83\x80", "da"},              // ダ
	{"\xE3\x81\xA7", "de"},              // で
	{"\xE3\x83\x87", "de"},              // デ
	{"\xE3\x83\x87\xE3\x82\xA3", "di"},  // ディ
	{"\xE3\x81\xA9", "do"},              // ど
	{"\xE3\x83\x89", "do"},              // ド
	{"\xE3\x83\x87\xE3\x82\xA5", "du"},  // デゥ
	{"\xE3\x83\x87\xE3\x82\xA5", "dyu"}, // デゥ
	{"\xE3\x81\x87", "e"},               // ぇ
	{"\xE3\x81\x88", "e"},               // え
	{"\xE3\x82\xA7", "e"},               // ェ
	{"\xE3\x82\xA8", "e"},               // エ
	{"\xE3\x83\xBC", "e"},               // ー
	{"\xE3\x83\x95\xE3\x82\xA1", "fa"},  // ファ
	{"\xE3\x83\x95\xE3\x82\xA7", "fe"},  // フェ
	{"\xE3\x83\x95\xE3\x82\xA3", "fi"},  // フィ
	{"\xE3\x83\x95\xE3\x82\xA9", "fo"},  // フォ
	{"\xE3\x81\xB5", "fu"},              // ふ
	{"\xE3\x83\x95", "fu"},              // フ
	{"\xE3\x83\x95\xE3\x83\xA5", "fyu"}, // フュ
	{"\xE3\x81\x8C", "ga"},              // が
	{"\xE3\x82\xAC", "ga"},              // ガ
	{"\xE3\x81\x92", "ge"},              // げ
	{"\xE3\x82\xB2", "ge"},              // ゲ
	{"\xE3\x81\x8E", "gi"},              // ぎ
	{"\xE3\x82\xAE", "gi"},              // ギ
	{"\xE3\x81\x94", "go"},              // ご
	{"\xE3\x82\xB4", "go"},              // ゴ
	{"\xE3\x81\x90", "gu"},              // ぐ
	{"\xE3\x82\xB0", "gu"},              // グ
	{"\xE3\x81\x8E\xE3\x82\x83", "gya"}, // ぎゃ
	{"\xE3\x82\xAE\xE3\x83\xA3", "gya"}, // ギャ
	{"\xE3\x81\x8E\xE3\x82\x87", "gyo"}, // ぎょ
	{"\xE3\x82\xAE\xE3\x83\xA7", "gyo"}, // ギョ
	{"\xE3\x81\x8E\xE3\x82\x85", "gyu"}, // ぎゅ
	{"\xE3\x82\xAE\xE3\x83\xA5", "gyu"}, // ギュ
	{"\xE3\x81\xAF", "ha"},              // は
	{"\xE3\x83\x8F", "ha"},              // ハ
	{"\xE3\x81\xB8", "he"},              // へ
	{"\xE3\x83\x98", "he"},              // ヘ
	{"\xE3\x81\xB2", "hi"},              // ひ
	{"\xE3\x83\x92", "hi"},              // ヒ
	{"\xE3\x81\xBB", "ho"},              // ほ
	{"\xE3\x83\x9B", "ho"},              // ホ
	{"\xE3\x81\xB2\xE3\x82\x83", "hya"}, // ひゃ
	{"\xE3\x83\x92\xE3\x83\xA3", "hya"}, // ヒャ
	{"\xE3\x81\xB2\xE3\x82\x87", "hyo"}, // ひょ
	{"\xE3\x83\x92\xE3\x83\xA7", "hyo"}, // ヒョ
	{"\xE3\x81\xB2\xE3\x82\x85", "hyu"}, // ひゅ
	{"\xE3\x83\x92\xE3\x83\xA5", "hyu"}, // ヒュ
	{"\xE3\x81\x83", "i"},               // ぃ
	{"\xE3\x81\x84", "i"},               // い
	{"\xE3\x82\xA3", "i"},               // ィ
	{"\xE3\x82\xA4", "i"},               // イ
	{"\xE3\x83\xBC", "i"},               // ー
	{"\xE3\x81\x98\xE3\x82\x83", "ja"},  // じゃ
	{"\xE3\x81\xA2\xE3\x82\x83", "ja"},  // ぢゃ
	{"\xE3\x82\xB8\xE3\x83\xA3", "ja"},  // ジャ
	{"\xE3\x83\x82\xE3\x83\xA3", "ja"},  // ヂャ
	{"\xE3\x82\xB8\xE3\x82\xA7", "je"},  // ジェ
	{"\xE3\x81\x98", "ji"},              // じ
	{"\xE3\x81\xA2", "ji"},              // ぢ
	{"\xE3\x82\xB8", "ji"},              // ジ
	{"\xE3\x83\x82", "ji"},              // ヂ
	{"\xE3\x81\x98\xE3\x82\x87", "jo"},  // じょ
	{"\xE3\x81\xA2\xE3\x82\x87", "jo"},  // ぢょ
	{"\xE3\x82\xB8\xE3\x83\xA7", "jo"},  // ジョ
	{"\xE3\x83\x82\xE3\x83\xA7", "jo"},  // ヂョ
	{"\xE3\x81\x98\xE3\x82\x85", "ju"},  // じゅ
	{"\xE3\x81\xA2\xE3\x82\x85", "ju"},  // ぢゅ
	{"\xE3\x82\xB8\xE3\x83\xA5", "ju"},  // ジュ
	{"\xE3\x83\x82\xE3\x83\xA5", "ju"},  // ヂュ
	{"\xE3\x81\xA3", "k"},               // っ
	{"\xE3\x83\x83", "k"},               // ッ
	{"\xE3\x81\x8B", "ka"},              // か
	{"\xE3\x82\xAB", "ka"},              // カ
	{"\xE3\x81\x91", "ke"},              // け
	{"\xE3\x82\xB1", "ke"},              // ケ
	{"\xE3\x81\x8D", "ki"},              // き
	{"\xE3\x82\xAD", "ki"},              // キ
	{"\xE3\x81\x93", "ko"},              // こ
	{"\xE3\x82\xB3", "ko"},              // コ
	{"\xE3\x81\x8F", "ku"},              // く
	{"\xE3\x82\xAF", "ku"},              // ク
	{"\xE3\x81\x8D\xE3\x82\x83", "kya"}, // きゃ
	{"\xE3\x82\xAD\xE3\x83\xA3", "kya"}, // キャ
	{"\xE3\x81\x8D\xE3\x82\x87", "kyo"}, // きょ
	{"\xE3\x82\xAD\xE3\x83\xA7", "kyo"}, // キョ
	{"\xE3\x81\x8D\xE3\x82\x85", "kyu"}, // きゅ
	{"\xE3\x82\xAD\xE3\x83\xA5", "kyu"}, // キュ
	{"\xE3\x82\x93", "m"},               // ん
	{"\xE3\x83\xB3", "m"},               // ン
	{"\xE3\x81\xBE", "ma"},              // ま
	{"\xE3\x83\x9E", "ma"},              // マ
	{"\xE3\x82\x81", "me"},              // め
	{"\xE3\x83\xA1", "me"},              // メ
	{"\xE3\x81\xBF", "mi"},              // み
	{"\xE3\x83\x9F", "mi"},              // ミ
	{"\xE3\x82\x82", "mo"},              // も
	{"\xE3\x83\xA2", "mo"},              // モ
	{"\xE3\x82\x80", "mu"},              // む
	{"\xE3\x83\xA0", "mu"},              // ム
	{"\xE3\x81\xBF\xE3\x82\x83", "mya"}, // みゃ
	{"\xE3\x83\x9F\xE3\x83\xA3", "mya"}, // ミャ
	{"\xE3\x81\xBF\xE3\x82\x87", "myo"}, // みょ
	{"\xE3\x83\x9F\xE3\x83\xA7", "myo"}, // ミョ
	{"\xE3\x81\xBF\xE3\x82\x85", "myu"}, // みゅ
	{"\xE3\x83\x9F\xE3\x83\xA5", "myu"}, // ミュ
	{"\xE3\x82\x93", "n"},               // ん
	{"\xE3\x83\xB3", "n"},               // ン
	{"\xE3\x81\xAA", "na"},              // な
	{"\xE3\x83\x8A", "na"},              // ナ
	{"\xE3\x81\xAD", "ne"},              // ね
	{"\xE3\x83\x8D", "ne"},              // ネ
	{"\xE3\x81\xAB", "ni"},              // に
	{"\xE3\x83\x8B", "ni"},              // ニ
	{"\xE3\x81\xAE", "no"},              // の
	{"\xE3\x83\x8E", "no"},              // ノ
	{"\xE3\x81\xAC", "nu"},              // ぬ
	{"\xE3\x83\x8C", "nu"},              // ヌ
	{"\xE3\x81\xAB\xE3\x82\x83", "nya"}, // にゃ
	{"\xE3\x83\x8B\xE3\x83\xA3", "nya"}, // ニャ
	{"\xE3\x81\xAB\xE3\x82\x87", "nyo"}, // にょ
	{"\xE3\x83\x8B\xE3\x83\xA7", "nyo"}, // ニョ
	{"\xE3\x81\xAB\xE3\x82\x85", "nyu"}, // にゅ
	{"\xE3\x83\x8B\xE3\x83\xA5", "nyu"}, // ニュ
	{"\xE3\x81\x89", "o"},               // ぉ
	{"\xE3\x81\x8A", "o"},               // お
	{"\xE3\x82\xA9", "o"},               // ォ
	{"\xE3\x82\xAA", "o"},               // オ
	{"\xE3\x83\xBC", "o"},               // ー
	{"\xE3\x81\xA3", "p"},               // っ
	{"\xE3\x83\x83", "p"},               // ッ
	{"\xE3\x81\xB1", "pa"},              // ぱ
	{"\xE3\x83\x91", "pa"},              // パ
	{"\xE3\x81\xBA", "pe"},              // ぺ
	{"\xE3\x83\x9A", "pe"},              // ペ
	{"\xE3\x81\xB4", "pi"},              // ぴ
	{"\xE3\x83\x94", "pi"},              // ピ
	{"\xE3\x81\xBD", "po"},              // ぽ
	{"\xE3\x83\x9D", "po"},              // ポ
	{"\xE3\x81\xB7", "pu"},              // ぷ
	{"\xE3\x83\x97", "pu"},              // プ
	{"\xE3\x81\xB4\xE3\x82\x83", "pya"}, // ぴゃ
	{"\xE3\x83\x94\xE3\x83\xA3", "pya"}, // ピャ
	{"\xE3\x81\xB4\xE3\x82\x87", "pyo"}, // ぴょ
	{"\xE3\x83\x94\xE3\x83\xA7", "pyo"}, // ピョ
	{"\xE3\x81\xB4\xE3\x82\x85", "pyu"}, // ぴゅ
	{"\xE3\x83\x94\xE3\x83\xA5", "pyu"}, // ピュ
	{"\xE3\x82\x89", "ra"},              // ら
	{"\xE3\x83\xA9", "ra"},              // ラ
	{"\xE3\x82\x8C", "re"},              // れ
	{"\xE3\x83\xAC", "re"},              // レ
	{"\xE3\x82\x8A", "ri"},              // り
	{"\xE3\x83\xAA", "ri"},              // リ
	{"\xE3\x82\x8D", "ro"},              // ろ
	{"\xE3\x83\xAD", "ro"},              // ロ
	{"\xE3\x82\x8B", "ru"},              // る
	{"\xE3\x83\xAB", "ru"},              // ル
	{"\xE3\x82\x8A\xE3\x82\x83", "rya"}, // りゃ
	{"\xE3\x83\xAA\xE3\x83\xA3", "rya"}, // リャ
	{"\xE3\x82\x8A\xE3\x82\x87", "ryo"}, // りょ
	{"\xE3\x83\xAA\xE3\x83\xA7", "ryo"}, // リョ
	{"\xE3\x82\x8A\xE3\x82\x85", "ryu"}, // りゅ
	{"\xE3\x83\xAA\xE3\x83\xA5", "ryu"}, // リュ
	{"\xE3\x81\xA3", "s"},               // っ
	{"\xE3\x83\x83", "s"},               // ッ
	{"\xE3\x81\x95", "sa"},              // さ
	{"\xE3\x82\xB5", "sa"},              // サ
	{"\xE3\x81\x9B", "se"},              // せ
	{"\xE3\x82\xBB", "se"},              // セ
	{"\xE3\x81\x97\xE3\x82\x83", "sha"}, // しゃ
	{"\xE3\x82\xB7\xE3\x83\xA3", "sha"}, // シャ
	{"\xE3\x82\xB7\xE3\x82\xA7", "she"}, // シェ
	{"\xE3\x81\x97", "shi"},             // し
	{"\xE3\x82\xB7", "shi"},             // シ
	{"\xE3\x81\x97\xE3\x82\x87", "sho"}, // しょ
	{"\xE3\x82\xB7\xE3\x83\xA7", "sho"}, // ショ
	{"\xE3\x81\x97\xE3\x82\x85", "shu"}, // しゅ
	{"\xE3\x82\xB7\xE3\x83\xA5", "shu"}, // シュ
	{"\xE3\x81\x9D", "so"},              // そ
	{"\xE3\x82\xBD", "so"},              // ソ
	{"\xE3\x81\x99", "su"},              // す
	{"\xE3\x82\xB9", "su"},              // ス
	{"\xE3\x81\xA3", "t"},               // っ
	{"\xE3\x83\x83", "t"},               // ッ
	{"\xE3\x81\x9F", "ta"},              // た
	{"\xE3\x82\xBF", "ta"},              // タ
	{"\xE3\x81\xA6", "te"},              // て
	{"\xE3\x83\x86", "te"},              // テ
	{"\xE3\x83\x86\xE3\x82\xA3", "ti"},  // ティ
	{"\xE3\x81\xA8", "to"},              // と
	{"\xE3\x83\x88", "to"},              // ト
	{"\xE3\x83\x84\xE3\x82\xA1", "tsa"}, // ツァ
	{"\xE3\x83\x84\xE3\x82\xA7", "tse"}, // ツェ
	{"\xE3\x83\x84\xE3\x82\xA3", "tsi"}, // ツィ
	{"\xE3\x83\x84\xE3\x82\xA9", "tso"}, // ツォ
	{"\xE3\x81\xA4", "tsu"},             // つ
	{"\xE3\x83\x84", "tsu"},             // ツ
	{"\xE3\x83\x86\xE3\x82\xA5", "tu"},  // テゥ
	{"\xE3\x83\x86\xE3\x83\xA5", "tyu"}, // テュ
	{"\xE3\x81\x85", "u"},               // ぅ
	{"\xE3\x81\x86", "u"},               // う
	{"\xE3\x82\xA5", "u"},               // ゥ
	{"\xE3\x82\xA6", "u"},               // ウ
	{"\xE3\x83\xBC", "u"},               // ー
	{"\xE3\x83\xB4\xE3\x82\xA1", "va"},  // ヴァ
	{"\xE3\x83\xB4\xE3\x82\xA7", "ve"},  // ヴェ
	{"\xE3\x83\xB4\xE3\x82\xA3", "vi"},  // ヴィ
	{"\xE3\x83\xB4\xE3\x82\xA9", "vo"},  // ヴォ
	{"\xE3\x83\xB4", "vu"},              // ヴ
	{"\xE3\x83\xB4\xE3\x83\xA3", "vya"}, // ヴャ
	{"\xE3\x83\xB4\xE3\x83\xA7", "vyo"}, // ヴョ
	{"\xE3\x83\xB4\xE3\x83\xA5", "vyu"}, // ヴュ
	{"\xE3\x81\xAF", "wa"},              // は
	{"\xE3\x82\x8F", "wa"},              // わ
	{"\xE3\x83\xAF", "wa"},              // ワ
	{"\xE3\x82\x91", "we"},              // ゑ
	{"\xE3\x82\xA6\xE3\x82\xA7", "we"},  // ウェ
	{"\xE3\x83\xB1", "we"},              // ヱ
	{"\xE3\x82\x90", "wi"},              // ゐ
	{"\xE3\x82\xA6\xE3\x82\xA3", "wi"},  // ウィ
	{"\xE3\x83\xB0", "wi"},              // ヰ
	{"\xE3\x82\x92", "wo"},              // を
	{"\xE3\x82\xA6\xE3\x82\xA9", "wo"},  // ウォ
	{"\xE3\x83\xB2", "wo"},              // ヲ
	{"\xE3\x82\x84", "ya"},              // や
	{"\xE3\x83\xA4", "ya"},              // ヤ
	{"\xE3\x82\xA4\xE3\x82\xA7", "ye"},  // イェ
	{"\xE3\x82\x88", "yo"},              // よ
	{"\xE3\x83\xA8", "yo"},              // ヨ
	{"\xE3\x82\x86", "yu"},              // ゆ
	{"\xE3\x83\xA6", "yu"},              // ユ
	{"\xE3\x81\x96", "za"},              // ざ
	{"\xE3\x82\xB6", "za"},              // ザ
	{"\xE3\x81\x9C", "ze"},              // ぜ
	{"\xE3\x82\xBC", "ze"},              // ゼ
	{"\xE3\x81\x9E", "zo"},              // ぞ
	{"\xE3\x82\xBE", "zo"},              // ゾ
	{"\xE3\x81\x9A", "zu"},              // ず
	{"\xE3\x81\xA5", "zu"},              // づ
	{"\xE3\x82\xBA", "zu"},              // ズ
	{"\xE3\x83\x85", "zu"},              // ヅ
};

bool cmp_kana(agi::kana_pair const& kp, std::string const& kana) {
	return strcmp(kp.kana, kana.c_str()) < 0;
}

struct cmp_romaji {
	bool operator()(agi::kana_pair const& kp, std::string const& romaji) const {
		return strcmp(kp.romaji, romaji.c_str()) < 0;
	}
	bool operator()(std::string const& romaji, agi::kana_pair const& kp) const {
		return strcmp(kp.romaji, romaji.c_str()) > 0;
	}

#ifdef _MSC_VER // debug iterator stuff needs this overload
	bool operator()(agi::kana_pair const& a, agi::kana_pair const& b) const {
		return strcmp(a.romaji, b.romaji) < 0;
	}
#endif
};

}

namespace agi {
std::vector<const char *> kana_to_romaji(std::string const& kana) {
	std::vector<const char *> ret;
	for (auto pair = boost::lower_bound(::kana_to_romaji, kana, cmp_kana);
		pair != std::end(::kana_to_romaji) && !strcmp(pair->kana, kana.c_str());
		++pair)
		ret.push_back(pair->romaji);
	return ret;
}

boost::iterator_range<const kana_pair *> romaji_to_kana(std::string const& romaji) {
	for (size_t len = std::min<size_t>(3, romaji.size()); len > 0; --len) {
		auto pair = boost::equal_range(::romaji_to_kana, romaji.substr(0, len).c_str(), cmp_romaji());
		if (pair.first != pair.second)
			return boost::make_iterator_range(pair.first, pair.second);
	}
	return boost::make_iterator_range(::romaji_to_kana, ::romaji_to_kana);
}
}
