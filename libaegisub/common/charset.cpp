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

#include "libaegisub/charset.h"

#include "libaegisub/file_mapping.h"
#include "libaegisub/log.h"
#include "libaegisub/scoped_ptr.h"

#include <array>
#include <memory>

#ifdef WITH_UCHARDET
#include <uchardet.h>
#include <unicode/ucnv.h>
#endif

namespace agi::charset {
std::string Detect(agi::fs::path const& file, DetectReason *reason) {
	auto set_reason = [reason](DetectReason r) { if (reason) *reason = r; };
	agi::read_file_mapping fp(file);

	// First check for known magic bytes which identify the file type
	if (fp.size() >= 4) {
		const char* header = fp.read(0, 4);
		if (!strncmp(header, "\xef\xbb\xbf", 3))
			return set_reason(DetectReason::Signature), "utf-8";
		if (!strncmp(header, "\x00\x00\xfe\xff", 4))
			return set_reason(DetectReason::Signature), "utf-32be";
		if (!strncmp(header, "\xff\xfe\x00\x00", 4))
			return set_reason(DetectReason::Signature), "utf-32le";
		if (!strncmp(header, "\xfe\xff", 2))
			return set_reason(DetectReason::Signature), "utf-16be";
		if (!strncmp(header, "\xff\xfe", 2))
			return set_reason(DetectReason::Signature), "utf-16le";
		if (!strncmp(header, "\x1a\x45\xdf\xa3", 4))
			return set_reason(DetectReason::Signature), "binary"; // Actually EBML/Matroska
	}

	// If it's over 100 MB it's either binary or big enough that we won't
	// be able to do anything useful with it anyway
	if (fp.size() > 100 * 1024 * 1024)
		return set_reason(DetectReason::SizeHeuristic), "binary";

	uint64_t binaryish = 0;

#ifdef WITH_UCHARDET
	agi::scoped_holder<uchardet_t> ud(uchardet_new(), uchardet_delete);

	UErrorCode utf8Status = U_ZERO_ERROR;
	std::unique_ptr<UConverter, decltype(&ucnv_close)> conv = {ucnv_open("UTF-8", &utf8Status), ucnv_close};
	if (conv)
		ucnv_setToUCallBack(conv.get(), UCNV_TO_U_CALLBACK_STOP, nullptr, nullptr, nullptr, &utf8Status);
	if (utf8Status != U_ZERO_ERROR)
		LOG_W("charset/detect") << "Unexpected ICU error: " << u_errorName(utf8Status);
	std::array<UChar, 2048> convBuffer;

	for (uint64_t offset = 0; offset < fp.size(); ) {
		auto read = std::min<uint64_t>(4096, fp.size() - offset);
		auto buf = fp.read(offset, read);
		uchardet_handle_data(ud, buf, read);

		offset += read;

		const char *source = buf;
		const char *sourceLimit = source + read;
		bool flush = offset >= fp.size();
		while (U_SUCCESS(utf8Status)) {
			UChar *target = convBuffer.data();
			UChar *targetLimit = target + convBuffer.size();
			ucnv_toUnicode(conv.get(), &target, targetLimit, &source, sourceLimit, nullptr, flush, &utf8Status);
			if (utf8Status == U_BUFFER_OVERFLOW_ERROR) {
				// result didn't fit in target buffer, try again
				utf8Status = U_ZERO_ERROR;
			} else if (source == sourceLimit) {
				break;
			}
		}

		// A dumb heuristic to detect binary files
		for (size_t i = 0; i < read; ++i) {
			if ((unsigned char)buf[i] < 32 && (buf[i] != '\r' && buf[i] != '\n' && buf[i] != '\t'))
				++binaryish;
		}

		if (binaryish > offset / 8)
			return set_reason(DetectReason::BinaryHeuristic), "binary";
	}
	LOG_D("charset/detect") << "UTF-8 detection result: " << u_errorName(utf8Status);
	if (U_SUCCESS(utf8Status))
		return set_reason(DetectReason::ValidUtf8), "utf-8";
	uchardet_data_end(ud);
	return set_reason(DetectReason::Uchardet), uchardet_get_charset(ud);
#else
	auto read = std::min<uint64_t>(4096, fp.size());
	auto buf = fp.read(0, read);
	for (size_t i = 0; i < read; ++i) {
		if ((unsigned char)buf[i] < 32 && (buf[i] != '\r' && buf[i] != '\n' && buf[i] != '\t'))
			++binaryish;
	}

	if (binaryish > read / 8)
		return set_reason(DetectReason::BinaryHeuristic), "binary";
	return set_reason(DetectReason::Utf8Fallback), "utf-8";
#endif
}
}
