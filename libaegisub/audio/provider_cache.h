// Aegisub Project http://www.aegisub.org/

#pragma once

#include "libaegisub/audio/provider.h"

#include <thread>

namespace agi {
/// Single decoder thread shared by the RAM and disk caches.
class CacheAudioProvider : public AudioProviderWrapper {
protected:
	static constexpr int64_t chunk_samples = 65536;
	const int sample_bytes = bytes_per_sample * channels;

	explicit CacheAudioProvider(std::unique_ptr<AudioProvider> src);
	void StartDecoder();
	void StopDecoder();

private:
	std::atomic<bool> cancelled{false};
	std::thread decoder;

	void Decode();
	void FillBuffer(void *buf, int64_t start, int64_t count) const override;

	virtual void ReadCache(void *buf, int64_t start, int64_t count) const = 0;
	virtual void *WriteCache(int64_t start, int64_t count) = 0;
};
}
