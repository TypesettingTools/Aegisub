/*
  Copyright (c) 2008-2009 Niels Martin Hansen

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the Aegisub Group nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 */

/** @file bundledirs.c
    @brief Get various paths from within an OS X bundle.
 */

#include <string.h>
#include <sys/param.h>
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>

#include "libosxutil.h"

/** @todo document me. */
typedef CFURLRef (*GetURLFunc)(CFBundleRef);

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

