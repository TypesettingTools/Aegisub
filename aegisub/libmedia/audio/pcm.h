// Copyright (c) 2007-2008, Niels Martin Hansen
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

/// @file audio_provider_pcm.h
/// @see audio_provider_pcm.cpp
/// @ingroup audio_input
///

#ifndef AGI_PRE
#include <vector>

#include <wx/file.h>
#include <wx/thread.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include "include/aegisub/audio_provider.h"


namespace media {

/// DOCME
/// @class PCMAudioProvider
/// @brief DOCME
///
/// DOCME
class PCMAudioProvider : public AudioProvider {
private:
#ifdef _WIN32

	/// DOCME
	HANDLE file_handle;

	/// DOCME
	HANDLE file_mapping;

	/// DOCME
	mutable void *current_mapping;

	/// DOCME
	mutable int64_t mapping_start;

	/// DOCME
	mutable size_t mapping_length;
#else
	int file_handle;
	mutable void *current_mapping;
	mutable off_t mapping_start;
	mutable size_t mapping_length;
#endif

protected:
	PCMAudioProvider(const wxString &filename); // Create base object and open the file mapping
	virtual ~PCMAudioProvider(); // Closes the file mapping
	char * EnsureRangeAccessible(int64_t range_start, int64_t range_length) const; // Ensure that the given range of bytes are accessible in the file mapping and return a pointer to the first byte of the requested range


	/// DOCME
	int64_t file_size; // Size of the opened file


	/// DOCME
	struct IndexPoint {

		/// DOCME
		int64_t start_byte;

		/// DOCME
		int64_t start_sample;

		/// DOCME
		int64_t num_samples;
	};

	/// DOCME
	typedef std::vector<IndexPoint> IndexVector;

	/// DOCME
	IndexVector index_points;

public:
	virtual void GetAudio(void *buf, int64_t start, int64_t count) const;
};

// Construct the right PCM audio provider (if any) for the file
AudioProvider *CreatePCMAudioProvider(const wxString &filename);

} // namespace

