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

/// @file avisynth_wrap.cpp
/// @brief Wrapper-layer for Avisynth
/// @ingroup video_input audio_input
///

#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"

#include "avisynth.h"
#include "options.h"

#include <mutex>

#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifdef _WIN32
#define AVISYNTH_SO "avisynth.dll"
#else
#define AVISYNTH_SO "libavisynth.so"
#endif

// Allocate storage for and initialise static members
namespace {
	int avs_refcount = 0;
#ifdef _WIN32
	HINSTANCE hLib = nullptr;
#else
	void* hLib = nullptr;
#endif
	IScriptEnvironment *env = nullptr;
	std::mutex AviSynthMutex;
}
// This needs to be visible so Avisynth sees it
const AVS_Linkage *AVS_linkage = nullptr;

typedef IScriptEnvironment* __stdcall FUNC(int);

AviSynthWrapper::AviSynthWrapper() {
	if (!avs_refcount){
#ifdef _WIN32
#define CONCATENATE(x, y) x ## y
#define _Lstr(x) CONCATENATE(L, x)
		hLib = LoadLibraryW(_Lstr(AVISYNTH_SO));
#undef _Lstr
#undef CONCATENATE
#else
		hLib = dlopen(AVISYNTH_SO, RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND);
#endif

		if (!hLib)
			throw AvisynthError("Could not load " AVISYNTH_SO);

#ifdef _WIN32
		FUNC* CreateScriptEnv = (FUNC*)GetProcAddress(hLib, "CreateScriptEnvironment");
#else
		FUNC* CreateScriptEnv = (FUNC*)dlsym(hLib, "CreateScriptEnvironment");
#endif
		if (!CreateScriptEnv)
			throw AvisynthError("Failed to get address of CreateScriptEnv from " AVISYNTH_SO);

		env = CreateScriptEnv(AVISYNTH_INTERFACE_VERSION);

		if (!env)
			throw AvisynthError("Failed to create a new avisynth script environment. Avisynth is too old?");

		avs_refcount++;

		AVS_linkage = env->GetAVSLinkage();

		// Set memory limit
		const int memoryMax = OPT_GET("Provider/Avisynth/Memory Max")->GetInt();
		if (memoryMax)
			env->SetMemoryMax(memoryMax);
	}
}

AviSynthWrapper::~AviSynthWrapper() {
	if (!--avs_refcount) {
		env->DeleteScriptEnvironment();
#ifdef _WIN32
		FreeLibrary(hLib);
#else
		dlclose(hLib);
#endif
		AVS_linkage = nullptr;
	}
}

std::mutex& AviSynthWrapper::GetMutex() const {
	return AviSynthMutex;
}

IScriptEnvironment *AviSynthWrapper::GetEnv() const {
	return env;
}

#endif
