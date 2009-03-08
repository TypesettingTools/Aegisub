// Copyright (c) 2006, Rodrigo Braz Monteiro
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
// Headers
#include "draw_prs.h"
#include "../prs/prs.h"


/////////////
// Libraries
#pragma comment(lib,"wxpng.lib")
#pragma comment(lib,"wxzlib.lib")


///////////////
// Constructor
DrawPRS::DrawPRS (IScriptEnvironment* _env, PClip _child, const char *filename)
: GenericVideoFilter(_child)
{
	// Set environment
	env = _env;

	// Load file
	try {
		file.Load(filename);
	}

	// Catch exception
	catch (const std::exception &e) {
		env->ThrowError("Error in PRS loading: %s",e.what());
	}
	catch (...) {
		env->ThrowError("Unhandled exception in PRS loading.");
	}
}


//////////////
// Destructor
DrawPRS::~DrawPRS() {
}


/////////////
// Get frame
PVideoFrame __stdcall DrawPRS::GetFrame(int n, IScriptEnvironment* env) {
	// Avisynth frame
	PVideoFrame avsFrame = child->GetFrame(n,env);

	try {
		// Check if there is anything to be drawn
		if (file.HasDataAtFrame(n)) {
			// Create the PRSFrame structure
			env->MakeWritable(&avsFrame);
			PRSVideoFrame frame;
			frame.data[0] = (char*) avsFrame->GetWritePtr();
			frame.w = avsFrame->GetRowSize()/4;
			frame.h = avsFrame->GetHeight();
			frame.pitch = avsFrame->GetPitch();
			frame.flipColors = true;
			frame.flipVertical = true;

			// Set colorspace
			VideoInfo vi = child->GetVideoInfo();
			if (vi.IsYV12()) frame.colorSpace = ColorSpace_YV12;
			else if (vi.IsYUY2()) frame.colorSpace = ColorSpace_YUY2;
			else if (vi.IsRGB32()) frame.colorSpace = ColorSpace_RGB32;
			else if (vi.IsRGB24()) frame.colorSpace = ColorSpace_RGB24;

			// Draw into the frame
			file.DrawFrame(n,&frame);
		}
	}

	// Catch exception
	catch (const std::exception &e) {
		env->ThrowError(e.what());
	}

	// Return frame
	return avsFrame;
}


//////////////////////////////
// Actually create the filter
// This is the function that created the filter, when the filter has been called.
AVSValue __cdecl CreateDrawPRS(AVSValue args, void* user_data, IScriptEnvironment* env) {
	return new DrawPRS(env, args[0].AsClip(), args[1].AsString());  
}


/////////////////////////
// Avisynth registration
// The following function is the function that actually registers the filter in AviSynth
// It is called automatically, when the plugin is loaded to see which functions this filter contains.
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
    env->AddFunction("DrawPRS", "c[file]s", CreateDrawPRS, 0);
    return "`Pre-Rendered Subtitles' by Rodrigo Braz Monteiro (ArchMage ZeratuL)";
}
