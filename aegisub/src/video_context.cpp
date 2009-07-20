// Copyright (c) 2005-2007, Rodrigo Braz Monteiro
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


////////////
// Includes
#include "config.h"

#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <wx/image.h>
#include <string.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/config.h>
#include "utils.h"
#include "video_display.h"
#include "video_context.h"
#include "video_provider_manager.h"
#include "visual_tool.h"
#include "subtitles_provider_manager.h"
#include "vfr.h"
#include "ass_file.h"
#include "ass_exporter.h"
#include "ass_time.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "subs_grid.h"
#include "options.h"
#include "subs_edit_box.h"
#include "audio_display.h"
#include "video_slider.h"
#include "video_box.h"
#include "utils.h"
#include "gl_wrap.h"
#include "standard_paths.h"


///////
// IDs
enum {
	VIDEO_PLAY_TIMER = 1300
};


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoContext, wxEvtHandler)
	EVT_TIMER(VIDEO_PLAY_TIMER,VideoContext::OnPlayTimer)
END_EVENT_TABLE()


////////////
// Instance
VideoContext *VideoContext::instance = NULL;


///////////////
// Constructor
VideoContext::VideoContext() {
	// Set GL context
	glContext = NULL;
	ownGlContext = false;
	lastTex = 0;
	lastFrame = -1;

	// Set options
	audio = NULL;
	provider = NULL;
	subsProvider = NULL;
	curLine = NULL;
	loaded = false;
	keyFramesLoaded = false;
	overKeyFramesLoaded = false;
	frame_n = 0;
	length = 0;
	fps = 0;
	arType = 0;
	arValue = 1.0;
	isPlaying = false;
	nextFrame = -1;
	keepAudioSync = true;

	// Threads
	//threaded = Options.AsBool(_T("Threaded Video"));
	threaded = false;
	threadLocked = false;
	threadNextFrame = -1;
}


//////////////
// Destructor
VideoContext::~VideoContext () {
	Reset();
	if (ownGlContext)
		delete glContext;
	glContext = NULL;
}


////////////////
// Get Instance
VideoContext *VideoContext::Get() {
	if (!instance) {
		instance = new VideoContext;
	}
	return instance;
}


/////////
// Clear
void VideoContext::Clear() {
	instance->audio = NULL;
	delete instance;
	instance = NULL;
}


/////////
// Reset
void VideoContext::Reset() {
	// Reset ?video path
	StandardPaths::SetPathValue(_T("?video"),_T(""));

	// Clear keyframes
	KeyFrames.Clear();
	keyFramesLoaded = false;

	// Remove temporary audio provider
	if (audio && audio->temporary) {
		delete audio->provider;
		audio->provider = NULL;
		delete audio->player;
		audio->player = NULL;
		audio->temporary = false;
	}

	// Remove video data
	loaded = false;
	frame_n = 0;
	length = 0;
	fps = 0;
	keyFramesLoaded = false;
	overKeyFramesLoaded = false;
	isPlaying = false;
	nextFrame = -1;
	curLine = NULL;

	// Update displays
	UpdateDisplays(true);

	// Remove textures
	UnloadTexture();

	// Clean up video data
	wxRemoveFile(tempfile);
	tempfile = _T("");
	videoName = _T("");
	tempFrame.Clear();

	// Remove provider
	if (provider) {
		delete provider;
		provider = NULL;
	}
	delete subsProvider;
	subsProvider = NULL;
}


////////////////
// Reload video
void VideoContext::Reload() {
	if (IsLoaded()) {
		wxString name = videoName;
		int n = frame_n;
		SetVideo(_T(""));
		SetVideo(name);
		JumpToFrame(n);
	}
}


//////////////////
// Unload texture
void VideoContext::UnloadTexture() {
	// Remove textures
	if (lastTex != 0) {
		glDeleteTextures(1,&lastTex);
		lastTex = 0;
	}
	lastFrame = -1;
}


