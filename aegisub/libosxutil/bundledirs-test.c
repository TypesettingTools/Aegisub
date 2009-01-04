/*
  Copyright (c) 2009 Niels Martin Hansen
  
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

#include <stdio.h>
#include "libosxutil.h"

void print_path(const char *desc, char *path)
{
	if (!path) path = "(null)";
	printf("%s: %s\n", desc, path);
	if (path) free(path);
}

int main()
{
	char *path;

	printf("Trying to get the various bundle locations:\n");
	
	print_path("Bundle path", OSX_GetBundlePath());
	print_path("Resources directory", OSX_GetBundleResourcesDirectory());
	print_path("Built-in plugins directory", OSX_GetBundleBuiltInPlugInsDirectory());
	print_path("Private Frameworks dircetory", OSX_GetBundlePrivateFrameworksDirectory());
	print_path("Shared Frameworks directory", OSX_GetBundleSharedFrameworksDirectory());
	print_path("Shared Support directory", OSX_GetBundleSharedSupportDirectory());
	print_path("Support Files directory", OSX_GetBundleSupportFilesDirectory());
	print_path("Executable path", OSX_GetBundleExecutablePath());
	
	printf("All done.\n");
}
