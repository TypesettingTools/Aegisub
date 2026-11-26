// Copyright (c) 2026, Aegisub contributors
// SPDX-License-Identifier: BSD-3-Clause

#include <wx/timer.h>
#include <CoreAudio/CoreAudio.h>
#include <libaegisub/dispatch.h>
#include <libaegisub/log.h>
#include <main.h>
#include <chrono>
#include <thread>

// Use the real Apple mixer, with manual maintenance ticks to test starvation.
class ManualTimer {
public:
	static inline ManualTimer *active = nullptr;
	virtual ~ManualTimer() = default;
	virtual void Notify() = 0;
	bool Start(int) { active = this; return true; }
	void Stop() { if (active == this) active = nullptr; }
	static void Tick() { if (active) active->Notify(); }
};

static double injected_latency = -1;
static OSStatus ProbePropertyData(AudioObjectID object, const AudioObjectPropertyAddress *address,
	UInt32 qualifier_size, const void *qualifier, UInt32 *size, void *data) {
	auto status = AudioObjectGetPropertyData(object, address, qualifier_size, qualifier, size, data);
	if (status || injected_latency < 0) return status;
	if (address->mSelector == kAudioDevicePropertyLatency &&
		address->mScope == kAudioDevicePropertyScopeOutput) {
		Float64 rate = 0;
		UInt32 rate_size = sizeof(rate);
		AudioObjectPropertyAddress rate_address{kAudioDevicePropertyNominalSampleRate,
			kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
		if (AudioObjectGetPropertyData(object, &rate_address, 0, nullptr, &rate_size, &rate))
			return kAudioHardwareUnspecifiedError;
		*static_cast<UInt32 *>(data) = injected_latency * rate;
	}
	else if (address->mSelector == kAudioDevicePropertySafetyOffset ||
			 address->mSelector == kAudioDevicePropertyBufferFrameSize ||
			 address->mSelector == kAudioStreamPropertyLatency)
		*static_cast<UInt32 *>(data) = 0;
	return status;
}

#define wxTimer ManualTimer
#define AudioObjectGetPropertyData ProbePropertyData
#include "../../src/audio_player_openal.cpp"
#undef AudioObjectGetPropertyData
#undef wxTimer

using namespace std::chrono_literals;

// This test uses silent audio and the real Apple OpenAL mixer. It skips on CI
// hosts (and in sandboxes) where the system audio service is unavailable.
TEST(NativeOpenAL, LatencyDrainUnderrunAndEndpointChanges) {
	auto provider = agi::CreateDummyAudioProvider("dummy-audio:", nullptr);
	const int rate = provider->GetSampleRate();
	std::unique_ptr<OpenALPlayer> player;
	try {
		player = std::make_unique<OpenALPlayer>(provider.get());
	}
	catch (AudioPlayerOpenError const&) {
		GTEST_SKIP() << "No native OpenAL output device available";
	}
	struct ResetLatency {
		~ResetLatency() { injected_latency = -1; }
	} reset_latency;
	auto pump = [](auto duration) {
		auto end = std::chrono::steady_clock::now() + duration;
		do {
			std::this_thread::sleep_for(10ms);
			ManualTimer::Tick();
		} while (std::chrono::steady_clock::now() < end);
	};

	// A 100 ms selection must not end before the reported 500 ms output delay.
	injected_latency = 0.5;
	player->Play(rate, rate / 10);
	if (alIsExtensionPresent("AL_SOFT_source_latency"))
		GTEST_SKIP() << "This test exercises Apple's native OpenAL latency fallback";
	pump(100ms);
	EXPECT_TRUE(player->IsPlaying());
	EXPECT_EQ(rate, player->GetCurrentPosition());
	pump(800ms);
	EXPECT_FALSE(player->IsPlaying());
	EXPECT_GE(player->GetCurrentPosition(), rate + rate / 10);
	EXPECT_EQ(AL_NO_ERROR, alGetError());

	// The device drains even when a delayed maintenance callback lets the
	// mixer run out of buffers. Do not hold the cursor behind already heard audio.
	player->Play(0, rate * 4 / 10);
	std::this_thread::sleep_for(1100ms);
	ManualTimer::Tick();
	EXPECT_FALSE(player->IsPlaying());

	// Starve the half-second queue. The clock must stop with the mixer instead
	// of advancing through the gap, and playback must resume after refilling.
	injected_latency = 0;
	player->Play(0, rate * 5);
	pump(150ms);
	auto before = player->GetCurrentPosition();
	std::this_thread::sleep_for(900ms);
	ManualTimer::Tick();
	auto recovered = player->GetCurrentPosition();
	EXPECT_GE(recovered, before);
	EXPECT_LT(recovered, before + rate * 0.7);
	pump(200ms);
	EXPECT_GT(player->GetCurrentPosition(), recovered);
	EXPECT_EQ(AL_NO_ERROR, alGetError());
	player->Stop();

	// Replace already queued silence/audio when the endpoint changes.
	player->Play(0, rate / 10);
	player->SetEndPosition(rate);
	EXPECT_EQ(rate, player->GetEndPosition());
	pump(150ms);
	EXPECT_TRUE(player->IsPlaying());
	player->SetEndPosition(player->GetCurrentPosition() + rate / 10);
	pump(250ms);
	EXPECT_FALSE(player->IsPlaying());
	player->Stop();
	EXPECT_EQ(AL_NO_ERROR, alGetError());
}