///////////////////////
// Sets video filename
void VideoContext::SetVideo(const wxString &filename) {
	// Unload video
	Reset();
	threaded = Options.AsBool(_T("Threaded Video"));
	
	// Load video
	if (!filename.IsEmpty()) {
		try {
			grid->CommitChanges(true);

			// Set GL context
#ifdef __WXMAC__
			GetGLContext(displayList.front())->SetCurrent();
#else
			GetGLContext(displayList.front())->SetCurrent(*displayList.front());
#endif

			// Choose a provider
			provider = VideoProviderFactoryManager::GetProvider(filename);
			loaded = provider != NULL;

			// Get subtitles provider
			try {
				subsProvider = SubtitlesProviderFactoryManager::GetProvider();
			}
			catch (wxString err) { wxMessageBox(_T("Error while loading subtitles provider: ") + err,_T("Subtitles provider"));	}
			catch (const wchar_t *err) { wxMessageBox(_T("Error while loading subtitles provider: ") + wxString(err),_T("Subtitles provider"));	}

			KeyFrames.Clear();
			// load keyframes if available
			if (provider->AreKeyFramesLoaded()) {
				KeyFrames = provider->GetKeyFrames();
				keyFramesLoaded = true;
			}
			else {
				keyFramesLoaded = false;
			}

			bool isVfr = provider->IsVFR();
			
			// Set frame rate
			fps = provider->GetFPS();
			// if the source is vfr and the provider isn't frame-based (i.e. is dshow),
			// we need to jump through some hoops to make VFR work properly.
			if (!provider->IsNativelyByFrames() && isVfr) {
				FrameRate temp = provider->GetTrueFrameRate();
				provider->OverrideFrameTimeList(temp.GetFrameTimeList());
			}
			// source not VFR? set as CFR
			else if (!isVfr) {
				VFR_Input.SetCFR(fps);
				if (VFR_Output.GetFrameRateType() != VFR) VFR_Output.SetCFR(fps);
			}

			// Gather video parameters
			length = provider->GetFrameCount();
			w = provider->GetWidth();
			h = provider->GetHeight();

			// Set filename
			videoName = filename;
			Options.AddToRecentList(filename,_T("Recent vid"));
			wxFileName fn(filename);
			StandardPaths::SetPathValue(_T("?video"),fn.GetPath());

			// Get frame
			frame_n = 0;

			// Show warning
			wxString warning = provider->GetWarning().c_str();
			if (!warning.IsEmpty()) wxMessageBox(warning,_T("Warning"),wxICON_WARNING | wxOK);
		}
		
		catch (wxString &e) {
			wxMessageBox(e,_T("Error setting video"),wxICON_ERROR | wxOK);
		}
	}
	loaded = provider != NULL;
}


///////////////////
// Add new display
void VideoContext::AddDisplay(VideoDisplay *display) {
	for (std::list<VideoDisplay*>::iterator cur=displayList.begin();cur!=displayList.end();cur++) {
		if ((*cur) == display) return;
	}
	displayList.push_back(display);
}


//////////////////
// Remove display
void VideoContext::RemoveDisplay(VideoDisplay *display) {
	displayList.remove(display);
}


///////////////////
// Update displays
void VideoContext::UpdateDisplays(bool full) {
	for (std::list<VideoDisplay*>::iterator cur=displayList.begin();cur!=displayList.end();cur++) {
		// Get display
		VideoDisplay *display = *cur;

		// Update slider
		if (full) {
			display->UpdateSize();
			display->ControlSlider->SetRange(0,GetLength()-1);
		}
		display->ControlSlider->SetValue(GetFrameN());
		//display->ControlSlider->Update();
		display->UpdatePositionDisplay();

		// If not shown, don't update the display itself
		if (!display->IsShownOnScreen()) continue;

		// Update visual controls
		if (display->visual) display->visual->Refresh();

		// Update controls
		//display->Refresh();
		//display->Update();
		display->Render();
	}

	// Update audio display
	if (audio && audio->loaded && audio->IsShownOnScreen()) {
		if (Options.AsBool(_T("Audio Draw Video Position"))) {
			audio->UpdateImage(false);
		}
	}
}


