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


////////////
// Includes
#include "ass_style_storage.h"
#include "ass_style.h"
#include "ass_file.h"
#include "main.h"
#include <fstream>


///////////////////////
// Save styles to disk
void AssStyleStorage::Save(wxString name) {
	if (name.IsEmpty()) return;

	using namespace std;
	ofstream file;

	wxString filename = AegisubApp::folderName;
	filename += _T("/catalog/");
	filename += name;
	filename += _T(".sty");

	file.open(filename.mb_str(wxConvLocal));
	for (list<AssStyle*>::iterator cur=style.begin();cur!=style.end();cur++) {
		file << (*cur)->GetEntryData().mb_str(wxConvUTF8) << endl;
	}

	file.close();
}


/////////////////////////
// Load styles from disk
void AssStyleStorage::Load(wxString name) {
	if (name.IsEmpty()) return;

	using namespace std;
	char buffer[65536];
	ifstream file;

	wxString filename = AegisubApp::folderName;
	filename += _T("/catalog/");
	filename += name;
	filename += _T(".sty");

	Clear();
	file.open(filename.mb_str(wxConvLocal));
	if (!file.is_open()) {
		throw _T("Failed opening file.");
	}

	AssStyle *curStyle;
	while (!file.eof()) {
		file.getline(buffer,65536);
		wxString data(buffer,wxConvUTF8);
		data.Trim();
		if (data.substr(0,6) == _T("Style:")) {
			try {
				curStyle = new AssStyle(data);
				style.push_back(curStyle);
			} catch(...) {
				/* just ignore invalid lines for now */
			}
		}
	}

	file.close();
}


/////////
// Clear
void AssStyleStorage::Clear () {
	using std::list;
	for (list<AssStyle*>::iterator cur=style.begin();cur!=style.end();cur++) {
		delete *cur;
	}
	style.clear();
}
