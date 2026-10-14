// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

/// @file protocol.h
/// @brief Messages exchanged between Aegisub and aegisub-csri-host
///
/// Aegisub sends a Request over a pipe, optionally followed by payload_size
/// bytes, and the host replies with a Response. Frames are passed through
/// shared memory sections rather than the pipe: either the memory of the
/// video frames themselves, so that they can be drawn on without being
/// copied, or a buffer which Aegisub copies frames into and out of.
///
/// Aegisub must treat everything it receives from the host as hostile: it
/// only ever reads fixed-size Responses, a bounded renderer list, and pixels.
/// The host can write to frames it has been given at any time, so Aegisub
/// never stores anything but pixels in them and never trusts their contents.

#pragma once

#include <cstdint>

namespace csri_host {

/// Sent by the host once it has loaded the renderers and locked itself down
constexpr uint32_t hello_magic = 0x49525343; // "CSRI"
/// Bumped whenever the layout of any message changes
constexpr uint32_t protocol_version = 2;

/// Upper bounds on sizes, checked by whichever side receives them
constexpr uint32_t max_renderer_list_size = 64 * 1024;
constexpr uint32_t max_renderer_name_size = 256;
constexpr uint32_t max_frame_dimension = 16384;
constexpr uint64_t max_script_size = uint64_t(2) * 1024 * 1024 * 1024;
/// Maximum number of buffers mapped into the host at once
constexpr uint32_t max_buffers = 16;

enum class Command : uint32_t {
	/// Reply with the names of the available renderers as NUL-terminated
	/// strings in a payload of Response::payload_size bytes
	ListRenderers = 1,
	/// Select the renderer whose name is the payload
	OpenRenderer = 2,
	/// Load the subtitle script which is the payload
	LoadSubtitles = 3,
	/// Map the section_size byte section which Aegisub has duplicated into
	/// the host process as section_handle, as buffer number `buffer`. Any
	/// existing buffer with that number is unmapped first.
	MapBuffer = 4,
	/// Unmap buffer number `buffer`
	UnmapBuffer = 5,
	/// Render the subtitles at time onto buffer number `buffer`, which holds
	/// a width x height BGRx image whose rows start pitch bytes apart, with
	/// the bottom row first if flipped is non-zero
	Render = 6,
};

struct Hello {
	uint32_t magic;
	uint32_t version;
};

struct Request {
	Command command;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	/// Number of bytes which follow the request
	uint64_t payload_size;
	uint64_t buffer;
	uint64_t section_handle;
	uint64_t section_size;
	/// Time to render at, in seconds
	double time;
	uint32_t flipped;
	uint32_t reserved;
};

enum class Status : uint32_t {
	Ok = 0,
	Failed = 1,
};

struct Response {
	Status status;
	uint32_t payload_size;
};

}