/////////////////////
// Refresh subtitles
void VideoContext::Refresh (bool video, bool subtitles) {
	// Reset frame
	lastFrame = -1;

	// Update subtitles
	if (subtitles && subsProvider) {
		// Re-export
		AssExporter exporter(grid->ass);
		exporter.AddAutoFilters();
		try {
			subsProvider->LoadSubtitles(exporter.ExportTransform());
		}
		catch (wxString err) { wxMessageBox(_T("Error while invoking subtitles provider: ") + err,_T("Subtitles provider")); }
		catch (const wchar_t *err) { wxMessageBox(_T("Error while invoking subtitles provider: ") + wxString(err),_T("Subtitles provider")); }
	}

	// Jump to frame
	JumpToFrame(frame_n);
}


///////////////////////////////////////
// Jumps to a frame and update display
void VideoContext::JumpToFrame(int n) {
	// Loaded?
	if (!loaded) return;

	// Prevent intervention during playback
	if (isPlaying && n != playNextFrame) return;

	// Threaded
	if (threaded && false) {	// Doesn't work, so it's disabled
		wxMutexLocker lock(vidMutex);
		threadNextFrame = n;
		if (!threadLocked) {
			threadLocked = true;
			thread = new VideoContextThread(this);
			thread->Create();
			thread->Run();
		}
	}

	// Not threaded
	else {
		try {
			// Set frame number
			frame_n = n;
			GetFrameAsTexture(n);

			// Display
			UpdateDisplays(false);

			// Update grid
			if (!isPlaying && Options.AsBool(_T("Highlight subs in frame"))) grid->Refresh(false);
		}
		catch (const wxChar *err) {
			wxLogError(
				_T("Failed seeking video. The video will be closed because of this.\n")
				_T("If you get this error regardless of which video file you use, and also if you use dummy video, Aegisub might not work with your graphics card's OpenGL driver.\n")
				_T("Error message reported: %s"),
				err);
			Reset();
		}
		catch (...) {
			wxLogError(
				_T("Failed seeking video. The video will be closed because of this.\n")
				_T("If you get this error regardless of which video file you use, and also if you use dummy video, Aegisub might not work with your graphics card's OpenGL driver.\n")
				_T("No further error message given."));
			Reset();
		}
	}
}


////////////////////////////
// Jumps to a specific time
void VideoContext::JumpToTime(int ms,bool exact) {
	int frame;
	if (exact) frame = VFR_Output.PFrameAtTime(ms);
	else frame = VFR_Output.GetFrameAtTime(ms); 
	JumpToFrame(frame);
}


//////////////////
// Get GL context
wxGLContext *VideoContext::GetGLContext(wxGLCanvas *canvas) {
	// wxGLCanvas and wxGLContext is a funky couple.
	// On wxMac wxGLContext has a different constructor than everywhere else...
	// But wxMac is also the only implementation that creates and initialises a context
	// in the canvas constructor, meaning a wxGLCanvas on wxMac comes with a context
	// for free, while we have to create our own everywhere else.
	// So let's first see if the canvas might already have a context of its own and
	// get that if we lack one.
	// That should always succeed on wxMac...
	// Everywhere else, we can just create a wxGLContext using the documented interface
	// and be over with it after that.
	// Also see bug #850.
#if wxCHECK_VERSION(2,9,0)
#else
	if (!glContext) {
		glContext = canvas->GetContext();
		ownGlContext = false;
	}
#endif
#ifndef __WXMAC__
	if (!glContext) {
		glContext = new wxGLContext(canvas);
		ownGlContext = true;
	}
#endif
	return glContext;
}


////////////////////////
// Requests a new frame
AegiVideoFrame VideoContext::GetFrame(int n,bool raw) {
	// Current frame if -1
	if (n == -1) n = frame_n;

	// Get available formats
	int formats = FORMAT_RGB32;

	// Get frame
	AegiVideoFrame frame = provider->GetFrame(n,formats);

	// Raster subtitles if available/necessary
	if (!raw && subsProvider) {
		tempFrame.CopyFrom(frame);
		subsProvider->DrawSubtitles(tempFrame,VFR_Input.GetTimeAtFrame(n,true,true)/1000.0);
		return tempFrame;
	}

	// Return pure frame
	else return frame;
}


