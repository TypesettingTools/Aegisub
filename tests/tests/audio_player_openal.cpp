// Copyright (c) 2026, Aegisub contributors
// SPDX-License-Identifier: BSD-3-Clause

#include <OpenAL/al.h>
#include <wx/timer.h>
#include <main.h>
#include <vector>

#include "../../src/audio_controller.h"

namespace {
// The test only needs the initial buffer fill, without a GUI event loop.
class InactiveTimer {
public:
	virtual ~InactiveTimer() = default;
	virtual void Notify() = 0;
	void Start(int) {}
	void Stop() {}
};

struct BufferInfo {
	ALint channels;
	ALint size;
	ALint rate;
};
std::vector<BufferInfo> uploaded_buffers;

void InspectBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei rate) {
	alBufferData(buffer, format, data, size, rate);
	BufferInfo info{};
	alGetBufferi(buffer, AL_CHANNELS, &info.channels);
	alGetBufferi(buffer, AL_SIZE, &info.size);
	alGetBufferi(buffer, AL_FREQUENCY, &info.rate);
	uploaded_buffers.push_back(info);
}
}

#define wxTimer InactiveTimer
#define alBufferData InspectBufferData
#include "../../src/audio_player_openal.cpp"
#undef alBufferData
#undef wxTimer

namespace {
class SilentStereoProvider : public agi::AudioProvider {
public:
	SilentStereoProvider() {
		channels = 2;
		sample_rate = 44100;
		bytes_per_sample = 2;
		num_samples = decoded_samples = sample_rate * 10;
	}
	void FillBuffer(void *buf, int64_t, int64_t count) const override {
		memset(buf, 0, count * channels * bytes_per_sample);
	}
};
}

TEST(NativeOpenAL, UploadsStereoBuffersWithWholeSampleFrames) {
	SilentStereoProvider provider;
	std::unique_ptr<AudioPlayer> player;
	try {
		player = CreateOpenALPlayer(&provider, nullptr);
	}
	catch (AudioPlayerOpenError const&) {
		GTEST_SKIP() << "No native OpenAL output device available";
	}

	// At 44.1 kHz, dividing the number of bytes before rounding to a
	// whole frame would leave each buffer two bytes short of alignment.
	uploaded_buffers.clear();
	player->Play(0, provider.GetSampleRate());
	EXPECT_EQ(AL_NO_ERROR, alGetError());
	ASSERT_FALSE(uploaded_buffers.empty());
	for (auto const& buffer : uploaded_buffers) {
		EXPECT_EQ(2, buffer.channels);
		EXPECT_EQ(0, buffer.size % 4);
		EXPECT_GT(buffer.size, 0);
		EXPECT_EQ(provider.GetSampleRate(), buffer.rate);
	}
}
