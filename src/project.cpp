// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "project.h"

#include "ass_file.h"
#include "async_video_provider.h"
#include "audio_controller.h"
#include "charset_detect.h"
#include "compat.h"
#include "dialog_progress.h"
#include "dialogs.h"
#include "format.h"
#include "include/aegisub/audio_provider.h"
#include "include/aegisub/context.h"
#include "include/aegisub/video_provider.h"
#include "mkv_wrap.h"
#include "options.h"
#include "subs_controller.h"
#include "video_controller.h"
#include "video_display.h"

#include <libaegisub/format_path.h>
#include <libaegisub/fs.h>
#include <libaegisub/keyframe.h>
#include <libaegisub/log.h>
#include <libaegisub/make_unique.h>
#include <libaegisub/path.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>
#include <wx/msgdlg.h>

Project::Project(agi::Context *c) : context(c) {
	OPT_SUB("Audio/Cache/Type", &Project::ReloadAudio, this);
	OPT_SUB("Audio/Provider", &Project::ReloadAudio, this);
	OPT_SUB("Provider/Audio/FFmpegSource/Decode Error Handling", &Project::ReloadAudio, this);
	OPT_SUB("Provider/Avisynth/Allow Ancient", &Project::ReloadVideo, this);
	OPT_SUB("Provider/Avisynth/Memory Max", &Project::ReloadVideo, this);
	OPT_SUB("Provider/Video/FFmpegSource/Decoding Threads", &Project::ReloadVideo, this);
	OPT_SUB("Provider/Video/FFmpegSource/Unsafe Seeking", &Project::ReloadVideo, this);
	OPT_SUB("Subtitle/Provider", &Project::ReloadVideo, this);
	OPT_SUB("Video/Force BT.601", &Project::ReloadVideo, this);
	OPT_SUB("Video/Provider", &Project::ReloadVideo, this);
}

Project::~Project() { }

void Project::UpdateRelativePaths() {
	context->ass->Properties.audio_file     = config::path->MakeRelative(audio_file, "?script").generic_string();
	context->ass->Properties.video_file     = config::path->MakeRelative(video_file, "?script").generic_string();
	context->ass->Properties.timecodes_file = config::path->MakeRelative(timecodes_file, "?script").generic_string();
	context->ass->Properties.keyframes_file = config::path->MakeRelative(keyframes_file, "?script").generic_string();
}

void Project::ReloadAudio() {
	if (audio_provider)
		LoadAudio(audio_file);
}

void Project::ReloadVideo() {
	if (video_provider) {
		DoLoadVideo(video_file);
		context->videoController->JumpToFrame(context->videoController->GetFrameN());
	}
}

void Project::ShowError(wxString const& message) {
	wxMessageBox(message, "Error loading file", wxOK | wxICON_ERROR | wxCENTER, context->parent);
}

void Project::ShowError(std::string const& message) {
	ShowError(to_wx(message));
}

void Project::SetPath(agi::fs::path& var, const char *token, const char *mru, agi::fs::path const& value) {
	var = value;
	if (*token)
		config::path->SetToken(token, value);
	if (*mru)
		config::mru->Add(mru, value);
	UpdateRelativePaths();
}

void Project::DoLoadSubtitles(agi::fs::path const& path, std::string encoding) {
	try {
		if (encoding.empty())
			encoding = CharSetDetect::GetEncoding(path);
	}
	catch (agi::UserCancelException const&) {
		return;
	}
	catch (agi::fs::FileNotFound const&) {
		config::mru->Remove("Subtitle", path);
		return ShowError(path.string() + " not found.");
	}

	if (encoding != "binary") {
		// Try loading as timecodes and keyframes first since we can't
		// distinguish them based on filename alone, and just ignore failures
		// rather than trying to differentiate between malformed timecodes
		// files and things that aren't timecodes files at all
		try { return DoLoadTimecodes(path); } catch (...) { }
		try { return DoLoadKeyframes(path); } catch (...) { }
	}

	try {
		context->subsController->Load(path, encoding);
	}
	catch (agi::UserCancelException const&) { return; }
	catch (agi::fs::FileNotFound const&) {
		config::mru->Remove("Subtitle", path);
		return ShowError(path.string() + " not found.");
	}
	catch (agi::Exception const& e) {
		return ShowError(e.GetMessage());
	}
	catch (std::exception const& e) {
		return ShowError(std::string(e.what()));
	}
	catch (...) {
		return ShowError(wxString("Unknown error"));
	}
}

