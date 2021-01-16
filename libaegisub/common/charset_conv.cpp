// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file charset_conv.cpp
/// @brief Wrapper for libiconv to present a more C++-friendly API
/// @ingroup libaegisub

#include <errno.h>
#include <cstdint>

#include <cassert>
#include <boost/range/algorithm.hpp>

#include <libaegisub/charset_conv.h>
#include <iconv.h>

#include "charset_6937.h"

// Check if we can use advanced fallback capabilities added in GNU's iconv
// implementation
#if !defined(_LIBICONV_VERSION) || _LIBICONV_VERSION < 0x010A || defined(LIBICONV_PLUG)
#define ICONV_POSIX
#endif

#ifdef AGI_ICONV_CONST
#define ICONV_CONST_CAST(a) a
#else
#define ICONV_CONST_CAST(a) const_cast<char **>(a)
#endif

static const iconv_t iconv_invalid = (iconv_t)-1;
static const size_t iconv_failed = (size_t)-1;

namespace {
	using namespace agi::charset;

	Converter *get_converter(bool subst, const char *src, const char *dst);

/// @brief Map a user-friendly encoding name to the real encoding name
	const char *get_real_encoding_name(const char *name) {
		struct pair { const char *pretty; const char *real; };
		static pair pretty_names[] = {
#			define ADD(pretty, real) pair{pretty, real},
#			include <libaegisub/charsets.def>
#			undef ADD
		};

		static bool init = false;
		if (!init) {
			init = true;
			boost::sort(pretty_names, [](pair a, pair b) {
				return strcmp(a.pretty, b.pretty) < 0;
			});
		}

		auto enc = boost::lower_bound(pretty_names, name, [](pair a, const char *b) {
			return strcmp(a.pretty, b) < 0;
		});

		if (enc != std::end(pretty_names) && strcmp(enc->pretty, name) == 0)
			return enc->real;
		return name;
	}

	size_t get_bom_size(Iconv& cd) {
		// Most (but not all) iconv implementations automatically insert a BOM
		// at the beginning of text converted to UTF-8, UTF-16 and UTF-32, but
		// we usually don't want this, as some of the wxString using code
		// assumes there is no BOM (as the exact encoding is known externally)
		// As such, when doing conversions we will strip the BOM if it exists,
		// then manually add it when writing files

		char buff[8];
		const char* src = "";
		char *dst = buff;
		size_t srcLen = 1;
		size_t dstLen = 8;

		size_t res = cd(&src, &srcLen, &dst, &dstLen);
		assert(res != iconv_failed);
		assert(srcLen == 0);

		size_t size = 0;
		for (src = buff; src < dst; ++src) {
			if (*src) ++size;
		}
		if (size) {
			// If there is a BOM, it will always be at least as big as the NUL
			size = std::max(size, (8 - dstLen) / 2);
		}
		return size;
	}

	void eat_bom(Iconv& cd, size_t bomSize, const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) {
		// If this encoding has a forced BOM (i.e. it's UTF-16 or UTF-32 without
		// a specified byte order), skip over it
		if (bomSize > 0 && inbytesleft && *inbytesleft) {
			// libiconv marks the bom as written after writing the first
			// character after the bom rather than when it writes the bom, so
			// convert at least one extra character
			char bom[8];
			char *dst = bom;
			size_t dstSize = std::min((size_t)8, bomSize + *outbytesleft);
			const char *src = *inbuf;
			size_t srcSize = *inbytesleft;
			cd(&src, &srcSize, &dst, &dstSize);
		}
	}

	// Calculate the size of NUL in the given character set
	size_t nul_size(const char *encoding) {
		// We need a character set to convert from with a known encoding of NUL
		// UTF-8 seems like the obvious choice
		std::unique_ptr<Converter> cd(get_converter(false, "UTF-8", encoding));

		char dbuff[4];
		char sbuff[] = "";
		char* dst = dbuff;
		const char* src = sbuff;
		size_t dstLen = sizeof(dbuff);
		size_t srcLen = 1;

		size_t ret = cd->Convert(&src, &srcLen, &dst, &dstLen);
		assert(ret != iconv_failed);
		assert(dst - dbuff > 0);

		return dst - dbuff;
	}

#ifdef ICONV_POSIX
	class ConverterImpl final : public Converter {
		size_t bomSize;
		Iconv cd;
	public:
		// subst is not used here because POSIX doesn't let you disable substitution
		ConverterImpl(bool, const char* sourceEncoding, const char* destEncoding)
		{
			const char *dstEnc = get_real_encoding_name(destEncoding);
			cd = Iconv("utf-8", dstEnc);

			bomSize = get_bom_size(cd);
			cd = Iconv(get_real_encoding_name(sourceEncoding), dstEnc);
		}

