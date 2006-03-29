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
#include "video_provider_avs.h"
#include "options.h"
#include "main.h"


#ifdef __WINDOWS__


AvisynthVideoProvider::AvisynthVideoProvider(wxString _filename, wxString _subfilename) {
	bool mpeg2dec3_priority = true;
	RGB32Video = NULL;
	SubtitledVideo = NULL;
	ResizedVideo = NULL;
	data = NULL;

	depth = 0;
	
	last_fnum = -1;

	subfilename = _subfilename;
	zoom = 1.0;

	LoadVSFilter();

	RGB32Video = OpenVideo(_filename,mpeg2dec3_priority);

	dar = GetSourceWidth()/(double)GetSourceHeight();

	if( _subfilename.IsEmpty() ) SubtitledVideo = RGB32Video;
	else SubtitledVideo = ApplySubtitles(subfilename, RGB32Video);

	ResizedVideo = ApplyDARZoom(zoom, dar, SubtitledVideo);

	vi = ResizedVideo->GetVideoInfo();
}

AvisynthVideoProvider::~AvisynthVideoProvider() {
	RGB32Video = NULL;
	SubtitledVideo = NULL;
	ResizedVideo = NULL;
	if( data ) delete data;
}

void AvisynthVideoProvider::RefreshSubtitles() {
	ResizedVideo = NULL;
	SubtitledVideo = NULL;

	SubtitledVideo = ApplySubtitles(subfilename, RGB32Video);
	ResizedVideo = ApplyDARZoom(zoom,dar,SubtitledVideo);
	GetFrame(last_fnum,true);
}

void AvisynthVideoProvider::SetDAR(double _dar) {
	dar = _dar;
	ResizedVideo = NULL;
	
	delete data;
	data = NULL;

	ResizedVideo = ApplyDARZoom(zoom,dar,SubtitledVideo);
	GetFrame(last_fnum,true);
}

void AvisynthVideoProvider::SetZoom(double _zoom) {
	zoom = _zoom;
	ResizedVideo = NULL;

	delete data;
	data = NULL;

	ResizedVideo = ApplyDARZoom(zoom,dar,SubtitledVideo);
	GetFrame(last_fnum,true);
}

PClip AvisynthVideoProvider::OpenVideo(wxString _filename, bool mpeg2dec3_priority) {
	wxMutexLocker lock(AviSynthMutex);
	AVSValue script;

	bool usedDirectshow = false;

	wxString extension = _filename.Right(4);
	extension.LowerCase();

	try {
		// Prepare filename
		char *videoFilename = env->SaveString(_filename.mb_str(wxConvLocal));

		// Load depending on extension
		if (extension == _T(".avs")) { 
			script = env->Invoke("Import", videoFilename);
		} else if (extension == _T(".avi")) {
			try {
				const char *argnames[2] = { 0, "audio" };
				AVSValue args[2] = { videoFilename, false };
				script = env->Invoke("AviSource", AVSValue(args,2), argnames);
			} catch (AvisynthError &) {
				goto directshowOpen;
			}
		}
		else if (extension == _T(".d2v") && env->FunctionExists("mpeg2dec3_Mpeg2Source") && mpeg2dec3_priority) //prefer mpeg2dec3 
			script = env->Invoke("mpeg2dec3_Mpeg2Source", videoFilename);
		else if (extension == _T(".d2v") && env->FunctionExists("Mpeg2Source")) //try other mpeg2source
			script = env->Invoke("Mpeg2Source", videoFilename);
		else {
			directshowOpen:

			if (env->FunctionExists("DirectShowSource")) {
				const char *argnames[3] = { 0, "video", "audio" };
				AVSValue args[3] = { videoFilename, true, false };
				script = env->Invoke("DirectShowSource", AVSValue(args,3), argnames);
				usedDirectshow = true;
			} else 
				throw AvisynthError("No function suitable for opening the video found");
		} 
	} catch (AvisynthError &err) {
		throw _T("AviSynth error: ") + wxString(err.msg,wxConvLocal);
	}

	
	if (!script.AsClip()->GetVideoInfo().HasVideo())
		throw _T("No usable video found in ") + _filename;

	// Convert to RGB32
	script = env->Invoke("ConvertToRGB32", script);

	// Directshow
	if (usedDirectshow) wxMessageBox(_T("Warning! The file is being opened using Avisynth's DirectShowSource, which has unreliable seeking. Frame numbers might not match the real number. PROCEED AT YOUR OWN RISK!"),_T("DirectShowSource warning"),wxICON_EXCLAMATION);

	// Cache
	return (env->Invoke("Cache", script)).AsClip();
}

PClip AvisynthVideoProvider::ApplySubtitles(wxString _filename, PClip videosource) {
	wxMutexLocker lock(AviSynthMutex);

	// Insert subs
	AVSValue script;
	char temp[512];
	strcpy(temp,_filename.mb_str(wxConvLocal));
	AVSValue args[2] = { videosource, temp };

	try {
		script = env->Invoke("TextSub", AVSValue(args,2));
	} catch (AvisynthError &err) {
		throw _T("AviSynth error: ") + wxString(err.msg,wxConvLocal);
	}

	// Cache
	return (env->Invoke("Cache", script)).AsClip();
}

PClip AvisynthVideoProvider::ApplyDARZoom(double _zoom, double _dar, PClip videosource) {
	wxMutexLocker lock(AviSynthMutex);

	AVSValue script;
	VideoInfo vil = videosource->GetVideoInfo();

	int w = vil.height * _zoom * _dar;
	int h = vil.height * _zoom;

	try {
		// Resize
		if (!env->FunctionExists(Options.AsText(_T("Video resizer")).mb_str(wxConvLocal)))
			throw AvisynthError("Selected resizer doesn't exist");

		AVSValue args[3] = { videosource, w, h };
		script = env->Invoke(Options.AsText(_T("Video resizer")).mb_str(wxConvLocal), AVSValue(args,3));
	} catch (AvisynthError &err) {
		throw _T("AviSynth error: ") + wxString(err.msg,wxConvLocal);
	}

	vi = script.AsClip()->GetVideoInfo();

	return (env->Invoke("Cache",script)).AsClip();
}

wxBitmap AvisynthVideoProvider::GetFrame(int n, bool force) {
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

void AvisynthVideoProvider::GetFloatFrame(float* Buffer, int n) {
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

void AvisynthVideoProvider::LoadVSFilter() {
	// Loading an avisynth plugin multiple times does almost nothing

	wxFileName vsfilterPath(AegisubApp::folderName + _T("vsfilter.dll"));

	if (vsfilterPath.FileExists())
		env->Invoke("LoadPlugin",env->SaveString(vsfilterPath.GetFullPath().mb_str(wxConvLocal)));
	else {
		wxRegKey reg(_T("HKEY_CLASSES_ROOT\\CLSID\\{9852A670-F845-491B-9BE6-EBD841B8A613}\\InprocServer32"));
		if (reg.Exists()) {
			wxString fn;
			reg.QueryValue(_T(""),fn);
			
			vsfilterPath = fn;

			if (vsfilterPath.FileExists()) {
				env->Invoke("LoadPlugin",env->SaveString(vsfilterPath.GetFullPath().mb_str(wxConvLocal)));
				return;
			}
			
			vsfilterPath = _T("vsfilter.dll");
		} else if (vsfilterPath.FileExists()) 
			env->Invoke("LoadPlugin",env->SaveString(vsfilterPath.GetFullPath().mb_str(wxConvLocal)));
		else if (!env->FunctionExists("TextSub"))
			throw _T("Couldn't locate VSFilter");
	}
}


#endif
