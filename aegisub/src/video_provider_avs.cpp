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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"

#include <wx/wxprec.h>
#ifdef WITH_AVISYNTH
#include <wx/filename.h>
#include <wx/msw/registry.h>
#include <wx/filename.h>
#include "video_provider_avs.h"
#include "video_context.h"
#include "options.h"
#include "standard_paths.h"
#include "vfr.h"
#include "gl_wrap.h"
#include "mkv_wrap.h"
#include "vfw_wrap.h"
#include "charset_conv.h"


///////////////
// Constructor
AvisynthVideoProvider::AvisynthVideoProvider(Aegisub::String _filename) {
	AVSTRACE(wxString::Format(_T("AvisynthVideoProvider: Creating new AvisynthVideoProvider: \"%s\", \"%s\""), _filename, _subfilename));
	bool mpeg2dec3_priority = true;
	RGB32Video = NULL;
	fps = 0;
	num_frames = 0;
	last_fnum = -1;
	byFrame = false;
	KeyFrames.Clear();
	keyFramesLoaded = false;
	isVfr = false;

	AVSTRACE(_T("AvisynthVideoProvider: Opening video"));
	RGB32Video = OpenVideo(_filename,mpeg2dec3_priority);
	AVSTRACE(_T("AvisynthVideoProvider: Video opened"));

	vi = RGB32Video->GetVideoInfo();
	AVSTRACE(_T("AvisynthVideoProvider: Got video info"));
	AVSTRACE(_T("AvisynthVideoProvider: Done creating AvisynthVideoProvider"));
}


//////////////
// Destructor
AvisynthVideoProvider::~AvisynthVideoProvider() {
	AVSTRACE(_T("AvisynthVideoProvider: Destroying AvisynthVideoProvider"));
	RGB32Video = NULL;
	AVSTRACE(_T("AvisynthVideoProvider: Destroying frame"));
	iframe.Clear();
	AVSTRACE(_T("AvisynthVideoProvider: AvisynthVideoProvider destroyed"));
}



////////////////////////////////////// VIDEO PROVIDER //////////////////////////////////////