///////////////////////////
// Get GL Texture of frame
GLuint VideoContext::GetFrameAsTexture(int n) {
	// Already uploaded
	if (n == lastFrame || n == -1) return lastTex;

	// Get frame
	AegiVideoFrame frame = GetFrame(n);

	// Set frame
	lastFrame = n;

	// Set context
#ifdef __APPLE__
	GetGLContext(displayList.front())->SetCurrent();
#else
	GetGLContext(displayList.front())->SetCurrent(*displayList.front());
#endif
	glEnable(GL_TEXTURE_2D);
	if (glGetError() != 0) throw _T("Error enabling texture.");

	// Image type
	GLenum format = GL_LUMINANCE;
	if (frame.format == FORMAT_RGB32) {
		if (frame.invertChannels) format = GL_BGRA_EXT;
		else format = GL_RGBA;
	}
	else if (frame.format == FORMAT_RGB24) {
		if (frame.invertChannels) format = GL_BGR_EXT;
		else format = GL_RGB;
	}
	else if (frame.format == FORMAT_YV12) {
		format = GL_LUMINANCE;
	}
	isInverted = frame.flipped;

	if (lastTex == 0) {
		// Enable
		glShadeModel(GL_FLAT);

		// Generate texture with GL
		//glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &lastTex);
		if (glGetError() != 0) throw _T("Error generating texture.");
		glBindTexture(GL_TEXTURE_2D, lastTex);
		if (glGetError() != 0) throw _T("Error binding texture.");

		// Texture parameters
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		if (glGetError() != 0) throw _T("Error setting min_filter texture parameter.");
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		if (glGetError() != 0) throw _T("Error setting mag_filter texture parameter.");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		if (glGetError() != 0) throw _T("Error setting wrap_s texture parameter.");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		if (glGetError() != 0) throw _T("Error setting wrap_t texture parameter.");

		// Allocate texture
		int height = frame.h;
		if (frame.format == FORMAT_YV12) height = height * 3 / 2;
		int tw = SmallestPowerOf2(MAX(frame.pitch[0]/frame.GetBpp(0),frame.pitch[1]+frame.pitch[2]));
		int th = SmallestPowerOf2(height);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,tw,th,0,format,GL_UNSIGNED_BYTE,NULL);
		if (glGetError() != 0) {
			tw = MAX(tw,th);
			th = tw;
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,tw,th,0,format,GL_UNSIGNED_BYTE,NULL);
			if (glGetError() != 0) {
				glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tw,th,0,format,GL_UNSIGNED_BYTE,NULL);
				if (glGetError() != 0) throw _T("Error allocating texture.");
			}
		}
		texW = float(frame.w)/float(tw);
		texH = float(frame.h)/float(th);

		// Set texture
		//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		//if (glGetError() != 0) throw _T("Error setting hinting.");

		// Set priority
		float priority = 1.0f;
		glPrioritizeTextures(1,&lastTex,&priority);
	}
	
	// Load texture data
	glBindTexture(GL_TEXTURE_2D, lastTex);
	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,frame.pitch[0]/frame.GetBpp(0),frame.h,format,GL_UNSIGNED_BYTE,frame.data[0]);
	if (glGetError() != 0) throw _T("Error uploading primary plane");

	// UV planes for YV12
	if (frame.format == FORMAT_YV12) {
		int u = 1;
		int v = 2;
		if (frame.invertChannels) {
			u = 2;
			v = 1;
		}
		glTexSubImage2D(GL_TEXTURE_2D,0,0,frame.h,frame.pitch[1],frame.h/2,format,GL_UNSIGNED_BYTE,frame.data[u]);
		if (glGetError() != 0) throw _T("Error uploading U plane.");
		glTexSubImage2D(GL_TEXTURE_2D,0,frame.pitch[1],frame.h,frame.pitch[2],frame.h/2,format,GL_UNSIGNED_BYTE,frame.data[v]);
		if (glGetError() != 0) throw _T("Error uploadinv V plane.");
	}

	// Return texture number
	return lastTex;
}


