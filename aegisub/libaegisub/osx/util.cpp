//  Copyright (c) 2008-2009 Niels Martin Hansen
//  Copyright (c) 2010 Amar Takhar
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

/// @file util.cpp
/// @brief OSX Utilities
/// @ingroup libosxutil osx


#include <string.h>
#include <sys/param.h>
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>
#include <ApplicationServices/ApplicationServices.h>

#include "libaegisub/util_osx.h"

namespace agi {
	namespace util {


/// @brief Type of functions that return a URL from a bundle.
/// @internal
typedef CFURLRef (*GetURLFunc)(CFBundleRef);

/// @brief Generic implementation to retrieve pathnames inside a bundle.
///
/// @internal Provide a generic implementation for most of the logic
///           in path-retrieval, since what differs for the various functions is
///           only the call used to retrieve the actual path inside the bundle.
static char * GetDir(GetURLFunc GetURL)
{
	CFBundleRef bundle;
	CFURLRef res_dir_url;
	char res_dir_str[MAXPATHLEN];
	Boolean res;

	bundle = CFBundleGetMainBundle();
	if (!bundle) return NULL;

	res_dir_url = (*GetURL)(bundle);
	/* we do not own 'bundle' so don't release it */
	if (!res_dir_url) return NULL;

	res = CFURLGetFileSystemRepresentation(res_dir_url, true, (UInt8*)res_dir_str, MAXPATHLEN);
	CFRelease(res_dir_url);

	if (res == false)
		return NULL;
	else
		return strdup(res_dir_str);
}


char * OSX_GetBundlePath()
{
	return GetDir(CFBundleCopyBundleURL);
}


char * OSX_GetBundleResourcesDirectory()
{
	return GetDir(CFBundleCopyResourcesDirectoryURL);
}


char * OSX_GetBundleExecutablePath()
{
	return GetDir(CFBundleCopyExecutableURL);
}


char * OSX_GetBundleBuiltInPlugInsDirectory()
{
	return GetDir(CFBundleCopyBuiltInPlugInsURL);
}


char * OSX_GetBundlePrivateFrameworksDirectory()
{
	return GetDir(CFBundleCopyPrivateFrameworksURL);
}


char * OSX_GetBundleSharedFrameworksDirectory()
{
	return GetDir(CFBundleCopySharedFrameworksURL);
}


char * OSX_GetBundleSharedSupportDirectory()
{
	return GetDir(CFBundleCopySharedSupportURL);
}


char * OSX_GetBundleSupportFilesDirectory()
{
	return GetDir(CFBundleCopySupportFilesDirectoryURL);
}


char * OSX_GetBundleAuxillaryExecutablePath(const char *executableName)
{
	CFStringRef exename_str;
	CFBundleRef bundle;
	CFURLRef res_dir_url;
	char res_dir_str[MAXPATHLEN];
	Boolean res;

	exename_str = CFStringCreateWithCString(NULL, executableName, kCFStringEncodingUTF8);
	if (!exename_str) return NULL;

	bundle = CFBundleGetMainBundle();
	if (!bundle) return NULL;

	res_dir_url = CFBundleCopyAuxiliaryExecutableURL(bundle, exename_str);
	CFRelease(exename_str);
	if (!res_dir_url) return NULL;

	res = CFURLGetFileSystemRepresentation(res_dir_url, true, (UInt8*)res_dir_str, MAXPATHLEN);
	CFRelease(res_dir_url);

	if (res == false)
		return NULL;
	else
		return strdup(res_dir_str);
}


void OSX_OpenLocation (const char *location) {

	CFStringRef CFSlocation = CFStringCreateWithCString(NULL, location, kCFStringEncodingUTF8);

	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, CFSlocation , kCFURLPOSIXPathStyle, false);
	OSStatus stat = LSOpenCFURLRef(url, NULL);
	printf("libosxutil::OSX_OpenLocation: %s\n", GetMacOSStatusCommentString(stat));
}

	} // namespace io
} // namespace agi