void Project::LoadSubtitles(agi::fs::path const& path, std::string encoding) {
	DoLoadSubtitles(path, encoding);
	LoadUnloadFiles();
}

void Project::CloseSubtitles() {
	context->subsController->Close();
	config::path->SetToken("?script", "");
	LoadUnloadFiles();
}

void Project::LoadUnloadFiles() {
	auto load_linked = OPT_GET("App/Auto/Load Linked Files")->GetInt();
	if (!load_linked) return;

	auto audio     = config::path->MakeAbsolute(context->ass->Properties.audio_file, "?script");
	auto video     = config::path->MakeAbsolute(context->ass->Properties.video_file, "?script");
	auto timecodes = config::path->MakeAbsolute(context->ass->Properties.timecodes_file, "?script");
	auto keyframes = config::path->MakeAbsolute(context->ass->Properties.keyframes_file, "?script");

	if (video == video_file && audio == audio_file && keyframes == keyframes_file && timecodes == timecodes_file)
		return;

	if (load_linked == 2) {
		wxString str = _("Do you want to load/unload the associated files?");
		str += "\n";

		auto append_file = [&](agi::fs::path const& p, wxString const& unload, wxString const& load) {
			if (p.empty())
				str += "\n" + unload;
			else
				str += "\n" + agi::wxformat(load, p);
		};

		if (audio != audio_file)
			append_file(audio, _("Unload audio"), _("Load audio file: %s"));
		if (video != video_file)
			append_file(video, _("Unload video"), _("Load video file: %s"));
		if (timecodes != timecodes_file)
			append_file(timecodes, _("Unload timecodes"), _("Load timecodes file: %s"));
		if (keyframes != keyframes_file)
			append_file(keyframes, _("Unload keyframes"), _("Load keyframes file: %s"));

		if (wxMessageBox(str, _("(Un)Load files?"), wxYES_NO | wxCENTRE, context->parent) != wxYES)
			return;
	}

	bool loaded_video = false;
	if (video != video_file) {
		if (video.empty())
			CloseVideo();
		else if ((loaded_video = DoLoadVideo(video))) {
			auto vc = context->videoController.get();
			vc->JumpToFrame(context->ass->Properties.video_position);

			auto ar_mode = static_cast<AspectRatio>(context->ass->Properties.ar_mode);
			if (ar_mode == AspectRatio::Custom)
				vc->SetAspectRatio(context->ass->Properties.ar_value);
			else
				vc->SetAspectRatio(ar_mode);
			context->videoDisplay->SetZoom(context->ass->Properties.video_zoom);
		}
	}

	if (!timecodes.empty()) LoadTimecodes(timecodes);
	if (!keyframes.empty()) LoadKeyframes(keyframes);

	if (audio != audio_file) {
		if (audio.empty())
			CloseAudio();
		else
			DoLoadAudio(audio, false);
	}
	else if (loaded_video && OPT_GET("Video/Open Audio")->GetBool() && audio_file != video_file && video_provider->HasAudio())
		DoLoadAudio(video, true);
}

void Project::DoLoadAudio(agi::fs::path const& path, bool quiet) {
	if (!progress)
		progress = new DialogProgress(context->parent);

	try {
		try {
			audio_provider = AudioProviderFactory::GetProvider(path, progress);
		}
		catch (agi::UserCancelException const&) { return; }
		catch (...) {
			config::mru->Remove("Audio", path);
			throw;
		}
	}
	catch (agi::fs::FileNotFound const& e) {
		return ShowError(_("The audio file was not found: ") + to_wx(e.GetMessage()));
	}
	catch (agi::AudioDataNotFoundError const& e) {
		if (quiet) {
			LOG_D("video/open/audio") << "File " << video_file << " has no audio data: " << e.GetMessage();
			return;
		}
		else
			return ShowError(_("None of the available audio providers recognised the selected file as containing audio data.\n\nThe following providers were tried:\n") + to_wx(e.GetMessage()));
	}
	catch (agi::AudioProviderOpenError const& e) {
		return ShowError(_("None of the available audio providers have a codec available to handle the selected file.\n\nThe following providers were tried:\n") + to_wx(e.GetMessage()));
	}
	catch (agi::Exception const& e) {
		return ShowError(e.GetMessage());
	}

	SetPath(audio_file, "?audio", "Audio", path);
	AnnounceAudioProviderModified(audio_provider.get());
}

