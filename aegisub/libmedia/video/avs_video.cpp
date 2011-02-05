// Copyright (c) 2006, Fredrik Mellbin
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

/// @file video_provider_avs.cpp
/// @brief Avisynth-based video provider
/// @ingroup video_input
///

#include "config.h"

#ifdef WITH_AVISYNTH
#ifndef AGI_PRE
#include <wx/filename.h>
#include <wx/msw/registry.h>
#endif

#include "charset_conv.h"
#include "compat.h"
#include "gl_wrap.h"
#include <libaegisub/log.h>
#include "mkv_wrap.h"
#include "standard_paths.h"
#include "vfw_wrap.h"
#include "video_context.h"
#include "video_provider_avs.h"

/// @brief Constructor 
/// @param _filename 
///
AvisynthVideoProvider::AvisynthVideoProvider(wxString filename) try
: usedDirectShow(false)
, decoderName(_("Unknown"))
, num_frames(0)
, last_fnum(-1)
, RGB32Video(NULL)
{
	RGB32Video = OpenVideo(filename);

	vi = RGB32Video->GetVideoInfo();
}
catch (AvisynthError const& err) {
	throw VideoOpenError("Avisynth error: " + std::string(err.msg));
}

/// @brief Destructor 
AvisynthVideoProvider::~AvisynthVideoProvider() {
	iframe.Clear();
}

AVSValue AvisynthVideoProvider::Open(wxFileName const& fname, wxString const& extension) {
	char *videoFilename = env->SaveString(fname.GetShortPath().mb_str(csConvLocal));

	// Avisynth file, just import it
	if (extension == L".avs") {
		LOG_I("avisynth/video") << "Opening .avs file with Import";
		decoderName = L"Import";
		return env->Invoke("Import", videoFilename);
	}

	// Open avi file with AviSource
	if (extension == L".avi") {
		LOG_I("avisynth/video") << "Opening .avi file with AviSource";
		try {
			const char *argnames[2] = { 0, "audio" };
			AVSValue args[2] = { videoFilename, false };
			decoderName = L"AviSource";
			return env->Invoke("AviSource", AVSValue(args,2), argnames);
		}

		// On Failure, fallback to DSS
		catch (AvisynthError &) {
			LOG_I("avisynth/video") << "Failed to open .avi file with AviSource, switching to DirectShowSource";
		}
	}

	// Open d2v with mpeg2dec3
	if (extension == L".d2v" && env->FunctionExists("Mpeg2Dec3_Mpeg2Source")) {
		LOG_I("avisynth/video") << "Opening .d2v file with Mpeg2Dec3_Mpeg2Source";
		AVSValue script = env->Invoke("Mpeg2Dec3_Mpeg2Source", videoFilename);
		decoderName = L"Mpeg2Dec3_Mpeg2Source";

		//if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
		if (env->FunctionExists("SetPlanarLegacyAlignment")) {
			AVSValue args[2] = { script, true };
			script = env->Invoke("SetPlanarLegacyAlignment", AVSValue(args,2));
		}
		return script;
	}

	// If that fails, try opening it with DGDecode
	if (extension == L".d2v" && env->FunctionExists("DGDecode_Mpeg2Source")) {
		LOG_I("avisynth/video") << "Opening .d2v file with DGDecode_Mpeg2Source";
		decoderName = L"DGDecode_Mpeg2Source";
		return env->Invoke("Mpeg2Source", videoFilename);

		//note that DGDecode will also have issues like if the version is too ancient but no sane person
		//would use that anyway
	}

	if (extension == L".d2v" && env->FunctionExists("Mpeg2Source")) {
		LOG_I("avisynth/video") << "Opening .d2v file with other Mpeg2Source";
		AVSValue script = env->Invoke("Mpeg2Source", videoFilename);
		decoderName = L"Mpeg2Source";

		//if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
		if (env->FunctionExists("SetPlanarLegacyAlignment"))
			script = env->Invoke("SetPlanarLegacyAlignment", script);

		return script;
	}

	// Try loading DirectShowSource2
	if (!env->FunctionExists("dss2")) {
		wxFileName dss2path(StandardPaths::DecodePath(_T("?data/avss.dll")));
		if (dss2path.FileExists()) {
			env->Invoke("LoadPlugin",env->SaveString(dss2path.GetFullPath().mb_str(csConvLocal)));
		}
	}

	// If DSS2 loaded properly, try using it
	if (env->FunctionExists("dss2")) {
		LOG_I("avisynth/video") << "Opening file with DSS2";
		decoderName = L"DSS2";
		return env->Invoke("DSS2", videoFilename);
	}

	// Try DirectShowSource
	// Load DirectShowSource.dll from app dir if it exists
	wxFileName dsspath(StandardPaths::DecodePath(_T("?data/DirectShowSource.dll")));
	if (dsspath.FileExists()) {
		env->Invoke("LoadPlugin",env->SaveString(dsspath.GetFullPath().mb_str(csConvLocal)));
	}

	// Then try using DSS
	if (env->FunctionExists("DirectShowSource")) {
		const char *argnames[3] = { 0, "video", "audio" };
		AVSValue args[3] = { videoFilename, true, false };
		usedDirectShow = true;
		decoderName = L"DirectShowSource";
		LOG_I("avisynth/video") << "Opening file with DirectShowSource";
		return env->Invoke("DirectShowSource", AVSValue(args,3), argnames);
	}

	// Failed to find a suitable function
	LOG_E("avisynth/video") << "DSS function not found";
	throw VideoNotSupported("No function suitable for opening the video found");
}

