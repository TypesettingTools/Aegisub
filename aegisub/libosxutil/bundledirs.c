#include <string.h>
#include <sys/param.h>
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>

#include "libosxutil.h"


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
	// we do not own 'bundle' so don't release it
	if (!res_dir_url) return NULL;

	res = CFURLGetFileSystemRepresentation(res_dir_url, true, (UInt8*)res_dir_str, MAXPATHLEN);
	CFRelease(res_dir_url);

	if (res == false)
		return NULL;
	else
		return strdup(res_dir_str);
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

char * OSX_GetSharedFrameworksDirectory()
{
	return GetDir(CFBundleCopySharedFrameworksURL);
}

char * OSX_GetSharedSupportDirectory()
{
	return GetDir(CFBundleCopySharedSupportURL);
}

char * OSX_GetSupportFilesDirectory()
{
	return GetDir(CFBundleCopySupportFilesDirectoryURL);
}

char * OSX_GetAuxillaryExecutablePath(const char *executableName)
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

