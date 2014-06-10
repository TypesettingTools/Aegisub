// Copyright (c) 2009, Karl Blomster
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file video_provider_yuv4mpeg.cpp
/// @brief Video provider reading YUV4MPEG files directly without depending on external libraries
/// @ingroup video_input
///

#include "include/aegisub/video_provider.h"

#include "utils.h"
#include "video_frame.h"

#include <libaegisub/file_mapping.h>
#include <libaegisub/log.h>
#include <libaegisub/make_unique.h>
#include <libaegisub/util.h>
#include <libaegisub/ycbcr_conv.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <memory>
#include <vector>

/// the maximum allowed header length, in bytes
#define YUV4MPEG_HEADER_MAXLEN 128

namespace {

/// @class YUV4MPEGVideoProvider
/// @brief Implements reading of YUV4MPEG uncompressed video files
class YUV4MPEGVideoProvider final : public VideoProvider {
	/// Pixel formats
	enum Y4M_PixelFormat {
		Y4M_PIXFMT_NONE		= -1,	/// not set/unknown

		/// 4:2:0 sampling variants.
		/// afaict the only difference between these three
		/// is the chroma sample location, and nobody cares about that.
		Y4M_PIXFMT_420JPEG,		/// 4:2:0, H/V centered, for JPEG/MPEG-1
		Y4M_PIXFMT_420MPEG2,	/// 4:2:0, H cosited, for MPEG-2
		Y4M_PIXFMT_420PALDV,	/// 4:2:0, alternating Cb/Cr, for PAL-DV

		Y4M_PIXFMT_411,			/// 4:1:1, H cosited
		Y4M_PIXFMT_422,			/// 4:2:2, H cosited
		Y4M_PIXFMT_444,			/// 4:4:4, i.e. no chroma subsampling
		Y4M_PIXFMT_444ALPHA,	/// 4:4:4 plus alpha channel

		Y4M_PIXFMT_MONO		/// luma only (grayscale)
	};

	/// Interlacing mode for an entire stream
	enum Y4M_InterlacingMode {
		Y4M_ILACE_NOTSET = -1,	/// undefined
		Y4M_ILACE_PROGRESSIVE,	/// progressive (no interlacing)

		Y4M_ILACE_TFF,			/// interlaced, top field first
		Y4M_ILACE_BFF,			/// interlaced, bottom field first

		Y4M_ILACE_MIXED,		/// mixed interlaced/progressive, possibly with RFF flags
		Y4M_ILACE_UNKNOWN		/// unknown interlacing mode (not the same as undefined)
	};

	/// Frame information flags
	enum Y4M_FrameFlags {
		Y4M_FFLAG_NOTSET	= -1,		/// undefined
		Y4M_FFLAG_NONE		= 0x0000,	/// no flags set

		/// field order/repeat field flags
		Y4M_FFLAG_R_TFF		= 0x0001,	/// top field first
		Y4M_FFLAG_R_TFF_R	= 0x0002,	/// top field first, and repeat that field
		Y4M_FFLAG_R_BFF		= 0x0004,	/// bottom field first
		Y4M_FFLAG_R_BFF_R	= 0x0008,	/// bottom field first, and repeat that field
		Y4M_FFLAG_R_P		= 0x0010,	/// progressive
		Y4M_FFLAG_R_P_R		= 0x0020,	/// progressive, and repeat frame once
		Y4M_FFLAG_R_P_RR	= 0x0040,	/// progressive, and repeat frame twice

		/// temporal sampling flags
		Y4M_FFLAG_T_P		= 0x0080,	/// progressive (fields sampled at the same time)
		Y4M_FFLAG_T_I		= 0x0100,	/// interlaced (fields sampled at different times)

		/// chroma subsampling flags
		Y4M_FFLAG_C_P		= 0x0200,	/// progressive (whole frame subsampled)
		Y4M_FFLAG_C_I		= 0x0400,	/// interlaced (fields subsampled independently)
		Y4M_FFLAG_C_UNKNOWN = 0x0800	/// unknown (only allowed for non-4:2:0 sampling)
	};

	agi::read_file_mapping file;
	bool inited = false;	/// initialization state

	int w = 0, h = 0;	/// frame width/height
	int num_frames = -1; /// length of file in frames
	int frame_sz;	/// size of each frame in bytes
	int luma_sz;	/// size of the luma plane of each frame, in bytes
	int chroma_sz;	/// size of one of the two chroma planes of each frame, in bytes

