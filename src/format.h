// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <libaegisub/format.h>

#include <wx/string.h>
#include <wx/translation.h>

namespace agi {
template<>
struct writer<char, wxString> {
	static void write(std::basic_ostream<char>& out, int max_len, wxString const& value) {
		writer<char, const wxChar *>::write(out, max_len, value.wx_str());
	}
};

template<>
struct writer<wchar_t, wxString> {
	static void write(std::basic_ostream<wchar_t>& out, int max_len, wxString const& value) {
		writer<wchar_t, const wxChar *>::write(out, max_len, value.wx_str());
	}
};

template<typename... Args>
std::string format(wxString const& fmt, Args&&... args) {
	boost::interprocess::basic_vectorstream<std::basic_string<char>> out;
	format(out, (const char *)fmt.utf8_str(), std::forward<Args>(args)...);
	return out.vector();
}

template<typename... Args>
wxString wxformat(wxString const& fmt, Args&&... args) {
	boost::interprocess::basic_vectorstream<std::basic_string<wxChar>> out;
	format(out, fmt.wx_str(), std::forward<Args>(args)...);
	return out.vector();
}

template<typename... Args>
wxString wxformat(const wxChar *fmt, Args&&... args) {
	boost::interprocess::basic_vectorstream<std::basic_string<wxChar>> out;
	format(out, fmt, std::forward<Args>(args)...);
	return out.vector();
}
}

#define fmt_wx(str, ...) agi::wxformat(wxS(str), __VA_ARGS__)
#define fmt_tl(str, ...) agi::wxformat(wxGetTranslation(wxS(str)), __VA_ARGS__)
#define fmt_plural(n, sing, plural, ...) \
	agi::wxformat(wxGetTranslation(wxS(sing), wxS(plural), (n)), __VA_ARGS__)