/////////////////
// Save snapshot
void VideoContext::SaveSnapshot(bool raw) {
	// Get folder
	wxString option = Options.AsText(_T("Video Screenshot Path"));
	wxFileName videoFile(videoName);
	wxString basepath;
	// Is it a path specifier and not an actual fixed path?
	if (option[0] == _T('?')) {
		// If dummy video is loaded, we can't save to the video location
		if (option.StartsWith(_T("?video")) && (videoName.Find(_T("?dummy")) != wxNOT_FOUND)) {
			// So try the script location instead
			option = _T("?script");
		}
		// Find out where the ?specifier points to
		basepath = StandardPaths::DecodePath(option);
		// If whereever that is isn't defined, we can't save there
		if ((basepath == _T("\\")) || (basepath == _T("/"))) {
			// So save to the current user's home dir instead
			basepath = wxGetHomeDir();
		}
	}
	// Actual fixed (possibly relative) path, decode it
	else basepath = DecodeRelativePath(option,StandardPaths::DecodePath(_T("?user/")));
	basepath += _T("/") + videoFile.GetName();

	// Get full path
	int session_shot_count = 1;
	wxString path;
	while (1) {
		path = basepath + wxString::Format(_T("_%03i_%i.png"),session_shot_count,frame_n);
		++session_shot_count;
		wxFileName tryPath(path);
		if (!tryPath.FileExists()) break;
	}

	// Save
	GetFrame(frame_n,raw).GetImage().SaveFile(path,wxBITMAP_TYPE_PNG);
}


////////////////////////////
// Get dimensions of script
void VideoContext::GetScriptSize(int &sw,int &sh) {
	grid->ass->GetResolution(sw,sh);
}


////////
// Play
void VideoContext::Play() {
	// Stop if already playing
	if (isPlaying) {
		Stop();
		return;
	}

	// Set variables
	startFrame = frame_n;
	endFrame = -1;

	// Start playing audio
	audio->Play(VFR_Output.GetTimeAtFrame(startFrame),-1);

	//audio->Play will override this if we put it before, so put it after.
	isPlaying = true;

	// Start timer
	playTime.Start();
	playback.SetOwner(this,VIDEO_PLAY_TIMER);
	playback.Start(10);
}


/////////////
// Play line
void VideoContext::PlayLine() {
	// Get line
	AssDialogue *curline = grid->GetDialogue(grid->editBox->linen);
	if (!curline) return;

	// Start playing audio
	audio->Play(curline->Start.GetMS(),curline->End.GetMS());

	// Set variables
	isPlaying = true;
	startFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	endFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);

	// Jump to start
	playNextFrame = startFrame;
	JumpToFrame(startFrame);

	// Set other variables
	playTime.Start();

	// Start timer
	playback.SetOwner(this,VIDEO_PLAY_TIMER);
	playback.Start(10);
}


////////
// Stop
void VideoContext::Stop() {
	if (isPlaying) {
		playback.Stop();
		isPlaying = false;
		audio->Stop();
	}
}


//////////////
// Play timer
void VideoContext::OnPlayTimer(wxTimerEvent &event) {
	// Lock
	wxMutexError res = playMutex.TryLock();
	if (res == wxMUTEX_BUSY) return;
	playMutex.Unlock();
	wxMutexLocker lock(playMutex);

	// Get time difference
	int dif = playTime.Time();

	// Find next frame
	int startMs = VFR_Output.GetTimeAtFrame(startFrame);
	int nextFrame = frame_n;
	int i=0;
	for (i=0;i<10;i++) {
		if (nextFrame >= length) break;
		if (dif < VFR_Output.GetTimeAtFrame(nextFrame) - startMs) {
			break;
		}
		nextFrame++;
	}

	// End
	if (nextFrame >= length || (endFrame != -1 && nextFrame > endFrame)) {
		Stop();
		return;
	}

	// Same frame
	if (nextFrame == frame_n) return;

	// Next frame is before or over 2 frames ahead, so force audio resync
	if (audio->player && keepAudioSync && (nextFrame < frame_n || nextFrame > frame_n + 2)) audio->player->SetCurrentPosition(audio->GetSampleAtMS(VFR_Output.GetTimeAtFrame(nextFrame)));

	// Jump to next frame
	playNextFrame = nextFrame;
	frame_n = nextFrame;
	JumpToFrame(nextFrame);

	// Sync audio
	if (keepAudioSync && nextFrame % 10 == 0 && audio && audio->provider && audio->player) {
		int64_t audPos = audio->GetSampleAtMS(VFR_Output.GetTimeAtFrame(nextFrame));
		int64_t curPos = audio->player->GetCurrentPosition();
		int delta = int(audPos-curPos);
		if (delta < 0) delta = -delta;
		int maxDelta = audio->provider->GetSampleRate();
		if (delta > maxDelta) audio->player->SetCurrentPosition(audPos);
	}
}