void Project::LoadAudio(agi::fs::path const& path) {
	DoLoadAudio(path, false);
}

void Project::CloseAudio() {
	AnnounceAudioProviderModified(nullptr);
	audio_provider.reset();
	SetPath(audio_file, "?audio", "", "");
}

bool Project::DoLoadVideo(agi::fs::path const& path) {
	if (!progress)
		progress = new DialogProgress(context->parent);

	try {
		auto old_matrix = context->ass->GetScriptInfo("YCbCr Matrix");
		video_provider = agi::make_unique<AsyncVideoProvider>(path, old_matrix, context->videoController.get(), progress);
	}
	catch (agi::UserCancelException const&) { return false; }
	catch (agi::fs::FileSystemError const& err) {
		config::mru->Remove("Video", path);
		ShowError(to_wx(err.GetMessage()));
		return false;
	}
	catch (VideoProviderError const& err) {
		ShowError(to_wx(err.GetMessage()));
		return false;
	}

	AnnounceVideoProviderModified(video_provider.get());

	UpdateVideoProperties(context->ass.get(), video_provider.get(), context->parent);
	video_provider->LoadSubtitles(context->ass.get());

	timecodes = video_provider->GetFPS();
	keyframes = video_provider->GetKeyFrames();

	timecodes_file.clear();
	keyframes_file.clear();
	SetPath(video_file, "?video", "Video", path);

	std::string warning = video_provider->GetWarning();
	if (!warning.empty())
		wxMessageBox(to_wx(warning), "Warning", wxICON_WARNING | wxOK);

	video_has_subtitles = false;
	if (agi::fs::HasExtension(path, "mkv"))
		video_has_subtitles = MatroskaWrapper::HasSubtitles(path);

	AnnounceKeyframesModified(keyframes);
	AnnounceTimecodesModified(timecodes);
	return true;
}

void Project::LoadVideo(agi::fs::path const& path) {
	if (path.empty()) return;
	if (!DoLoadVideo(path)) return;
	if (OPT_GET("Video/Open Audio")->GetBool() && audio_file != video_file && video_provider->HasAudio())
		DoLoadAudio(video_file, true);

	double dar = video_provider->GetDAR();
	if (dar > 0)
		context->videoController->SetAspectRatio(dar);
	else
		context->videoController->SetAspectRatio(AspectRatio::Default);
	context->videoController->JumpToFrame(0);
}

void Project::CloseVideo() {
	AnnounceVideoProviderModified(nullptr);
	video_provider.reset();
	SetPath(video_file, "?video", "", "");
	video_has_subtitles = false;
	context->ass->Properties.ar_mode = 0;
	context->ass->Properties.ar_value = 0.0;
	context->ass->Properties.video_position = 0;
}

void Project::DoLoadTimecodes(agi::fs::path const& path) {
	timecodes = agi::vfr::Framerate(path);
	SetPath(timecodes_file, "", "Timecodes", path);
	AnnounceTimecodesModified(timecodes);
}

void Project::LoadTimecodes(agi::fs::path const& path) {
	try {
		DoLoadTimecodes(path);
	}
	catch (agi::fs::FileSystemError const& e) {
		ShowError(e.GetMessage());
		config::mru->Remove("Timecodes", path);
	}
	catch (agi::vfr::Error const& e) {
		ShowError("Failed to parse timecodes file: " + e.GetMessage());
		config::mru->Remove("Timecodes", path);
	}
}

void Project::CloseTimecodes() {
	timecodes = video_provider ? video_provider->GetFPS() : agi::vfr::Framerate{};
	SetPath(timecodes_file, "", "", "");
	AnnounceTimecodesModified(timecodes);
}

