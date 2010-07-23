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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file avisynth_wrap.cpp
/// @brief Wrapper-layer for Avisynth
/// @ingroup video_input audio_input
///

#include "config.h"

#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"
#include "main.h"

// Allocate storage for and initialise static members
int AviSynthWrapper::avs_refcount = 0;
HINSTANCE AviSynthWrapper::hLib = NULL;
IScriptEnvironment *AviSynthWrapper::env = NULL;
wxMutex AviSynthWrapper::AviSynthMutex;

/// @brief AviSynth constructor 
///
AviSynthWrapper::AviSynthWrapper() {
	if (!avs_refcount) {
		hLib=LoadLibrary(_T("avisynth.dll"));

		if (hLib == NULL) {
			throw wxString(_T("Could not load avisynth.dll"));
		}

		FUNC *CreateScriptEnv = (FUNC*)GetProcAddress(hLib, "CreateScriptEnvironment");

		if (CreateScriptEnv == NULL) {
			throw wxString(_T("Failed to get address of CreateScriptEnv from avisynth.dll"));
		}

		// Require Avisynth 2.5.6+?
		if (OPT_GET("Provider/Avisynth/Allow Ancient")->GetBool())
			env = CreateScriptEnv(AVISYNTH_INTERFACE_VERSION-1);
		else
			env = CreateScriptEnv(AVISYNTH_INTERFACE_VERSION);

		if (env == NULL) {
			throw wxString(_T("Failed to create a new avisynth script environment. Avisynth is too old?"));
		}
		// Set memory limit
		const int memoryMax = OPT_GET("Provider/Avisynth/Memory Max")->GetInt();
		if (memoryMax != 0) {
			env->SetMemoryMax(memoryMax);
		}
	}

	avs_refcount++;
}

/// @brief AviSynth destructor 
///
AviSynthWrapper::~AviSynthWrapper() {
	if (!--avs_refcount) {
		delete env;
		FreeLibrary(hLib);
	}
}

/// @brief Get environment 
///
IScriptEnvironment *AviSynthWrapper::GetEnv() {
	return env;
}
#endif
