// Copyright (c) 2007, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//


#include "auto3.h"

#include <stdlib.h>


// Win32 DLL entry point
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	// TODO: Destroy any still-alive scripts/interpreters here on unload?
    return TRUE;
}
#endif


// Create a new interpreter
// Returns pointer to interpreter object if successful, otherwise NULL
// If this function fails, the error message is filled into the char* pointed to by error (may be NULL)
AUTO3_API struct Auto3Interpreter *CreateAuto3Script(filename_t filename, char **error)
{
	return NULL;
}

// Release an interpreter
AUTO3_API void DestroyAuto3Script(struct Auto3Interpreter *script)
{
}

// Our "malloc" function, allocate memory for strings with this
AUTO3_API void *Auto3Malloc(size_t amount)
{
	return malloc(amount);
}

// Our "free" function, free generated error messages with this
AUTO3_API void Auto3Free(void *ptr)
{
	free(ptr);
}

// Start the script execution
// script->logcbdata and log->rwcbdata should be set to sensible values before this call.
// The value fields in the config dialog should also be set to values entered by the user here.
// This will first call get_meta_info,
// then reset_style pointer followed by a number of calls to get_next_style,
// then a call to reset_subs_pointer followed by a number of calls to get_next_sub,
// then actual processing will take place.
// After processing, start_subs_write will be called, followed by a number of calls to write_sub.
// Any number of calls to the logging/status functions can take place during script execution
AUTO3_API void RunAuto3Script(struct Auto3Interpreter *script)
{
}

