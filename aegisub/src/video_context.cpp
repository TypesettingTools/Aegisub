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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file video_context.cpp
/// @brief Keep track of loaded video and video displays
/// @ingroup video
///


////////////
// Includes
#include "config.h"

#ifndef AGI_PRE
#include <string.h>

#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/image.h>
#endif

#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "ass_dialogue.h"
#include "ass_exporter.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_time.h"
#include "audio_display.h"
#include "mkv_wrap.h"
#include "options.h"
#include "standard_paths.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "subtitles_provider_manager.h"
#include "utils.h"
#include "vfr.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_provider_manager.h"


///////
// IDs
enum {

	/// DOCME
	VIDEO_PLAY_TIMER = 1300
};


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoContext, wxEvtHandler)
	EVT_TIMER(VIDEO_PLAY_TIMER,VideoContext::OnPlayTimer)
END_EVENT_TABLE()

/// DOCME
VideoContext *VideoContext::instance = NULL;

/// @brief Constructor 
///
VideoContext::VideoContext()
: ownGlContext(false)
, glContext(NULL)
, provider(NULL)
, subsProvider(NULL)
, keyFramesLoaded(false)
, overKeyFramesLoaded(false)
, startFrame(-1)
, endFrame(-1)
, playNextFrame(-1)
, nextFrame(-1)
, loaded(false)
, isPlaying(false)
, keepAudioSync(true)
, w(-1)
, h(-1)
, frame_n(0)
, length(0)
, fps(0)
, arValue(1.)
, arType(0)
, hasSubtitles(false)
, grid(NULL)
, curLine(NULL)
, audio(NULL)
{
}

/// @brief Destructor 
///
VideoContext::~VideoContext () {
	Reset();
	if (ownGlContext)
		delete glContext;
	glContext = NULL;
}

/// @brief Get Instance 
/// @return 
///
VideoContext *VideoContext::Get() {
	if (!instance) {
		instance = new VideoContext;
	}
	return instance;
}

/// @brief Clear 
///
void VideoContext::Clear() {
	instance->audio = NULL;
	delete instance;
	instance = NULL;
}

