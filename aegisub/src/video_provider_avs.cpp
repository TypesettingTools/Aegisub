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

/// @file video_provider_avs.cpp
/// @brief Avisynth-based video provider
/// @ingroup video_input
///

#include "config.h"

#ifdef WITH_AVISYNTH

#include "video_provider_avs.h"

#include <wx/filename.h>
#include <wx/msw/registry.h>

#ifdef _WIN32
#include <vfw.h>
#endif

#include <libaegisub/log.h>

#include "charset_conv.h"
#include "compat.h"
#include "main.h"
#include "standard_paths.h"

AvisynthVideoProvider::AvisynthVideoProvider(wxString filename)
: last_fnum(-1)
{
	iframe.flipped = true;
	iframe.invertChannels = true;

	wxMutexLocker lock(avs.GetMutex());

	wxFileName fname(filename);
	if (!fname.FileExists())
		throw agi::FileNotFoundError(STD_STR(filename));

	wxString extension = filename.Right(4).Lower();

#ifdef _WIN32
	if (extension == ".avi") {
		// Try to read the keyframes before actually opening the file as trying
		// to open the file while it's already open can cause problems with
		// badly written VFW decoders
		AVIFileInit();

		PAVIFILE pfile;
		long hr = AVIFileOpen(&pfile, filename.wc_str(), OF_SHARE_DENY_WRITE, 0);
		if (hr) {
			warning = "Unable to open AVI file for reading keyframes:\n";
			switch (hr) {
				case AVIERR_BADFORMAT:
					warning += "The file is corrupted, incomplete or has an otherwise bad format.";
					break;
				case AVIERR_MEMORY:
					warning += "The file could not be opened because of insufficient memory.";
					break;
				case AVIERR_FILEREAD:
					warning += "An error occurred reading the file. There might be a problem with the storage media.";
					break;
				case AVIERR_FILEOPEN:
					warning += "The file could not be opened. It might be in use by another application, or you do not have permission to access it.";
					break;
				case REGDB_E_CLASSNOTREG:
					warning += "There is no handler installed for the file extension. This might indicate a fundamental problem in your Video for Windows installation, and can be caused by extremely stripped Windows installations.";
					break;
				default:
					warning += "Unknown error.";
					break;
			}
			goto file_exit;
		}

		PAVISTREAM ppavi;
		if (hr = AVIFileGetStream(pfile, &ppavi, streamtypeVIDEO, 0)) {
			warning = "Unable to open AVI video stream for reading keyframes:\n";
			switch (hr) {
				case AVIERR_NODATA:
					warning += "The file does not contain a usable video stream.";
					break;
				case AVIERR_MEMORY:
					warning += "Not enough memory.";
					break;
				default:
					warning += "Unknown error.";
					break;
			}
			goto file_release;
		}

		AVISTREAMINFO avis;
		if (FAILED(AVIStreamInfo(ppavi,&avis,sizeof(avis)))) {
			warning = "Unable to read keyframes from AVI file:\nCould not get stream information.";
			goto stream_release;
		}

		bool all_keyframes = true;
		for (size_t i = 0; i < avis.dwLength; i++) {
			if (AVIStreamIsKeyFrame(ppavi, i))
				KeyFrames.push_back(i);
			else
				all_keyframes = false;
		}

		// If every frame is a keyframe then just discard the keyframe data as it's useless
		if (all_keyframes) KeyFrames.clear();

		// Clean up
stream_release:
		AVIStreamRelease(ppavi);
file_release:
		AVIFileRelease(pfile);
file_exit:
		AVIFileExit();
	}
#endif

	try {
		AVSValue script = Open(fname, extension);

		// Check if video was loaded properly
		if (!script.IsClip() || !script.AsClip()->GetVideoInfo().HasVideo())
			throw VideoNotSupported("No usable video found");

		vi = script.AsClip()->GetVideoInfo();
		if (!vi.IsRGB()) {
			/// @todo maybe read ColorMatrix hints for d2v files?

			AVSValue args[2] = { script, "Rec601" };
			if (!OPT_GET("Video/Force BT.601")->GetBool() && (vi.width > 1024 || vi.height >= 600)) {
				args[1] = "Rec709";
				colorspace = "TV.709";
			}
			else
				colorspace = "TV.601";
			const char *argnames[2] = { 0, "matrix" };
			script = avs.GetEnv()->Invoke("ConvertToRGB32", AVSValue(args, 2), argnames);
		}
		else
			colorspace = "None";

		RGB32Video = avs.GetEnv()->Invoke("Cache", script).AsClip();
		vi = RGB32Video->GetVideoInfo();
		fps = (double)vi.fps_numerator / vi.fps_denominator;
	}
	catch (AvisynthError const& err) {
		throw VideoOpenError("Avisynth error: " + std::string(err.msg));
	}
}

AvisynthVideoProvider::~AvisynthVideoProvider() {
	iframe.Clear();
}

