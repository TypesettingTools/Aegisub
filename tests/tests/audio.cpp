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

#include <main.h>

#include <libaegisub/audio/provider.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>

TEST(lagi_audio, dummy_blank) {
	auto provider = agi::CreateDummyAudioProvider("dummy-audio:", nullptr);

	char buff[1024];
	memset(buff, 1, sizeof(buff));
	provider->GetAudio(buff, 12356, 512);
	for (size_t i = 0; i < sizeof(buff); ++i) ASSERT_EQ(0, buff[i]);
}

TEST(lagi_audio, dummy_noise) {
	auto provider = agi::CreateDummyAudioProvider("dummy-audio:noise?", nullptr);

	char buff[1024];
	memset(buff, 0, sizeof(buff));
	provider->GetAudio(buff, 12356, 512);
	for (size_t i = 0; i < sizeof(buff); ++i) {
		if (buff[i] != 0)
			return;
	}
	bool all_zero = true;
	ASSERT_FALSE(all_zero);
}

TEST(lagi_audio, dummy_rejects_non_dummy_url) {
	auto provider = agi::CreateDummyAudioProvider("/tmp", nullptr);
	ASSERT_EQ(nullptr, provider.get());
}

template<typename Sample=uint16_t>
struct TestAudioProvider : agi::AudioProvider {
	int bias = 0;

	TestAudioProvider(int64_t duration = 90, int rate=48000) {
		channels = 1;
		num_samples = duration * 48000;
		decoded_samples = num_samples;
		sample_rate = rate;
		bytes_per_sample = sizeof(Sample);
		float_samples = false;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto out = static_cast<Sample *>(buf);
		for (int64_t end = start + count; start < end; ++start)
			*out++ = (Sample)(start + bias);
	}
};

TEST(lagi_audio, before_sample_zero) {
	TestAudioProvider<> provider;

	uint16_t buff[16];
	memset(buff, 1, sizeof(buff));
	provider.GetAudio(buff, -8, 16);

	for (int i = 0; i < 8; ++i)
		ASSERT_EQ(0, buff[i]);
	for (int i = 8; i < 16; ++i)
		ASSERT_EQ(i - 8, buff[i]);
}

TEST(lagi_audio, before_sample_zero_8bit) {
	TestAudioProvider<uint8_t> provider;
	provider.bias = 128;

	uint8_t buff[16];
	memset(buff, 1, sizeof(buff));
	provider.GetAudio(buff, -8, 16);

	for (int i = 0; i < 8; ++i)
		ASSERT_EQ(128, buff[i]);
	for (int i = 8; i < 16; ++i)
		ASSERT_EQ(128 + i - 8, buff[i]);
}

TEST(lagi_audio, after_end) {
	TestAudioProvider<> provider(1);

	uint16_t buff[16];
	memset(buff, 1, sizeof(buff));
	provider.GetAudio(buff, provider.GetNumSamples() - 8, 16);

	for (int i = 0; i < 8; ++i)
		ASSERT_NE(0, buff[i]);
	for (int i = 8; i < 16; ++i)
		ASSERT_EQ(0, buff[i]);
}

TEST(lagi_audio, save_audio_clip) {
	auto path = agi::Path().Decode("?temp/save_clip");
	agi::fs::Remove(path);

	auto provider = agi::CreateDummyAudioProvider("dummy-audio:noise?", nullptr);
	agi::SaveAudioClip(*provider, path, 60 * 60 * 1000, (60 * 60 + 10) * 1000);

	{
		std::ifstream s(path, std::ios_base::binary);
		ASSERT_TRUE(s.good());
		s.seekg(0, std::ios::end);
		// 10 seconds of 44.1 kHz samples per second of 16-bit mono, plus 44 bytes of header
		EXPECT_EQ(10 * 44100 * 2 + 44, s.tellg());
	}
	agi::fs::Remove(path);
}