/// @brief Reset 
///
void VideoContext::Reset() {
	loaded = false;
	StandardPaths::SetPathValue(_T("?video"),_T(""));

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

/// @brief Reload video 
///
void VideoContext::Reload() {
	if (IsLoaded()) {
		wxString name = videoName;
		int n = frame_n;
		SetVideo(_T(""));
		SetVideo(name);
		JumpToFrame(n);
	}
}

/// @brief Sets video filename 
/// @param filename 
///
void VideoContext::SetVideo(const wxString &filename) {
	// Unload video
	Reset();

	// Load video
	if (!filename.IsEmpty()) {
		try {
			grid->CommitChanges(true);

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

			hasSubtitles = false;
			if (filename.Right(4).Lower() == L".mkv") {
				hasSubtitles = MatroskaWrapper::HasSubtitles(filename);
			}

			UpdateDisplays(true);
		}
		
		catch (wxString &e) {
			wxMessageBox(e,_T("Error setting video"),wxICON_ERROR | wxOK);
		}
	}
	loaded = provider != NULL;
}

/// @brief Add new display 
/// @param display 
/// @return 
///
void VideoContext::AddDisplay(VideoDisplay *display) {
	for (std::list<VideoDisplay*>::iterator cur=displayList.begin();cur!=displayList.end();cur++) {
		if ((*cur) == display) return;
	}
	displayList.push_back(display);
}

/// @brief Remove display 
/// @param display 
///
void VideoContext::RemoveDisplay(VideoDisplay *display) {
	displayList.remove(display);
}

/// @brief Update displays 
/// @param full 
///
void VideoContext::UpdateDisplays(bool full) {
	for (std::list<VideoDisplay*>::iterator cur=displayList.begin();cur!=displayList.end();cur++) {
		VideoDisplay *display = *cur;

		if (full) {
			display->UpdateSize();
			display->SetFrameRange(0,GetLength()-1);
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

/// @brief Refresh subtitles 
/// @param video     
/// @param subtitles 
///
void VideoContext::Refresh (bool video, bool subtitles) {
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

/// @brief Jumps to a frame and update display 
/// @param n 
/// @return 
///
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



/// @brief Jumps to a specific time 
/// @param ms    
/// @param exact 
///
void VideoContext::JumpToTime(int ms,bool exact) {
	int frame;
	if (exact) frame = VFR_Output.PFrameAtTime(ms);
	else frame = VFR_Output.GetFrameAtTime(ms); 
	JumpToFrame(frame);
}



/// @brief Get GL context 
/// @param canvas 
/// @return 
///
wxGLContext *VideoContext::GetGLContext(wxGLCanvas *canvas) {
	if (!glContext) {
		glContext = new wxGLContext(canvas);
		ownGlContext = true;
	}
	return glContext;
}



/// @brief Requests a new frame 
/// @param n   
/// @param raw 
/// @return 
///
AegiVideoFrame VideoContext::GetFrame(int n,bool raw) {
	// Current frame if -1
	if (n == -1) n = frame_n;

	// Get frame
	AegiVideoFrame frame = provider->GetFrame(n);

	// Raster subtitles if available/necessary
	if (!raw && subsProvider) {
		tempFrame.CopyFrom(frame);
		try {
			subsProvider->DrawSubtitles(tempFrame,VFR_Input.GetTimeAtFrame(n,true,true)/1000.0);
		}
		catch (...) {
			wxLogError(L"Subtitle rendering for the current frame failed.\n");
		}
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

/// @brief Get dimensions of script 
/// @param sw 
/// @param sh 
///
void VideoContext::GetScriptSize(int &sw,int &sh) {
	grid->ass->GetResolution(sw,sh);
}

/// @brief Play the next frame, possibly with audio
/// @return 
///
void VideoContext::PlayNextFrame() {
	if (isPlaying)
		return;

	int thisFrame = frame_n;
	JumpToFrame(frame_n + 1);
	// Start playing audio
	if (Options.AsBool(_T("Audio Plays When Stepping Video")))
		audio->Play(VFR_Output.GetTimeAtFrame(thisFrame),VFR_Output.GetTimeAtFrame(thisFrame + 1));
}

/// @brief Play the previous frame, possibly with audio
/// @return 
///
void VideoContext::PlayPrevFrame() {
	if (isPlaying)
		return;

	int thisFrame = frame_n;
	JumpToFrame(frame_n -1);
	// Start playing audio
	if (Options.AsBool(_T("Audio Plays When Stepping Video")))
		audio->Play(VFR_Output.GetTimeAtFrame(thisFrame - 1),VFR_Output.GetTimeAtFrame(thisFrame));
}

/// @brief Play 
/// @return 
///
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




/// @brief Play line 
/// @return 
///
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

/// @brief Stop 
///
void VideoContext::Stop() {
	if (isPlaying) {
		playback.Stop();
		isPlaying = false;
		audio->Stop();
	}
}

/// @brief Play timer 
/// @param event 
/// @return 
///
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

/// @brief Get name of temp work file 
/// @return 
///
wxString VideoContext::GetTempWorkFile () {
	if (tempfile.IsEmpty()) {
		tempfile = wxFileName::CreateTempFileName(_T("aegisub"));
		wxRemoveFile(tempfile);
		tempfile += _T(".ass");
	}
	return tempfile;
}

/// @brief Get keyframes 
/// @return 
///
wxArrayInt VideoContext::GetKeyFrames() {
	if (OverKeyFramesLoaded()) return overKeyFrames;
	return KeyFrames;
}

/// @brief Set keyframes 
/// @param frames 
///
void VideoContext::SetKeyFrames(wxArrayInt frames) {
	KeyFrames = frames;
}

/// @brief Set keyframe override 
/// @param frames 
///
void VideoContext::SetOverKeyFrames(wxArrayInt frames) {
	overKeyFrames = frames;
	overKeyFramesLoaded = true;
}

/// @brief Close keyframes 
///
void VideoContext::CloseOverKeyFrames() {
	overKeyFrames.Clear();
	overKeyFramesLoaded = false;
}

/// @brief Check if override keyframes are loaded 
/// @return 
///
bool VideoContext::OverKeyFramesLoaded() {
	return overKeyFramesLoaded;
}

/// @brief Check if keyframes are loaded 
/// @return 
///
bool VideoContext::KeyFramesLoaded() {
	return overKeyFramesLoaded || keyFramesLoaded;
}

/// @brief Calculate aspect ratio 
/// @param type 
/// @return 
///
double VideoContext::GetARFromType(int type) {
	if (type == 0) return (double)VideoContext::Get()->GetWidth()/(double)VideoContext::Get()->GetHeight();
	if (type == 1) return 4.0/3.0;
	if (type == 2) return 16.0/9.0;
	if (type == 3) return 2.35;
	return 1.0;  //error
}

/// @brief Sets aspect ratio 
/// @param _type 
/// @param value 
///
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
