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

#include "video_context.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "audio_controller.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "include/aegisub/video_provider.h"
#include "mkv_wrap.h"
#include "options.h"
#include "selection_controller.h"
#include "subs_controller.h"
#include "time_range.h"
#include "threaded_frame_source.h"
#include "utils.h"
#include "video_frame.h"

#include <libaegisub/fs.h>
#include <libaegisub/keyframe.h>
#include <libaegisub/path.h>

#include <wx/msgdlg.h>

VideoContext::VideoContext()
: playback(this)
, start_ms(0)
, end_frame(0)
, frame_n(0)
, ar_value(1.)
, ar_type(AspectRatio::Default)
, has_subtitles(false)
, playAudioOnStep(OPT_GET("Audio/Plays When Stepping Video"))
, no_amend(false)
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
	config::path->SetToken("?video", "");

	// Remove video data
	Stop();
	frame_n = 0;

	// Clean up video data
	video_filename.clear();

	// Remove provider
	provider.reset();
	video_provider = nullptr;

	keyframes.clear();
	keyframes_filename.clear();
	video_fps = agi::vfr::Framerate();
	KeyframesOpen(keyframes);
	if (!ovr_fps.IsLoaded()) TimecodesOpen(video_fps);
}

void VideoContext::SetContext(agi::Context *context) {
	this->context = context;
	context->ass->AddCommitListener(&VideoContext::OnSubtitlesCommit, this);
	context->subsController->AddFileSaveListener(&VideoContext::OnSubtitlesSave, this);
}

void VideoContext::SetVideo(const agi::fs::path &filename) {
	Reset();
	if (filename.empty()) {
		VideoOpen();
		return;
	}

	bool commit_subs = false;
	try {
		provider.reset(new ThreadedFrameSource(filename, this));
		video_provider = provider->GetVideoProvider();
		video_filename = filename;

		// Check that the script resolution matches the video resolution
		int sx = context->ass->GetScriptInfoAsInt("PlayResX");
		int sy = context->ass->GetScriptInfoAsInt("PlayResY");
		int vx = GetWidth();
		int vy = GetHeight();

		// If the script resolution hasn't been set at all just force it to the
		// video resolution
		if (sx == 0 && sy == 0) {
			context->ass->SetScriptInfo("PlayResX", std::to_string(vx));
			context->ass->SetScriptInfo("PlayResY", std::to_string(vy));
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
				context->ass->SetScriptInfo("PlayResX", std::to_string(vx));
				context->ass->SetScriptInfo("PlayResY", std::to_string(vy));
				commit_subs = true;
				break;
			default: // Never change
				break;
			}
		}

		keyframes = video_provider->GetKeyFrames();

		// Set frame rate
		video_fps = video_provider->GetFPS();
		if (ovr_fps.IsLoaded()) {
			int ovr = wxMessageBox(_("You already have timecodes loaded. Would you like to replace them with timecodes from the video file?"), _("Replace timecodes?"), wxYES_NO | wxICON_QUESTION);
			if (ovr == wxYES) {
				ovr_fps = agi::vfr::Framerate();
				timecodes_filename.clear();
			}
		}

		// Set aspect ratio
		double dar = video_provider->GetDAR();
		if (dar > 0)
			SetAspectRatio(dar);

		// Set filename
		config::mru->Add("Video", filename);
		config::path->SetToken("?video", filename);

		// Show warning
		std::string warning = video_provider->GetWarning();
		if (!warning.empty())
			wxMessageBox(to_wx(warning), "Warning", wxICON_WARNING | wxOK);

		has_subtitles = false;
		if (agi::fs::HasExtension(filename, "mkv"))
			has_subtitles = MatroskaWrapper::HasSubtitles(filename);

		provider->LoadSubtitles(context->ass);
		VideoOpen();
		KeyframesOpen(keyframes);
		TimecodesOpen(FPS());
	}
	catch (agi::UserCancelException const&) { }
	catch (agi::fs::FileSystemError const& err) {
		config::mru->Remove("Video", filename);
		wxMessageBox(to_wx(err.GetMessage()), "Error setting video", wxOK | wxICON_ERROR | wxCENTER);
	}
	catch (VideoProviderError const& err) {
		wxMessageBox(to_wx(err.GetMessage()), "Error setting video", wxOK | wxICON_ERROR | wxCENTER);
	}

	if (commit_subs)
		context->ass->Commit(_("change script resolution"), AssFile::COMMIT_SCRIPTINFO);
	else
		JumpToFrame(0);
}

