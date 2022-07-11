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

/// @file access.cpp
/// @brief Windows access methods.
/// @ingroup libaegisub windows

#include <libaegisub/access.h>

#include <libaegisub/format.h>
#include <libaegisub/format_path.h>
#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/util.h>

#include <boost/filesystem.hpp>

#include <windows.h>

namespace {
	bool check_permission(bool is_read, SECURITY_DESCRIPTOR *sd, HANDLE client_token) {
		DWORD access_check = is_read ? FILE_READ_DATA : FILE_APPEND_DATA | FILE_WRITE_DATA;

		GENERIC_MAPPING generic_mapping;
		MapGenericMask(&access_check, &generic_mapping);

		PRIVILEGE_SET priv_set;
		DWORD priv_set_size = sizeof(PRIVILEGE_SET);
		DWORD access;
		BOOL access_ok;
		if(!AccessCheck(sd, client_token, access_check, &generic_mapping, &priv_set, &priv_set_size, &access, &access_ok))
			LOG_W("acs/check") << "AccessCheck failed: " << agi::util::ErrorString(GetLastError());
		return !!access;
	}
}

namespace agi {
	namespace acs {

/*
This function is still a proof of concept, it's probably rife with bugs, below
is a short (and incomplete) todo
 * "Basic" checks (Read/Write/File/Dir) checks for FAT32 filesystems which
   requires detecting the filesystem being used.
*/
void Check(fs::path const& file, acs::Type type) {
	DWORD file_attr = GetFileAttributes(file.c_str());
	if ((file_attr & INVALID_FILE_ATTRIBUTES) == INVALID_FILE_ATTRIBUTES) {
		switch (GetLastError()) {
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				throw fs::FileNotFound(file);
			case ERROR_ACCESS_DENIED:
				throw fs::ReadDenied(file);
			default:
				throw fs::FileSystemUnknownError(agi::format("Unexpected error when getting attributes for \"%s\": %s", file, util::ErrorString(GetLastError())));
		}
	}

	switch (type) {
		case FileRead:
		case FileWrite:
			if ((file_attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				throw fs::NotAFile(file);
			break;
		case DirRead:
		case DirWrite:
			if ((file_attr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
				throw fs::NotADirectory(file);
			break;
	}

	SECURITY_INFORMATION info = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
	DWORD len = 0;
	GetFileSecurity(file.c_str(), info, nullptr, 0, &len);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		LOG_W("acs/check") << "GetFileSecurity: fatal: " << util::ErrorString(GetLastError());

	std::vector<uint8_t> sd_buff(len);
	SECURITY_DESCRIPTOR *sd = (SECURITY_DESCRIPTOR *)&sd_buff[0];

	if (!GetFileSecurity(file.c_str(), info, sd, len, &len))
		LOG_W("acs/check") << "GetFileSecurity failed: " << util::ErrorString(GetLastError());

	ImpersonateSelf(SecurityImpersonation);
	HANDLE client_token;
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &client_token))
		LOG_W("acs/check") << "OpenThreadToken failed: " << util::ErrorString(GetLastError());

	if (!check_permission(true, sd, client_token))
		throw fs::ReadDenied(file);
	if ((type == DirWrite || type == FileWrite) && !check_permission(false, sd, client_token))
		throw fs::WriteDenied(file);
}

	} // namespace Access
} // namespace agi