/////////////////////////////////////////
// Actually open the video into Avisynth
PClip AvisynthVideoProvider::OpenVideo(Aegisub::String _filename, bool mpeg2dec3_priority) {
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening video"));
	wxMutexLocker lock(AviSynthMutex);
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Got AVS mutex"));
	AVSValue script;

	byFrame = false;
	usedDirectShow = false;
	decoderName = _("Unknown");

	wxString extension = wxString(_filename.c_str()).Right(4);
	extension.LowerCase();

	try {
		// Prepare filename
		//char *videoFilename = env->SaveString(_filename.mb_str(wxConvLocal));
		wxFileName fname(_filename);
		char *videoFilename = env->SaveString(fname.GetShortPath().mb_str(csConvLocal));

		// Avisynth file, just import it
		if (extension == _T(".avs")) { 
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .avs file with Import"));
			script = env->Invoke("Import", videoFilename);
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Finished"));
			decoderName = _T("Import");
		}

		// Open avi file with AviSource
		else if (extension == _T(".avi")) {
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .avi file with AviSource"));
			try {
				const char *argnames[2] = { 0, "audio" };
				AVSValue args[2] = { videoFilename, false };
				script = env->Invoke("AviSource", AVSValue(args,2), argnames);
				AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Successfully opened .avi file without audio"));
				byFrame = true;
				decoderName = _T("AviSource");
			}
			
			// On Failure, fallback to DSS
			catch (AvisynthError &) {
				AVSTRACE(_T("Failed to open .avi file with AviSource, switching to DirectShowSource"));
				goto directshowOpen;
			}
		}

		// Open d2v with mpeg2dec3
		else if (extension == _T(".d2v") && env->FunctionExists("Mpeg2Dec3_Mpeg2Source") && mpeg2dec3_priority) {
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .d2v file with Mpeg2Dec3_Mpeg2Source"));
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
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .d2v file with DGDecode_Mpeg2Source"));
			script = env->Invoke("Mpeg2Source", videoFilename);
			decoderName = _T("DGDecode_Mpeg2Source");

            //note that DGDecode will also have issues like if the version is too ancient but no sane person
            //would use that anyway
		}

		else if (extension == _T(".d2v") && env->FunctionExists("Mpeg2Source")) {
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .d2v file with other Mpeg2Source"));
			script = env->Invoke("Mpeg2Source", videoFilename);
			decoderName = _T("Mpeg2Source");

            //if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
        	if (env->FunctionExists("SetPlanarLegacyAlignment"))
        		script = env->Invoke("SetPlanarLegacyAlignment", script);
		}

		// Some other format, such as mkv, mp4, ogm... try FFMpegSource and DirectShowSource
		else {
			// Try loading FFMpegSource
			directshowOpen:
			bool ffsource = false;
			if (env->FunctionExists("ffvideosource")) ffsource = true;
			if (!ffsource) {
				wxFileName ffsourcepath(StandardPaths::DecodePath(_T("?data/ffms2.dll")));
				if (ffsourcepath.FileExists()) {
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loading FFMpegSource2"));
					env->Invoke("LoadPlugin",env->SaveString(ffsourcepath.GetFullPath().mb_str(csConvLocal)));
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loaded FFMpegSource2"));
					byFrame = true;
				}
			}

			// If FFMpegSource loaded properly, try using it
			ffsource = false;
			if (env->FunctionExists("ffvideosource")) {
				AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Invoking FFMpegSource2"));
				const char *argnames[2] = { "source", "cache" };
				AVSValue args[2] = { videoFilename, false };
				script = env->Invoke("ffvideosource", AVSValue(args,2), argnames);
				//script = env->Invoke("ffmpegsource", videoFilename);
				AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Successfully opened file with FFMpegSource2"));
				ffsource = true;
				decoderName = _T("FFmpegSource2");
			}

			// DirectShowSource
			if (!ffsource) {
				AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening file with DirectShowSource"));

				// Try loading DirectShowSource2
				bool dss2 = false;
				if (env->FunctionExists("dss2")) dss2 = true;
				if (!dss2) {
					wxFileName dss2path(StandardPaths::DecodePath(_T("?data/avss.dll")));
					if (dss2path.FileExists()) {
						AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loading DirectShowSource2"));
						env->Invoke("LoadPlugin",env->SaveString(dss2path.GetFullPath().mb_str(csConvLocal)));
						AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loaded DirectShowSource2"));
					}
				}

				// If DSS2 loaded properly, try using it
				dss2 = false;
				if (env->FunctionExists("dss2")) {
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Invoking DSS2"));
					script = env->Invoke("DSS2", videoFilename);
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Successfully opened file with DSS2"));
					dss2 = true;
					decoderName = _T("DSS2");
				}

				// Try DirectShowSource
				if (!dss2) {
					// Load DirectShowSource.dll from app dir if it exists
					wxFileName dsspath(StandardPaths::DecodePath(_T("?data/DirectShowSource.dll")));
					if (dsspath.FileExists()) {
						AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loading DirectShowSource"));
						env->Invoke("LoadPlugin",env->SaveString(dsspath.GetFullPath().mb_str(csConvLocal)));
						AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loaded DirectShowSource"));
					}

					// Then try using DSS
					if (env->FunctionExists("DirectShowSource")) {
						const char *argnames[3] = { 0, "video", "audio" };
						AVSValue args[3] = { videoFilename, true, false };
						script = env->Invoke("DirectShowSource", AVSValue(args,3), argnames);
						AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Successfully opened file with DSS without audio"));
						usedDirectShow = true;
						decoderName = _T("DirectShowSource");
					}

					// Failed to find a suitable function
					else {
						AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: DSS function not found"));
						throw AvisynthError("No function suitable for opening the video found");
					}
				}
			}
		}
	}
	
	// Catch errors
	catch (AvisynthError &err) {
		AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Avisynth error: ") + wxString(err.msg,csConvLocal));
		throw _T("AviSynth error: ") + wxString(err.msg,csConvLocal);
	}

	// Check if video was loaded properly
	if (!script.IsClip() || !script.AsClip()->GetVideoInfo().HasVideo()) {
		AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: No suitable video found"));
		throw _T("Avisynth: No usable video found in ") + _filename;
	}

	// Read keyframes and timecodes from MKV file
	isVfr = false;
	FrameRate temp;
	double overFps = 0;
	bool mkvOpen = MatroskaWrapper::wrapper.IsOpen();
	KeyFrames.Clear();
	if (extension == _T(".mkv") || mkvOpen) {
		// Parse mkv
		if (!mkvOpen) MatroskaWrapper::wrapper.Open(_filename);
		
		// Get keyframes
		KeyFrames = MatroskaWrapper::wrapper.GetKeyFrames();
		keyFramesLoaded = true;
		
		// Ask to override timecodes
		int override = wxYES;
		if (VFR_Output.IsLoaded()) override = wxMessageBox(_("You already have timecodes loaded. Replace them with the timecodes from the Matroska file?"),_("Replace timecodes?"),wxYES_NO | wxICON_QUESTION);
		if (override == wxYES) {
			MatroskaWrapper::wrapper.SetToTimecodes(temp);
			isVfr = temp.GetFrameRateType() == VFR;
			if (isVfr) {
				overFps = temp.GetCommonFPS();
				MatroskaWrapper::wrapper.SetToTimecodes(VFR_Input);
				MatroskaWrapper::wrapper.SetToTimecodes(VFR_Output);
				trueFrameRate = temp;
			}
		}

		// Close mkv
		MatroskaWrapper::wrapper.Close();
	}
// check if we have windows, if so we can load keyframes from AVI files using VFW
#ifdef __WINDOWS__
	else if (extension == _T(".avi")) {
		keyFramesLoaded = false;
		KeyFrames.Clear();
		KeyFrames = VFWWrapper::GetKeyFrames(_filename);
		keyFramesLoaded = true;
	}
#endif /* __WINDOWS__ */

	// Check if the file is all keyframes
	bool isAllKeyFrames = true;
	for (unsigned int i=1; i<KeyFrames.GetCount(); i++) {
		// Is the last keyframe not this keyframe -1?
		if (KeyFrames[i-1] != (int)(i-1)) {
			// It's not all keyframes, go ahead
			isAllKeyFrames = false;
			break;
		}
	}

	// If it is all keyframes, discard the keyframe info as it is useless
	if (isAllKeyFrames) {
		KeyFrames.Clear();
		keyFramesLoaded = false;
	}

	// Convert to RGB32
	script = env->Invoke("ConvertToRGB32", script);
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Converted to RGB32"));

	// Cache
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Finished opening video, AVS mutex will be released now"));
	return (env->Invoke("Cache", script)).AsClip();
}


