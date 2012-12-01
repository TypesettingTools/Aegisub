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

/// @file video_context.cpp
/// @brief Keep track of loaded video
/// @ingroup video
///

#include "config.h"

#ifndef AGI_PRE
#include <cstring>

#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#endif

#include <libaegisub/keyframe.h>
#include <libaegisub/log.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_time.h"
#include "audio_controller.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "include/aegisub/video_provider.h"
#include "main.h"
#include "mkv_wrap.h"
#include "selection_controller.h"
#include "standard_paths.h"
#include "time_range.h"
#include "threaded_frame_source.h"
#include "utils.h"
#include "video_context.h"
#include "video_frame.h"

/// @brief Constructor
///
VideoContext::VideoContext()
: playback(this)
, startMS(0)
, endFrame(0)
, frame_n(0)
, arValue(1.)
, arType(0)
, hasSubtitles(false)
, playAudioOnStep(OPT_GET("Audio/Plays When Stepping Video"))
{
	Bind(EVT_VIDEO_ERROR, &VideoContext::OnVideoError, this);
	Bind(EVT_SUBTITLES_ERROR, &VideoContext::OnSubtitlesError, this);
	Bind(wxEVT_TIMER, &VideoContext::OnPlayTimer, this);

	OPT_SUB("Subtitle/Provider", &VideoContext::Reload, this);
	OPT_SUB("Video/Provider", &VideoContext::Reload, this);

	// It would be nice to find a way to move these to the individual providers
	OPT_SUB("Provider/Avisynth/Allow Ancient", &VideoContext::Reload, this);
	OPT_SUB("Provider/Avisynth/Memory Max", &VideoContext::Reload, this);

	OPT_SUB("Provider/Video/FFmpegSource/Decoding Threads", &VideoContext::Reload, this);
	OPT_SUB("Provider/Video/FFmpegSource/Unsafe Seeking", &VideoContext::Reload, this);
	OPT_SUB("Video/Force BT.601", &VideoContext::Reload, this);
}

VideoContext::~VideoContext () {
}

VideoContext *VideoContext::Get() {
	static VideoContext instance;
	return &instance;
}

void VideoContext::Reset() {
	StandardPaths::SetPathValue("?video", "");

	// Remove video data
	Stop();
	frame_n = 0;

	// Clean up video data
	videoFile.clear();

	// Remove provider
	provider.reset();
	videoProvider = 0;

	keyFrames.clear();
	keyFramesFilename.clear();
	videoFPS = agi::vfr::Framerate();
	KeyframesOpen(keyFrames);
	if (!ovrFPS.IsLoaded()) TimecodesOpen(videoFPS);
}

void VideoContext::SetContext(agi::Context *context) {
	this->context = context;
	context->ass->AddCommitListener(&VideoContext::OnSubtitlesCommit, this);
	context->ass->AddFileSaveListener(&VideoContext::OnSubtitlesSave, this);
}

