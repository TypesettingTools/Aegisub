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

#include "libaegisub/io.h"
#include "libaegisub/scoped_ptr.h"

#include "../../universalchardet/nsCharSetProber.h"


namespace agi {
	namespace charset {

UCDetect::UCDetect(const std::string &file): nsUniversalDetector(NS_FILTER_ALL) {
	{
		agi::scoped_ptr<std::ifstream> fp(io::Open(file, true));

		// If it's over 100 MB it's either binary or big enough that we won't
		// be able to do anything useful with it anyway
		fp->seekg(0, std::ios::end);
		if (fp->tellg() > 100 * 1024 * 1024) {
			list.insert(CLDPair(1.f, "binary"));
			return;
		}
		fp->seekg(0, std::ios::beg);

		std::streamsize binaryish = 0;
		std::streamsize bytes = 0;

		while (!mDone && *fp) {
			char buf[4096];
			fp->read(buf, sizeof(buf));
			std::streamsize read = fp->gcount();
			HandleData(buf, (PRUint32)read);

			// A dumb heuristic to detect binary files
			if (!mDone) {
				bytes += read;
				for (std::streamsize i = 0; i < read; ++i) {
					if ((unsigned char)buf[i] < 32 && (buf[i] != '\r' && buf[i] != '\n' && buf[i] != '\t'))
						++binaryish;
				}

				if (binaryish > bytes / 8) {
					list.insert(CLDPair(1.f, "binary"));
					return;
				}
			}
		}
	}

	DataEnd();

	if (mDetectedCharset) {
		list.insert(CLDPair(1.f, mDetectedCharset));
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
				list.insert(CLDPair(1.f, "US-ASCII"));
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
