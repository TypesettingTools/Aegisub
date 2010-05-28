// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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
// $Id$

/// @file charset_ucd.h
/// @brief Character set detection using Universalchardet
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include "../../universalchardet/nscore.h"
#include "../../universalchardet/nsUniversalDetector.h"
#include "../../universalchardet/nsMBCSGroupProber.h"
#endif

namespace agi {
	namespace charset {

class UCDetect : public nsUniversalDetector {

	/// For insertion into CharsetListDetected
	typedef std::pair<float, std::string> CLDPair;

	/// List of detected character sets.
	CharsetListDetected list;

	/// Stub.
	void Report(const char* aCharset) {};

public:

	/// @brief Detect character set of a file using UniversalCharDetect
	/// @param file File to check
	UCDetect(const std::string file);

	/// @brief Detect character set of a file using UniversalCharDet
	/// @param out[out] Map to load list into ordered by confidence
	void List(CharsetListDetected &out) { out = list; }

	/// @brief Return a single character set (highest confidence)
	/// @return Character set
	std::string Single();
};

	} // namespace util
} // namespace agi
