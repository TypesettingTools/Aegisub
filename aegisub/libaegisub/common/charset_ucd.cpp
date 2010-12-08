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

/// @file charset_ucd.cpp
/// @brief Character set detection using Universalchardet
/// @ingroup libaegisub

#include "charset_ucd.h"

#ifndef LAGI_PRE
#include <memory>
#endif

#include "libaegisub/io.h"

#include "../../universalchardet/nsCharSetProber.h"


namespace agi {
	namespace charset {

UCDetect::UCDetect(const std::string &file): nsUniversalDetector(NS_FILTER_ALL) {
	{
		std::auto_ptr<std::ifstream> fp(io::Open(file));

		while (!mDone && !fp->eof()) {
			char buf[512];
			fp->read(buf, 512);
			size_t bytes = fp->gcount();
			HandleData(buf, bytes);
		}
	}

	DataEnd();

	if (mDetectedCharset) {
		list.insert(CLDPair(1, mDetectedCharset));
	} else {

		switch (mInputState) {
			case eHighbyte: {
				for (PRInt32 i=0; i<NUM_OF_CHARSET_PROBERS; i++) {
					if (mCharSetProbers[i]) {
						float conf = mCharSetProbers[i]->GetConfidence();
						if (conf > 0.01f) {
							list.insert(CLDPair(conf, mCharSetProbers[i]->GetCharSetName()));
						}
					}
				}

				break;
			}
			case ePureAscii:
				list.insert(CLDPair(1, "US-ASCII"));
				break;

			default:
				throw UnknownCharset("Unknown chararacter set.");
		}

		if (list.empty() && (mInputState == eHighbyte))
			throw UnknownCharset("Unknown chararacter set.");


	} // if mDetectedCharset else
}

std::string UCDetect::Single() {
	/// @todo Add a debug log here since this shouldn't happen.
	if (list.empty()) {
		throw UnknownCharset("Unknown chararacter set.");
	}

	CharsetListDetected::const_iterator i_lst = list.begin();
	return i_lst->second;
}



	} // namespace util
} // namespace agi