//////////////////////////////
// Get name of temp work file
wxString VideoContext::GetTempWorkFile () {
	if (tempfile.IsEmpty()) {
		tempfile = wxFileName::CreateTempFileName(_T("aegisub"));
		wxRemoveFile(tempfile);
		tempfile += _T(".ass");
	}
	return tempfile;
}


/////////////////
// Get keyframes
wxArrayInt VideoContext::GetKeyFrames() {
	if (OverKeyFramesLoaded()) return overKeyFrames;
	return KeyFrames;
}


/////////////////
// Set keyframes
void VideoContext::SetKeyFrames(wxArrayInt frames) {
	KeyFrames = frames;
}


/////////////////////////
// Set keyframe override
void VideoContext::SetOverKeyFrames(wxArrayInt frames) {
	overKeyFrames = frames;
	overKeyFramesLoaded = true;
}


///////////////////
// Close keyframes
void VideoContext::CloseOverKeyFrames() {
	overKeyFrames.Clear();
	overKeyFramesLoaded = false;
}


//////////////////////////////////////////
// Check if override keyframes are loaded
bool VideoContext::OverKeyFramesLoaded() {
	return overKeyFramesLoaded;
}


/////////////////////////////////
// Check if keyframes are loaded
bool VideoContext::KeyFramesLoaded() {
	return overKeyFramesLoaded || keyFramesLoaded;
}


//////////////////////////
// Calculate aspect ratio
double VideoContext::GetARFromType(int type) {
	if (type == 0) return (double)VideoContext::Get()->GetWidth()/(double)VideoContext::Get()->GetHeight();
	if (type == 1) return 4.0/3.0;
	if (type == 2) return 16.0/9.0;
	if (type == 3) return 2.35;
	return 1.0;  //error
}


/////////////////////
// Sets aspect ratio
void VideoContext::SetAspectRatio(int _type, double value) {
	// Get value
	if (_type != 4) value = GetARFromType(_type);
	if (value < 0.5) value = 0.5;
	if (value > 5.0) value = 5.0;

	// Set
	arType = _type;
	arValue = value;
	UpdateDisplays(true);
}


//////////////////////
// Thread constructor
VideoContextThread::VideoContextThread(VideoContext *par)
: wxThread(wxTHREAD_DETACHED)
{
	parent = par;
}


//////////////////////
// Thread entry point
wxThread::ExitCode VideoContextThread::Entry() {
	// Set up thread
	int frame = parent->threadNextFrame;
	int curFrame = parent->frame_n;
	bool highSubs = Options.AsBool(_T("Highlight subs in frame"));

	// Loop while there is work to do
	while (true) {
		// Get frame and set frame number
		{
			wxMutexLocker glLock(OpenGLWrapper::glMutex);
			parent->GetFrameAsTexture(frame);
			parent->frame_n = frame;
		}

		// Display
		parent->UpdateDisplays(false);

		// Update grid
		if (!parent->isPlaying && highSubs) parent->grid->Refresh(false);

		// Get lock and check if there is more to do
		wxMutexLocker lock(parent->vidMutex);
		curFrame = parent->frame_n;
		frame = parent->threadNextFrame;

		// Work done, kill thread and release context
		if (curFrame == frame) {
			parent->threadLocked = false;
			parent->threadNextFrame = -1;
			Delete();
			return 0;
		}
	}
}