void VideoContext::Reload() {
	if (IsLoaded()) {
		int frame = frame_n;
		SetVideo(agi::fs::path(video_filename)); // explicitly copy videoFile since it's cleared in SetVideo
		JumpToFrame(frame);
	}
}

void VideoContext::OnSubtitlesCommit(int type, std::set<const AssEntry *> const& changed) {
	if (!IsLoaded()) return;

	if (changed.empty() || no_amend)
		provider->LoadSubtitles(context->ass);
	else
		provider->UpdateSubtitles(context->ass, changed);
	if (!IsPlaying())
		GetFrameAsync(frame_n);

	no_amend = false;
}

void VideoContext::OnSubtitlesSave() {
	no_amend = true;

	context->ass->SetScriptInfo("VFR File", config::path->MakeRelative(GetTimecodesName(), "?script").generic_string());
	context->ass->SetScriptInfo("Keyframes File", config::path->MakeRelative(GetKeyFramesName(), "?script").generic_string());

	if (!IsLoaded()) {
		context->ass->SetScriptInfo("Video File", "");
		context->ass->SaveUIState("Video Aspect Ratio", "");
		context->ass->SaveUIState("Video Position", "");
		return;
	}

	std::string ar;
	if (ar_type == AspectRatio::Custom)
		ar = "c" + std::to_string(ar_value);
	else
		ar = std::to_string((int)ar_type);

	context->ass->SetScriptInfo("Video File", config::path->MakeRelative(video_filename, "?script").generic_string());
	context->ass->SetScriptInfo("YCbCr Matrix", video_provider->GetColorSpace());
	context->ass->SaveUIState("Video Aspect Ratio", ar);
	context->ass->SaveUIState("Video Position", std::to_string(frame_n));
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
	provider->RequestFrame(n, TimeAtFrame(n));
}

std::shared_ptr<VideoFrame> VideoContext::GetFrame(int n, bool raw) {
	return provider->GetFrame(n, TimeAtFrame(n), raw);
}

int VideoContext::GetWidth() const { return video_provider->GetWidth(); }
int VideoContext::GetHeight() const { return video_provider->GetHeight(); }
int VideoContext::GetLength() const { return video_provider->GetFrameCount(); }

void VideoContext::NextFrame() {
	if (!video_provider || IsPlaying() || frame_n == video_provider->GetFrameCount())
		return;

	JumpToFrame(frame_n + 1);
	if (playAudioOnStep->GetBool())
		context->audioController->PlayRange(TimeRange(TimeAtFrame(frame_n - 1), TimeAtFrame(frame_n)));
}

void VideoContext::PrevFrame() {
	if (!video_provider || IsPlaying() || frame_n == 0)
		return;

	JumpToFrame(frame_n - 1);
	if (playAudioOnStep->GetBool())
		context->audioController->PlayRange(TimeRange(TimeAtFrame(frame_n), TimeAtFrame(frame_n + 1)));
}

void VideoContext::Play() {
	if (IsPlaying()) {
		Stop();
		return;
	}

	if (!IsLoaded()) return;

	start_ms = TimeAtFrame(frame_n);
	end_frame = GetLength() - 1;

	context->audioController->PlayToEnd(start_ms);

	playback_start_time = std::chrono::steady_clock::now();
	playback.Start(10);
}

void VideoContext::PlayLine() {
	Stop();

	AssDialogue *curline = context->selectionController->GetActiveLine();
	if (!curline) return;

	context->audioController->PlayRange(TimeRange(curline->Start, curline->End));

	// Round-trip conversion to convert start to exact
	int startFrame = FrameAtTime(context->selectionController->GetActiveLine()->Start, agi::vfr::START);
	start_ms = TimeAtFrame(startFrame);
	end_frame = FrameAtTime(context->selectionController->GetActiveLine()->End, agi::vfr::END) + 1;

	JumpToFrame(startFrame);

	playback_start_time = std::chrono::steady_clock::now();
	playback.Start(10);
}

void VideoContext::Stop() {
	if (IsPlaying()) {
		playback.Stop();
		context->audioController->Stop();
	}
}

void VideoContext::OnPlayTimer(wxTimerEvent &) {
	using namespace std::chrono;
	int next_frame = FrameAtTime(start_ms + duration_cast<milliseconds>(steady_clock::now() - playback_start_time).count());
	if (next_frame == frame_n) return;

	if (next_frame >= end_frame)
		Stop();
	else {
		frame_n = next_frame;
		GetFrameAsync(frame_n);
		Seek(frame_n);
	}
}

