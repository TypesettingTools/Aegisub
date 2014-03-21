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

/// @file charset.cpp
/// @brief Character set detection and manipulation utilities.
/// @ingroup libaegisub

#include "libaegisub/charset.h"

#include "libaegisub/file_mapping.h"

#include <string>

#ifndef _WIN32
#define _X86_ 1
#endif

#include "../../vendor/universalchardet/nscore.h"
#include "../../vendor/universalchardet/nsUniversalDetector.h"
#include "../../vendor/universalchardet/nsMBCSGroupProber.h"
#include "../../vendor/universalchardet/nsCharSetProber.h"

namespace {
using namespace agi::charset;

class UCDetect final : public nsUniversalDetector {
	/// List of detected character sets
	CharsetListDetected list;

	void Report(const char*) override {}

public:
	/// @brief Detect character set of a file using UniversalCharDetect
	/// @param file File to check
	UCDetect(agi::fs::path const& file)
	: nsUniversalDetector(NS_FILTER_ALL)
	{
		{
			agi::read_file_mapping fp(file);

			// If it's over 100 MB it's either binary or big enough that we won't
			// be able to do anything useful with it anyway
			if (fp.size() > 100 * 1024 * 1024) {
				list.emplace_back(1.f, "binary");
				return;
			}

			uint64_t binaryish = 0;
			for (uint64_t offset = 0; !mDone && offset < fp.size(); ) {
				auto read = std::min<uint64_t>(4096, fp.size() - offset);
				auto buf = fp.read(offset, read);
				HandleData(buf, (PRUint32)read);
				offset += read;

				// A dumb heuristic to detect binary files
				if (!mDone) {
					for (size_t i = 0; i < read; ++i) {
						if ((unsigned char)buf[i] < 32 && (buf[i] != '\r' && buf[i] != '\n' && buf[i] != '\t'))
							++binaryish;
					}

					if (binaryish > offset / 8) {
						list.emplace_back(1.f, "binary");
						return;
					}
				}
			}
		}

		DataEnd();

		if (mDetectedCharset)
			list.emplace_back(1.f, mDetectedCharset);
		else {
			switch (mInputState) {
			case eHighbyte:
				for (auto& elem : mCharSetProbers) {
					if (!elem) continue;

					float conf = elem->GetConfidence();
					if (conf > 0.01f)
						list.emplace_back(conf, elem->GetCharSetName());
				}
				break;

			case ePureAscii:
				list.emplace_back(1.f, "US-ASCII");
				break;

			default:
				throw UnknownCharset("Unknown character set.");
			}

			if (list.empty() && (mInputState == eHighbyte))
				throw UnknownCharset("Unknown character set.");

			typedef std::pair<float, std::string> const& result;
			sort(begin(list), end(list), [](result lft, result rgt) { return lft.first > rgt.first; });
		}
	}

	/// @brief Detect character set of a file using UniversalCharDet
	CharsetListDetected List() const { return list; }
};
}

namespace agi { namespace charset {
	std::string Detect(agi::fs::path const& file) {
		return DetectAll(file).front().second;
	}

	CharsetListDetected DetectAll(agi::fs::path const& file) {
		return UCDetect(file).List();
	}
} }
