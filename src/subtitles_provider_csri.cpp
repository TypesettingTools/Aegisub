// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file subtitles_provider_csri.cpp
/// @brief Wrapper for CSRI-based subtitle renderers
/// @ingroup subtitle_rendering
///
/// CSRI renderers (in practice, VSFilter) aren't trusted with untrusted
/// subtitle files, so they're run in a sandboxed aegisub-csri-host process
/// rather than being loaded into Aegisub. See csri_host/client.cpp.

#ifdef WITH_CSRI
#include "subtitles_provider_csri.h"

#include "csri_host/client.h"
#include "include/aegisub/subtitles_provider.h"
#include "video_frame.h"

#include <libaegisub/exception.h>
#include <libaegisub/log.h>

#include <atomic>
#include <mutex>

namespace {
DEFINE_EXCEPTION(CSRIHostError, agi::Exception);

/// Number of times a renderer host has crashed, hung or misbehaved this
/// session. Restarts are capped so that an exploit which only works some of
/// the time can't simply keep retrying.
std::atomic<int> host_failures{0};
constexpr int max_host_failures = 3;

constexpr const char *disabled_message = "The CSRI subtitle renderer has failed too many times and has been disabled until Aegisub is restarted";

class CSRISubtitlesProvider final : public SubtitlesProvider {
	std::string renderer;
	/// Null if the host failed and hasn't been restarted yet
	std::unique_ptr<csri_host::Host> host;
	/// The most recently loaded script, to load into a restarted host
	std::vector<char> script;
	/// Frames are requested from both the video thread and the main thread
	std::mutex mutex;

	void StartHost();
	template<typename Func> void WithHost(Func&& func);
	void LoadSubtitles(const char *data, size_t len) override;

public:
	CSRISubtitlesProvider(std::string renderer);

	void DrawSubtitles(VideoFrame &dst, double time) override;
};

CSRISubtitlesProvider::CSRISubtitlesProvider(std::string renderer)
: renderer(std::move(renderer))
{
	if (host_failures >= max_host_failures) {
		LOG_E("subtitle/provider/csri") << disabled_message;
		throw CSRIHostError(disabled_message);
	}
	std::lock_guard<std::mutex> lock(mutex);
	WithHost([](csri_host::Host&) { });
}

void CSRISubtitlesProvider::StartHost() {
	auto new_host = std::make_unique<csri_host::Host>();
	if (!new_host->OpenRenderer(renderer))
		throw agi::InternalError("CSRI renderer vanished between initial list and creation?");
	if (!script.empty())
		new_host->LoadSubtitles(script.data(), script.size());
	host = std::move(new_host);
}

template<typename Func>
void CSRISubtitlesProvider::WithHost(Func&& func) {
	try {
		if (!host) {
			// The failure which used up the last restart has already been
			// reported, so just stop rendering
			if (host_failures >= max_host_failures) return;
			StartHost();
		}
		func(*host);
	}
	catch (csri_host::Error const& err) {
		host.reset();
		LOG_E("subtitle/provider/csri") << err.what();
		if (++host_failures >= max_host_failures)
			throw CSRIHostError(std::string(err.what()) + ".\n\n" + disabled_message + ".");
		throw CSRIHostError(std::string(err.what()) + ".\n\nThe CSRI subtitle renderer will be restarted.");
	}
}

void CSRISubtitlesProvider::LoadSubtitles(const char *data, size_t len) {
	std::lock_guard<std::mutex> lock(mutex);
	script.assign(data, data + len);
	WithHost([&](csri_host::Host& h) { h.LoadSubtitles(script.data(), script.size()); });
}

void CSRISubtitlesProvider::DrawSubtitles(VideoFrame &dst, double time) {
	if (dst.data.size() < size_t(dst.pitch) * dst.height) return;

	std::lock_guard<std::mutex> lock(mutex);
	WithHost([&](csri_host::Host& h) {
		// Add 1e-9 to guard against floating point imprecision errors on int -> float -> *1000 -> int round trips
		h.Render(dst.data.data(), dst.width, dst.height, dst.pitch, dst.flipped, time + 1e-9);
	});
}
}

namespace csri {
std::vector<std::string> List() {
	std::vector<std::string> final;
	try {
		csri_host::Host host;
		for (auto const& name : host.ListRenderers()) {
			if (name.find("aegisub") != name.npos)
				final.insert(final.begin(), name);
			else
				final.push_back(name);
		}
	}
	catch (csri_host::Error const& err) {
		LOG_E("subtitle/provider/csri") << "Could not list CSRI renderers: " << err.what();
	}
	return final;
}

std::unique_ptr<SubtitlesProvider> Create(std::string const& name, agi::BackgroundRunner *) {
	return std::make_unique<CSRISubtitlesProvider>(name);
}
}
#endif // WITH_CSRI