////////////////////////
// Actually get a frame
const AegiVideoFrame AvisynthVideoProvider::GetFrame(int _n) {
	// Transform n if overriden
	int n = _n;
	if (frameTime.Count()) {
		if (n < 0) n = 0;
		if (n >= (signed) frameTime.Count()) n = frameTime.Count()-1;
		int time = frameTime[n];
		double curFps = (double)vi.fps_numerator/(double)vi.fps_denominator;
		n = time * curFps / 1000.0;
	}

	// Get avs frame
	AVSTRACE(_T("AvisynthVideoProvider::GetFrame"));
	wxMutexLocker lock(AviSynthMutex);
	PVideoFrame frame = RGB32Video->GetFrame(n,env);
	int Bpp = vi.BitsPerPixel() / 8;

	// Aegisub's video frame
	AegiVideoFrame &final = iframe;
	final.flipped = false;
	final.cppAlloc = true;
	final.invertChannels = false;

	// Format
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

////////////////////////
// Override frame times
void AvisynthVideoProvider::OverrideFrameTimeList(wxArrayInt list) {
	frameTime = list;
	num_frames = frameTime.Count();
}


///////////////
// Get warning
Aegisub::String AvisynthVideoProvider::GetWarning() {
	if (usedDirectShow) return L"Warning! The file is being opened using Avisynth's DirectShowSource, which has unreliable seeking. Frame numbers might not match the real number. PROCEED AT YOUR OWN RISK!";
	else return L"";
}

#endif