void VideoContext::SetVideo(const wxString &filename) {
	Reset();
	if (filename.empty()) {
		VideoOpen();
		return;
	}

	bool commit_subs = false;
	try {
		provider.reset(new ThreadedFrameSource(filename, this));
		videoProvider = provider->GetVideoProvider();
		videoFile = filename;

		// Check that the script resolution matches the video resolution
		int sx = context->ass->GetScriptInfoAsInt("PlayResX");
		int sy = context->ass->GetScriptInfoAsInt("PlayResY");
		int vx = GetWidth();
		int vy = GetHeight();

		// If the script resolution hasn't been set at all just force it to the
		// video resolution
		if (sx == 0 && sy == 0) {
			context->ass->SetScriptInfo("PlayResX", wxString::Format("%d", vx));
			context->ass->SetScriptInfo("PlayResY", wxString::Format("%d", vy));
			commit_subs = true;
		}
		// If it has been set to something other than a multiple of the video
		// resolution, ask the user if they want it to be fixed
		else if (sx % vx != 0 || sy % vy != 0) {
			switch (OPT_GET("Video/Check Script Res")->GetInt()) {
			case 1: // Ask to change on mismatch
				if (wxYES != wxMessageBox(
					wxString::Format(_("The resolution of the loaded video and the resolution specified for the subtitles don't match.\n\nVideo resolution:\t%d x %d\nScript resolution:\t%d x %d\n\nChange subtitles resolution to match video?"), vx, vy, sx, sy),
					_("Resolution mismatch"),
					wxYES_NO | wxCENTER,
					context->parent))

					break;
				// Fallthrough to case 2
			case 2: // Always change script res
				context->ass->SetScriptInfo("PlayResX", wxString::Format("%d", vx));
				context->ass->SetScriptInfo("PlayResY", wxString::Format("%d", vy));
				commit_subs = true;
				break;
			default: // Never change
				break;
			}
		}

		keyFrames = videoProvider->GetKeyFrames();

		// Set frame rate
		videoFPS = videoProvider->GetFPS();
		if (ovrFPS.IsLoaded()) {
			int ovr = wxMessageBox(_("You already have timecodes loaded. Would you like to replace them with timecodes from the video file?"), _("Replace timecodes?"), wxYES_NO | wxICON_QUESTION);
			if (ovr == wxYES) {
				ovrFPS = agi::vfr::Framerate();
				ovrTimecodeFile.clear();
			}
		}

		// Set aspect ratio
		double dar = videoProvider->GetDAR();
		if (dar > 0)
			SetAspectRatio(4, dar);

		// Set filename
		config::mru->Add("Video", STD_STR(filename));
		StandardPaths::SetPathValue("?video", wxFileName(filename).GetPath());

		// Show warning
		wxString warning = videoProvider->GetWarning();
		if (!warning.empty()) wxMessageBox(warning, "Warning", wxICON_WARNING | wxOK);

		hasSubtitles = false;
		if (filename.Right(4).Lower() == ".mkv") {
			hasSubtitles = MatroskaWrapper::HasSubtitles(filename);
		}

		provider->LoadSubtitles(context->ass);
		VideoOpen();
		KeyframesOpen(keyFrames);
		TimecodesOpen(FPS());
	}
	catch (agi::UserCancelException const&) { }
	catch (agi::FileNotAccessibleError const& err) {
		config::mru->Remove("Video", STD_STR(filename));
		wxMessageBox(lagi_wxString(err.GetMessage()), "Error setting video", wxOK | wxICON_ERROR | wxCENTER);
	}
	catch (VideoProviderError const& err) {
		wxMessageBox(lagi_wxString(err.GetMessage()), "Error setting video", wxOK | wxICON_ERROR | wxCENTER);
	}

	if (commit_subs)
		context->ass->Commit(_("change script resolution"), AssFile::COMMIT_SCRIPTINFO);
	else
		JumpToFrame(0);
}

void VideoContext::Reload() {
	if (IsLoaded()) {
		int frame = frame_n;
		SetVideo(videoFile.Clone());
		JumpToFrame(frame);
	}
}

void VideoContext::OnSubtitlesCommit() {
	if (!IsLoaded()) return;

	provider->LoadSubtitles(context->ass);
	if (!IsPlaying())
		GetFrameAsync(frame_n);
}

void VideoContext::OnSubtitlesSave() {
	context->ass->SetScriptInfo("VFR File", MakeRelativePath(GetTimecodesName(), context->ass->filename));
	context->ass->SetScriptInfo("Keyframes File", MakeRelativePath(GetKeyFramesName(), context->ass->filename));

	if (!IsLoaded()) {
		context->ass->SetScriptInfo("Video File", "");
		context->ass->SetScriptInfo("Video Aspect Ratio", "");
		context->ass->SetScriptInfo("Video Position", "");
		return;
	}

	wxString ar;
	if (arType == 4)
		ar = wxString::Format("c%g", arValue);
	else
		ar = wxString::Format("%d", arType);

	context->ass->SetScriptInfo("Video File", MakeRelativePath(videoFile, context->ass->filename));
	context->ass->SetScriptInfo("YCbCr Matrix", videoProvider->GetColorSpace());
	context->ass->SetScriptInfo("Video Aspect Ratio", ar);
	context->ass->SetScriptInfo("Video Position", wxString::Format("%d", frame_n));
}

