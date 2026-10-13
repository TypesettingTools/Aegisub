// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

/// @file protocol.h
/// @brief Messages exchanged between Aegisub and aegisub-csri-host
///
/// CSRI renderers (i.e. VSFilter) are run in a separate sandboxed process so
/// that memory safety bugs in them can't compromise Aegisub. Aegisub sends a
/// Request over a pipe, optionally followed by payload_size bytes, and the
/// host replies with a Response. Rendered frames are passed through a shared
/// memory section rather than the pipe.
///
/// Aegisub must treat everything it receives from the host as hostile: it
/// only ever reads fixed-size Responses, a bounded renderer list, and pixels.

#pragma once

#include <cstdint>

namespace csri_host {

/// Sent by the host once it has loaded the renderers and locked itself down
constexpr uint32_t hello_magic = 0x49525343; // "CSRI"
/// Bumped whenever the layout of any message changes
constexpr uint32_t protocol_version = 1;

/// Upper bounds on sizes, checked by whichever side receives them
constexpr uint32_t max_renderer_list_size = 64 * 1024;
constexpr uint32_t max_renderer_name_size = 256;
constexpr uint32_t max_frame_dimension = 16384;
constexpr uint64_t max_script_size = uint64_t(2) * 1024 * 1024 * 1024;

enum class Command : uint32_t {
	/// Reply with the names of the available renderers as NUL-terminated
	/// strings in a payload of Response::payload_size bytes
	ListRenderers = 1,
	/// Select the renderer whose name is the payload
	OpenRenderer = 2,
	/// Load the subtitle script which is the payload
	LoadSubtitles = 3,
	/// Use the section_size byte section which Aegisub has duplicated into
	/// the host process as section_handle as the frame buffer
	SetFrameBuffer = 4,
	/// Render the subtitles at time onto the frame buffer, which holds a
	/// top-down width x height BGRx image with no padding between rows
	Render = 5,
};

struct Hello {
	uint32_t magic;
	uint32_t version;
};

struct Request {
	Command command;
	uint32_t width;
	uint32_t height;
	uint32_t reserved;
	/// Number of bytes which follow the request
	uint64_t payload_size;
	uint64_t section_handle;
	uint64_t section_size;
	/// Time to render at, in seconds
	double time;
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
