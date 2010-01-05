#include <ApplicationServices/ApplicationServices.h>
#include "libosxutil.h"

void OSX_OpenLocation (const char *location) {

	CFStringRef CFSlocation = CFStringCreateWithCString(NULL, location, kCFStringEncodingUTF8);

	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, CFSlocation , kCFURLPOSIXPathStyle, false);
	OSStatus stat = LSOpenCFURLRef(url, NULL);
	printf("libosxutil::OSX_OpenLocation: %s\n", GetMacOSStatusCommentString(stat));
}