void VideoContext::JumpToFrame(int n) {
	if (!IsLoaded()) return;

	bool was_playing = IsPlaying();
	if (was_playing)
		Stop();

	frame_n = mid(0, n, GetLength() - 1);

	GetFrameAsync(frame_n);
	Seek(frame_n);

	if (was_playing)
		Play();
}

void VideoContext::JumpToTime(int ms, agi::vfr::Time end) {
	JumpToFrame(FrameAtTime(ms, end));
}

void VideoContext::GetFrameAsync(int n) {
	provider->RequestFrame(n, TimeAtFrame(n) / 1000.0);
}

std::shared_ptr<AegiVideoFrame> VideoContext::GetFrame(int n, bool raw) {
	return provider->GetFrame(n, TimeAtFrame(n) / 1000.0, raw);
}

int VideoContext::GetWidth() const {
	return videoProvider->GetWidth();
}
int VideoContext::GetHeight() const {
	return videoProvider->GetHeight();
}

int VideoContext::GetLength() const {
	return videoProvider->GetFrameCount();
}

void VideoContext::NextFrame() {
	if (!videoProvider || IsPlaying() || frame_n == videoProvider->GetFrameCount())
		return;

	JumpToFrame(frame_n + 1);
	// Start playing audio
	if (playAudioOnStep->GetBool()) {
		context->audioController->PlayRange(TimeRange(TimeAtFrame(frame_n - 1), TimeAtFrame(frame_n)));
	}
}

void VideoContext::PrevFrame() {
	if (!videoProvider || IsPlaying() || frame_n == 0)
		return;

	JumpToFrame(frame_n - 1);
	// Start playing audio
	if (playAudioOnStep->GetBool()) {
		context->audioController->PlayRange(TimeRange(TimeAtFrame(frame_n), TimeAtFrame(frame_n + 1)));
	}
}

void VideoContext::Play() {
	if (IsPlaying()) {
		Stop();
		return;
	}

	if (!IsLoaded()) return;

	// Set variables
	startMS = TimeAtFrame(frame_n);
	endFrame = GetLength() - 1;

	// Start playing audio
	context->audioController->PlayToEnd(startMS);

	// Start timer
	playTime.Start();
	playback.Start(10);
}

void VideoContext::PlayLine() {
	Stop();

	AssDialogue *curline = context->selectionController->GetActiveLine();
	if (!curline) return;

	// Start playing audio
	context->audioController->PlayRange(TimeRange(curline->Start, curline->End));

	// Round-trip conversion to convert start to exact
	int startFrame = FrameAtTime(context->selectionController->GetActiveLine()->Start,agi::vfr::START);
	startMS = TimeAtFrame(startFrame);
	endFrame = FrameAtTime(context->selectionController->GetActiveLine()->End,agi::vfr::END) + 1;

	// Jump to start
	JumpToFrame(startFrame);

	// Start timer
	playTime.Start();
	playback.Start(10);
}

void VideoContext::Stop() {
	if (IsPlaying()) {
		playback.Stop();
		context->audioController->Stop();
	}
}

void VideoContext::OnPlayTimer(wxTimerEvent &) {
	int nextFrame = FrameAtTime(startMS + playTime.Time());

	// Same frame
	if (nextFrame == frame_n) return;

	// End
	if (nextFrame >= endFrame) {
		Stop();
		return;
	}

	// Jump to next frame
	frame_n = nextFrame;
	GetFrameAsync(frame_n);
	Seek(frame_n);
}

double VideoContext::GetARFromType(int type) const {
	if (type == 0) return (double)GetWidth()/(double)GetHeight();
	if (type == 1) return 4.0/3.0;
	if (type == 2) return 16.0/9.0;
	if (type == 3) return 2.35;
	return 1.0;  //error
}