void Project::DoLoadKeyframes(agi::fs::path const& path) {
	keyframes = agi::keyframe::Load(path);
	SetPath(keyframes_file, "", "Keyframes", path);
	AnnounceKeyframesModified(keyframes);
}

void Project::LoadKeyframes(agi::fs::path const& path) {
	try {
		DoLoadKeyframes(path);
	}
	catch (agi::fs::FileSystemError const& e) {
		ShowError(e.GetMessage());
		config::mru->Remove("Keyframes", path);
	}
	catch (agi::keyframe::Error const& e) {
		ShowError("Failed to parse keyframes file: " + e.GetMessage());
		config::mru->Remove("Keyframes", path);
	}
}

void Project::CloseKeyframes() {
	keyframes = video_provider ? video_provider->GetKeyFrames() : std::vector<int>{};
	SetPath(keyframes_file, "", "", "");
	AnnounceKeyframesModified(keyframes);
}

void Project::LoadList(std::vector<agi::fs::path> const& files) {
	// Keep these lists sorted

	// Video formats
	const char *videoList[] = {
		".asf",
		".avi",
		".avs",
		".d2v",
		".m2ts",
		".m4v",
		".mkv",
		".mov",
		".mp4",
		".mpeg",
		".mpg",
		".ogm",
		".rm",
		".rmvb",
		".ts",
		".webm"
		".wmv",
		".y4m",
		".yuv"
	};

	// Subtitle formats
	const char *subsList[] = {
		".ass",
		".srt",
		".ssa",
		".sub",
		".ttxt"
	};

	// Audio formats
	const char *audioList[] = {
		".aac",
		".ac3",
		".ape",
		".dts",
		".flac",
		".m4a",
		".mka",
		".mp3",
		".ogg",
		".w64",
		".wav",
		".wma"
	};

	auto search = [](const char **begin, const char **end, std::string const& str) {
		return std::binary_search(begin, end, str.c_str(), [](const char *a, const char *b) {
			return strcmp(a, b) < 0;
		});
	};

	agi::fs::path audio, video, subs, timecodes, keyframes;
	for (auto file : files) {
		if (file.is_relative()) file = absolute(file);
		if (!agi::fs::FileExists(file)) continue;

		auto ext = file.extension().string();
		boost::to_lower(ext);

		// Could be subtitles, keyframes or timecodes, so try loading as each
		if (ext == ".txt" || ext == ".log") {
			if (timecodes.empty()) {
				try {
					DoLoadTimecodes(file);
					timecodes = file;
					continue;
				} catch (...) { }
			}

			if (keyframes.empty()) {
				try {
					DoLoadKeyframes(file);
					keyframes = file;
					continue;
				} catch (...) { }
			}

			if (subs.empty() && ext != ".log")
				subs = file;
			continue;
		}

		if (subs.empty() && search(std::begin(subsList), std::end(subsList), ext))
			subs = file;
		if (video.empty() && search(std::begin(videoList), std::end(videoList), ext))
			video = file;
		if (audio.empty() && search(std::begin(audioList), std::end(audioList), ext))
			audio = file;
	}

	if (!subs.empty())
		DoLoadSubtitles(subs);

	// Loading video will clear the audio file script header, so make sure we
	// end up loading the audio if the newly loaded subs has some
	if (!video.empty() && audio.empty() && !context->ass->Properties.audio_file.empty())
		audio = config::path->MakeAbsolute(context->ass->Properties.audio_file, "?script");

	if (!video.empty()) {
		DoLoadVideo(video);

		double dar = video_provider->GetDAR();
		if (dar > 0)
			context->videoController->SetAspectRatio(dar);
		else
			context->videoController->SetAspectRatio(AspectRatio::Default);
		context->videoController->JumpToFrame(0);

		// We loaded these earlier, but loading video unloaded them
		// Non-Do version of Load in case they've vanished or changed between
		// then and now
		if (!timecodes.empty())
			LoadTimecodes(timecodes);
		if (!keyframes.empty())
			LoadKeyframes(keyframes);
	}

	if (!audio.empty())
		DoLoadAudio(audio, false);
	else if (OPT_GET("Video/Open Audio")->GetBool() && audio_file != video_file)
		DoLoadAudio(video_file, true);

	if (!subs.empty())
		LoadUnloadFiles();
}
