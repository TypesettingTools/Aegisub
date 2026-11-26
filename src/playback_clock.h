// Copyright (c) 2026, Aegisub contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <chrono>
#include <optional>

/// Follow audible audio time, or continue from the last observation without audio.
class PlaybackClock {
public:
	using Clock = std::chrono::steady_clock;

private:
	Clock::time_point last_update;
	int position = 0;

public:
	void Start(int start, Clock::time_point now = Clock::now()) {
		position = start;
		last_update = now;
	}

	int GetPosition(std::optional<int> audio_position, Clock::time_point now = Clock::now()) {
		if (audio_position) {
			// Latency changes and backend rounding must not seek video backwards.
			position = std::max(position, *audio_position);
			last_update = now;
		}
		else {
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update);
			position += elapsed.count();
			last_update += elapsed;
		}
		return position;
	}
};
