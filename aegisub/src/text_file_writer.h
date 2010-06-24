// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file text_file_writer.h
/// @see text_file_writer.cpp
/// @ingroup utility
///



#ifndef AGI_PRE
#include <fstream>
#include <memory>

#include <wx/string.h>
#endif

namespace agi {
	namespace charset {
		class IconvWrapper;
	}
}


/// DOCME
/// @class TextFileWriter
/// @brief DOCME
///
/// DOCME
class TextFileWriter {
private:

	/// DOCME
	std::ofstream file;

	/// DOCME
	std::auto_ptr<agi::charset::IconvWrapper> conv;

	TextFileWriter(const TextFileWriter&);
	TextFileWriter& operator=(const TextFileWriter&);

public:
	TextFileWriter(wxString const& filename, wxString encoding="");
	~TextFileWriter();

	void WriteLineToFile(wxString line, bool addLineBreak=true);
};

#if wxUSE_UNICODE_UTF8
#define wxSTRING_ENCODING "utf-8"
#elif defined(_WIN32)
#define wxSTRING_ENCODING "utf-16le"
#else
#error wx must be built with wxUSE_UNICODE_UTF8
#endif
