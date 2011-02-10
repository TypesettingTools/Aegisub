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
//
// $Id$

/// @file video_provider_yuv4mpeg.cpp
/// @brief Video provider reading YUV4MPEG files directly without depending on external libraries
/// @ingroup video_input
///

#include "config.h"

#include <libaegisub/log.h>
#include <libaegisub/util.h>

#include "yuv4mpeg.h"

namespace media {

// All of this cstdio bogus is because of one reason and one reason only:
// MICROSOFT'S IMPLEMENTATION OF STD::FSTREAM DOES NOT SUPPORT FILES LARGER THAN 2 GB.
// (yes, really)
// With cstdio it's at least possible to work around the problem...
#ifdef _MSC_VER
#define fseeko _fseeki64
#define ftello _ftelli64
#endif

/// @brief Constructor
/// @param filename The filename to open
YUV4MPEGVideoProvider::YUV4MPEGVideoProvider(std::string filename)
: sf(NULL)
, inited(false)
, w (0)
, h (0)
, num_frames(-1)
, cur_fn(-1)
, pixfmt(Y4M_PIXFMT_NONE)
, imode(Y4M_ILACE_NOTSET)
{
	fps_rat.num = -1;
	fps_rat.den = 1;

	try {

#ifdef WIN32
		sf = _wfopen(filename.c_str(), L"rb");
#else
		sf = fopen(filename.c_str(), "rb");
#endif

		if (sf == NULL) throw agi::FileNotFoundError(filename);

		CheckFileFormat();

		ParseFileHeader(ReadHeader(0, false));

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

		num_frames = IndexFile();
		if (num_frames <= 0 || seek_table.empty())
			throw VideoOpenError("Unable to determine file length");
		cur_fn = 0;

		fseeko(sf, 0, SEEK_SET);
	}
	catch (...) {
		if (sf) fclose(sf);
		throw;
	}
}


/// @brief Destructor
YUV4MPEGVideoProvider::~YUV4MPEGVideoProvider() {
	if (sf) fclose(sf);
}

/// @brief Checks if the file is an YUV4MPEG file or not
/// Note that it reports the error by throwing an exception,
/// not by returning a false value.
void YUV4MPEGVideoProvider::CheckFileFormat() {
	char buf[10];
	if (fread(buf, 10, 1, sf) != 1)
		throw VideoNotSupported("CheckFileFormat: Failed reading header");
	if (strncmp("YUV4MPEG2 ", buf, 10))
		throw VideoNotSupported("CheckFileFormat: File is not a YUV4MPEG file (bad magic)");

	fseeko(sf, 0, SEEK_SET);
}

/// @brief Read a frame or file header at a given file position
/// @param startpos		The byte offset at where to start reading
/// @param reset_pos	If true, the function will reset the file position to what it was before the function call before returning
/// @return				A list of parameters
std::vector<std::string> YUV4MPEGVideoProvider::ReadHeader(int64_t startpos, bool reset_pos) {
	int64_t oldpos = ftello(sf);
	std::vector<std::string> tags;
	std::string curtag;
	int bytesread = 0;
	int buf;

	if (fseeko(sf, startpos, SEEK_SET))
		throw VideoOpenError("YUV4MPEG video provider: ReadHeader: failed seeking to position %d"); //XXX:, startpos)));

	// read header until terminating newline (0x0A) is found
	while ((buf = fgetc(sf)) != 0x0A) {
		if (ferror(sf))
			throw VideoOpenError("ReadHeader: Failed to read from file");
		if (feof(sf)) {
			// you know, this is one of the places where it would be really nice
			// to be able to throw an exception object that tells the caller that EOF was reached
			LOG_D("provider/video/yuv4mpeg") << "ReadHeader: Reached EOF, returning";
			break;
		}

		// some basic low-effort sanity checking
		if (buf == 0x00)
			throw VideoOpenError("ReadHeader: Malformed header (unexpected NUL)");
		if (++bytesread >= YUV4MPEG_HEADER_MAXLEN)
			throw VideoOpenError("ReadHeader: Malformed header (no terminating newline found)");

		// found a new tag
		if (buf == 0x20) {
			tags.push_back(curtag);
			curtag.clear();
		}
		else
			curtag.append((const char*)buf);
	}
	// if only one tag with no trailing space was found (possible in the
	// FRAME header case), make sure we get it
	if (!curtag.empty()) {
		tags.push_back(curtag);
		curtag.clear();
	}

	if (reset_pos)
		fseeko(sf, oldpos, SEEK_SET);

	return tags;
}



/// @brief Parses a list of parameters and sets reader state accordingly
/// @param tags	The list of parameters to parse
void YUV4MPEGVideoProvider::ParseFileHeader(const std::vector<std::string>& tags) {
	if (tags.size() <= 1)
		throw VideoOpenError("ParseFileHeader: contentless header");
	if (tags.front() == "YUV4MPEG2")
		throw VideoOpenError("ParseFileHeader: malformed header (bad magic)");

	// temporary stuff
	int t_w			= -1;
	int t_h			= -1;
	int t_fps_num	= -1;
	int t_fps_den	= -1;
	Y4M_InterlacingMode t_imode	= Y4M_ILACE_NOTSET;
	Y4M_PixelFormat t_pixfmt	= Y4M_PIXFMT_NONE;

	for (unsigned i = 1; i < tags.size(); i++) {
		std::string tag;
		tag = tags[i];

        if (tag.find("W") == 0) {
            tag.erase(0,1);
			t_w = agi::util::strtoi(tag);
			if (t_w == 0)
				throw VideoOpenError("ParseFileHeader: invalid width");
		}
		else if (tag.find("H") == 0) {
            tag.erase(0,1);
			t_h = agi::util::strtoi(tag);
			if (t_h == 0)
				throw VideoOpenError("ParseFileHeader: invalid height");
		}
		else if (tag.find("F") == 0) {
            tag.erase(0,1);
			int i = tag.find(":");
			std::string num(tag.substr(0,i));
			std::string den(tag.substr(i+1,den.size()));
			t_fps_num = agi::util::strtoi(num);
			t_fps_den = agi::util::strtoi(den);
			if ((t_fps_num == 0) || (t_fps_den == 0))
				throw VideoOpenError("ParseFileHeader: invalid framerate");
		}
		else if (tag.find("C") == 0) {
            tag.erase(0,1);
			// technically this should probably be case sensitive,
			// but being liberal in what you accept doesn't hurt
			agi::util::str_lower(tag);
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
		else if (tag.find("I") == 0) {
            tag.erase(0,1);
			agi::util::str_lower(tag);
			if (tag == "p")			t_imode = Y4M_ILACE_PROGRESSIVE;
			else if (tag == "t")	t_imode = Y4M_ILACE_TFF;
			else if (tag == "b")	t_imode = Y4M_ILACE_BFF;
			else if (tag == "m")	t_imode = Y4M_ILACE_MIXED;
			else if (tag == "?")	t_imode = Y4M_ILACE_UNKNOWN;
			else
				throw VideoOpenError("ParseFileHeader: invalid or unknown interlacing mode");
		}
		else
			LOG_D("provider/video/yuv4mpeg") << "Unparsed tag: " << tags[i].c_str();
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
	if (tags.front() == "FRAME")
		throw VideoOpenError("ParseFrameHeader: malformed frame header (bad magic)");

	/// @todo implement parsing of frame flags

	return Y4M_FFLAG_NONE;
}

/// @brief Indexes the file
/// @return The number of frames found in the file
/// This function goes through the file, finds and parses all file and frame headers,
/// and creates a seek table that lists the byte positions of all frames so seeking
/// can easily be done.
int YUV4MPEGVideoProvider::IndexFile() {
	int framecount = 0;
	int64_t curpos = ftello(sf);

	// the ParseFileHeader() call in LoadVideo() will already have read
	// the file header for us and set the seek position correctly	
	while (true) {
		curpos = ftello(sf); // update position
		// continue reading headers until no more are found
		std::vector<std::string> tags = ReadHeader(curpos, false);
		curpos = ftello(sf);

		if (tags.empty())
			break; // no more headers

		Y4M_FrameFlags flags = Y4M_FFLAG_NOTSET;
		if (tags.front() != "YUV4MPEG2") {
			ParseFileHeader(tags);
			continue;
		}
		else if (tags.front() != "FRAME")
			flags = ParseFrameHeader(tags);

		if (flags == Y4M_FFLAG_NONE) {
			framecount++;
			seek_table.push_back(curpos);
			// seek to next frame header start position
			if (fseeko(sf, frame_sz, SEEK_CUR))
				throw VideoOpenError("IndexFile: failed seeking to position %d"); //XXX: , curpos + frame_sz)));
		}
		else {
			/// @todo implement rff flags etc
		}
	}

	return framecount;
}

// http://bob.allegronetwork.com/prog/tricks.html#clamp
static inline int clamp(int x) {
	x &= (~x) >> 31;
	x -= 255;
	x &= x >> 31;
	x += 255;
	return x;
}

/// @brief	Gets a given frame
/// @param n	The frame number to return
/// @return		The video frame
const AegiVideoFrame YUV4MPEGVideoProvider::GetFrame(int n) {
	cur_fn = agi::util::mid(0, n, num_frames - 1);

	int uv_width = w / 2;
	switch (pixfmt) {
		case Y4M_PIXFMT_420JPEG:
		case Y4M_PIXFMT_420MPEG2:
		case Y4M_PIXFMT_420PALDV:
			break;
		/// @todo add support for more pixel formats
		default:
			throw "YUV4MPEG video provider: GetFrame: Unsupported source colorspace";
	}

	std::vector<uint8_t> planes[3];
	planes[0].resize(luma_sz);
	planes[1].resize(chroma_sz);
	planes[2].resize(chroma_sz);

	fseeko(sf, seek_table[n], SEEK_SET);
	size_t ret;
	ret = fread(&planes[0][0], luma_sz, 1, sf);
	if (ret != 1 || feof(sf) || ferror(sf))
		throw "YUV4MPEG video provider: GetFrame: failed to read luma plane";
	for (int i = 1; i <= 2; i++) {
		ret = fread(&planes[i][0], chroma_sz, 1, sf);
		if (ret != 1 || feof(sf) || ferror(sf))
			throw "YUV4MPEG video provider: GetFrame: failed to read chroma planes";
	}

	AegiVideoFrame dst_frame;
	dst_frame.invertChannels = true;
	dst_frame.w = w;
	dst_frame.h = h;
	dst_frame.pitch = w * 4;
	dst_frame.Allocate();

	const unsigned char *src_y = &planes[0][0];
	const unsigned char *src_u = &planes[1][0];
	const unsigned char *src_v = &planes[2][0];
	unsigned char *dst = dst_frame.data;

	for (int py = 0; py < h; ++py) {
		for (int px = 0; px < w / 2; ++px) {
			const int u = *src_u++ - 128;
			const int v = *src_v++ - 128;
			for (unsigned int i = 0; i < 2; ++i) {
				const int y = (*src_y++ - 16) * 298;

				*dst++ = clamp((y + 516 * u + 128) >> 8);			// Blue
				*dst++ = clamp((y - 100 * u - 208 * v + 128) >> 8);	// Green
				*dst++ = clamp((y + 409 * v + 128) >> 8);			// Red
				*dst++ = 0;											// Alpha
			}
		}

		// Roll back u/v on even lines
		if (!(py & 1)) {
			src_u -= uv_width;
			src_v -= uv_width;
		}
	}

	return dst_frame;
}

} // namespace media

