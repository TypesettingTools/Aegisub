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

#include "video_provider_yuv4mpeg.h"

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
YUV4MPEGVideoProvider::YUV4MPEGVideoProvider(wxString filename) {
	sf			= NULL;
	w			= 0;
	h			= 0;
	cur_fn		= -1;
	inited		= false;
	pixfmt		= Y4M_PIXFMT_NONE;
	imode		= Y4M_ILACE_NOTSET;
	num_frames	= -1;
	fps_rat.num	= -1;
	fps_rat.den = 1;
	seek_table.clear();

	errmsg = _T("YUV4MPEG video provider: ");

	try {
		LoadVideo(filename);
	}
	catch (wxString temp) {
		Close();
		errmsg.Append(temp);
		throw errmsg;
	}
	catch (...) {
		Close();
		throw;
	}
}


/// @brief Destructor
YUV4MPEGVideoProvider::~YUV4MPEGVideoProvider() {
	Close();
}


/// @brief Open a video file
/// @param _filename	The video file to open
void YUV4MPEGVideoProvider::LoadVideo(const wxString _filename) {
	Close();

	wxString filename = wxFileName(_filename).GetShortPath();

#ifdef WIN32
	sf = _wfopen(filename.wc_str(), _T("rb"));
#else
	sf = fopen(filename.utf8_str(), "rb");
#endif

	if (sf == NULL)
		throw wxString::Format(_T("Failed to open file"));

	CheckFileFormat();
	
	ParseFileHeader(ReadHeader(0, false));

	if (w <= 0 || h <= 0)
		throw wxString(_T("Invalid resolution"));
	if (fps_rat.num <= 0 || fps_rat.den <= 0) {
		fps_rat.num = 25;
		fps_rat.den = 1;
		wxLogDebug(_T("YUV4MPEG video provider: framerate info unavailable, assuming 25fps"));
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
			throw wxString(_T("Unsupported pixel format"));
	}
	frame_sz	= luma_sz + chroma_sz*2; 

	num_frames = IndexFile();
	if (num_frames <= 0 || seek_table.empty())
		throw wxString(_T("Unable to determine file length"));
	cur_fn = 0;

	fseeko(sf, 0, SEEK_SET);
}


/// @brief Closes the currently open file (if any) and resets reader state
void YUV4MPEGVideoProvider::Close() {
	seek_table.clear();
	if (sf)
		fclose(sf);
	sf = NULL;
}


/// @brief Checks if the file is an YUV4MPEG file or not
/// Note that it reports the error by throwing an exception,
/// not by returning a false value.
void YUV4MPEGVideoProvider::CheckFileFormat() {
	char buf[10];
	if (fread(buf, 10, 1, sf) != 1)
		throw wxString(_T("CheckFileFormat: Failed reading header"));
	if (strncmp("YUV4MPEG2 ", buf, 10))
		throw wxString(_T("CheckFileFormat: File is not a YUV4MPEG file (bad magic)"));

	fseeko(sf, 0, SEEK_SET);
}


/// @brief Read a frame or file header at a given file position
/// @param startpos		The byte offset at where to start reading
/// @param reset_pos	If true, the function will reset the file position to what it was before the function call before returning
/// @return				A list of parameters
std::vector<wxString> YUV4MPEGVideoProvider::ReadHeader(int64_t startpos, bool reset_pos) {
	int64_t oldpos = ftello(sf);
	std::vector<wxString> tags;
	wxString curtag	= _T("");
	int bytesread	= 0;
	int buf;

	if (fseeko(sf, startpos, SEEK_SET))
		throw wxString::Format(_T("YUV4MPEG video provider: ReadHeader: failed seeking to position %d"), startpos);

	// read header until terminating newline (0x0A) is found
	while ((buf = fgetc(sf)) != 0x0A) {
		if (ferror(sf))
			throw wxString(_T("ReadHeader: Failed to read from file"));
		if (feof(sf)) {
			// you know, this is one of the places where it would be really nice
			// to be able to throw an exception object that tells the caller that EOF was reached
			wxLogDebug(_T("YUV4MPEG video provider: ReadHeader: Reached EOF, returning"));
			break;
		}

		// some basic low-effort sanity checking
		if (buf == 0x00)
			throw wxString(_T("ReadHeader: Malformed header (unexpected NUL)"));
		if (++bytesread >= YUV4MPEG_HEADER_MAXLEN)
			throw wxString(_T("ReadHeader: Malformed header (no terminating newline found)"));

		// found a new tag
		if (buf == 0x20) {
			tags.push_back(curtag);
			curtag.Clear();
		}
		else
			curtag.Append(static_cast<wxChar>(buf));
	}
	// if only one tag with no trailing space was found (possible in the
	// FRAME header case), make sure we get it
	if (!curtag.IsEmpty()) {
		tags.push_back(curtag);
		curtag.Clear();
	}

	if (reset_pos)
		fseeko(sf, oldpos, SEEK_SET);

	return tags;
}


