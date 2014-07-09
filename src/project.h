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

#include <libaegisub/fs_fwd.h>
#include <libaegisub/signal.h>
#include <libaegisub/vfr.h>

#include <boost/filesystem/path.hpp>
#include <memory>
#include <vector>

class AsyncVideoProvider;
class DialogProgress;
class wxString;
namespace agi { class AudioProvider; }
namespace agi { struct Context; }
struct ProjectProperties;

class Project {
	std::unique_ptr<agi::AudioProvider> audio_provider;
	std::unique_ptr<AsyncVideoProvider> video_provider;
	agi::vfr::Framerate timecodes;
	std::vector<int> keyframes;

	agi::fs::path audio_file;
	agi::fs::path video_file;
	agi::fs::path timecodes_file;
	agi::fs::path keyframes_file;

	agi::signal::Signal<agi::AudioProvider *> AnnounceAudioProviderModified;
	agi::signal::Signal<AsyncVideoProvider *> AnnounceVideoProviderModified;
	agi::signal::Signal<agi::vfr::Framerate const&> AnnounceTimecodesModified;
	agi::signal::Signal<std::vector<int> const&> AnnounceKeyframesModified;

	bool video_has_subtitles = false;
	DialogProgress *progress = nullptr;
	agi::Context *context = nullptr;

	void ShowError(wxString const& message);
	void ShowError(std::string const& message);

	bool DoLoadSubtitles(agi::fs::path const& path, std::string encoding, ProjectProperties &properties);
	void DoLoadAudio(agi::fs::path const& path, bool quiet);
	bool DoLoadVideo(agi::fs::path const& path);
	void DoLoadTimecodes(agi::fs::path const& path);
	void DoLoadKeyframes(agi::fs::path const& path);

	void LoadUnloadFiles(ProjectProperties properties);
	void UpdateRelativePaths();
	void ReloadAudio();
	void ReloadVideo();

	void SetPath(agi::fs::path& var, const char *token, const char *mru, agi::fs::path const& value);

public:
	Project(agi::Context *context);
	~Project();

	void LoadSubtitles(agi::fs::path path, std::string encoding="");
	void CloseSubtitles();
	bool CanLoadSubtitlesFromVideo() const { return video_has_subtitles; }

	void LoadAudio(agi::fs::path path);
	void CloseAudio();
	agi::AudioProvider *AudioProvider() const { return audio_provider.get(); }
	agi::fs::path const& AudioName() const { return audio_file; }

	void LoadVideo(agi::fs::path path);
	void CloseVideo();
	AsyncVideoProvider *VideoProvider() const { return video_provider.get(); }
	agi::fs::path const& VideoName() const { return video_file; }

	void LoadTimecodes(agi::fs::path path);
	void CloseTimecodes();
	bool CanCloseTimecodes() const { return !timecodes_file.empty(); }
	agi::vfr::Framerate const& Timecodes() const { return timecodes; }

	void LoadKeyframes(agi::fs::path path);
	void CloseKeyframes();
	bool CanCloseKeyframes() const { return !keyframes_file.empty(); }
	std::vector<int> const& Keyframes() const { return keyframes; }

	void LoadList(std::vector<agi::fs::path> const& files);

	DEFINE_SIGNAL_ADDERS(AnnounceAudioProviderModified, AddAudioProviderListener)
	DEFINE_SIGNAL_ADDERS(AnnounceVideoProviderModified, AddVideoProviderListener)
	DEFINE_SIGNAL_ADDERS(AnnounceTimecodesModified, AddTimecodesListener)
	DEFINE_SIGNAL_ADDERS(AnnounceKeyframesModified, AddKeyframesListener)
};
