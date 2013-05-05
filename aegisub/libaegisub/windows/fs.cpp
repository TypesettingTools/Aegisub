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

#include "config.h"

#include "libaegisub/fs.h"

#include "libaegisub/access.h"
#include "libaegisub/charset_conv_win.h"
#include "libaegisub/exception.h"
#include "libaegisub/scoped_ptr.h"
#include "libaegisub/util_win.h"

using agi::charset::ConvertW;
using agi::charset::ConvertLocal;

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#undef CreateDirectory

namespace {
	FINDEX_INFO_LEVELS find_info_level() {
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		if (osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1))
			return FindExInfoBasic;
		else
			return FindExInfoStandard;
	}
}

namespace agi { namespace fs {
std::string ShortName(path const& p) {
	std::wstring out(MAX_PATH + 1, 0);
	DWORD len = GetShortPathName(p.c_str(), &out[0], out.size());
	if (!len)
		return p.string();
	out.resize(len);
	return ConvertLocal(out);
}

void Touch(path const& file) {
	CreateDirectory(file.parent_path());

	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	if(!SystemTimeToFileTime(&st, &ft))
		throw EnvironmentError("SystemTimeToFileTime failed with error: " + util::ErrorString(GetLastError()));

	scoped_holder<HANDLE, BOOL (__stdcall *)(HANDLE)>
		h(CreateFile(file.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr), CloseHandle);
	// error handling etc.
	if (!SetFileTime(h, nullptr, nullptr, &ft))
		throw EnvironmentError("SetFileTime failed with error: " + util::ErrorString(GetLastError()));
}

void Copy(fs::path const& from, fs::path const& to) {
	acs::CheckFileRead(from);
	CreateDirectory(to.parent_path());
	acs::CheckDirWrite(to.parent_path());

	if (!CopyFile(from.wstring().c_str(), to.wstring().c_str(), false)) {
		switch (GetLastError()) {
		case ERROR_FILE_NOT_FOUND:
			throw FileNotFound(from);
		case ERROR_ACCESS_DENIED:
			throw fs::WriteDenied("Could not overwrite " + to.string());
		default:
			throw fs::WriteDenied("Could not copy: " + util::ErrorString(GetLastError()));
		}
	}
}

struct DirectoryIterator::PrivData {
	scoped_holder<HANDLE, BOOL (__stdcall *)(HANDLE)> h;
	PrivData() : h(INVALID_HANDLE_VALUE, FindClose) { }
};

DirectoryIterator::DirectoryIterator() { }
DirectoryIterator::DirectoryIterator(path const& p, std::string const& filter)
: privdata(new PrivData)
{
	WIN32_FIND_DATA data;
	privdata->h = FindFirstFileEx((p/(filter.empty() ? "*.*" : filter)).c_str(), find_info_level(), &data, FindExSearchNameMatch, nullptr, 0);
	if (privdata->h == INVALID_HANDLE_VALUE) {
		privdata.reset();
		return;
	}

	value = ConvertW(data.cFileName);
	while (value[0] == '.' && (value[1] == 0 || value[1] == '.'))
		++*this;
}

bool DirectoryIterator::operator==(DirectoryIterator const& rhs) const {
	return privdata.get() == rhs.privdata.get();
}

DirectoryIterator& DirectoryIterator::operator++() {
	WIN32_FIND_DATA data;
	if (FindNextFile(privdata->h, &data))
		value = ConvertW(data.cFileName);
	else {
		privdata.reset();
		value.clear();
	}
	return *this;
}

DirectoryIterator::~DirectoryIterator() { }

} }