/// @brief Parses a list of parameters and sets reader state accordingly
/// @param tags	The list of parameters to parse
void YUV4MPEGVideoProvider::ParseFileHeader(const std::vector<wxString>& tags) {
	if (tags.size() <= 1)
		throw wxString(_T("ParseFileHeader: contentless header"));
	if (tags.front().Cmp(_T("YUV4MPEG2")))
		throw wxString(_T("ParseFileHeader: malformed header (bad magic)"));

	// temporary stuff
	int t_w			= -1;
	int t_h			= -1;
	int t_fps_num	= -1;
	int t_fps_den	= -1;
	Y4M_InterlacingMode t_imode	= Y4M_ILACE_NOTSET;
	Y4M_PixelFormat t_pixfmt	= Y4M_PIXFMT_NONE;

	for (unsigned i = 1; i < tags.size(); i++) {
		wxString tag = _T("");
		long tmp_long1 = 0;
		long tmp_long2 = 0;

		if (tags.at(i).StartsWith(_T("W"), &tag)) {
			if (!tag.ToLong(&tmp_long1))
				throw wxString(_T("ParseFileHeader: invalid width"));
			t_w = (int)tmp_long1;
		}
		else if (tags.at(i).StartsWith(_T("H"), &tag)) {
			if (!tag.ToLong(&tmp_long1))
				throw wxString(_T("ParseFileHeader: invalid height"));
			t_h = (int)tmp_long1;
		}
		else if (tags.at(i).StartsWith(_T("F"), &tag)) {
			if (!(tag.BeforeFirst(':')).ToLong(&tmp_long1) && tag.AfterFirst(':').ToLong(&tmp_long2))
				throw wxString(_T("ParseFileHeader: invalid framerate"));
			t_fps_num = (int)tmp_long1;
			t_fps_den = (int)tmp_long2;
		}
		else if (tags.at(i).StartsWith(_T("C"), &tag)) {
			// technically this should probably be case sensitive,
			// but being liberal in what you accept doesn't hurt
			if (!tag.CmpNoCase(_T("420")))				t_pixfmt = Y4M_PIXFMT_420JPEG; // is this really correct?
			else if (!tag.CmpNoCase(_T("420jpeg")))		t_pixfmt = Y4M_PIXFMT_420JPEG; 
			else if (!tag.CmpNoCase(_T("420mpeg2")))	t_pixfmt = Y4M_PIXFMT_420MPEG2;
			else if (!tag.CmpNoCase(_T("420paldv")))	t_pixfmt = Y4M_PIXFMT_420PALDV;
			else if (!tag.CmpNoCase(_T("411")))			t_pixfmt = Y4M_PIXFMT_411;
			else if (!tag.CmpNoCase(_T("422")))			t_pixfmt = Y4M_PIXFMT_422;
			else if (!tag.CmpNoCase(_T("444")))			t_pixfmt = Y4M_PIXFMT_444;
			else if (!tag.CmpNoCase(_T("444alpha")))	t_pixfmt = Y4M_PIXFMT_444ALPHA;
			else if (!tag.CmpNoCase(_T("mono")))		t_pixfmt = Y4M_PIXFMT_MONO;
			else
				throw wxString(_T("ParseFileHeader: invalid or unknown colorspace"));
		}
		else if (tags.at(i).StartsWith(_T("I"), &tag)) {
			if (!tag.CmpNoCase(_T("p")))		t_imode = Y4M_ILACE_PROGRESSIVE;
			else if (!tag.CmpNoCase(_T("t")))	t_imode = Y4M_ILACE_TFF;
			else if (!tag.CmpNoCase(_T("b")))	t_imode = Y4M_ILACE_BFF;
			else if (!tag.CmpNoCase(_T("m")))	t_imode = Y4M_ILACE_MIXED;
			else if (!tag.CmpNoCase(_T("?")))	t_imode = Y4M_ILACE_UNKNOWN;
			else
				throw wxString(_T("ParseFileHeader: invalid or unknown interlacing mode"));
		}
		else
			wxLogDebug(_T("ParseFileHeader: unparsed tag: %s"), tags.at(i).c_str());
	}

	// The point of all this is to allow multiple YUV4MPEG2 headers in a single file
	// (can happen if you concat several files) as long as they have identical
	// header flags. The spec doesn't explicitly say you have to allow this,
	// but the "reference implementation" (mjpegtools) does, so I'm doing it too.
	if (inited) {
		if (t_w > 0 && t_w != w)
			throw wxString(_T("ParseFileHeader: illegal width change"));
		if (t_h > 0 && t_h != h)
			throw wxString(_T("ParseFileHeader: illegal height change"));
		if ((t_fps_num > 0 && t_fps_den > 0) && (t_fps_num != fps_rat.num || t_fps_den != fps_rat.den))
			throw wxString(_T("ParseFileHeader: illegal framerate change"));
		if (t_pixfmt != Y4M_PIXFMT_NONE && t_pixfmt != pixfmt)
			throw wxString(_T("ParseFileHeader: illegal colorspace change"));
		if (t_imode != Y4M_ILACE_NOTSET && t_imode != imode)
			throw wxString(_T("ParseFileHeader: illegal interlacing mode change"));
	}
	else {
		w = t_w;
		h = t_h;
		fps_rat.num = t_fps_num;
		fps_rat.den = t_fps_den;
		pixfmt		= t_pixfmt	!= Y4M_PIXFMT_NONE	? t_pixfmt	: Y4M_PIXFMT_420JPEG;
		imode		= t_imode	!= Y4M_ILACE_NOTSET	? t_imode	: Y4M_ILACE_UNKNOWN;
		inited = true;
	}
}


