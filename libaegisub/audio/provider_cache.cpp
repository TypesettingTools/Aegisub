// Aegisub Project http://www.aegisub.org/

#include "provider_cache.h"

#include <libaegisub/log.h>

#include <algorithm>
#include <iterator>

namespace agi {
CacheAudioProvider::CacheAudioProvider(std::unique_ptr<AudioProvider> src)
: AudioProviderWrapper(std::move(src))
, ready((num_samples + chunk_samples - 1) / chunk_samples)
{
	decoded_samples = 0;
}

void CacheAudioProvider::StartDecoder() {
	decoder = std::thread(&CacheAudioProvider::Decode, this);
}

void CacheAudioProvider::StopDecoder() {
	cancelled = true;
	decoder.join();
}

void CacheAudioProvider::Decode() {
	const auto chunk_count = std::ssize(ready);
	int64_t first_missing = 0;
	int64_t chunk = 0;
	while (first_missing < chunk_count && !cancelled) {
		auto target = requested_chunk.exchange(-1, std::memory_order_relaxed);
		if (target >= 0) {
			target = std::max(target, first_missing);
			while (target < chunk_count && ready[target].load(std::memory_order_relaxed))
				++target;
			if (target < chunk_count)
				chunk = target;
		}
		// Continue forward until we reach another filled region or EOF.
		if (chunk == chunk_count || ready[chunk].load(std::memory_order_relaxed))
			chunk = first_missing;

		if (!DecodeChunk(chunk)) {
			if (!cancelled)
				RetrySequentially();
			break;
		}

		++chunk;
		while (first_missing < chunk_count && ready[first_missing].load(std::memory_order_relaxed))
			++first_missing;
	}
	decode_position = -1;
}

bool CacheAudioProvider::DecodeChunk(int64_t chunk, void *discard) try {
	const auto start = chunk * chunk_samples;
	const auto count = std::min(chunk_samples, num_samples - start);
	decode_position = start;
	source->GetAudio(discard ? discard : WriteCache(start, count), start, count);
	if (!discard) {
		ready[chunk].store(true, std::memory_order_release);
		decoded_samples += count;
	}
	return true;
}
catch (Exception const& e) {
	LOG_E("audio_provider") << e.GetMessage();
	return false;
}
catch (...) {
	LOG_E("audio_provider") << "Unknown audio cache fill error";
	return false;
}

void CacheAudioProvider::RetrySequentially() try {
	// Reopen the source. Decode through ready chunks to advance the decoder
	// without rewriting samples already published to cache readers.
	decode_position = 0;
	source->ResetAudioDecoder();
	auto discard = std::make_unique_for_overwrite<char[]>(chunk_samples * sample_bytes);
	for (int64_t chunk = 0; !cancelled && decoded_samples < num_samples; ++chunk) {
		if (!DecodeChunk(chunk, ready[chunk].load(std::memory_order_relaxed) ? discard.get() : nullptr))
			break;
	}
}
catch (Exception const& e) {
	LOG_E("audio_provider") << e.GetMessage();
}
catch (...) {
	LOG_E("audio_provider") << "Unknown error restarting the audio cache";
}

bool CacheAudioProvider::IsAudioReady(int64_t start, int64_t end) const {
	start = std::max<int64_t>(start, 0);
	end = std::min(end, num_samples);
	if (start >= end || GetDecodedSamples() == num_samples)
		return true;

	for (auto chunk = start / chunk_samples; chunk <= (end - 1) / chunk_samples; ++chunk) {
		if (!ready[chunk].load(std::memory_order_acquire))
			return false;
	}
	return true;
}

std::vector<AudioProvider::SampleRange> CacheAudioProvider::GetCachedSampleRanges() const {
	if (num_samples > 0 && GetDecodedSamples() == num_samples)
		return {{0, num_samples}};

	std::vector<SampleRange> ranges;
	const auto chunk_count = std::ssize(ready);
	for (int64_t first = 0; first < chunk_count; ) {
		if (!ready[first].load(std::memory_order_acquire)) {
			++first;
			continue;
		}

		auto end = first + 1;
		while (end < chunk_count && ready[end].load(std::memory_order_acquire))
			++end;
		ranges.emplace_back(first * chunk_samples, std::min(num_samples, end * chunk_samples));
		first = end;
	}
	return ranges;
}

void CacheAudioProvider::RequestAudio(int64_t start) const {
	if (!SupportsCacheSeeking()) return;

	const auto chunk = start < num_samples ? std::max<int64_t>(start, 0) / chunk_samples : -1;
	requested_chunk.store(chunk, std::memory_order_relaxed);
}

void CacheAudioProvider::FillBuffer(void *buf, int64_t start, int64_t count) const {
	auto out = static_cast<char *>(buf);
	while (count > 0) {
		const auto chunk = start / chunk_samples;
		const auto size = std::min(count, chunk_samples - start % chunk_samples);
		if (ready[chunk].load(std::memory_order_acquire))
			ReadCache(out, start, size);
		else
			ZeroFill(out, size);
		start += size;
		count -= size;
		out += size * sample_bytes;
	}
}
}