/// @brief Actually open the video into Avisynth 
/// @param _filename          
/// @return 
///
PClip AvisynthVideoProvider::OpenVideo(wxString filename) {
	wxMutexLocker lock(AviSynthMutex);

	wxFileName fname(filename);
	if (!fname.FileExists())
		throw agi::FileNotFoundError(STD_STR(filename));

	AVSValue script;
	wxString extension = filename.Right(4).Lower();
	try {
		script = Open(fname, extension);
	}
	catch (AvisynthError const& err) {
		throw VideoOpenError("Avisynth error: " + std::string(err.msg));
	}

	// Check if video was loaded properly
	if (!script.IsClip() || !script.AsClip()->GetVideoInfo().HasVideo()) {
		throw VideoNotSupported("No usable video found");
	}

	// Read keyframes and timecodes from MKV file
	bool mkvOpen = MatroskaWrapper::wrapper.IsOpen();
	KeyFrames.clear();
	if (extension == L".mkv" || mkvOpen) {
		// Parse mkv
		if (!mkvOpen) MatroskaWrapper::wrapper.Open(filename);
		
		// Get keyframes
		KeyFrames = MatroskaWrapper::wrapper.GetKeyFrames();

		MatroskaWrapper::wrapper.SetToTimecodes(vfr_fps);

		// Close mkv
		MatroskaWrapper::wrapper.Close();
	}
// check if we have windows, if so we can load keyframes from AVI files using VFW
#ifdef __WINDOWS__
	else if (extension == L".avi") {
		KeyFrames.clear();
		KeyFrames = VFWWrapper::GetKeyFrames(filename);
	}
#endif /* __WINDOWS__ */

	// Check if the file is all keyframes
	bool isAllKeyFrames = true;
	for (unsigned int i=1; i<KeyFrames.size(); i++) {
		// Is the last keyframe not this keyframe -1?
		if (KeyFrames[i-1] != (int)(i-1)) {
			// It's not all keyframes, go ahead
			isAllKeyFrames = false;
			break;
		}
	}

	// If it is all keyframes, discard the keyframe info as it is useless
	if (isAllKeyFrames) {
		KeyFrames.clear();
	}

	real_fps = (double)vi.fps_numerator / vi.fps_denominator;

	// Convert to RGB32
	script = env->Invoke("ConvertToRGB32", script);

	// Cache
	return (env->Invoke("Cache", script)).AsClip();
}

/// @brief Actually get a frame 
/// @param _n 
/// @return 
///
const AegiVideoFrame AvisynthVideoProvider::GetFrame(int n) {
	if (vfr_fps.IsLoaded()) {
		n = real_fps.FrameAtTime(vfr_fps.TimeAtFrame(n));
	}
	// Get avs frame
	wxMutexLocker lock(AviSynthMutex);
	PVideoFrame frame = RGB32Video->GetFrame(n,env);
	int Bpp = vi.BitsPerPixel() / 8;

	// Aegisub's video frame
	AegiVideoFrame &final = iframe;
	final.flipped = true;
	final.invertChannels = true;

	// Set size properties
	final.pitch = frame->GetPitch();
	final.w = frame->GetRowSize() / Bpp;
	final.h = frame->GetHeight();

	// Allocate
	final.Allocate();

	// Copy
	memcpy(final.data,frame->GetReadPtr(),final.pitch * final.h);

	// Set last number
	last_fnum = n;
	return final;
}

/// @brief Get warning 
///
wxString AvisynthVideoProvider::GetWarning() const {
	if (usedDirectShow) return L"Warning! The file is being opened using Avisynth's DirectShowSource, which has unreliable seeking. Frame numbers might not match the real number. PROCEED AT YOUR OWN RISK!";
	else return L"";
}

#endif
