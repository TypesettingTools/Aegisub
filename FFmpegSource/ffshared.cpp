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

AVSValue __cdecl CreateFFmpegSource(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
	if (!UserData) {
		av_register_all();
		UserData = (void *)-1;
	}

	if (!Args[0].Defined())
    	Env->ThrowError("FFmpegSource: No source specified");

	const char *Source = Args[0].AsString();
	int VTrack = Args[1].AsInt(-1);
	int ATrack = Args[2].AsInt(-2);
	const char *Timecodes = Args[3].AsString("");
	bool VCache = Args[4].AsBool(true);
	const char *VCacheFile = Args[5].AsString("");
	const char *ACacheFile = Args[6].AsString("");
	int ACCompression = Args[7].AsInt(-1);
	const char *PPString = Args[8].AsString("");
	int PPQuality = Args[9].AsInt(PP_QUALITY_MAX);
	int SeekMode = Args[10].AsInt(1);

	if (VTrack <= -2 && ATrack <= -2)
		Env->ThrowError("FFmpegSource: No tracks selected");

#ifdef FLAC_CACHE
	if (ACCompression < -1 || ACCompression > 8)
#else
	if (ACCompression != -1)
#endif // FLAC_CACHE
		Env->ThrowError("FFmpegSource: Invalid audio cache compression selected");


	AVFormatContext *FormatContext;

	if (av_open_input_file(&FormatContext, Source, NULL, 0, NULL) != 0)
		Env->ThrowError("FFmpegSource: Couldn't open %s", Args[0].AsString());
	bool IsMatroska = !strcmp(FormatContext->iformat->name, "matroska");
	av_close_input_file(FormatContext);

	if (IsMatroska)
		return new FFMatroskaSource(Source, VTrack, ATrack, Timecodes, VCache, VCacheFile, ACacheFile, ACCompression, PPString, PPQuality, Env);
	else
		return new FFmpegSource(Source, VTrack, ATrack, Timecodes, VCache, VCacheFile, ACacheFile, ACCompression, PPString, PPQuality, SeekMode, Env);
}

AVSValue __cdecl CreateFFPP(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
	return new FFPP(Args[0].AsClip(), Args[1].AsString(""), Args[2].AsInt(PP_QUALITY_MAX), Env);
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* Env) {
    Env->AddFunction("FFmpegSource", "[source]s[vtrack]i[atrack]i[timecodes]s[vcache]b[vcachefile]s[acachefile]s[accompression]i[pp]s[ppquality]i[seekmode]i", CreateFFmpegSource, 0);
    Env->AddFunction("FFPP", "c[pp]s[ppquality]i", CreateFFPP, 0);
    return "FFmpegSource";
};
