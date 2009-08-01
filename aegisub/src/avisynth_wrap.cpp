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


////////////
// Includes

#include "config.h"

#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"
#include "options.h"

#ifdef DEBUG_AVISYNTH_CODE
#include "main.h"
#include "wx/textfile.h"

/// DOCME
wxTextFile avs_trace_file;

/// @brief DOCME
/// @param s 
///
void DoAvsTrace(const wxString &s)
{
	if (!avs_trace_file.IsOpened()) {
		if (!avs_trace_file.Open(AegisubApp::folderName + _T("avstrace.txt"))) {
			avs_trace_file.Create(AegisubApp::folderName + _T("avstrace.txt"));
		}
		avs_trace_file.AddLine(_T(""));
		avs_trace_file.AddLine(_T("======= NEW SESSION ======="));
	}
	avs_trace_file.AddLine(s);
	avs_trace_file.Write();
}
#endif


// Allocate storage for and initialise static members
int AviSynthWrapper::avs_refcount = 0;
HINSTANCE AviSynthWrapper::hLib = NULL;
IScriptEnvironment *AviSynthWrapper::env = NULL;
wxMutex AviSynthWrapper::AviSynthMutex;



/// @brief AviSynth constructor 
///
AviSynthWrapper::AviSynthWrapper() {
	if (!avs_refcount) {
		AVSTRACE(_T("Avisynth not loaded, trying to load it now..."));
		hLib=LoadLibrary(_T("avisynth.dll"));

		if (hLib == NULL) {
			AVSTRACE(_T("Avisynth loading failed"));
			throw wxString(_T("Could not load avisynth.dll"));
		}
		AVSTRACE(_T("Avisynth loading successful"));
		
		FUNC *CreateScriptEnv = (FUNC*)GetProcAddress(hLib, "CreateScriptEnvironment");

		if (CreateScriptEnv == NULL) {
			AVSTRACE(_T("Failed to get address of CreateScriptEnv"));
			throw wxString(_T("Failed to get function from avisynth.dll"));
		}
		AVSTRACE(_T("Got address of CreateScriptEnv"));

		// Require Avisynth 2.5.6+?
		if (Options.AsBool(_T("Allow Ancient Avisynth")))
			env = CreateScriptEnv(AVISYNTH_INTERFACE_VERSION-1);
		else
			env = CreateScriptEnv(AVISYNTH_INTERFACE_VERSION);

		if (env == NULL) {
			AVSTRACE(_T("Failed to create script environment"));
			throw wxString(_T("Failed to create a new avisynth script environment. Avisynth is too old?"));
		}
		AVSTRACE(_T("Created script environment"));
		// Set memory limit
		int memoryMax = Options.AsInt(_T("Avisynth MemoryMax"));
		if (memoryMax != 0) {
			env->SetMemoryMax(memoryMax);
			AVSTRACE(_T("Set Avisynth memory limit"));
		}
	}

	avs_refcount++;
	AVSTRACE(_T("Increased reference count"));
}



/// @brief AviSynth destructor 
///
AviSynthWrapper::~AviSynthWrapper() {
	AVSTRACE(_T("Decreasing reference count"));
	if (!--avs_refcount) {
		AVSTRACE(_T("Reference count reached zero, deleting environment"));
		delete env;
		AVSTRACE(_T("Environment deleted"));
		FreeLibrary(hLib);
		AVSTRACE(_T("Free'd library, unloading complete"));
	}
}



/// @brief Get environment 
///
IScriptEnvironment *AviSynthWrapper::GetEnv() {
	return env;
}

#endif