TEST(lagi_audio, save_audio_clip_out_of_audio_range) {
	const auto path = agi::Path().Decode("?temp/save_clip");
	agi::fs::Remove(path);

	const auto provider = agi::CreateDummyAudioProvider("dummy-audio:noise?", nullptr);
	const auto end_time = 150 * 60 * 1000;

	// Start time after end of clip: empty file
	agi::SaveAudioClip(*provider, path, end_time, end_time + 1);
	{
		std::ifstream s(path, std::ios_base::binary);
		ASSERT_TRUE(s.good());
		s.seekg(0, std::ios::end);
		EXPECT_EQ(44, s.tellg());
	}
	agi::fs::Remove(path);

	// Start time >= end time: empty file
	agi::SaveAudioClip(*provider, path, end_time - 1, end_time - 1);
	{
		std::ifstream s(path, std::ios_base::binary);
		ASSERT_TRUE(s.good());
		s.seekg(0, std::ios::end);
		EXPECT_EQ(44, s.tellg());
	}
	agi::fs::Remove(path);

	// Start time during clip, end time after end of clip: save only the part that exists
	agi::SaveAudioClip(*provider, path, end_time - 1000, end_time + 1000);
	{
		std::ifstream s(path, std::ios_base::binary);
		ASSERT_TRUE(s.good());
		s.seekg(0, std::ios::end);
		// 1 second of 44.1 kHz samples per second of 16-bit mono, plus 44 bytes of header
		EXPECT_EQ(44100 * 2 + 44, s.tellg());
	}
	agi::fs::Remove(path);
}

TEST(lagi_audio, get_with_volume) {
	TestAudioProvider<> provider;
	uint16_t buff[4];

	provider.GetAudioWithVolume(buff, 0, 4, 1.0);
	EXPECT_EQ(0, buff[0]);
	EXPECT_EQ(1, buff[1]);
	EXPECT_EQ(2, buff[2]);
	EXPECT_EQ(3, buff[3]);

	provider.GetAudioWithVolume(buff, 0, 4, 0.0);
	EXPECT_EQ(0, buff[0]);
	EXPECT_EQ(0, buff[1]);
	EXPECT_EQ(0, buff[2]);
	EXPECT_EQ(0, buff[3]);

	provider.GetAudioWithVolume(buff, 0, 4, 2.0);
	EXPECT_EQ(0, buff[0]);
	EXPECT_EQ(2, buff[1]);
	EXPECT_EQ(4, buff[2]);
	EXPECT_EQ(6, buff[3]);
}

TEST(lagi_audio, volume_should_clamp_rather_than_wrap) {
	TestAudioProvider<> provider;
	uint16_t buff[1];
	provider.GetAudioWithVolume(buff, 30000, 1, 2.0);
	EXPECT_EQ(SHRT_MAX, buff[0]);
}

TEST(lagi_audio, ram_cache) {
	auto provider = agi::CreateRAMAudioProvider(std::make_unique<TestAudioProvider<>>());
	EXPECT_EQ(1, provider->GetChannels());
	EXPECT_EQ(90 * 48000, provider->GetNumSamples());
	EXPECT_EQ(48000, provider->GetSampleRate());
	EXPECT_EQ(2, provider->GetBytesPerSample());
	EXPECT_EQ(false, provider->AreSamplesFloat());
	EXPECT_EQ(false, provider->NeedsCache());
	while (provider->GetDecodedSamples() != provider->GetNumSamples()) agi::util::sleep_for(0);

	uint16_t buff[512];
	provider->GetAudio(buff, (1 << 22) - 256, 512); // Stride two cache blocks

	for (size_t i = 0; i < 512; ++i)
		ASSERT_EQ(static_cast<uint16_t>((1 << 22) - 256 + i), buff[i]);
}

TEST(lagi_audio, hd_cache) {
	auto provider = agi::CreateHDAudioProvider(std::make_unique<TestAudioProvider<>>(), agi::Path().Decode("?temp"));
	while (provider->GetDecodedSamples() != provider->GetNumSamples()) agi::util::sleep_for(0);

	uint16_t buff[512];
	provider->GetAudio(buff, (1 << 22) - 256, 512);

	for (size_t i = 0; i < 512; ++i)
		ASSERT_EQ(static_cast<uint16_t>((1 << 22) - 256 + i), buff[i]);
}

namespace {
using namespace std::chrono_literals;

// Allow the test to seek and read the cache while its source decode is blocked.
class SteppedAudioProvider final : public TestAudioProvider<> {
	mutable std::mutex mutex;
	mutable std::condition_variable cv;
	mutable std::vector<int64_t> reads;
	size_t allowed_reads = 0;
	const bool seekable;

public:
	static constexpr int64_t chunk_samples = 65536;

	explicit SteppedAudioProvider(bool seekable = true) : seekable(seekable) {
		decoded_samples = num_samples = 8 * chunk_samples + 17;
		bias = 3;
	}

