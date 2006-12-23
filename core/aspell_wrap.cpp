// Copyright (c) 2005, Ghassan Nassar
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
// Contact: mailto:zeratul@cellosoft.com
//


///////////
//Includes
#include "setup.h"
#if USE_ASPELL == 1
#include "aspell_wrap.h"


//////////////////////
// Aspell Constructor
AspellWrapper::AspellWrapper(void) {
	loaded = false;
}


/////////////////////
// Aspell destructor
AspellWrapper::~AspellWrapper(void) {
	Unload();
}


//////////////////////
// Load Aspell-15.dll
void AspellWrapper::Load() {
	if (!loaded) {
		hLib=LoadLibrary(L"aspell-15.dll");
		if (hLib == NULL) {
			throw L"Could not load aspell.dll";
		}
		loaded = true;
	}
}


/////////////////////////
// Unloads Aspell-15.dll
void AspellWrapper::Unload() {
	if (loaded) {
		//delete_aspell_config();
		FreeLibrary(hLib);
		loaded = false;
	}
}



//////////////////
// Declare global
AspellWrapper Aspell;

#endif