// Copyright (c) 2026, Aegisub contributors
// SPDX-License-Identifier: BSD-3-Clause

#include <main.h>
#include "../../src/playback_clock.h"

using namespace std::chrono_literals;

TEST(PlaybackClock, WaitsForAudibleAudioAtStartup) {
	PlaybackClock clock;
	auto start = PlaybackClock::Clock::time_point{};
	clock.Start(5000, start);
	EXPECT_EQ(5000, clock.GetPosition(5000, start + 100ms));
	EXPECT_EQ(5000, clock.GetPosition(5000, start + 250ms));
	EXPECT_EQ(5050, clock.GetPosition(5050, start + 300ms));
}

TEST(PlaybackClock, FollowsAudioThroughTimerJitterAndStalls) {
	PlaybackClock clock;
	auto start = PlaybackClock::Clock::time_point{};
	clock.Start(5000, start);
	EXPECT_EQ(5030, clock.GetPosition(5030, start + 40ms));
	EXPECT_EQ(5400, clock.GetPosition(5400, start + 500ms));
	EXPECT_EQ(5400, clock.GetPosition(5400, start + 1000ms));
	EXPECT_EQ(5420, clock.GetPosition(5420, start + 1020ms));
}

TEST(PlaybackClock, ContinuesFromAudioWhenAudioEndsOrCloses) {
	PlaybackClock clock;
	auto start = PlaybackClock::Clock::time_point{};
	clock.Start(5000, start);
	EXPECT_EQ(5100, clock.GetPosition(5100, start + 400ms));
	EXPECT_EQ(5120, clock.GetPosition(std::nullopt, start + 420ms));
	EXPECT_EQ(5500, clock.GetPosition(std::nullopt, start + 800ms));
}

TEST(PlaybackClock, SilentPlaybackPreservesFractionalMilliseconds) {
	PlaybackClock clock;
	auto start = PlaybackClock::Clock::time_point{};
	clock.Start(5000, start);
	EXPECT_EQ(5000, clock.GetPosition(std::nullopt, start + 900us));
	EXPECT_EQ(5001, clock.GetPosition(std::nullopt, start + 1800us));
	EXPECT_EQ(5002, clock.GetPosition(std::nullopt, start + 2700us));
}

TEST(PlaybackClock, LatencyChangesDoNotSeekBackwardsAndRestartResetsTime) {
	PlaybackClock clock;
	auto start = PlaybackClock::Clock::time_point{};
	clock.Start(5000, start);
	EXPECT_EQ(5100, clock.GetPosition(5100, start + 100ms));
	EXPECT_EQ(5100, clock.GetPosition(5050, start + 120ms));
	EXPECT_EQ(5110, clock.GetPosition(5110, start + 180ms));
	clock.Start(1000, start + 200ms);
	EXPECT_EQ(1000, clock.GetPosition(1000, start + 250ms));
}