	bool SupportsCacheSeeking() const override { return seekable; }

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		{
			std::unique_lock lock(mutex);
			reads.push_back(start);
			const auto read = reads.size();
			cv.notify_all();
			cv.wait_for(lock, 5s, [&] { return allowed_reads >= read; });
		}
		TestAudioProvider<>::FillBuffer(buf, start, count);
	}

	int64_t WaitForRead(size_t read) const {
		std::unique_lock lock(mutex);
		if (!cv.wait_for(lock, 5s, [&] { return reads.size() >= read; }))
			return -1;
		return reads[read - 1];
	}

	void AllowReads(size_t count) {
		std::lock_guard lock(mutex);
		allowed_reads = count;
		cv.notify_all();
	}
};

class AudioCacheSeekTest : public testing::TestWithParam<bool> {
protected:
	std::unique_ptr<agi::AudioProvider> MakeCache(std::unique_ptr<agi::AudioProvider> src) {
		if (GetParam())
			return agi::CreateHDAudioProvider(std::move(src), agi::Path().Decode("?temp"));
		return agi::CreateRAMAudioProvider(std::move(src));
	}
};

TEST_P(AudioCacheSeekTest, PrioritizesLatestSeekAndFillsGaps) {
	auto src = std::make_unique<SteppedAudioProvider>();
	auto decoder = src.get();
	auto cache = MakeCache(std::move(src));
	const auto chunk = SteppedAudioProvider::chunk_samples;

	ASSERT_EQ(0, decoder->WaitForRead(1));
	cache->RequestAudio(4 * chunk);
	cache->RequestAudio(7 * chunk + 5);
	decoder->AllowReads(1);

	ASSERT_EQ(7 * chunk, decoder->WaitForRead(2));
	EXPECT_EQ(chunk, cache->GetDecodedSamples());
	EXPECT_FALSE(cache->IsAudioReady(7 * chunk, 7 * chunk + 1));
	uint16_t samples[4];
	cache->GetAudio(samples, chunk - 1, 2);
	EXPECT_EQ(static_cast<uint16_t>(chunk + 2), samples[0]);
	EXPECT_EQ(0, samples[1]);
	decoder->AllowReads(2);

	ASSERT_EQ(8 * chunk, decoder->WaitForRead(3));
	EXPECT_EQ(2 * chunk, cache->GetDecodedSamples());
	EXPECT_TRUE(cache->IsAudioReady(7 * chunk, 8 * chunk));
	EXPECT_FALSE(cache->IsAudioReady(7 * chunk - 1, 7 * chunk + 2));
	cache->GetAudio(samples, 7 * chunk - 1, 4);
	EXPECT_EQ(0, samples[0]);
	for (int i = 1; i < 4; ++i)
		EXPECT_EQ(static_cast<uint16_t>(7 * chunk + 2 + i), samples[i]);

	decoder->AllowReads(3);
	ASSERT_EQ(chunk, decoder->WaitForRead(4));
	cache->RequestAudio(4 * chunk);
	decoder->AllowReads(4);
	ASSERT_EQ(4 * chunk, decoder->WaitForRead(5));
	decoder->AllowReads(7);
	ASSERT_EQ(2 * chunk, decoder->WaitForRead(8));
	EXPECT_EQ(6 * chunk + 17, cache->GetDecodedSamples());
	EXPECT_TRUE(cache->IsAudioReady(4 * chunk, cache->GetNumSamples() + 100));
	EXPECT_FALSE(cache->IsAudioReady(0, cache->GetNumSamples()));

	decoder->AllowReads(9);
	const auto deadline = std::chrono::steady_clock::now() + 5s;
	while (cache->GetDecodedSamples() != cache->GetNumSamples() && std::chrono::steady_clock::now() < deadline)
		std::this_thread::sleep_for(1ms);
	ASSERT_EQ(cache->GetNumSamples(), cache->GetDecodedSamples());
	EXPECT_TRUE(cache->IsAudioReady(-100, cache->GetNumSamples() + 100));
	cache->GetAudio(samples, cache->GetNumSamples() - 2, 4);
	EXPECT_EQ(18, samples[0]);
	EXPECT_EQ(19, samples[1]);
	EXPECT_EQ(0, samples[2]);
	EXPECT_EQ(0, samples[3]);
}

TEST_P(AudioCacheSeekTest, KeepsNonseekableSourceSequential) {
	auto src = std::make_unique<SteppedAudioProvider>(false);
	auto decoder = src.get();
	auto cache = MakeCache(std::move(src));

	ASSERT_EQ(0, decoder->WaitForRead(1));
	cache->RequestAudio(cache->GetNumSamples() - 1);
	decoder->AllowReads(9);
	EXPECT_EQ(SteppedAudioProvider::chunk_samples, decoder->WaitForRead(2));
}

