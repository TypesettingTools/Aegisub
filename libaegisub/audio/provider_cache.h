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
	// Each chunk is written once, then published to concurrent readers.
	std::vector<std::atomic<bool>> ready;
	mutable std::atomic<int64_t> requested_chunk{-1};
	std::atomic<bool> cancelled{false};
	std::atomic<bool> filling{true};
	std::thread decoder;

	void Decode();
	bool DecodeChunk(int64_t chunk, void *discard = nullptr);
	void RetrySequentially();
	void FillBuffer(void *buf, int64_t start, int64_t count) const override;

	virtual void ReadCache(void *buf, int64_t start, int64_t count) const = 0;
	virtual void *WriteCache(int64_t start, int64_t count) = 0;

public:
	bool IsAudioReady(int64_t start, int64_t end) const override;
	void RequestAudio(int64_t start) const override;
	bool IsAudioCacheFilling() const override { return filling; }
};
}
