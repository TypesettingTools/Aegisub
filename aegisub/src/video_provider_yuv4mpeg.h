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

/// @file video_provider_yuv4mpeg.h
/// @see video_provider_yuv4mpeg.cpp
/// @ingroup video_input
///


#pragma once

#include "include/aegisub/video_provider.h"
#include <wx/wxprec.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <stdio.h>
#include <vector>


/// DOCME
#define YUV4MPEG_HEADER_MAXLEN 128



/// @class YUV4MPEGVideoProvider
/// @brief DOCME
///
/// DOCME
class YUV4MPEGVideoProvider : public VideoProvider {
private:

	/// DOCME
	enum Y4M_PixelFormat {

		/// DOCME
		Y4M_PIXFMT_NONE		= -1,

		/// DOCME
		Y4M_PIXFMT_420JPEG,			// afaict the only difference between

		/// DOCME
		Y4M_PIXFMT_420MPEG2,		// these three is the chroma sample location,

		/// DOCME
		Y4M_PIXFMT_420PALDV,		// and nobody cares about that.

		/// DOCME
		Y4M_PIXFMT_411,

		/// DOCME
		Y4M_PIXFMT_422,

		/// DOCME
		Y4M_PIXFMT_444,

		/// DOCME
		Y4M_PIXFMT_444ALPHA,

		/// DOCME
		Y4M_PIXFMT_MONO,
	};


	/// DOCME
	enum Y4M_InterlacingMode {

		/// DOCME
		Y4M_ILACE_NOTSET = -1, // not to be confused with Y4M_ILACE_UNKNOWN

		/// DOCME
		Y4M_ILACE_PROGRESSIVE,

		/// DOCME
		Y4M_ILACE_TFF,

		/// DOCME
		Y4M_ILACE_BFF,

		/// DOCME
		Y4M_ILACE_MIXED,

		/// DOCME
		Y4M_ILACE_UNKNOWN,
	};


	/// DOCME
	enum Y4M_FrameFlags {

		/// DOCME
		Y4M_FFLAG_NOTSET	= -1,

		/// DOCME
		Y4M_FFLAG_NONE		= 0x0000,

		/// DOCME
		Y4M_FFLAG_R_TFF		= 0x0001, // TFF

		/// DOCME
		Y4M_FFLAG_R_TFF_R	= 0x0002, // TFF and repeat

		/// DOCME
		Y4M_FFLAG_R_BFF		= 0x0004, // BFF

		/// DOCME
		Y4M_FFLAG_R_BFF_R	= 0x0008, // BFF and repeat

		/// DOCME
		Y4M_FFLAG_R_P		= 0x0010, // progressive

		/// DOCME
		Y4M_FFLAG_R_P_R		= 0x0020, // progressive and repeat once

		/// DOCME
		Y4M_FFLAG_R_P_RR	= 0x0040, // progressive and repeat twice

		/// DOCME
		Y4M_FFLAG_T_P		= 0x0080, // progressive (fields sampled at the same time)

		/// DOCME
		Y4M_FFLAG_T_I		= 0x0100, // interlaced (fields sampled at different times)

		/// DOCME
		Y4M_FFLAG_C_P		= 0x0200, // progressive (whole frame subsampled)

		/// DOCME
		Y4M_FFLAG_C_I		= 0x0400, // interlaced (fields subsampled independently)

		/// DOCME
		Y4M_FFLAG_C_UNKNOWN = 0x0800, // unknown (only allowed for non-4:2:0 sampling)
	};


	/// DOCME
	FILE *sf;		// source file

	/// DOCME
	bool inited;

	/// DOCME

	/// DOCME
	int w, h;		// width/height

	/// DOCME
	int num_frames; // length of file in frames

	/// DOCME
	int frame_sz;	// size of each frame in bytes

	/// DOCME
	Y4M_PixelFormat pixfmt; // colorspace/pixel format

	/// DOCME
	Y4M_InterlacingMode imode; // interlacing mode
	struct {

		/// DOCME
		int num;

		/// DOCME
		int den;

	/// DOCME
	} fps_rat;		// framerate


	/// DOCME
	std::vector<int64_t> seek_table;	// the position in the file of each frame, in bytes

	/// DOCME
	int cur_fn;							// current frame number


	/// DOCME
	wxString errmsg;

	void LoadVideo(const wxString filename);
	void Close();
	
	void CheckFileFormat();
	void ParseFileHeader(const std::vector<wxString>& tags);
	Y4M_FrameFlags ParseFrameHeader(const std::vector<wxString>& tags);
	std::vector<wxString> ReadHeader(int64_t startpos, bool reset_pos=false);
	int IndexFile();

public:
	YUV4MPEGVideoProvider(wxString filename);
	~YUV4MPEGVideoProvider();

	const AegiVideoFrame GetFrame(int n);
	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();

	/// @brief DOCME
	/// @return 
	///
	bool AreKeyFramesLoaded() { return false; }

	/// @brief DOCME
	/// @return 
	///
	wxArrayInt GetKeyFrames() { return wxArrayInt(); }

	/// @brief DOCME
	/// @return 
	///
	bool IsVFR() { return false; };

	/// @brief DOCME
	/// @return 
	///
	FrameRate GetTrueFrameRate() { return FrameRate(); }

	/// @brief DOCME
	/// @return 
	///
	wxString GetDecoderName() { return L"YUV4MPEG"; }

	/// @brief DOCME
	///
	int GetDesiredCacheSize() { return 8; }
};