TEST_P(AudioCacheSeekTest, SeeksPastCachedChunksToFirstGap) {
	auto src = std::make_unique<SteppedAudioProvider>();
	auto decoder = src.get();
	auto cache = MakeCache(std::move(src));
	const auto chunk = SteppedAudioProvider::chunk_samples;

	ASSERT_EQ(0, decoder->WaitForRead(1));
	cache->RequestAudio(4 * chunk);
	decoder->AllowReads(1);
	ASSERT_EQ(4 * chunk, decoder->WaitForRead(2));
	decoder->AllowReads(2);
	ASSERT_EQ(5 * chunk, decoder->WaitForRead(3));
	cache->RequestAudio(7 * chunk);
	decoder->AllowReads(3);
	ASSERT_EQ(7 * chunk, decoder->WaitForRead(4));
	EXPECT_TRUE(cache->IsAudioReady(4 * chunk, 6 * chunk));

	cache->RequestAudio(4 * chunk + 1);
	decoder->AllowReads(4);
	EXPECT_EQ(6 * chunk, decoder->WaitForRead(5));
	decoder->AllowReads(9);
}

INSTANTIATE_TEST_SUITE_P(RAMAndDisk, AudioCacheSeekTest, testing::Bool());
}

TEST(lagi_audio, convert_8bit) {
	auto provider = agi::CreateConvertAudioProvider(std::make_unique<TestAudioProvider<uint8_t>>());

	int16_t data[256];
	provider->GetAudio(data, 0, 256);
	for (int i = 0; i < 256; ++i)
		ASSERT_EQ((i - 128) * 256, data[i]);
}

TEST(lagi_audio, convert_32bit) {
	auto src = std::make_unique<TestAudioProvider<uint32_t>>(100000);
	src->bias = INT_MIN;
	auto provider = agi::CreateConvertAudioProvider(std::move(src));

	int16_t sample;
	provider->GetAudio(&sample, 0, 1);
	EXPECT_EQ(SHRT_MIN, sample);

	provider->GetAudio(&sample, 1LL << 31, 1);
	EXPECT_EQ(0, sample);

	provider->GetAudio(&sample, (1LL << 32) - 1, 1);
	EXPECT_EQ(SHRT_MAX, sample);
}

TEST(lagi_audio, sample_doubling) {
	struct AudioProvider : agi::AudioProvider {
		AudioProvider() {
			channels = 1;
			num_samples = 90 * 20000;
			decoded_samples = num_samples;
			sample_rate = 20000;
			bytes_per_sample = 2;
			float_samples = false;
		}

		void FillBuffer(void *buf, int64_t start, int64_t count) const override {
			auto out = static_cast<int16_t *>(buf);
			for (int64_t end = start + count; start < end; ++start)
				*out++ = (int16_t)(start * 2);
		}
	};

	auto provider = agi::CreateConvertAudioProvider(std::make_unique<AudioProvider>());
	EXPECT_EQ(40000, provider->GetSampleRate());

	int16_t samples[6];
	for (int k = 0; k < 6; ++k) {
		SCOPED_TRACE(k);
		for (int i = k; i < 6; ++i) {
			SCOPED_TRACE(i);
			memset(samples, 0, sizeof(samples));
			provider->GetAudio(samples, k, i - k);
			for (int j = 0; j < i - k; ++j)
				EXPECT_EQ(j + k, samples[j]);
			for (int j = i - k; j < 6 - k; ++j)
				EXPECT_EQ(0, samples[j]);
		}
	}
}

TEST(lagi_audio, stereo_downmix) {
	struct AudioProvider : agi::AudioProvider {
		AudioProvider() {
			channels = 2;
			num_samples = 90 * 480000;
			decoded_samples = num_samples;
			sample_rate = 480000;
			bytes_per_sample = 2;
			float_samples = false;
		}

		void FillBuffer(void *buf, int64_t start, int64_t count) const override {
			auto out = static_cast<int16_t *>(buf);
			for (int64_t end = start + count; start < end; ++start) {
				*out++ = (int16_t)(start * 2);
				*out++ = 0;
			}
		}
	};

	auto provider = agi::CreateConvertAudioProvider(std::make_unique<AudioProvider>());
	EXPECT_EQ(1, provider->GetChannels());

	int16_t samples[100];
	provider->GetAudio(samples, 0, 100);
	for (int i = 0; i < 100; ++i)
		EXPECT_EQ(i, samples[i]);
}