		size_t Convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) {
			eat_bom(cd, bomSize, inbuf, inbytesleft, outbuf, outbytesleft);

			size_t res = cd(inbuf, inbytesleft, outbuf, outbytesleft);

			// This loop never does anything useful with a POSIX-compliant iconv
			// implementation, but those don't seem to actually exist
			while (res == iconv_failed && errno != E2BIG) {
				++*inbuf;
				--*inbytesleft;
				res = cd(inbuf, inbytesleft, outbuf, outbytesleft);
			}

			return res;
		}
	};

#else

	class ConverterImpl final : public iconv_fallbacks, public Converter {
		size_t bomSize;
		char invalidRep[8];
		size_t invalidRepSize;
		Iconv cd;
		static void fallback(
			unsigned int code,
			void (*callback) (const char *buf, size_t buflen, void* callback_arg),
			void *callback_arg,
			void *convPtr)
		{
			// At some point in the future, this should probably switch to a real mapping
			// For now, there's just three cases: BOM to nothing, '\' to itself
			// (for Shift-JIS, which does not have \) and everything else to '?'
			if (code == 0xFEFF) return;
			if (code == 0x5C) callback("\\", 1, callback_arg);
			else {
				ConverterImpl *self = static_cast<ConverterImpl *>(convPtr);
				callback(self->invalidRep, self->invalidRepSize, callback_arg);
			}
		}

	public:
		ConverterImpl(bool subst, const char* sourceEncoding, const char* destEncoding)
		{
			const char *dstEnc = get_real_encoding_name(destEncoding);
			cd = Iconv("utf-8", dstEnc);

			bomSize = get_bom_size(cd);

			// Get fallback character
			const char sbuff[] = "?";
			const char *src = sbuff;
			char *dst = invalidRep;
			size_t dstLen = 4;
			size_t srcLen = 1;

			size_t res = Convert(&src, &srcLen, &dst, &dstLen);
			assert(res != iconv_failed);
			assert(srcLen == 0);

			invalidRepSize = 4 - dstLen;

			cd = Iconv(get_real_encoding_name(sourceEncoding), dstEnc);

			if (subst) {
				data = this;
				mb_to_uc_fallback = nullptr;
				mb_to_wc_fallback = nullptr;
				uc_to_mb_fallback = fallback;
				wc_to_mb_fallback = nullptr;

				int transliterate = 1;
				iconvctl(cd, ICONV_SET_TRANSLITERATE, &transliterate);
				iconvctl(cd, ICONV_SET_FALLBACKS, static_cast<iconv_fallbacks*>(this));
			}
		}

		size_t Convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) override {
			eat_bom(cd, bomSize, inbuf, inbytesleft, outbuf, outbytesleft);
			size_t res = cd(inbuf, inbytesleft, outbuf, outbytesleft);

			if (res == iconv_failed && errno == E2BIG && *outbytesleft == 0) {
				// libiconv checks if there are any bytes left in the output buffer
				// before checking if the conversion would actually write any
				// characters to the output buffer, resulting in occasional invalid
				// E2BIG false positives
				char buff[8];
				size_t buffsize = 8;
				char* out = buff;
				const char* in = *inbuf;
				size_t insize = *inbytesleft;

				res = cd(&in, &insize, &out, &buffsize);
				// If no bytes of the output buffer were used, the original
				// conversion may have been successful
				if (buffsize != 8) {
					errno = E2BIG;
					res = iconv_failed;
				}
			}

			return res;
		}
	};
#endif

	Converter *get_converter(bool subst, const char *src, const char *dst) {
		try {
			return new ConverterImpl(subst, src, dst);
		}
		catch (UnsupportedConversion const&) {
			if (strcmp(dst, "ISO-6937-2"))
				throw;
			return new Converter6937(subst, src);
		}
	}
} // namespace {

