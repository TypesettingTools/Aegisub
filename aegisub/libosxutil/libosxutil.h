/*
  Copyright (c) 2008 Niels Martin Hansen
  
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

/*
  Utility functions for running regular *NIX libraries inside application
  bundles on Apple Macintosh OS X.
  
  The GetBundle*Directory functions return the paths of directories inside
  the appliaction bundle where the application can store static data and
  shared libraries for its own use.
  (The bundle and its contents should be considered read-only.)
  
  When linking with this library, be sure to add '-framework CoreFoundation'
  to the GCC commandline.
 */

/*
  All strings returned by these functions are allocated by strdup(), it is
  the responsibility of the caller to free() them.
  All of the functions may return NULL on error.
 */

/* Get the resources directory, mainly for user interface elements
   such as graphics and strings */
char * OSX_GetBundleResourcesDirectory();
/* Get the built-in plugins directory. This is generaly only used by native
   Carbon and Cocoa applications. It is not for general shared libraries. */
char * OSX_GetBundleBuiltInPlugInsDirectory();
/* Get the frameworks directories. These are suitable locations for shared
   libraries.
   Does anyone know the difference between private and shared frameworks
   inside a bundle? */
char * OSX_GetBundlePrivateFrameworksDirectory();
char * OSX_GetBundleSharedFrameworksDirectory();
/* Get the shared support directory, this is a suitable location for static
   configuration files. (Remember, bundle is considered read-only.) */
char * OSX_GetBundleSharedSupportDirectory();
/* Again, what is the difference between Support and SharedSupport? */
char * OSX_GetBundleSupportFilesDirectory();

/* Get the path to the main executable of the bundle, ie. the binary run
   when the user launches the bundle from Finder. */
char * OSX_GetBundleExecutablePath();
/* Get the path of an auxillary exetutable within the bundle.
   Pass the basename of the exetutable to get the path of. */
char * OSX_GetBundleAuxillaryExecutablePath(const char *executableName);
