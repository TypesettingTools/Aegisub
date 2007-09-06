//  Copyright (c) 2007 Fredrik Mellbin
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "ffmpegsource.h"

int GetPPCPUFlags(IScriptEnvironment *Env) {
	int Flags = 0;
	long CPUFlags = Env->GetCPUFlags();

	if (CPUFlags & CPUF_MMX)
		CPUFlags |= PP_CPU_CAPS_MMX;
	if (CPUFlags & CPUF_INTEGER_SSE)
		CPUFlags |= PP_CPU_CAPS_MMX2;
	if (CPUFlags & CPUF_3DNOW)
		CPUFlags |= PP_CPU_CAPS_3DNOW;

	return Flags;
}

int GetSWSCPUFlags(IScriptEnvironment *Env) {
	int Flags = 0;
	long CPUFlags = Env->GetCPUFlags();

	if (CPUFlags & CPUF_MMX)
		CPUFlags |= SWS_CPU_CAPS_MMX;
	if (CPUFlags & CPUF_INTEGER_SSE)
		CPUFlags |= SWS_CPU_CAPS_MMX2;
	if (CPUFlags & CPUF_3DNOW)
		CPUFlags |= SWS_CPU_CAPS_3DNOW;

	return Flags;
}