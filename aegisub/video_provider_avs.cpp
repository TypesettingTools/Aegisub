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

#include <wx/filename.h>
#include <wx/msw/registry.h>
#include <wx/filename.h>
#include "video_provider_avs.h"
#include "options.h"
#include "main.h"
#include "vfr.h"


#ifdef __WINDOWS__


///////////////
// Constructor
AvisynthVideoProvider::AvisynthVideoProvider(wxString _filename, wxString _subfilename, double _fps) {
	AVSTRACE(wxString::Format(_T("AvisynthVideoProvider: Creating new AvisynthVideoProvider: \"%s\", \"%s\""), _filename, _subfilename));
	bool mpeg2dec3_priority = true;
	RGB32Video = NULL;
	SubtitledVideo = NULL;
	ResizedVideo = NULL;
	data = NULL;
	fps = _fps;

	depth = 0;
	
	last_fnum = -1;
	num_frames = 0;

	subfilename = _subfilename;
	zoom = 1.0;

	AVSTRACE(_T("AvisynthVideoProvider: Loading Subtitles Renderer"));
	LoadRenderer();
	AVSTRACE(_T("AvisynthVideoProvider: Subtitles Renderer loaded"));

	AVSTRACE(_T("AvisynthVideoProvider: Opening video"));
	RGB32Video = OpenVideo(_filename,mpeg2dec3_priority);
	AVSTRACE(_T("AvisynthVideoProvider: Video opened"));

	dar = GetSourceWidth()/(double)GetSourceHeight();
	AVSTRACE(_T("AvisynthVideoProvider: Calculated aspect ratio"));

	if( _subfilename.IsEmpty() ) SubtitledVideo = RGB32Video;
	else SubtitledVideo = ApplySubtitles(subfilename, RGB32Video);
	AVSTRACE(_T("AvisynthVideoProvider: Applied subtitles"));

	ResizedVideo = ApplyDARZoom(zoom, dar, SubtitledVideo);
	AVSTRACE(_T("AvisynthVideoProvider: Applied zoom"));

	vi = ResizedVideo->GetVideoInfo();
	AVSTRACE(_T("AvisynthVideoProvider: Got video info"));
	AVSTRACE(_T("AvisynthVideoProvider: Done creating AvisynthVideoProvider"));
}


//////////////
// Destructor
AvisynthVideoProvider::~AvisynthVideoProvider() {
	AVSTRACE(_T("AvisynthVideoProvider: Destroying AvisynthVideoProvider"));
	RGB32Video = NULL;
	SubtitledVideo = NULL;
	ResizedVideo = NULL;
	if( data ) delete data;
	AVSTRACE(_T("AvisynthVideoProvider: AvisynthVideoProvider destroyed"));
}


/////////////////////
// Refresh subtitles
void AvisynthVideoProvider::RefreshSubtitles() {
	AVSTRACE(_T("AvisynthVideoProvider::RefreshSubtitles: Refreshing subtitles"));
	ResizedVideo = NULL;
	SubtitledVideo = NULL;

	SubtitledVideo = ApplySubtitles(subfilename, RGB32Video);
	ResizedVideo = ApplyDARZoom(zoom,dar,SubtitledVideo);
	GetFrame(last_fnum,true);
	AVSTRACE(_T("AvisynthVideoProvider::RefreshSubtitles: Subtitles refreshed"));
}


////////////////////////////
// Set Display Aspect Ratio
void AvisynthVideoProvider::SetDAR(double _dar) {
	AVSTRACE(_T("AvisynthVideoProvider::SetDAR: Setting DAR"));
	dar = _dar;
	ResizedVideo = NULL;
	
	delete data;
	data = NULL;

	ResizedVideo = ApplyDARZoom(zoom,dar,SubtitledVideo);
	GetFrame(last_fnum,true);
	AVSTRACE(_T("AvisynthVideoProvider::SetDAR: DAR set"));
}


