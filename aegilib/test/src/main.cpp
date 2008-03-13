// Copyright (c) 2008, Rodrigo Braz Monteiro
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
// AEGISUB/AEGILIB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include <aegilib/aegilib.h>
#include <wx/wfstream.h>
#include <iostream>
#include "text_file_reader.h"
#include "text_file_writer.h"

int main () {
	using namespace std;
	using namespace Aegilib;

	cout << "Aegilib test program by amz.\n\n";

	try {
		// Set up the lib
		FormatManager::InitializeFormats();
		Aegilib::SetHostApplicationName(L"Aegilib test program");

		// Subtitles model
		Model subs;

		// Load subtitles
		cout << "Loading file... ";
		subs.LoadFile(L"subs_in.ass",L"UTF-8");
		cout << "Done.\n";

		// Modify subtitles
		cout << "Modifying file...";
		// TODO
		cout << "Done.\n";

		// Save subtitles
		cout << "Saving file... ";
		subs.SaveFile(L"subs_out.ass",L"UTF-8");
		cout << "Done.\n";
	}

	catch (std::exception &e) {
		cout << "\n\nException: " << e.what() << endl << endl;
	}
}