AVSValue AvisynthVideoProvider::Open(wxFileName const& fname, wxString const& extension) {
	IScriptEnvironment *env = avs.GetEnv();
	char *videoFilename = env->SaveString(fname.GetShortPath().mb_str(csConvLocal));

	// Avisynth file, just import it
	if (extension == ".avs") {
		LOG_I("avisynth/video") << "Opening .avs file with Import";
		decoderName = "Avisynth/Import";
		return env->Invoke("Import", videoFilename);
	}

	// Open avi file with AviSource
	if (extension == ".avi") {
		LOG_I("avisynth/video") << "Opening .avi file with AviSource";
		try {
			const char *argnames[2] = { 0, "audio" };
			AVSValue args[2] = { videoFilename, false };
			decoderName = "Avisynth/AviSource";
			return env->Invoke("AviSource", AVSValue(args,2), argnames);
		}
		// On Failure, fallback to DSS
		catch (AvisynthError &err) {
			LOG_E("avisynth/video") << err.msg;
			LOG_I("avisynth/video") << "Failed to open .avi file with AviSource, trying DirectShowSource";
		}
	}

	// Open d2v with mpeg2dec3
	if (extension == ".d2v" && env->FunctionExists("Mpeg2Dec3_Mpeg2Source")) {
		LOG_I("avisynth/video") << "Opening .d2v file with Mpeg2Dec3_Mpeg2Source";
		AVSValue script = env->Invoke("Mpeg2Dec3_Mpeg2Source", videoFilename);
		decoderName = "Avisynth/Mpeg2Dec3_Mpeg2Source";

		//if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
		if (env->FunctionExists("SetPlanarLegacyAlignment")) {
			AVSValue args[2] = { script, true };
			script = env->Invoke("SetPlanarLegacyAlignment", AVSValue(args,2));
		}
		return script;
	}

	// If that fails, try opening it with DGDecode
	if (extension == ".d2v" && env->FunctionExists("DGDecode_Mpeg2Source")) {
		LOG_I("avisynth/video") << "Opening .d2v file with DGDecode_Mpeg2Source";
		decoderName = "DGDecode_Mpeg2Source";
		return env->Invoke("Avisynth/Mpeg2Source", videoFilename);

		//note that DGDecode will also have issues like if the version is too
		// ancient but no sane person would use that anyway
	}

	if (extension == ".d2v" && env->FunctionExists("Mpeg2Source")) {
		LOG_I("avisynth/video") << "Opening .d2v file with other Mpeg2Source";
		AVSValue script = env->Invoke("Mpeg2Source", videoFilename);
		decoderName = "Avisynth/Mpeg2Source";

		//if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
		if (env->FunctionExists("SetPlanarLegacyAlignment"))
			script = env->Invoke("SetPlanarLegacyAlignment", script);

		return script;
	}

	// Try loading DirectShowSource2
	if (!env->FunctionExists("dss2")) {
		wxFileName dss2path(StandardPaths::DecodePath("?data/avss.dll"));
		if (dss2path.FileExists()) {
			env->Invoke("LoadPlugin", env->SaveString(dss2path.GetFullPath().mb_str(csConvLocal)));
		}
	}

	// If DSS2 loaded properly, try using it
	if (env->FunctionExists("dss2")) {
		LOG_I("avisynth/video") << "Opening file with DSS2";
		decoderName = "Avisynth/DSS2";
		return env->Invoke("DSS2", videoFilename);
	}

	// Try DirectShowSource
	// Load DirectShowSource.dll from app dir if it exists
	wxFileName dsspath(StandardPaths::DecodePath("?data/DirectShowSource.dll"));
	if (dsspath.FileExists()) {
		env->Invoke("LoadPlugin",env->SaveString(dsspath.GetFullPath().mb_str(csConvLocal)));
	}

	// Then try using DSS
	if (env->FunctionExists("DirectShowSource")) {
		const char *argnames[3] = { 0, "video", "audio" };
		AVSValue args[3] = { videoFilename, true, false };
		decoderName = "Avisynth/DirectShowSource";
		warning = "Warning! The file is being opened using Avisynth's DirectShowSource, which has unreliable seeking. Frame numbers might not match the real number. PROCEED AT YOUR OWN RISK!";
		LOG_I("avisynth/video") << "Opening file with DirectShowSource";
		return env->Invoke("DirectShowSource", AVSValue(args,3), argnames);
	}

	// Failed to find a suitable function
	LOG_E("avisynth/video") << "DSS function not found";
	throw VideoNotSupported("No function suitable for opening the video found");
}

const AegiVideoFrame AvisynthVideoProvider::GetFrame(int n) {
	if (n == last_fnum) return iframe;

	wxMutexLocker lock(avs.GetMutex());

	PVideoFrame frame = RGB32Video->GetFrame(n, avs.GetEnv());
	iframe.pitch = frame->GetPitch();
	iframe.w = frame->GetRowSize() / (vi.BitsPerPixel() / 8);
	iframe.h = frame->GetHeight();

	iframe.Allocate();

	memcpy(iframe.data, frame->GetReadPtr(), iframe.pitch * iframe.h);

	last_fnum = n;
	return iframe;
}
#endif // HAVE_AVISYNTH