double VideoContext::GetARFromType(AspectRatio type) const {
	switch (type) {
		case AspectRatio::Default: return (double)GetWidth()/(double)GetHeight();
		case AspectRatio::Fullscreen: return 4.0/3.0;
		case AspectRatio::Widescreen: return 16.0/9.0;
		case AspectRatio::Cinematic: return 2.35;
	}
	throw agi::InternalError("Bad AR type", nullptr);
}

void VideoContext::SetAspectRatio(double value) {
	ar_type = AspectRatio::Custom;
	ar_value = mid(.5, value, 5.);
	ARChange(ar_type, ar_value);
}

void VideoContext::SetAspectRatio(AspectRatio type) {
	ar_value = mid(.5, GetARFromType(type), 5.);
	ar_type = type;
	ARChange(ar_type, ar_value);
}

void VideoContext::LoadKeyframes(agi::fs::path const& filename) {
	if (filename == keyframes_filename || filename.empty()) return;
	try {
		keyframes = agi::keyframe::Load(filename);
		keyframes_filename = filename;
		KeyframesOpen(keyframes);
		config::mru->Add("Keyframes", filename);
	}
	catch (agi::keyframe::Error const& err) {
		wxMessageBox(to_wx(err.GetMessage()), "Error opening keyframes file", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		config::mru->Remove("Keyframes", filename);
	}
	catch (agi::fs::FileSystemError const& err) {
		wxMessageBox(to_wx(err.GetMessage()), "Error opening keyframes file", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		config::mru->Remove("Keyframes", filename);
	}
}

void VideoContext::SaveKeyframes(agi::fs::path const& filename) {
	agi::keyframe::Save(filename, GetKeyFrames());
	config::mru->Add("Keyframes", filename);
}

void VideoContext::CloseKeyframes() {
	keyframes_filename.clear();
	if (video_provider)
		keyframes = video_provider->GetKeyFrames();
	else
		keyframes.clear();
	KeyframesOpen(keyframes);
}

void VideoContext::LoadTimecodes(agi::fs::path const& filename) {
	if (filename == timecodes_filename || filename.empty()) return;
	try {
		ovr_fps = agi::vfr::Framerate(filename);
		timecodes_filename = filename;
		config::mru->Add("Timecodes", filename);
		OnSubtitlesCommit(0, std::set<const AssEntry*>());
		TimecodesOpen(ovr_fps);
	}
	catch (agi::fs::FileSystemError const& err) {
		wxMessageBox(to_wx(err.GetMessage()), "Error opening timecodes file", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		config::mru->Remove("Timecodes", filename);
	}
	catch (const agi::vfr::Error& e) {
		wxLogError("Timecode file parse error: %s", to_wx(e.GetMessage()));
		config::mru->Remove("Timecodes", filename);
	}
}
void VideoContext::SaveTimecodes(agi::fs::path const& filename) {
	try {
		FPS().Save(filename, IsLoaded() ? GetLength() : -1);
		config::mru->Add("Timecodes", filename);
	}
	catch (agi::fs::FileSystemError const& err) {
		wxMessageBox(to_wx(err.GetMessage()), "Error saving timecodes", wxOK | wxICON_ERROR | wxCENTER, context->parent);
	}
}
void VideoContext::CloseTimecodes() {
	ovr_fps = agi::vfr::Framerate();
	timecodes_filename.clear();
	OnSubtitlesCommit(0, std::set<const AssEntry*>());
	TimecodesOpen(video_fps);
}

int VideoContext::TimeAtFrame(int frame, agi::vfr::Time type) const {
	return (ovr_fps.IsLoaded() ? ovr_fps : video_fps).TimeAtFrame(frame, type);
}

int VideoContext::FrameAtTime(int time, agi::vfr::Time type) const {
	return (ovr_fps.IsLoaded() ? ovr_fps : video_fps).FrameAtTime(time, type);
}

void VideoContext::OnVideoError(VideoProviderErrorEvent const& err) {
	wxLogError(
		"Failed seeking video. The video file may be corrupt or incomplete.\n"
		"Error message reported: %s",
		to_wx(err.GetMessage()));
}
void VideoContext::OnSubtitlesError(SubtitlesProviderErrorEvent const& err) {
	wxLogError(
		"Failed rendering subtitles. Error message reported: %s",
		to_wx(err.GetMessage()));
}