template<typename Float>
struct FloatAudioProvider : agi::AudioProvider {
	FloatAudioProvider() {
		channels = 1;
		num_samples = 90 * 480000;
		decoded_samples = num_samples;
		sample_rate = 480000;
		bytes_per_sample = sizeof(Float);
		float_samples = true;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto out = static_cast<Float *>(buf);
		for (int64_t end = start + count; start < end; ++start) {
			auto shifted = start + SHRT_MIN;
			*out++ = (Float)(1.0 * shifted / (shifted < 0 ? -SHRT_MIN : SHRT_MAX));
		}
	}
};

TEST(lagi_audio, float_conversion) {
	auto provider = agi::CreateConvertAudioProvider(std::make_unique<FloatAudioProvider<float>>());
	EXPECT_FALSE(provider->AreSamplesFloat());

	int16_t samples[1 << 16];
	provider->GetAudio(samples, 0, 1 << 16);
	for (int i = 0; i < (1 << 16); ++i)
		ASSERT_EQ(i + SHRT_MIN, samples[i]);
}

TEST(lagi_audio, double_conversion) {
	auto provider = agi::CreateConvertAudioProvider(std::make_unique<FloatAudioProvider<double>>());
	EXPECT_FALSE(provider->AreSamplesFloat());

	int16_t samples[1 << 16];
	provider->GetAudio(samples, 0, 1 << 16);
	for (int i = 0; i < (1 << 16); ++i)
		ASSERT_EQ(i + SHRT_MIN, samples[i]);
}

TEST(lagi_audio, pcm_simple) {
	auto path = agi::Path().Decode("?temp/pcm_simple");
	{
		TestAudioProvider<> provider;
		agi::SaveAudioClip(provider, path, 0, 1000);
	}

	{
		auto provider = agi::CreatePCMAudioProvider(path, nullptr);
		EXPECT_EQ(1, provider->GetChannels());
		EXPECT_EQ(48000, provider->GetNumSamples());
		EXPECT_EQ(48000, provider->GetSampleRate());
		EXPECT_EQ(2, provider->GetBytesPerSample());
		EXPECT_EQ(false, provider->AreSamplesFloat());
		EXPECT_EQ(false, provider->NeedsCache());

		for (int i = 0; i < 100; ++i) {
			uint16_t sample;
			provider->GetAudio(&sample, i, 1);
			ASSERT_EQ(i, sample);
		}
	}

	agi::fs::Remove(path);
}

TEST(lagi_audio, pcm_truncated) {
	auto path = agi::Path().Decode("?temp/pcm_truncated");
	{
		TestAudioProvider<> provider;
		agi::SaveAudioClip(provider, path, 0, 1000);
	}

	char file[1000];

	{ std::ifstream s(path, std::ios_base::binary); s.read(file, sizeof file); }
	{ std::ofstream s(path, std::ios_base::binary); s.write(file, sizeof file); }

	{
		auto provider = agi::CreatePCMAudioProvider(path, nullptr);

		// Should still report full duration
		EXPECT_EQ(48000, provider->GetNumSamples());

		// And should zero-pad past the end
		int64_t sample_count = (1000 - 44) / 2;
		uint16_t sample;

		provider->GetAudio(&sample, sample_count - 1, 1);
		EXPECT_EQ(sample_count - 1, sample);

		provider->GetAudio(&sample, sample_count, 1);
		EXPECT_EQ(0, sample);
	}

	agi::fs::Remove(path);
}

#define RIFF "RIFF\0\0\0\x60WAVE"
#define FMT_VALID "fmt \x10\0\0\0\1\0\1\0\x10\0\0\0\x20\0\0\0\2\0\x10\0"
#define DATA_VALID "data\1\0\0\0\0\0"
#define WRITE(str) do { std::ofstream s(path, std::ios_base::binary); s.write(str, sizeof(str) - 1); } while (false)

