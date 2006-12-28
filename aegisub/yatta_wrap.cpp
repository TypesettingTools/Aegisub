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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <fstream>
#include "yatta_wrap.h"
#include "text_file_reader.h"


/////////////////////////////////
// Read scene change information
wxArrayInt YattaWrap::GetKeyFrames(wxString filename) {
	// Prepare
	wxArrayInt result;
	TextFileReader reader(filename);
	bool isIn = false;

	// Read
	while (reader.HasMoreLines()) {
		// Get line
		wxString line = reader.ReadLineFromFile();

		// Start section
		if (line.Upper() == _T("[SECTIONS]")) isIn = true;
		else if (isIn && line.Left(1) == _T("[")) {
			isIn = false;
			break;
		}

		// Read line
		if (isIn) {
			// Remove preset number
			size_t pos = line.Find(_T(","));
			if (pos != -1) line = line.Left(pos);

			// Get value
			long temp;
			if (line.IsNumber()) {
				line.ToLong(&temp);
				result.Add(temp);
			}
		}
	}

	// Done
	return result;
}
