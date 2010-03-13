// Copyright (c) 2009, Thomas Goyne
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the Aegisub Group nor the names of its contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.
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

#include <fstream>
#include <iostream>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/regex.h>
#include <wx/arrstr.h>

using namespace std;

class FileIterator {
private:
	unsigned int currentItem;
	bool inDir;
	wxDir currentDir;
	wxArrayString items;
public:
	FileIterator(int argc, const char **argv);
	bool Next(wxString *filename);
};

FileIterator::FileIterator(int argc, const char **argv)
 : currentItem(2), inDir(false), currentDir(), items(argc, argv) {
	if (argc == 2) {
		// No names passed on the command line, so read them from stdin
		char buffer[1024];
		while (cin.getline(buffer, 1024), cin.good()) {
			items.Add(buffer);
		}
	}
}

bool FileIterator::Next(wxString *filename) {
	// Currently iterating through a directory, just return the next file in that directory (if any)
	if (inDir) {
		if (currentDir.GetNext(filename)) {
			wxString path = currentDir.GetName();
			*filename = wxFileName(path, *filename).GetFullPath();
			return true;
		}
		inDir = false;
	}

	// No more items
	if (currentItem >= items.GetCount()) return false;
		
	wxFileName next;
	wxString current = items[currentItem];

	currentItem++;
	
	// Test if it's a directory
	if (wxFileName::DirExists(current)) {
		currentDir.Open(current);
		if (currentDir.GetFirst(filename, "", wxDIR_FILES)) {
			wxString path = currentDir.GetName();
			*filename = wxFileName(path, *filename).GetFullPath();
			inDir = true;
			return true;
		}
		// dir is empty, process next item instead
		return Next(filename);
	}
	// Test if it's a file
	if (wxFileName::FileExists(current)) {
		*filename = current;
		return true;
	}
	
	// Entry is neither a file nor a directory, just skip it
	return Next(filename);
}

int main(int argc, const char *argv[]) {
	if (argc < 2) {
		return 1;
	}
	wxFileName headerFileName(argv[1]);
	headerFileName.SetExt(L"h");
	ofstream outH(headerFileName.GetFullPath().char_str());
	ofstream outC(argv[1]);

	wxRegEx nameCleaner("[^A-Za-z_0-9]");
	wxString filename;
	FileIterator iter(argc, argv);

	while (iter.Next(&filename)) {
		ifstream infile(filename.char_str(), ios::binary);
		infile.seekg(0, ios::end);
		int infile_end = infile.tellg();
		infile.seekg(0, ios::beg);

		wxFileName file(filename);

		wxString identifier = file.GetName();

		// Hack to work around inserting files in the current directory
		if (file.GetDirs().Last() != ".")
			identifier.Append("_" + file.GetDirs().Last());

		nameCleaner.ReplaceAll(&identifier, "_");

		outC << "#include \"libresrc.h\"" << endl;
		outC << "const unsigned char " << identifier << "[] = {";
		bool first = true;
		char c[1];
		while (infile.read(c, 1).gcount() > 0) {
			if (!first) outC << ",";
			outC << (int)*(unsigned char *)c;
			first = false;
		}
		outC << "};" << endl;
		outH << "extern const unsigned char " << identifier << "[" << infile_end << "];" << endl;
	}

	return 0;
}