TEST(lagi_audio, pcm_incomplete) {
	auto path = agi::Path().Decode("?temp/pcm_incomplete");

	agi::fs::Remove(path);
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::fs::FileNotFound);

	{std::ofstream(path, std::ios_base::binary); }
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);

	// Invalid tags
	WRITE("ASDF");
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);

	WRITE("RIFF\0\0\0\x60" "ASDF");
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);

	// Incomplete files
	const char valid_file[] = RIFF FMT_VALID DATA_VALID;

	// -1 for nul term, -3 so that longest file is still invalid
	for (size_t i = 0; i < sizeof(valid_file) - 4; ++i) {
		{
			std::ofstream s(path, std::ios_base::binary);
			s.write(valid_file, i);
		}
		ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);
	}

	// fmt must come before data
	WRITE(RIFF "data\0\0\0\x60");
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioProviderError);

	// Bad compression format
	WRITE(RIFF "fmt \x60\0\0\0\2\0");
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioProviderError);

	// Multiple fmt chunks not supported
	WRITE(RIFF FMT_VALID FMT_VALID);
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioProviderError);

	agi::fs::Remove(path);
}

TEST(lagi_audio, multiple_data_chunks) {
	auto path = agi::Path().Decode("?temp/multiple_data");

	WRITE(RIFF FMT_VALID "data\2\0\0\0\1\0" "data\2\0\0\0\2\0" "data\2\0\0\0\3\0");

	{
		auto provider = agi::CreatePCMAudioProvider(path, nullptr);
		ASSERT_EQ(3, provider->GetNumSamples());

		uint16_t samples[3];

		provider->GetAudio(samples, 0, 3);
		EXPECT_EQ(1, samples[0]);
		EXPECT_EQ(2, samples[1]);
		EXPECT_EQ(3, samples[2]);

		samples[1] = 5;
		provider->GetAudio(samples, 2, 1);
		EXPECT_EQ(3, samples[0]);
		EXPECT_EQ(5, samples[1]);

		provider->GetAudio(samples, 1, 1);
		EXPECT_EQ(2, samples[0]);
		EXPECT_EQ(5, samples[1]);

		provider->GetAudio(samples, 0, 1);
		EXPECT_EQ(1, samples[0]);
		EXPECT_EQ(5, samples[1]);
	}

	agi::fs::Remove(path);
}

#define WAVE64_FILE \
	"riff\x2e\x91\xcf\x11\xa5\xd6\x28\xdb\x04\xc1\x00\x00"   /* RIFF GUID */          \
	"\x74\x00\0\0\0\0\0\0"                                   /* file size */          \
	"wave\xf3\xac\xd3\x11\x8c\xd1\x00\xc0\x4f\x8e\xdb\x8a"   /* WAVE GUID */          \
	"fmt \xf3\xac\xd3\x11\x8c\xd1\x00\xc0\x4f\x8e\xdb\x8a"   /* fmt GUID */           \
	"\x30\x00\0\0\0\0\0\0"                                   /* fmt chunk size */     \
	"\1\0\1\0\x10\0\0\0\x20\0\0\0\2\0\x10\0\0\0\0\0\0\0\0\0" /* fmt chunk */          \
	"data\xf3\xac\xd3\x11\x8c\xd1\x00\xc0\x4f\x8e\xdb\x8a"   /* data GUID */          \
	"\x1c\0\0\0\0\0\0\0"                                     /* data chunk size */    \
	"\1\0\2\0"                                               /* actual sample data */ \

TEST(lagi_audio, wave64_simple) {
	auto path = agi::Path().Decode("?temp/w64_valid");
	WRITE(WAVE64_FILE);

	{
		auto provider = agi::CreatePCMAudioProvider(path, nullptr);
		ASSERT_EQ(2, provider->GetNumSamples());

		uint16_t samples[2];
		provider->GetAudio(samples, 0, 2);
		EXPECT_EQ(1, samples[0]);
		EXPECT_EQ(2, samples[1]);
	}

	agi::fs::Remove(path);
}

TEST(lagi_audio, wave64_truncated) {
	auto path = agi::Path().Decode("?temp/w64_truncated");

	// Should be invalid until there's an entire sample
	for (size_t i = 0; i < sizeof(WAVE64_FILE) - 4; ++i) {
		{
			std::ofstream s(path, std::ios_base::binary);
			s.write(WAVE64_FILE, i);
		}
		ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);
	}

	{
		std::ofstream s(path, std::ios_base::binary);
		s.write(WAVE64_FILE, sizeof(WAVE64_FILE) - 3);
	}
	ASSERT_NO_THROW(agi::CreatePCMAudioProvider(path, nullptr));

	{
		auto provider = agi::CreatePCMAudioProvider(path, nullptr);
		uint16_t sample;
		provider->GetAudio(&sample, 0, 1);
		EXPECT_EQ(1, sample);
	}

	agi::fs::Remove(path);
}
