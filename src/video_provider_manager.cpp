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

#include "config.h"

#include "video_provider_manager.h"

#include "include/aegisub/video_provider.h"
#include "options.h"

#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/util.h>

std::unique_ptr<VideoProvider> CreateDummyVideoProvider(agi::fs::path const&, std::string const&);
std::unique_ptr<VideoProvider> CreateYUV4MPEGVideoProvider(agi::fs::path const&, std::string const&);
std::unique_ptr<VideoProvider> CreateFFmpegSourceVideoProvider(agi::fs::path const&, std::string const&);
std::unique_ptr<VideoProvider> CreateAvisynthVideoProvider(agi::fs::path const&, std::string const&);

std::unique_ptr<VideoProvider> CreateCacheVideoProvider(std::unique_ptr<VideoProvider>);

namespace {
	using factory_fn = std::unique_ptr<VideoProvider> (*)(agi::fs::path const&, std::string const&);
	struct factory {
		const char *name;
		factory_fn create;
		bool hidden;
	};

	const factory providers[] = {
		{"Dummy", CreateDummyVideoProvider, true},
		{"YUV4MPEG", CreateYUV4MPEGVideoProvider, true},
#ifdef WITH_FFMS2
		{"FFmpegSource", CreateFFmpegSourceVideoProvider, false},
#endif
#ifdef WITH_AVISYNTH
		{"Avisynth", CreateAvisynthVideoProvider, false},
#endif
	};
}

std::vector<std::string> VideoProviderFactory::GetClasses() {
	std::vector<std::string> list;
	for (auto const& provider : providers) {
		if (!provider.hidden)
			list.push_back(provider.name);
	}
	return list;
}

std::unique_ptr<VideoProvider> VideoProviderFactory::GetProvider(agi::fs::path const& filename, std::string const& colormatrix) {
	auto preferred = OPT_GET("Video/Provider")->GetString();
	std::vector<const factory *> sorted;
	auto preferred_insertion_point = sorted.end();
	for (auto const& provider : providers) {
		if (provider.hidden)
			sorted.push_back(&provider);
		else if (preferred_insertion_point == sorted.end()) {
			sorted.push_back(&provider);
			preferred_insertion_point = prev(sorted.end());
		}
		else if (preferred == provider.name)
			sorted.insert(preferred_insertion_point, &provider);
		else
			sorted.push_back(&provider);
	}

	bool found = false;
	bool supported = false;
	std::string errors;
	errors.reserve(1024);

	for (auto factory : sorted) {
		std::string err;
		try {
			auto provider = factory->create(filename, colormatrix);
			if (!provider) continue;
			LOG_I("manager/video/provider") << factory->name << ": opened " << filename;
			return provider->WantsCaching() ? CreateCacheVideoProvider(std::move(provider)) : std::move(provider);
		}
		catch (agi::fs::FileNotFound const&) {
			err = "file not found.";
			// Keep trying other providers as this one may just not be able to
			// open a valid path
		}
		catch (VideoNotSupported const&) {
			found = true;
			err = "video is not in a supported format.";
		}
		catch (VideoOpenError const& ex) {
			supported = true;
			err = ex.GetMessage();
		}
		catch (agi::vfr::Error const& ex) {
			supported = true;
			err = ex.GetMessage();
		}

		errors += std::string(factory->name) + ": " + err + "\n";
		LOG_D("manager/video/provider") << factory->name << ": " << err;
	}

	// No provider could open the file
	LOG_E("manager/video/provider") << "Could not open " << filename;
	std::string msg = "Could not open " + filename.string() + ":\n" + errors;

	if (!found) throw agi::fs::FileNotFound(filename.string());
	if (!supported) throw VideoNotSupported(msg);
	throw VideoOpenError(msg);
}
