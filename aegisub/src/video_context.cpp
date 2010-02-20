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
	keyframesRevision = 0;
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
	loaded = false;
	StandardPaths::SetPathValue(_T("?video"),_T(""));

	// Clear keyframes
	KeyFrames.Clear();
	keyFramesLoaded = false;
	keyframesRevision++;

	// Remove temporary audio provider
	if (audio && audio->temporary) {
		delete audio->provider;
		audio->provider = NULL;
		delete audio->player;
		audio->player = NULL;
		audio->temporary = false;
	}

	// Remove video data
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
			// double overFps = 0;
			FrameRate temp;

			// Unload timecodes
			//int unload = wxYES;
			//if (VFR_Output.IsLoaded()) unload = wxMessageBox(_("Do you want to unload timecodes, too?"),_("Unload timecodes?"),wxYES_NO | wxICON_QUESTION);
			//if (unload == wxYES) VFR_Output.Unload();

			// Set GL context
			GetGLContext(displayList.front())->SetCurrent(*displayList.front());

			// Choose a provider
			provider = VideoProviderFactoryManager::GetProvider(filename);

			// Get subtitles provider
			try {
				subsProvider = SubtitlesProviderFactoryManager::GetProvider();
			}
			catch (wxString err) { wxMessageBox(_T("Error while loading subtitles provider: ") + err,_T("Subtitles provider"));	}
			catch (const wchar_t *err) { wxMessageBox(_T("Error while loading subtitles provider: ") + wxString(err),_T("Subtitles provider"));	}

			KeyFrames.Clear();
			keyframesRevision++;
			// load keyframes if available
			if (provider->AreKeyFramesLoaded()) {
				KeyFrames = provider->GetKeyFrames();
				keyFramesLoaded = true;
			}
			else {
				keyFramesLoaded = false;
			}
			
			// Set frame rate
			fps = provider->GetFPS();
			// does this provider need special vfr treatment?
			if (provider->NeedsVFRHack()) {
				// FIXME:
				// Unfortunately, this hack does not actually work for the one
				// provider that needs it (Avisynth). Go figure.
				bool isVfr = provider->IsVFR();
				if (!isVfr || provider->IsNativelyByFrames()) {
					VFR_Input.SetCFR(fps);
					if (VFR_Output.GetFrameRateType() != VFR) VFR_Output.SetCFR(fps);
				}
				else {
					FrameRate temp = provider->GetTrueFrameRate();
					provider->OverrideFrameTimeList(temp.GetFrameTimeList());
				}
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

			UpdateDisplays(true);
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
		VideoDisplay *display = *cur;

		if (full) {
			display->UpdateSize();
			display->ControlSlider->SetRange(0,GetLength()-1);
		}
		display->SetFrame(GetFrameN());
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

	// Set frame number
	frame_n = n;

	// Display
	UpdateDisplays(false);

	// Update grid
	if (!isPlaying && Options.AsBool(_T("Highlight subs in frame"))) grid->Refresh(false);
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
	if (!glContext) {
		glContext = new wxGLContext(canvas);
		ownGlContext = true;
	}
	return glContext;
}


////////////////////////
// Requests a new frame
AegiVideoFrame VideoContext::GetFrame(int n,bool raw) {
	// Current frame if -1
	if (n == -1) n = frame_n;

	// Get frame
	AegiVideoFrame frame = provider->GetFrame(n);

	// Raster subtitles if available/necessary
	if (!raw && subsProvider) {
		tempFrame.CopyFrom(frame);
		subsProvider->DrawSubtitles(tempFrame,VFR_Input.GetTimeAtFrame(n,true,true)/1000.0);
		return tempFrame;
	}

	// Return pure frame
	else return frame;
}

/// @brief Save snapshot 
/// @param raw 
///
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
const wxArrayInt & VideoContext::GetKeyFrames() {
	if (OverKeyFramesLoaded()) return overKeyFrames;
	return KeyFrames;
}


/////////////////
// Set keyframes
void VideoContext::SetKeyFrames(wxArrayInt frames) {
	KeyFrames = frames;
	keyframesRevision++;
}


/////////////////////////
// Set keyframe override
void VideoContext::SetOverKeyFrames(wxArrayInt frames) {
	overKeyFrames = frames;
	overKeyFramesLoaded = true;
	keyframesRevision++;
}


///////////////////
// Close keyframes
void VideoContext::CloseOverKeyFrames() {
	overKeyFrames.Clear();
	overKeyFramesLoaded = false;
	keyframesRevision++;
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
