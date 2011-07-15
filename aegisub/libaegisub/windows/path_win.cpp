// Copyright (c) 2011, Niels Martin Hansen <nielsm@aegisub.org>
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

/// @file path.cpp
/// @brief Common paths.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <string>
#endif

#include <libaegisub/path.h>

#include <libaegisub/charset_conv_win.h>
#include <libaegisub/util_win.h>


namespace {
#include <Shlobj.h>
#include <Shellapi.h>

std::string WinGetFolderPath(int folder) {
	wchar_t path[MAX_PATH+1] = {0};
	HRESULT res = SHGetFolderPathW(
		0,      // hwndOwner
		folder, // nFolder
		0,      // hToken
		0,      // dwFlags
		path    // pszPath
		);
	if (FAILED(res))
		throw agi::PathErrorInternal("SHGetFolderPath() failed"); //< @fixme error message?

	return agi::charset::ConvertW(path);
}

std::string get_install_path() {
	static std::string install_path;
	if (install_path.empty()) {
		// Excerpt from <http://msdn.microsoft.com/en-us/library/bb776391.aspx>:
		// lpCmdLine [in]
		//     If this parameter is an empty string the function returns
		//     the path to the current executable file.
		int argc;
		LPWSTR *argv = CommandLineToArgvW(L"", &argc);
		
		wchar_t path[MAX_PATH+1] = {0};
		wchar_t *fn;
		DWORD res = GetFullPathNameW(argv[0], MAX_PATH, path, &fn);
		LocalFree(argv);
		
		if (res > 0 && GetLastError() == 0) {
			*fn = '\0'; // fn points to filename part of path, set an end marker there
			install_path = agi::charset::ConvertW(std::wstring(path));
		} else {
			throw agi::PathErrorInternal(agi::util::ErrorString(GetLastError()));
		}
	}

	return install_path;
}

}


namespace agi {

std::string Path::Data() {
	return get_install_path();
}

std::string Path::Doc() {
	return Data() + "docs\\";
}

std::string Path::User() {
	return WinGetFolderPath(CSIDL_PERSONAL);
}

std::string Path::Locale() {
	return Data() + "locale\\";
}

std::string Path::Config() {
	return WinGetFolderPath(CSIDL_APPDATA) + "Aegisub3";
	/// @fixme should get version number in a more dynamic manner
}

std::string Path::Temp() {
	wchar_t path[MAX_PATH+1] = {0};
	if (GetTempPath(MAX_PATH, path) == 0)
		throw PathErrorInternal(util::ErrorString(GetLastError()));
	else
		return charset::ConvertW(path);
}

} // namespace agi
