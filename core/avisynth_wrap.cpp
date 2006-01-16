// Copyright (c) 2005, Rodrigo Braz Monteiro
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


////////////
// Includes
#include "avisynth_wrap.h"
#include "options.h"

///////////////////////////////
// Static field initialization
int AviSynthWrapper::avs_refcount = 0;
HINSTANCE AviSynthWrapper::hLib = NULL;
IScriptEnvironment *AviSynthWrapper::env = NULL;
wxMutex AviSynthWrapper::AviSynthMutex;

////////////////////////
// AviSynth constructor
AviSynthWrapper::AviSynthWrapper() {
	if (!avs_refcount++) {
		hLib=LoadLibrary(_T("avisynth.dll"));

		if (hLib == NULL) 
			throw _T("Could not load avisynth.dll");
		
		FUNC *CreateScriptEnv = (FUNC*)GetProcAddress(hLib, "CreateScriptEnvironment");

		if (CreateScriptEnv == NULL)
			throw _T("Failed to get function from avisynth.dll");

		env = CreateScriptEnv(AVISYNTH_INTERFACE_VERSION);

		if (env == NULL)
			throw _T("Failed to create a new avisynth script environment. Avisynth is too old?");

		// Check for a new enough avisynth version by looking for the most obscure function used
		if (!env->FunctionExists("InternalCache"))
			throw _T("Installed version of avisynth is too old");

		// Set memory limit
		int memoryMax = Options.AsInt(_T("Avisynth MemoryMax"));
		if (memoryMax != 0)
			env->SetMemoryMax(memoryMax);

	}
}

///////////////////////
// AviSynth destructor
AviSynthWrapper::~AviSynthWrapper() {
	if (!--avs_refcount) {
		delete env;
		FreeLibrary(hLib);
	}
}