	Y4M_PixelFormat pixfmt = Y4M_PIXFMT_NONE;		/// colorspace/pixel format
	Y4M_InterlacingMode imode = Y4M_ILACE_NOTSET;	/// interlacing mode (for the entire stream)
	struct {
		int num = -1;	/// numerator
		int den = 1;	/// denominator
	} fps_rat;          /// framerate

	agi::vfr::Framerate fps;

	agi::ycbcr_converter conv{agi::ycbcr_matrix::bt601, agi::ycbcr_range::tv};

	/// a list of byte positions detailing where in the file
	/// each frame header can be found
	std::vector<uint64_t> seek_table;

	void CheckFileFormat();
	void ParseFileHeader(const std::vector<std::string>& tags);
	Y4M_FrameFlags ParseFrameHeader(const std::vector<std::string>& tags);
	std::vector<std::string> ReadHeader(uint64_t &startpos);
	int IndexFile(uint64_t pos);

public:
	YUV4MPEGVideoProvider(agi::fs::path const& filename);

	std::shared_ptr<VideoFrame> GetFrame(int n) override;
	void SetColorSpace(std::string const&) override { }

	int GetFrameCount() const override             { return num_frames; }
	int GetWidth() const override                  { return w; }
	int GetHeight() const override                 { return h; }
	double GetDAR() const override                 { return 0; }
	agi::vfr::Framerate GetFPS() const override    { return fps; }
	std::vector<int> GetKeyFrames() const override { return {}; }
	std::string GetColorSpace() const override     { return "TV.601"; }
	std::string GetDecoderName() const override    { return "YU4MPEG"; }
	bool WantsCaching() const override             { return true; }
};

/// @brief Constructor
/// @param filename The filename to open
YUV4MPEGVideoProvider::YUV4MPEGVideoProvider(agi::fs::path const& filename)
: file(filename)
{
	CheckFileFormat();

	uint64_t pos = 0;
	ParseFileHeader(ReadHeader(pos));

	if (w <= 0 || h <= 0)
		throw VideoOpenError("Invalid resolution");
	if (fps_rat.num <= 0 || fps_rat.den <= 0) {
		fps_rat.num = 25;
		fps_rat.den = 1;
		LOG_D("provider/video/yuv4mpeg") << "framerate info unavailable, assuming 25fps";
	}
	if (pixfmt == Y4M_PIXFMT_NONE)
		pixfmt = Y4M_PIXFMT_420JPEG;
	if (imode == Y4M_ILACE_NOTSET)
		imode = Y4M_ILACE_UNKNOWN;

	luma_sz = w * h;
	switch (pixfmt) {
	case Y4M_PIXFMT_420JPEG:
	case Y4M_PIXFMT_420MPEG2:
	case Y4M_PIXFMT_420PALDV:
		chroma_sz	= (w * h) >> 2; break;
	case Y4M_PIXFMT_422:
		chroma_sz	= (w * h) >> 1; break;
		/// @todo add support for more pixel formats
	default:
		throw VideoOpenError("Unsupported pixel format");
	}
	frame_sz	= luma_sz + chroma_sz*2;

	num_frames = IndexFile(pos);
	if (num_frames <= 0 || seek_table.empty())
		throw VideoOpenError("Unable to determine file length");
}

/// @brief Checks if the file is an YUV4MPEG file or not
/// Note that it reports the error by throwing an exception,
/// not by returning a false value.
void YUV4MPEGVideoProvider::CheckFileFormat() {
	if (file.size() < 10)
		throw VideoNotSupported("CheckFileFormat: File is not a YUV4MPEG file (too small)");
	if (strncmp("YUV4MPEG2 ", file.read(0, 10), 10))
		throw VideoNotSupported("CheckFileFormat: File is not a YUV4MPEG file (bad magic)");
}

/// @brief Read a frame or file header at a given file position
/// @param startpos		The byte offset at where to start reading
/// @return				A list of parameters
std::vector<std::string> YUV4MPEGVideoProvider::ReadHeader(uint64_t &pos) {
	std::vector<std::string> tags;
	if (pos >= file.size())
		return tags;

	auto len = std::min<uint64_t>(YUV4MPEG_HEADER_MAXLEN, file.size() - pos);
	auto buff = file.read(pos, len);

	// read header until terminating newline (0x0A) is found
	auto curtag = buff;
	auto end = buff + len;
	for (; buff < end && *buff != 0x0A; ++buff, ++pos) {
		if (*buff == 0)
			throw VideoOpenError("ReadHeader: Malformed header (unexpected NUL)");

		if (*buff == 0x20) {
			if (curtag != buff)
				tags.emplace_back(curtag, buff);
			curtag = buff + 1;
		}
	}

	if (buff == end)
		throw VideoOpenError("ReadHeader: Malformed header (no terminating newline found)");

	// if only one tag with no trailing space was found (possible in the
	// FRAME header case), make sure we get it
	if (curtag != buff)
		tags.emplace_back(curtag, buff);

	pos += 1; // Move past newline

	return tags;
}

/// @brief Parses a list of parameters and sets reader state accordingly
/// @param tags	The list of parameters to parse
void YUV4MPEGVideoProvider::ParseFileHeader(const std::vector<std::string>& tags) {
	if (tags.size() <= 1)
		throw VideoOpenError("ParseFileHeader: contentless header");
	if (tags.front() != "YUV4MPEG2")
		throw VideoOpenError("ParseFileHeader: malformed header (bad magic)");

	// temporary stuff
	int t_w			= -1;
	int t_h			= -1;
	int t_fps_num	= -1;
	int t_fps_den	= -1;
	Y4M_InterlacingMode t_imode	= Y4M_ILACE_NOTSET;
	Y4M_PixelFormat t_pixfmt	= Y4M_PIXFMT_NONE;

	for (unsigned i = 1; i < tags.size(); i++) {
		char type = tags[i][0];
		std::string tag = tags[i].substr(1);

		if (type == 'W') {
			if (!agi::util::try_parse(tag, &t_w))
				throw VideoOpenError("ParseFileHeader: invalid width");
		}
		else if (type == 'H') {
			if (!agi::util::try_parse(tag, &t_h))
				throw VideoOpenError("ParseFileHeader: invalid height");
		}
		else if (type == 'F') {
			size_t pos = tag.find(':');
			if (pos == tag.npos)
				throw VideoOpenError("ParseFileHeader: invalid framerate");

			if (!agi::util::try_parse(tag.substr(0, pos), &t_fps_num) ||
				!agi::util::try_parse(tag.substr(pos + 1), &t_fps_den))
				throw VideoOpenError("ParseFileHeader: invalid framerate");
		}
		else if (type == 'C') {
			// technically this should probably be case sensitive,
			// but being liberal in what you accept doesn't hurt
			boost::to_lower(tag);
			if (tag == "420")			t_pixfmt = Y4M_PIXFMT_420JPEG; // is this really correct?
			else if (tag == "420jpeg")	t_pixfmt = Y4M_PIXFMT_420JPEG;
			else if (tag == "420mpeg2")	t_pixfmt = Y4M_PIXFMT_420MPEG2;
			else if (tag == "420paldv")	t_pixfmt = Y4M_PIXFMT_420PALDV;
			else if (tag == "411")		t_pixfmt = Y4M_PIXFMT_411;
			else if (tag == "422")		t_pixfmt = Y4M_PIXFMT_422;
			else if (tag == "444")		t_pixfmt = Y4M_PIXFMT_444;
			else if (tag == "444alpha")	t_pixfmt = Y4M_PIXFMT_444ALPHA;
			else if (tag == "mono")		t_pixfmt = Y4M_PIXFMT_MONO;
			else
				throw VideoOpenError("ParseFileHeader: invalid or unknown colorspace");
		}
		else if (type == 'I') {
			boost::to_lower(tag);
			if (tag == "p")			t_imode = Y4M_ILACE_PROGRESSIVE;
			else if (tag == "t")	t_imode = Y4M_ILACE_TFF;
			else if (tag == "b")	t_imode = Y4M_ILACE_BFF;
			else if (tag == "m")	t_imode = Y4M_ILACE_MIXED;
			else if (tag == "?")	t_imode = Y4M_ILACE_UNKNOWN;
			else
				throw VideoOpenError("ParseFileHeader: invalid or unknown interlacing mode");
		}
		else
			LOG_D("provider/video/yuv4mpeg") << "Unparsed tag: " << tags[i];
	}

	// The point of all this is to allow multiple YUV4MPEG2 headers in a single file
	// (can happen if you concat several files) as long as they have identical
	// header flags. The spec doesn't explicitly say you have to allow this,
	// but the "reference implementation" (mjpegtools) does, so I'm doing it too.
	if (inited) {
		if (t_w > 0 && t_w != w)
			throw VideoOpenError("ParseFileHeader: illegal width change");
		if (t_h > 0 && t_h != h)
			throw VideoOpenError("ParseFileHeader: illegal height change");
		if ((t_fps_num > 0 && t_fps_den > 0) && (t_fps_num != fps_rat.num || t_fps_den != fps_rat.den))
			throw VideoOpenError("ParseFileHeader: illegal framerate change");
		if (t_pixfmt != Y4M_PIXFMT_NONE && t_pixfmt != pixfmt)
			throw VideoOpenError("ParseFileHeader: illegal colorspace change");
		if (t_imode != Y4M_ILACE_NOTSET && t_imode != imode)
			throw VideoOpenError("ParseFileHeader: illegal interlacing mode change");
	}
	else {
		w = t_w;
		h = t_h;
		fps_rat.num = t_fps_num;
		fps_rat.den = t_fps_den;
		pixfmt		= t_pixfmt	!= Y4M_PIXFMT_NONE	? t_pixfmt	: Y4M_PIXFMT_420JPEG;
		imode		= t_imode	!= Y4M_ILACE_NOTSET	? t_imode	: Y4M_ILACE_UNKNOWN;
		fps = double(fps_rat.num) / fps_rat.den;
		inited = true;
	}
}

/// @brief Parses a frame header
/// @param tags	The list of parameters to parse
/// @return	The flags set, as a binary mask
///	This function is currently unimplemented (it will always return Y4M_FFLAG_NONE).
YUV4MPEGVideoProvider::Y4M_FrameFlags YUV4MPEGVideoProvider::ParseFrameHeader(const std::vector<std::string>& tags) {
	if (tags.front() != "FRAME")
		throw VideoOpenError("ParseFrameHeader: malformed frame header (bad magic)");

	/// @todo implement parsing of frame flags

	return Y4M_FFLAG_NONE;
}

/// @brief Indexes the file
/// @return The number of frames found in the file
/// This function goes through the file, finds and parses all file and frame headers,
/// and creates a seek table that lists the byte positions of all frames so seeking
/// can easily be done.
int YUV4MPEGVideoProvider::IndexFile(uint64_t pos) {
	int framecount = 0;

	// the ParseFileHeader() call in LoadVideo() will already have read
	// the file header for us and set the seek position correctly
	while (true) {
		// continue reading headers until no more are found
		std::vector<std::string> tags = ReadHeader(pos);
		if (tags.empty())
			break; // no more headers

		Y4M_FrameFlags flags = Y4M_FFLAG_NOTSET;
		if (tags.front() == "YUV4MPEG2") {
			ParseFileHeader(tags);
			continue;
		}
		else if (tags.front() == "FRAME")
			flags = ParseFrameHeader(tags);

		if (flags == Y4M_FFLAG_NONE) {
			framecount++;
			seek_table.push_back(pos);
			pos += frame_sz;
		}
		else {
			/// @todo implement rff flags etc
		}
	}

	return framecount;
}

std::shared_ptr<VideoFrame> YUV4MPEGVideoProvider::GetFrame(int n) {
	n = mid(0, n, num_frames - 1);

	int uv_width = w / 2;
	switch (pixfmt) {
		case Y4M_PIXFMT_420JPEG:
		case Y4M_PIXFMT_420MPEG2:
		case Y4M_PIXFMT_420PALDV:
			break;
		/// @todo add support for more pixel formats
		default:
			throw VideoNotSupported("YUV4MPEG video provider: GetFrame: Unsupported source colorspace");
	}

	auto src_y = reinterpret_cast<const unsigned char *>(file.read(seek_table[n], luma_sz + chroma_sz * 2));
	auto src_u = src_y + luma_sz;
	auto src_v = src_u + chroma_sz;
	std::vector<unsigned char> data;
	data.resize(w * h * 4);
	unsigned char *dst = &data[0];

	for (int py = 0; py < h; ++py) {
		for (int px = 0; px < w / 2; ++px) {
			const uint8_t u = *src_u++;
			const uint8_t v = *src_v++;
			for (unsigned int i = 0; i < 2; ++i) {
				const uint8_t y = *src_y++;
				auto rgb = conv.ycbcr_to_rgb({{y, u, v}});
				*dst++ = rgb[2];
				*dst++ = rgb[1];
				*dst++ = rgb[0];
				*dst++ = 0;
			}
		}

		// Roll back u/v on even lines
		if (!(py & 1)) {
			src_u -= uv_width;
			src_v -= uv_width;
		}
	}

	return std::make_shared<VideoFrame>(std::move(data), w, h, w * 4, false);
}
}

namespace agi { class BackgroundRunner; }
std::unique_ptr<VideoProvider> CreateYUV4MPEGVideoProvider(agi::fs::path const& path, std::string const&, agi::BackgroundRunner *) {
	return agi::make_unique<YUV4MPEGVideoProvider>(path);
}
