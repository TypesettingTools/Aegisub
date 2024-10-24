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

#include "video_provider_manager.h"

#include "factory_manager.h"
#include "include/aegisub/video_provider.h"
#include "options.h"

#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/string.h>
#include <libaegisub/split.h>

std::unique_ptr<VideoProvider> CreateDummyVideoProvider(agi::fs::path const&, std::string_view, agi::BackgroundRunner *);
std::unique_ptr<VideoProvider> CreateYUV4MPEGVideoProvider(agi::fs::path const&, std::string_view, agi::BackgroundRunner *);
std::unique_ptr<VideoProvider> CreateFFmpegSourceVideoProvider(agi::fs::path const&, std::string_view, agi::BackgroundRunner *);
std::unique_ptr<VideoProvider> CreateAvisynthVideoProvider(agi::fs::path const&, std::string_view, agi::BackgroundRunner *);

std::unique_ptr<VideoProvider> CreateCacheVideoProvider(std::unique_ptr<VideoProvider>);

namespace ColorMatrix {

std::string colormatrix_description(int cs, int cr) {
	// Assuming TV for unspecified
	std::string str = cr == AGI_CR_JPEG ? "PC" : "TV";

	switch (cs) {
		case AGI_CS_RGB:
			return "None";
		case AGI_CS_BT709:
			return str + ".709";
		case AGI_CS_FCC:
			return str + ".FCC";
		case AGI_CS_BT470BG:
		case AGI_CS_SMPTE170M:
			return str + ".601";
		case AGI_CS_SMPTE240M:
			return str + ".240M";
		default:
			return "";
	}
}

std::pair<int, int> parse_colormatrix(std::string_view matrix) {
	int cs = AGI_CS_UNSPECIFIED;
	int cr = AGI_CR_UNSPECIFIED;

	std::vector<std::string> parts;
	agi::Split(parts, matrix, '.');
	if (parts.size() == 2) {
		if (parts[0] == "TV") {
			cr = AGI_CR_MPEG;
		} else if (parts[0] == "PC") {
			cr = AGI_CR_JPEG;
		}

		if (parts[1] == "709") {
			cs = AGI_CS_BT709;
		} else if (parts[1] == "601") {
			cs = AGI_CS_BT470BG;
		} else if (parts[1] == "FCC") {
			cs = AGI_CS_FCC;
		} else if (parts[1] == "240M") {
			cs = AGI_CS_SMPTE240M;
		}
	}

	return std::make_pair(cs, cr);
}

void guess_colorspace(int &CS, int &CR, int Width, int Height) {
	if (CS == AGI_CS_UNSPECIFIED)
		CS = Width > 1024 || Height >= 600 ? AGI_CS_BT709 : AGI_CS_BT470BG;
	if (CR != AGI_CR_MPEG)
		CR = AGI_CR_MPEG;
}

void override_colormatrix(int &CS, int &CR, std::string_view matrix, int Width, int Height) {
	guess_colorspace(CS, CR, Width, Height);
	auto [oCS, oCR] = parse_colormatrix(matrix);
	if (oCS != AGI_CS_UNSPECIFIED && oCR != AGI_CR_UNSPECIFIED) {
		CS = oCS;
		CR = oCR;
	}
}

}

namespace {
	struct factory {
		const char *name;
		std::unique_ptr<VideoProvider> (*create)(agi::fs::path const&, std::string_view, agi::BackgroundRunner *);
		bool hidden;
	};

	const std::initializer_list<factory> providers = {
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
	return ::GetClasses(providers);
}

std::unique_ptr<VideoProvider> VideoProviderFactory::GetProvider(agi::fs::path const& filename, std::string_view colormatrix, agi::BackgroundRunner *br) {
	auto preferred = OPT_GET("Video/Provider")->GetString();
	auto sorted = GetSorted(providers, preferred);

	bool found = false;
	bool supported = false;
	std::string errors;
	errors.reserve(1024);

	for (auto factory : sorted) {
		std::string err;
		try {
			auto provider = factory->create(filename, colormatrix, br);
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

		agi::AppendStr(errors, factory->name, ": ", err, "\n");
		LOG_D("manager/video/provider") << factory->name << ": " << err;
	}

	// No provider could open the file
	LOG_E("manager/video/provider") << "Could not open " << filename;
	std::string msg = "Could not open " + filename.string() + ":\n" + errors;

	if (!found) throw agi::fs::FileNotFound(filename.string());
	if (!supported) throw VideoNotSupported(msg);
	throw VideoOpenError(msg);
}
