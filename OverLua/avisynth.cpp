/*
 * Avisynth interface for OverLua
 *

    Copyright 2007  Niels Martin Hansen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Contact:
    E-mail: <jiifurusu@gmail.com>
    IRC: jfs in #aegisub on irc.rizon.net

 */

#include <windows.h>
#include <string.h>
#include <memory.h>

#include "avisynth.h"

#include "overlua.h"

// Lots of code lifted from the CSRI avisynth.cpp

class OverLuaAvisynth : public GenericVideoFilter {
private:
	OverLuaScript *script;
	double spf; // seconds per frame - for frame/timestamp conversion

public:
	OverLuaAvisynth(PClip _child, IScriptEnvironment *env, const char *file, const char *datastring, const char *vfrfile)
		: GenericVideoFilter(_child)
	{
		switch (vi.pixel_type) {
			case VideoInfo::CS_BGR24:
			case VideoInfo::CS_BGR32:
				// safe
				break;
			default:
				env->ThrowError("OverLua: Unsupported pixel format, only RGB formats supported");
		}

		try {
			script = new OverLuaScript(file, datastring);
			spf = (double)vi.fps_denominator / (double)vi.fps_numerator;
		}
		catch (const char *e) {
			env->ThrowError(e);
		}
		catch (...) {
			env->ThrowError("Unknown exception in OverLua");
		}
	}

	~OverLuaAvisynth()
	{
		delete script;
	}

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env)
	{
		OutputDebugStringW(L"Entering OverLua GetFrame\n");
		PVideoFrame avsframe = child->GetFrame(n, env);
		env->MakeWritable(&avsframe);

		double frametime = n * spf;
		ptrdiff_t stride = avsframe->GetPitch();
		unsigned char *plane = avsframe->GetWritePtr();

		plane += (vi.height - 1) * stride;
		stride = -stride;

		try {
			switch (vi.pixel_type) {
				case VideoInfo::CS_BGR24: {
					OverLuaVideoFrameBGR *frame = new OverLuaVideoFrameBGR(vi.width, vi.height, stride, plane);
					script->RenderFrameRGB(*frame, frametime);
					} break;
				case VideoInfo::CS_BGR32: {
					OverLuaVideoFrameBGRX *frame = new OverLuaVideoFrameBGRX(vi.width, vi.height, stride, plane);
					script->RenderFrameRGB(*frame, frametime);
					} break;
			}
		}
		catch (const char *e) {
			wchar_t *ew = new wchar_t[2048];
			MultiByteToWideChar(CP_UTF8, 0, e, -1, ew, 2048);
			MessageBoxW(0, ew, L"OverLua execution error", MB_ICONERROR);
			delete[] ew;
			env->ThrowError(e);
		}
		catch (...) {
			MessageBoxW(0, L"Unknown error", L"OverLua execution error", MB_ICONERROR);
			env->ThrowError("OverLua: unknown execution error");
		}

		OutputDebugStringW(L"Leaving OverLua GetFrame\n");
		return avsframe;
	}

	static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env)
	{
		return new OverLuaAvisynth(args[0].AsClip(), env, args[1].AsString(), args[2].AsString(0), args[3].AsString(0));
	}
};


extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
	env->AddFunction("OverLua", "cs[data]s[vfr]s", OverLuaAvisynth::Create, 0);
	return "OverLua";
}

