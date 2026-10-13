// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

/// @file client.h
/// @brief Launching and talking to a sandboxed aegisub-csri-host process

#pragma once

#include "../video_frame.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace csri_host {

/// Any failure to start or talk to the host. The Host which threw it is no
/// longer usable and should be destroyed, which kills the process.
struct Error : std::runtime_error {
	using std::runtime_error::runtime_error;
};

/// Allocates frame memory which hosts can draw on without copying it. Hosts
/// can write to it at any time once they've drawn on it, so it mustn't be
/// used for anything but frames which subtitles are drawn on.
FrameAllocator SharedFrameAllocator();

/// A running aegisub-csri-host process. Not thread-safe.
class Host {
	struct Impl;
	std::unique_ptr<Impl> impl;

public:
	/// Start a new sandboxed host process
	Host();
	~Host();

	std::vector<std::string> ListRenderers();
	/// Select the renderer to use, returning false if it doesn't exist
	bool OpenRenderer(std::string const& name);
	/// Load a script, returning false if the renderer couldn't load it
	bool LoadSubtitles(const char *data, size_t len);
	/// Render subtitles onto a BGRx frame. Frames whose memory came from
	/// SharedFrameAllocator() are drawn on in place, and others are copied.
	void Render(VideoFrame &frame, double time);
};

}
