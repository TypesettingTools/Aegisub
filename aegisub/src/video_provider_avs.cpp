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
AvisynthVideoProvider::AvisynthVideoProvider(wxString _filename) {
	bool mpeg2dec3_priority = true;
	RGB32Video = NULL;
	num_frames = 0;
	last_fnum = -1;
	KeyFrames.clear();

	RGB32Video = OpenVideo(_filename,mpeg2dec3_priority);

	vi = RGB32Video->GetVideoInfo();
}

/// @brief Destructor 
///
AvisynthVideoProvider::~AvisynthVideoProvider() {
	iframe.Clear();
}

/// @brief Actually open the video into Avisynth 
/// @param _filename          
/// @param mpeg2dec3_priority 
/// @return 
///
PClip AvisynthVideoProvider::OpenVideo(wxString _filename, bool mpeg2dec3_priority) {
	wxMutexLocker lock(AviSynthMutex);
	AVSValue script;

	usedDirectShow = false;
	decoderName = _("Unknown");

	wxString extension = _filename.Right(4);
	extension.LowerCase();

	try {
		// Prepare filename
		//char *videoFilename = env->SaveString(_filename.mb_str(wxConvLocal));
		wxFileName fname(_filename);
		char *videoFilename = env->SaveString(fname.GetShortPath().mb_str(csConvLocal));

		// Avisynth file, just import it
		if (extension == _T(".avs")) {
			LOG_I("avisynth/video") << "Opening .avs file with Import";
			script = env->Invoke("Import", videoFilename);
			decoderName = _T("Import");
		}

		// Open avi file with AviSource
		else if (extension == _T(".avi")) {
			LOG_I("avisynth/video") << "Opening .avi file with AviSource";
			try {
				const char *argnames[2] = { 0, "audio" };
				AVSValue args[2] = { videoFilename, false };
				script = env->Invoke("AviSource", AVSValue(args,2), argnames);
				decoderName = _T("AviSource");
			}
			
			// On Failure, fallback to DSS
			catch (AvisynthError &) {
				LOG_I("avisynth/video") << "Failed to open .avi file with AviSource, switching to DirectShowSource";
				goto directshowOpen;
			}
		}

		// Open d2v with mpeg2dec3
		else if (extension == _T(".d2v") && env->FunctionExists("Mpeg2Dec3_Mpeg2Source") && mpeg2dec3_priority) {
			LOG_I("avisynth/video") << "Opening .d2v file with Mpeg2Dec3_Mpeg2Source";
			script = env->Invoke("Mpeg2Dec3_Mpeg2Source", videoFilename);
			decoderName = _T("Mpeg2Dec3_Mpeg2Source");

			//if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
			if (env->FunctionExists("SetPlanarLegacyAlignment")) {
				AVSValue args[2] = { script, true };
				script = env->Invoke("SetPlanarLegacyAlignment", AVSValue(args,2));
			}
		}

		// If that fails, try opening it with DGDecode
		else if (extension == _T(".d2v") && env->FunctionExists("DGDecode_Mpeg2Source")) {
			LOG_I("avisynth/video") << "Opening .d2v file with DGDecode_Mpeg2Source";
			script = env->Invoke("Mpeg2Source", videoFilename);
			decoderName = _T("DGDecode_Mpeg2Source");

			//note that DGDecode will also have issues like if the version is too ancient but no sane person
			//would use that anyway
		}

		else if (extension == _T(".d2v") && env->FunctionExists("Mpeg2Source")) {
			LOG_I("avisynth/video") << "Opening .d2v file with other Mpeg2Source";
			script = env->Invoke("Mpeg2Source", videoFilename);
			decoderName = _T("Mpeg2Source");

			//if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
			if (env->FunctionExists("SetPlanarLegacyAlignment"))
				script = env->Invoke("SetPlanarLegacyAlignment", script);
		}

		// Some other format, such as mkv, mp4, ogm... try both flavors of DirectShowSource
		else {
			directshowOpen:

			// Try loading DirectShowSource2
			bool dss2 = false;
			if (env->FunctionExists("dss2")) dss2 = true;
			if (!dss2) {
				wxFileName dss2path(StandardPaths::DecodePath(_T("?data/avss.dll")));
				if (dss2path.FileExists()) {
					env->Invoke("LoadPlugin",env->SaveString(dss2path.GetFullPath().mb_str(csConvLocal)));
				}
			}

			// If DSS2 loaded properly, try using it
			dss2 = false;
			if (env->FunctionExists("dss2")) {
				LOG_I("avisynth/video") << "Opening video with DSS2";
				script = env->Invoke("DSS2", videoFilename);
				dss2 = true;
				decoderName = _T("DSS2");
			}

			// Try DirectShowSource
			if (!dss2) {
				// Load DirectShowSource.dll from app dir if it exists
				wxFileName dsspath(StandardPaths::DecodePath(_T("?data/DirectShowSource.dll")));
				if (dsspath.FileExists()) {
					env->Invoke("LoadPlugin",env->SaveString(dsspath.GetFullPath().mb_str(csConvLocal)));
				}

				// Then try using DSS
				if (env->FunctionExists("DirectShowSource")) {
					const char *argnames[3] = { 0, "video", "audio" };
					AVSValue args[3] = { videoFilename, true, false };
					LOG_I("avisynth/video") << "Opening video with DirectShowSource";
					script = env->Invoke("DirectShowSource", AVSValue(args,3), argnames);
					usedDirectShow = true;
					decoderName = _T("DirectShowSource");
				}

				// Failed to find a suitable function
				else {
					LOG_E("avisynth/video") << "DSS function not found";
					throw AvisynthError("No function suitable for opening the video found");
				}
			}
		}
	}
	
	// Catch errors
	catch (AvisynthError &err) {
		LOG_E("avisynth/video") << "Avisynth error: " << err.msg;
		throw _T("AviSynth error: ") + wxString(err.msg,csConvLocal);
	}

	// Check if video was loaded properly
	if (!script.IsClip() || !script.AsClip()->GetVideoInfo().HasVideo()) {
		LOG_E("avisynth/video") << "AvisynthVideoProvider::OpenVideo: No suitable video found";
		throw _T("Avisynth: No usable video found in ") + _filename;
	}

	// Read keyframes and timecodes from MKV file
	bool mkvOpen = MatroskaWrapper::wrapper.IsOpen();
	KeyFrames.clear();
	if (extension == _T(".mkv") || mkvOpen) {
		// Parse mkv
		if (!mkvOpen) MatroskaWrapper::wrapper.Open(_filename);
		
		// Get keyframes
		KeyFrames = MatroskaWrapper::wrapper.GetKeyFrames();

		MatroskaWrapper::wrapper.SetToTimecodes(vfr_fps);

		// Close mkv
		MatroskaWrapper::wrapper.Close();
	}
// check if we have windows, if so we can load keyframes from AVI files using VFW
#ifdef __WINDOWS__
	else if (extension == _T(".avi")) {
		KeyFrames.clear();
		KeyFrames = VFWWrapper::GetKeyFrames(_filename);
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
	final.format = FORMAT_RGB32;
	final.flipped = true;
	final.invertChannels = true;

	// Set size properties
	final.pitch[0] = frame->GetPitch();
	final.w = frame->GetRowSize() / Bpp;
	final.h = frame->GetHeight();

	// Allocate
	final.Allocate();

	// Copy
	memcpy(final.data[0],frame->GetReadPtr(),final.pitch[0] * final.h);

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