void VideoContext::SetAspectRatio(int type, double value) {
	if (type != 4) value = GetARFromType(type);

	arType = type;
	arValue = mid(.5, value, 5.);
	ARChange(arType, arValue);
}

void VideoContext::LoadKeyframes(wxString filename) {
	if (filename == keyFramesFilename || filename.empty()) return;
	try {
		keyFrames = agi::keyframe::Load(STD_STR(filename));
		keyFramesFilename = filename;
		KeyframesOpen(keyFrames);
		config::mru->Add("Keyframes", STD_STR(filename));
	}
	catch (agi::keyframe::Error const& err) {
		wxMessageBox(err.GetMessage(), "Error opening keyframes file", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		config::mru->Remove("Keyframes", STD_STR(filename));
	}
	catch (agi::FileSystemError const&) {
		wxLogError("Could not open file " + filename);
		config::mru->Remove("Keyframes", STD_STR(filename));
	}
}

void VideoContext::SaveKeyframes(wxString filename) {
	agi::keyframe::Save(STD_STR(filename), GetKeyFrames());
	config::mru->Add("Keyframes", STD_STR(filename));
}

void VideoContext::CloseKeyframes() {
	keyFramesFilename.clear();
	if (videoProvider)
		keyFrames = videoProvider->GetKeyFrames();
	else
		keyFrames.clear();
	KeyframesOpen(keyFrames);
}

void VideoContext::LoadTimecodes(wxString filename) {
	if (filename == ovrTimecodeFile || filename.empty()) return;
	try {
		ovrFPS = agi::vfr::Framerate(STD_STR(filename));
		ovrTimecodeFile = filename;
		config::mru->Add("Timecodes", STD_STR(filename));
		OnSubtitlesCommit();
		TimecodesOpen(ovrFPS);
	}
	catch (const agi::FileSystemError&) {
		wxLogError("Could not open file " + filename);
		config::mru->Remove("Timecodes", STD_STR(filename));
	}
	catch (const agi::vfr::Error& e) {
		wxLogError("Timecode file parse error: %s", e.GetMessage());
	}
}
void VideoContext::SaveTimecodes(wxString filename) {
	try {
		FPS().Save(STD_STR(filename), IsLoaded() ? GetLength() : -1);
		config::mru->Add("Timecodes", STD_STR(filename));
	}
	catch(const agi::FileSystemError&) {
		wxLogError("Could not write to " + filename);
	}
}
void VideoContext::CloseTimecodes() {
	ovrFPS = agi::vfr::Framerate();
	ovrTimecodeFile.clear();
	OnSubtitlesCommit();
	TimecodesOpen(videoFPS);
}

int VideoContext::TimeAtFrame(int frame, agi::vfr::Time type) const {
	if (ovrFPS.IsLoaded()) {
		return ovrFPS.TimeAtFrame(frame, type);
	}
	return videoFPS.TimeAtFrame(frame, type);
}
int VideoContext::FrameAtTime(int time, agi::vfr::Time type) const {
	if (ovrFPS.IsLoaded()) {
		return ovrFPS.FrameAtTime(time, type);
	}
	return videoFPS.FrameAtTime(time, type);
}

void VideoContext::OnVideoError(VideoProviderErrorEvent const& err) {
	wxLogError(
		"Failed seeking video. The video file may be corrupt or incomplete.\n"
		"Error message reported: %s",
		lagi_wxString(err.GetMessage()));
}
void VideoContext::OnSubtitlesError(SubtitlesProviderErrorEvent const& err) {
	wxLogError(
		"Failed rendering subtitles. Error message reported: %s",
		lagi_wxString(err.GetMessage()));
}

void VideoContext::OnExit() {
	// On unix wxThreadModule will shut down any still-running threads (and
	// display a warning that it's doing so) before the destructor for
	// VideoContext runs, so manually kill the thread
	Get()->provider.reset();
}
