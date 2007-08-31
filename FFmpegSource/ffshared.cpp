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