/// @brief Parses a frame header
/// @param tags	The list of parameters to parse
/// @return	The flags set, as a binary mask
///	This function is currently unimplemented (it will always return Y4M_FFLAG_NONE).
YUV4MPEGVideoProvider::Y4M_FrameFlags YUV4MPEGVideoProvider::ParseFrameHeader(const std::vector<wxString>& tags) {
	if (tags.front().Cmp(_("FRAME")))
		throw wxString(_T("ParseFrameHeader: malformed frame header (bad magic)"));

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
		std::vector<wxString> tags = ReadHeader(curpos, false);
		curpos = ftello(sf);

		if (tags.empty())
			break; // no more headers

		Y4M_FrameFlags flags = Y4M_FFLAG_NOTSET;
		if (!tags.front().Cmp(_T("YUV4MPEG2"))) {
			ParseFileHeader(tags);
			continue;
		}
		else if (!tags.front().Cmp(_T("FRAME")))
			flags = ParseFrameHeader(tags);

		if (flags == Y4M_FFLAG_NONE) {
			framecount++;
			seek_table.push_back(curpos);
			// seek to next frame header start position
			if (fseeko(sf, frame_sz, SEEK_CUR))
				throw wxString::Format(_T("IndexFile: failed seeking to position %d"), curpos + frame_sz);
		}
		else {
			/// @todo implement rff flags etc
		}
	}

	return framecount;
}



/// @brief	Gets a given frame
/// @param n	The frame number to return
/// @return		The video frame
const AegiVideoFrame YUV4MPEGVideoProvider::GetFrame(int n) {
	// don't try to seek to insane places
	if (n < 0)
		n = 0;
	if (n >= num_frames)
		n = num_frames-1;
	// set position
	cur_fn = n;

	VideoFrameFormat src_fmt, dst_fmt;
	dst_fmt = FORMAT_RGB32;
	int uv_width;
	switch (pixfmt) {
		case Y4M_PIXFMT_420JPEG:
		case Y4M_PIXFMT_420MPEG2:
		case Y4M_PIXFMT_420PALDV:
			src_fmt = FORMAT_YV12; uv_width = w / 2; break;
		case Y4M_PIXFMT_422:
			src_fmt = FORMAT_YUY2; uv_width = w / 2; break; 
		/// @todo add support for more pixel formats
		default:
			throw wxString(_T("YUV4MPEG video provider: GetFrame: Unsupported source colorspace"));
	}
	
	AegiVideoFrame tmp_frame;

	tmp_frame.format = src_fmt;
	tmp_frame.w = w;
	tmp_frame.h = h;
	tmp_frame.invertChannels = false;
	tmp_frame.pitch[0] = w;
	for (int i=1;i<=2;i++)
		tmp_frame.pitch[i] = uv_width;
	tmp_frame.Allocate();

	fseeko(sf, seek_table[n], SEEK_SET);
	size_t ret;
	ret = fread(tmp_frame.data[0], luma_sz, 1, sf);
	if (ret != 1 || feof(sf) || ferror(sf))
		throw wxString(_T("YUV4MPEG video provider: GetFrame: failed to read luma plane"));
	for (int i = 1; i <= 2; i++) {
		ret = fread(tmp_frame.data[i], chroma_sz, 1, sf);
		if (ret != 1 || feof(sf) || ferror(sf))
			throw wxString(_T("YUV4MPEG video provider: GetFrame: failed to read chroma planes"));
	}

	AegiVideoFrame dst_frame;
	dst_frame.invertChannels = true;
	dst_frame.ConvertFrom(tmp_frame, dst_fmt);

	tmp_frame.Clear();

	return dst_frame;
}



// Utility functions
int YUV4MPEGVideoProvider::GetWidth() {
	return w;
}

int YUV4MPEGVideoProvider::GetHeight() {
	return h;
}

int YUV4MPEGVideoProvider::GetFrameCount() {
	return num_frames;
}

int YUV4MPEGVideoProvider::GetPosition() {
	return cur_fn;
}

double YUV4MPEGVideoProvider::GetFPS() {
	return double(fps_rat.num) / double(fps_rat.den);
}