namespace agi { namespace charset {
Iconv::Iconv() : cd(iconv_invalid) { }

Iconv::Iconv(const char *source, const char *dest)
: cd(iconv_open(dest, source))
{
	if (cd == iconv_invalid)
		throw UnsupportedConversion(std::string("Cannot convert from ") + source + " to " + dest);
}

Iconv::~Iconv() {
	if (cd != iconv_invalid) iconv_close(cd);
}

size_t Iconv::operator()(const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) {
	return iconv(cd, ICONV_CONST_CAST(inbuf), inbytesleft, outbuf, outbytesleft);
}

IconvWrapper::IconvWrapper(const char* sourceEncoding, const char* destEncoding, bool enableSubst)
: conv(get_converter(enableSubst, sourceEncoding, destEncoding))
{
	// These need to be set only after we verify that the source and dest
	// charsets are valid
	toNulLen = nul_size(destEncoding);
	fromNulLen = nul_size(sourceEncoding);
}

IconvWrapper::~IconvWrapper() { }

std::string IconvWrapper::Convert(const char *source, size_t len) {
	std::string dest;
	Convert(source, len, dest);
	return dest;
}

void IconvWrapper::Convert(const char *src, size_t srcLen, std::string &dest) {
	char buff[512];

	size_t res;
	do {
		char *dst = buff;
		size_t dstLen = sizeof(buff);
		res = conv->Convert(&src, &srcLen, &dst, &dstLen);
		if (res == 0) conv->Convert(nullptr, nullptr, &dst, &dstLen);

		dest.append(buff, sizeof(buff) - dstLen);
	} while (res == iconv_failed && errno == E2BIG);

	if (res == iconv_failed) {
		switch (errno) {
			case EILSEQ:
			case EINVAL:
				throw BadInput(
					"One or more characters in the input string were not valid "
					"characters in the given input encoding");
			default:
				throw ConversionFailure("An unknown conversion failure occurred");
		}
	}
}

size_t IconvWrapper::Convert(const char* source, size_t sourceSize, char *dest, size_t destSize) {
	if (sourceSize == (size_t)-1)
		sourceSize = SrcStrLen(source);

	size_t res = conv->Convert(&source, &sourceSize, &dest, &destSize);
	if (res == 0) res = conv->Convert(nullptr, nullptr, &dest, &destSize);

	if (res == iconv_failed) {
		switch (errno) {
			case E2BIG:
				throw BufferTooSmall(
					"Destination buffer was not large enough to fit converted "
					"string.");
			case EINVAL:
			case EILSEQ:
				throw BadInput(
					"One or more characters in the input string were not valid "
					"characters in the given input encoding");
			default:
				throw ConversionFailure("An unknown conversion failure occurred");
		}
	}
	return res;
}

size_t IconvWrapper::Convert(const char** source, size_t* sourceSize, char** dest, size_t* destSize) {
	return conv->Convert(source, sourceSize, dest, destSize);
}

size_t IconvWrapper::RequiredBufferSize(std::string const& str) {
	return RequiredBufferSize(str.data(), str.size());
}

size_t IconvWrapper::RequiredBufferSize(const char* src, size_t srcLen) {
	char buff[4096];
	size_t charsWritten = 0;
	size_t res;

	do {
		char* dst = buff;
		size_t dstSize = sizeof(buff);
		res = conv->Convert(&src, &srcLen, &dst, &dstSize);
		conv->Convert(nullptr, nullptr, &dst, &dstSize);

		charsWritten += dst - buff;
	} while (res == iconv_failed && errno == E2BIG);

	if (res == iconv_failed) {
		switch (errno) {
			case EINVAL:
			case EILSEQ:
				throw BadInput(
					"One or more characters in the input string were not valid "
					"characters in the given input encoding");
			default:
				throw ConversionFailure("An unknown conversion failure occurred");
		}
	}
	return charsWritten;
}

static size_t mbstrlen(const char* str, size_t nulLen) {
	const char *ptr;
	switch (nulLen) {
		case 1:
			return strlen(str);
		case 2:
			for (ptr = str; *reinterpret_cast<const uint16_t *>(ptr) != 0; ptr += 2) ;
			return ptr - str;
		case 4:
			for (ptr = str; *reinterpret_cast<const uint32_t *>(ptr) != 0; ptr += 4) ;
			return ptr - str;
		default:
			return (size_t)-1;
	}
}

size_t IconvWrapper::SrcStrLen(const char* str) {
	return mbstrlen(str, fromNulLen);

}
size_t IconvWrapper::DstStrLen(const char* str) {
	return mbstrlen(str, toNulLen);
}

bool IsConversionSupported(const char *src, const char *dst) {
	iconv_t cd = iconv_open(dst, src);
	bool supported = cd != iconv_invalid;
	iconv_close(cd);
	return supported;
}

	}
}
