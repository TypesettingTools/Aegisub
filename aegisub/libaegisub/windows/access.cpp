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

/// @file access.cpp
/// @brief Windows access methods.
/// @ingroup libaegisub windows

#ifndef LAGI_PRE
#include <windows.h>

#include <iostream>
#include <fstream>
#endif

#include "libaegisub/util.h"
#include "libaegisub/util_win.h"

namespace agi {
	namespace acs {

void CheckFileRead(const std::string &file) {
	Check(file, acs::FileRead);
}


void CheckFileWrite(const std::string &file) {
	Check(file, acs::FileWrite);
}


void CheckDirRead(const std::string &dir) {
	Check(dir, acs::DirRead);
}


void CheckDirWrite(const std::string &dir) {
	Check(dir, acs::DirWrite);
}

/*
This function is still a proof of concept, it's probably rife with bugs, below
is a short (and incomplete) todo
 * "Basic" checks (Read/Write/File/Dir) checks for FAT32 filesystems which
   requires detecting the filesystem being used.
*/
void Check(const std::string &file, acs::Type type) {
	std::wstring wfile;
	wfile.assign(file.begin(), file.end());

	SECURITY_DESCRIPTOR* sd;
	DWORD len = 0;
	SECURITY_INFORMATION info = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
	HANDLE client_token;
	PRIVILEGE_SET priv_set;
	DWORD priv_set_size = sizeof(PRIVILEGE_SET);
	BOOL access_ok;
	GENERIC_MAPPING generic_mapping;
	DWORD access_check;
	DWORD access;
	DWORD file_attr;

	file_attr = GetFileAttributes(wfile.c_str());

	if ((file_attr & INVALID_FILE_ATTRIBUTES) == INVALID_FILE_ATTRIBUTES) {

		switch (GetLastError()) {
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				throw AcsNotFound("File or path not found.");
			break;

			case ERROR_ACCESS_DENIED:
				throw AcsAccess("Access denied to file or path component");
			break;

			default:
				throw AcsFatal("Fatal I/O error occurred.");
			break;
		}
	}

	switch (type) {
		case FileRead:
		case FileWrite: {
			if ((file_attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				throw AcsNotAFile("Not a file.");
			break;
		}
		case DirRead:
		case DirWrite: {
			if ((file_attr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
				throw AcsNotADirectory("Not a directory.");
			break;
		}
	}

	GetFileSecurity(wfile.c_str(), info, NULL, 0, &len);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		std::cout << "GetFileSecurity: fatal: " << util::ErrorString(GetLastError()) << std::endl;
	}

	sd = (SECURITY_DESCRIPTOR *)malloc(len);
	if (sd == NULL) {
		std::cout << "GetFileSecurity: insufficient memory" << std::endl;
	} else {
		if (!GetFileSecurity(wfile.c_str(), info, sd, len, &len)) {
			std::cout << "GetFileSecurity failed:  " << util::ErrorString(GetLastError()) << std::endl;
			free(sd);
		}
	}

	ImpersonateSelf(SecurityImpersonation);
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &client_token)) {
		std::cout << "OpenThreadToken failed: " << util::ErrorString(GetLastError()) << std::endl;
	}

	switch (type) {
		case DirRead:
		case FileRead: {
			access_check = FILE_READ_DATA;
			MapGenericMask(&access_check, &generic_mapping);
			if(!AccessCheck(sd, client_token, access_check, &generic_mapping, &priv_set, &priv_set_size, &access, &access_ok)) {
				std::cout << "AccessCheck failed: " << util::ErrorString(GetLastError()) << std::endl;
			}
			free(sd);
			if (!access)
				throw AcsRead("File or directory is not readable.");
			break;
		}
		case DirWrite:
		case FileWrite: {
			access_check = FILE_APPEND_DATA | FILE_WRITE_DATA;
			MapGenericMask(&access_check, &generic_mapping);
			if(!AccessCheck(sd, client_token, access_check, &generic_mapping, &priv_set, &priv_set_size, &access, &access_ok)) {
				std::cout << "AccessCheck failed: " << util::ErrorString(GetLastError()) << std::endl;
			}
			free(sd);
			if (!access)
				throw AcsWrite("File or directory is not writable.");
			break;
		}
		default: {
			std::cout << "Warning: type not handled" << std::endl;
			free(sd);
		}
	}

}

	} // namespace Access
} // namespace agi
