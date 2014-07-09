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
#include <libaegisub/make_unique.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <boost/filesystem/fstream.hpp>

namespace bfs = boost::filesystem;

TEST(lagi_audio, dummy_blank) {
	auto provider = agi::CreateDummyAudioProvider("dummy-audio:", nullptr);

	char buff[1024];
	memset(buff, sizeof(buff), 1);
	provider->GetAudio(buff, 12356, 512);
	for (size_t i = 0; i < sizeof(buff); ++i) ASSERT_EQ(0, buff[i]);
}

TEST(lagi_audio, dummy_noise) {
	auto provider = agi::CreateDummyAudioProvider("dummy-audio:noise?", nullptr);

	char buff[1024];
	memset(buff, sizeof(buff), 0);
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

struct TestAudioProvider : agi::AudioProvider {
	TestAudioProvider(int64_t duration = 90) {
		channels = 1;
		num_samples = duration * 48000;
		decoded_samples = num_samples;
		sample_rate = 48000;
		bytes_per_sample = 2;
		float_samples = false;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto out = static_cast<uint16_t *>(buf);
		for (int64_t end = start + count; start < end; ++start)
			*out++ = (uint16_t)start;
	}
};

TEST(lagi_audio, before_sample_zero) {
	TestAudioProvider provider;

	uint16_t buff[16];
	memset(buff, sizeof(buff), 1);
	provider.GetAudio(buff, -8, 16);

	for (int i = 0; i < 8; ++i)
		ASSERT_EQ(0, buff[i]);
	for (int i = 8; i < 16; ++i)
		ASSERT_EQ(i - 8, buff[i]);
}

TEST(lagi_audio, after_end) {
	TestAudioProvider provider(1);

	uint16_t buff[16];
	memset(buff, sizeof(buff), 1);
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
	agi::SaveAudioClip(provider.get(), path, 60 * 60 * 1000, (60 * 60 + 10) * 1000);

	{
		bfs::ifstream s(path);
		ASSERT_TRUE(s.good());
		s.seekg(0, std::ios::end);
		// 10 seconds of 44.1 kHz samples per second of 16-bit mono, plus 44 bytes of header
		EXPECT_EQ(10 * 44100 * 2 + 44, s.tellg());
	}
	agi::fs::Remove(path);
}

TEST(lagi_audio, get_with_volume) {
	TestAudioProvider provider;
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
	TestAudioProvider provider;
	uint16_t buff[1];
	provider.GetAudioWithVolume(buff, 30000, 1, 2.0);
	EXPECT_EQ(SHRT_MAX, buff[0]);
}

TEST(lagi_audio, ram_cache) {
	auto provider = agi::CreateRAMAudioProvider(agi::make_unique<TestAudioProvider>());
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
	auto provider = agi::CreateHDAudioProvider(agi::make_unique<TestAudioProvider>(), agi::Path().Decode("?temp"));
	while (provider->GetDecodedSamples() != provider->GetNumSamples()) agi::util::sleep_for(0);

	uint16_t buff[512];
	provider->GetAudio(buff, (1 << 22) - 256, 512);

	for (size_t i = 0; i < 512; ++i)
		ASSERT_EQ(static_cast<uint16_t>((1 << 22) - 256 + i), buff[i]);
}

TEST(lagi_audio, pcm_simple) {
	auto path = agi::Path().Decode("?temp/pcm_simple");
	{
		TestAudioProvider provider;
		agi::SaveAudioClip(&provider, path, 0, 1000);
	}

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

	agi::fs::Remove(path);
}

TEST(lagi_audio, pcm_truncated) {
	auto path = agi::Path().Decode("?temp/pcm_truncated");
	{
		TestAudioProvider provider;
		agi::SaveAudioClip(&provider, path, 0, 1000);
	}

	char file[1000];

	{ bfs::ifstream s(path); s.read(file, sizeof file); }
	{ bfs::ofstream s(path); s.write(file, sizeof file); }

	auto provider = agi::CreatePCMAudioProvider(path, nullptr);

	// Should still report full duration
	EXPECT_EQ(48000, provider->GetNumSamples());

	// And should zero-pad past the end
	auto sample_count = (1000 - 44) / 2;
	uint16_t sample;

	provider->GetAudio(&sample, sample_count - 1, 1);
	EXPECT_EQ(sample_count - 1, sample);

	provider->GetAudio(&sample, sample_count, 1);
	EXPECT_EQ(0, sample);

	agi::fs::Remove(path);
}

#define RIFF "RIFF\0\0\0\x60WAVE"
#define FMT_VALID "fmt \20\0\0\0\1\0\1\0\20\0\0\0\0\0\0\0\0\0\20\0"
#define DATA_VALID "data\1\0\0\0\0\0"
#define WRITE(str) do { bfs::ofstream s(path); s.write(str, sizeof(str) - 1); } while (false)

TEST(lagi_audio, pcm_incomplete) {
	auto path = agi::Path().Decode("?temp/pcm_incomplete");

	agi::fs::Remove(path);
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::fs::FileNotFound);

	bfs::ofstream{path};
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);

	// Invalid tags
	WRITE("ASDF");
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);

	WRITE("RIFF\0\0\0\x60" "ASDF");
	ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);

	// Incomplete files
	auto valid_file = RIFF FMT_VALID DATA_VALID;

	// -1 for nul term, -3 so that longest file is still invalid
	for (size_t i = 0; i < sizeof(valid_file) - 4; ++i) {
		bfs::ofstream s(path);
		s.write(valid_file, i);
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

	auto provider = agi::CreatePCMAudioProvider(path, nullptr);
	ASSERT_EQ(2, provider->GetNumSamples());

	uint16_t samples[2];
	provider->GetAudio(samples, 0, 2);
	EXPECT_EQ(1, samples[0]);
	EXPECT_EQ(2, samples[1]);

	agi::fs::Remove(path);
}

TEST(lagi_audio, wave64_truncated) {
	auto path = agi::Path().Decode("?temp/w64_truncated");

	// Should be invalid until there's an entire sample
	for (size_t i = 0; i < sizeof(WAVE64_FILE) - 4; ++i) {
		bfs::ofstream s(path);
		s.write(WAVE64_FILE, i);
		ASSERT_THROW(agi::CreatePCMAudioProvider(path, nullptr), agi::AudioDataNotFound);
	}

	{
		bfs::ofstream s(path);
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
