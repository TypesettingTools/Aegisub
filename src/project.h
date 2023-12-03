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

#include <libaegisub/signal.h>
#include <libaegisub/vfr.h>

#include <filesystem>
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

	std::filesystem::path audio_file;
	std::filesystem::path video_file;
	std::filesystem::path timecodes_file;
	std::filesystem::path keyframes_file;

	agi::signal::Signal<agi::AudioProvider *> AnnounceAudioProviderModified;
	agi::signal::Signal<AsyncVideoProvider *> AnnounceVideoProviderModified;
	agi::signal::Signal<agi::vfr::Framerate const&> AnnounceTimecodesModified;
	agi::signal::Signal<std::vector<int> const&> AnnounceKeyframesModified;

	bool video_has_subtitles = false;
	DialogProgress *progress = nullptr;
	agi::Context *context = nullptr;

	void ShowError(wxString const& message);
	void ShowError(std::string const& message);

	bool DoLoadSubtitles(std::filesystem::path const& path, std::string encoding, ProjectProperties &properties);
	void DoLoadAudio(std::filesystem::path const& path, bool quiet);
	bool DoLoadVideo(std::filesystem::path const& path);
	void DoLoadTimecodes(std::filesystem::path const& path);
	void DoLoadKeyframes(std::filesystem::path const& path);

	void LoadUnloadFiles(ProjectProperties properties);
	void UpdateRelativePaths();
	void ReloadAudio();
	void ReloadVideo();

	void SetPath(std::filesystem::path& var, const char *token, const char *mru, std::filesystem::path const& value);

public:
	Project(agi::Context *context);
	~Project();

	void LoadSubtitles(std::filesystem::path path, std::string encoding="", bool load_linked=true);
	void CloseSubtitles();
	bool CanLoadSubtitlesFromVideo() const { return video_has_subtitles; }

	void LoadAudio(std::filesystem::path path);
	void CloseAudio();
	agi::AudioProvider *AudioProvider() const { return audio_provider.get(); }
	std::filesystem::path const& AudioName() const { return audio_file; }

	void LoadVideo(std::filesystem::path path);
	void CloseVideo();
	AsyncVideoProvider *VideoProvider() const { return video_provider.get(); }
	std::filesystem::path const& VideoName() const { return video_file; }

	void LoadTimecodes(std::filesystem::path path);
	void CloseTimecodes();
	bool CanCloseTimecodes() const { return !timecodes_file.empty(); }
	agi::vfr::Framerate const& Timecodes() const { return timecodes; }

	void LoadKeyframes(std::filesystem::path path);
	void CloseKeyframes();
	bool CanCloseKeyframes() const { return !keyframes_file.empty(); }
	std::vector<int> const& Keyframes() const { return keyframes; }

	void LoadList(std::vector<std::filesystem::path> const& files);

	DEFINE_SIGNAL_ADDERS(AnnounceAudioProviderModified, AddAudioProviderListener)
	DEFINE_SIGNAL_ADDERS(AnnounceVideoProviderModified, AddVideoProviderListener)
	DEFINE_SIGNAL_ADDERS(AnnounceTimecodesModified, AddTimecodesListener)
	DEFINE_SIGNAL_ADDERS(AnnounceKeyframesModified, AddKeyframesListener)
};