////////////
// Set Zoom
void AvisynthVideoProvider::SetZoom(double _zoom) {
	AVSTRACE(_T("AvisynthVideoProvider::SetZoom: Setting zoom"));
	zoom = _zoom;
	ResizedVideo = NULL;

	delete data;
	data = NULL;

	ResizedVideo = ApplyDARZoom(zoom,dar,SubtitledVideo);
	GetFrame(last_fnum,true);
	AVSTRACE(_T("AvisynthVideoProvider::SetZoom: Zoom set"));
}


/////////////////////////////////////////
// Actually open the video into Avisynth
PClip AvisynthVideoProvider::OpenVideo(wxString _filename, bool mpeg2dec3_priority) {
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening video"));
	wxMutexLocker lock(AviSynthMutex);
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Got AVS mutex"));
	AVSValue script;

	bool usedDirectshow = false;

	wxString extension = _filename.Right(4);
	extension.LowerCase();

	try {
		// Prepare filename
		//char *videoFilename = env->SaveString(_filename.mb_str(wxConvLocal));
		wxFileName fname(_filename);
		char *videoFilename = env->SaveString(fname.GetShortPath().mb_str(wxConvLocal));

		// Avisynth file, just import it
		if (extension == _T(".avs")) { 
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .avs file with Import"));
			script = env->Invoke("Import", videoFilename);
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Finished"));
		}

		// Open avi file with AviSource
		else if (extension == _T(".avi")) {
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .avi file with AviSource"));
			try {
				const char *argnames[2] = { 0, "audio" };
				AVSValue args[2] = { videoFilename, false };
				script = env->Invoke("AviSource", AVSValue(args,2), argnames);
				AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Successfully opened .avi file without audio"));
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

            //note that DGDecode will also have issues like if the version is too ancient but no sane person
            //would use that anyway
		}

		else if (extension == _T(".d2v") && env->FunctionExists("Mpeg2Source")) {
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening .d2v file with other Mpeg2Source"));
			script = env->Invoke("Mpeg2Source", videoFilename);

            //if avisynth is 2.5.7 beta 2 or newer old mpeg2decs will crash without this
        	if (env->FunctionExists("SetPlanarLegacyAlignment"))
        		script = env->Invoke("SetPlanarLegacyAlignment", script);
		}

		// Some other format, such as mkv, mp4, ogm... try DirectShowSource
		else {
			directshowOpen:
			AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Opening file with DirectShowSource"));

			// Try loading DirectShowSource2
			bool dss2 = false;
			if (env->FunctionExists("dss2")) dss2 = true;
			if (!dss2) {
				wxFileName dss2path(AegisubApp::folderName + _T("avss.dll"));
				if (dss2path.FileExists()) {
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loading DirectShowSource2"));
					env->Invoke("LoadPlugin",env->SaveString(dss2path.GetFullPath().mb_str(wxConvLocal)));
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Loaded DirectShowSource2"));
				}
			}

			// If DSS2 loaded properly, try using it
			dss2 = false;
			if (env->FunctionExists("dss2")) {
				AVSTRACE(_T("visynthVideoProvider::OpenVideo: Invoking DSS2"));
				if (fps == 0.0) script = env->Invoke("DSS2", videoFilename);
				else {
					const char *argnames[2] = { 0, "fps" };
					AVSValue args[2] = { videoFilename, fps };
					script = env->Invoke("DSS2", AVSValue(args,2), argnames);
				}
				AVSTRACE(_T("visynthVideoProvider::OpenVideo: Successfully opened file with DSS2"));
				dss2 = true;
			}

			// Try DirectShowSource
			if (!dss2) {
				if (env->FunctionExists("DirectShowSource")) {
					if (fps == 0.0) {
						const char *argnames[3] = { 0, "video", "audio" };
						AVSValue args[3] = { videoFilename, true, false };
						script = env->Invoke("DirectShowSource", AVSValue(args,3), argnames);
					}
					else {
						const char *argnames[4] = { 0, "video", "audio" , "fps" };
						AVSValue args[4] = { videoFilename, true, false , fps };
						script = env->Invoke("DirectShowSource", AVSValue(args,4), argnames);
					}
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Successfully opened file with DSS without audio"));
					usedDirectshow = true;
				}

				// Failed to find a suitable function
				else {
					AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: DSS function not found"));
					throw AvisynthError("No function suitable for opening the video found");
				}
			}
		}
	}
	
	// Catch errors
	catch (AvisynthError &err) {
		AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Avisynth error: ") + wxString(err.msg,wxConvLocal));
		throw _T("AviSynth error: ") + wxString(err.msg,wxConvLocal);
	}

	// Check if video was loaded properly
	if (!script.AsClip()->GetVideoInfo().HasVideo()) {
		AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: No suitable video found"));
		throw _T("No usable video found in ") + _filename;
	}

	// Convert to RGB32
	script = env->Invoke("ConvertToRGB32", script);
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Converted to RGB32"));

	// Directshow
	//if (usedDirectshow) wxMessageBox(_T("Warning! The file is being opened using Avisynth's DirectShowSource, which has unreliable seeking. Frame numbers might not match the real number. PROCEED AT YOUR OWN RISK!"),_T("DirectShowSource warning"),wxICON_EXCLAMATION);

	// Cache
	AVSTRACE(_T("AvisynthVideoProvider::OpenVideo: Finished opening video, AVS mutex will be released now"));
	return (env->Invoke("Cache", script)).AsClip();
}


////////////////////////////////////////////////////////
// Apply VSFilter subtitles, or whatever is appropriate
PClip AvisynthVideoProvider::ApplySubtitles(wxString _filename, PClip videosource) {
	AVSTRACE(_T("AvisynthVideoProvider::ApplySutitles: Applying subtitles"));
	wxMutexLocker lock(AviSynthMutex);
	AVSTRACE(_T("AvisynthVideoProvider::ApplySutitles: Got AVS mutex"));

	// Insert subs
	AVSValue script;
	char temp[512];
	wxFileName fname(_filename);
	strcpy(temp,fname.GetShortPath().mb_str(wxConvLocal));
	AVSValue args[2] = { videosource, temp };

	try {
		AVSTRACE(_T("AvisynthVideoProvider::ApplySutitles: Now invoking ") + rendererCallString);
		script = env->Invoke(rendererCallString.mb_str(wxConvUTF8), AVSValue(args,2));
		AVSTRACE(_T("AvisynthVideoProvider::ApplySutitles: Invoked successfully"));
	}
	catch (AvisynthError &err) {
		AVSTRACE(_T("AvisynthVideoProvider::ApplySutitles: Avisynth error: ") + wxString(err.msg,wxConvLocal));
		throw _T("AviSynth error: ") + wxString(err.msg,wxConvLocal);
	}

	// Cache
	AVSTRACE(_T("AvisynthVideoProvider::ApplySutitles: Subtitles applied, AVS mutex will be released now"));
	return (env->Invoke("Cache", script)).AsClip();
}


/////////////////////////////////////
// Apply Display Aspect Ratio + Zoom
PClip AvisynthVideoProvider::ApplyDARZoom(double _zoom, double _dar, PClip videosource) {
	AVSTRACE(_T("AvisynthVideoProvider::ApplyDARZoom: Applying DAR zoom"));
	wxMutexLocker lock(AviSynthMutex);
	AVSTRACE(_T("AvisynthVideoProvider::ApplyDARZoom: Got AVS mutex"));

	AVSValue script;
	VideoInfo vil = videosource->GetVideoInfo();

	int w = vil.height * _zoom * _dar;
	int h = vil.height * _zoom;
	if (w == vil.width && h == vil.height) {
		vi = vil;
		return (env->Invoke("Cache",videosource)).AsClip();
	}

	try {
		// Resize
		if (!env->FunctionExists(Options.AsText(_T("Video resizer")).mb_str(wxConvLocal)))
			throw AvisynthError("Selected resizer doesn't exist");

		AVSValue args[3] = { videosource, w, h };
		AVSTRACE(_T("AvisynthVideoProvider::ApplyDARZoom: Invoking resizing function"));
		script = env->Invoke(Options.AsText(_T("Video resizer")).mb_str(wxConvLocal), AVSValue(args,3));
		AVSTRACE(_T("AvisynthVideoProvider::ApplyDARZoom: Resizer invoked successfully"));
	} catch (AvisynthError &err) {
		AVSTRACE(_T("AvisynthVideoProvider::ApplyDARZoom: Avisynth error: ") + wxString(err.msg,wxConvLocal));
		throw _T("AviSynth error: ") + wxString(err.msg,wxConvLocal);
	}

	vi = script.AsClip()->GetVideoInfo();

	AVSTRACE(_T("AvisynthVideoProvider::ApplyDARZoom: DAR zoom applied successfully, AVS mutex will be released now"));
	return (env->Invoke("Cache",script)).AsClip();
}


////////////////////////
// Actually get a frame
wxBitmap AvisynthVideoProvider::GetFrame(int _n, bool force) {
	// Transform n if overriden
	int n = _n;
	if (frameTime.Count()) {
		if (n < 0) n = 0;
		if (n >= (signed) frameTime.Count()) n = frameTime.Count()-1;
		int time = frameTime[n];
		double curFps = (double)vi.fps_numerator/(double)vi.fps_denominator;
		n = time * curFps / 1000.0;
	}

	// Get frame
	AVSTRACE(_T("AvisynthVideoProvider::GetFrame"));
	if (n != last_fnum || force) {
		wxMutexLocker lock(AviSynthMutex);

		PVideoFrame frame = ResizedVideo->GetFrame(n,env);

		int ndepth = wxDisplayDepth();

		if (depth != ndepth) {
			depth = ndepth;
			delete data;
			data = NULL;
		}

		if (!data)
			data = new unsigned char[vi.width*vi.height*depth/8];

		unsigned char* dst = data+(vi.width*(vi.height-1)*depth/8);

		if (depth == 32) {
			int rs = vi.RowSize();
			const unsigned char* src = frame->GetReadPtr();
			int srcpitch = frame->GetPitch();

			for (int y = 0; y < vi.height; y++) {
				memcpy(dst,src,rs);
				src+=srcpitch;
				dst-=rs;
			}
		}

		else if (depth == 24) {
			//fail
		}

		else if (depth == 16) {
			const unsigned char *read_ptr = frame->GetReadPtr();
			unsigned short *write_ptr = (unsigned short*) dst;
			unsigned char r,g,b;
			int srcpitch = frame->GetPitch();
			int rs = vi.RowSize();

			for (int y = 0; y < vi.height; y++) {

				for (int x=0,dx=0;x<rs;x+=4,dx++) {
					r = read_ptr[x+2];
					g = read_ptr[x+1];
					b = read_ptr[x];
					write_ptr[dx] = ((r>>3)<<11) | ((g>>2)<<5) | b>>3;
				}

				write_ptr -= vi.width;
				read_ptr += srcpitch;
			}
		}
		
		else {
			//fail
		}

		last_frame = wxBitmap((const char*)data, vi.width, vi.height, depth);
		last_fnum = n;
	}

	return wxBitmap(last_frame);
}


///////////////////////////////////
// Get a frame intensity as floats
void AvisynthVideoProvider::GetFloatFrame(float* Buffer, int n) {
	AVSTRACE(_T("AvisynthVideoProvider::GetFloatFrame"));
	wxMutexLocker lock(AviSynthMutex);

	PVideoFrame frame = ResizedVideo->GetFrame(n,env);

	int rs = vi.RowSize();
	const unsigned char* src = frame->GetReadPtr();
	int srcpitch = frame->GetPitch();

	for( int i = 0; i < vi.height; i++ ) 
	{
		for( int x=0; x<vi.width;x++ )
		{
			Buffer[(vi.height-i-1)*vi.width+x] = src[x*4+0]*0.3 + src[x*4+1]*0.4 + src[x*4+2]*0.3;
		}
		src+=srcpitch;
	}
}


/////////////////////////////
// Load appropriate renderer
void AvisynthVideoProvider::LoadRenderer() {
	// Get prefferred
	wxString prefferred = Options.AsText(_T("Avisynth subs renderer"));

	// Load
	if (prefferred.Lower() == _T("asa")) LoadASA();
	else LoadVSFilter();
}


/////////////////
// Load VSFilter
void AvisynthVideoProvider::LoadVSFilter() {
	AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Loading VSFilter"));
	// Loading an avisynth plugin multiple times does almost nothing

	wxFileName vsfilterPath(AegisubApp::folderName + _T("vsfilter.dll"));
	rendererCallString = _T("TextSub");

	if (vsfilterPath.FileExists()) {
		AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Invoking LoadPlugin"));
		env->Invoke("LoadPlugin",env->SaveString(vsfilterPath.GetFullPath().mb_str(wxConvLocal)));
		AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Loaded"));
	}
	else {
		AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: VSFilter.dll not found in Aegisub dir, trying to locate registered DShow filter"));
		wxRegKey reg(_T("HKEY_CLASSES_ROOT\\CLSID\\{9852A670-F845-491B-9BE6-EBD841B8A613}\\InprocServer32"));
		if (reg.Exists()) {
			wxString fn;
			reg.QueryValue(_T(""),fn);
			
			vsfilterPath = fn;

			if (vsfilterPath.FileExists()) {
				AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Found as DShow filter, loading"));
				env->Invoke("LoadPlugin",env->SaveString(vsfilterPath.GetFullPath().mb_str(wxConvLocal)));
				AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Loaded"));
				return;
			}
			
			vsfilterPath = _T("vsfilter.dll");
		}
		else if (vsfilterPath.FileExists()) {
			AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Found on system path, loading"));
			env->Invoke("LoadPlugin",env->SaveString(vsfilterPath.GetFullPath().mb_str(wxConvLocal)));
			AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Loaded"));
		}
		else if (!env->FunctionExists("TextSub")) {
			AVSTRACE(_T("AvisynthVideoProvider::LoadVSFilter: Couldn't locate VSFilter"));
			throw _T("Couldn't locate VSFilter");
		}
	}
}


////////////
// Load asa
void AvisynthVideoProvider::LoadASA() {
	AVSTRACE(_T("AvisynthVideoProvider::LoadASA: Loading asa"));
	// Loading an avisynth plugin multiple times does almost nothing

	wxFileName asaPath(AegisubApp::folderName + _T("asa.dll"));
	rendererCallString = _T("asa");

	if (asaPath.FileExists()) {
		AVSTRACE(_T("AvisynthVideoProvider::LoadASA: Invoking LoadPlugin"));
		env->Invoke("LoadPlugin",env->SaveString(asaPath.GetFullPath().mb_str(wxConvLocal)));
		AVSTRACE(_T("AvisynthVideoProvider::LoadASA: Loaded"));
	}
	else {
		asaPath = _T("asa.dll");
		if (asaPath.FileExists()) {
			AVSTRACE(_T("AvisynthVideoProvider::LoadASA: Invoking LoadPlugin"));
			env->Invoke("LoadPlugin",env->SaveString(asaPath.GetFullPath().mb_str(wxConvLocal)));
			AVSTRACE(_T("AvisynthVideoProvider::LoadASA: Loaded"));
		}
		else if (!env->FunctionExists("asa")) {
			AVSTRACE(_T("AvisynthVideoProvider::LoadASA: Couldn't locate asa"));
			throw _T("Couldn't locate asa");
		}
	}
}


////////////////////////
// Override frame times
void AvisynthVideoProvider::OverrideFrameTimeList(wxArrayInt list) {
	frameTime = list;
	num_frames = frameTime.Count();
}


#endif
