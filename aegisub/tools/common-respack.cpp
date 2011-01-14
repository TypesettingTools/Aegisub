// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file common-respack.cpp
/// @brief Load any file into a byte array.
/// @ingroup util

#include <iterator>
#include <fstream>
#include <iostream>
#include <string>


/// Clean a filename for use as an identity.
/// @param str[in] String containing filename.
inline void clean(std::string &str) {
	// Remove path.
	std::string::size_type pos = str.rfind('/');
	if (pos != std::string::npos) {
		str = str.substr(pos+1, str.size());
	}

	// Chop extension.
	pos = str.rfind('.');
	if (pos != std::string::npos) {
		str = str.substr(0, pos);
	}

	for (unsigned int i = 0; i != str.size(); i++) {
		int c = (int)str[i];
		if (((c >= 65) && (c <= 90)) ||   /* A-Z */
			((c >= 97) && (c <= 122)) ||  /* a-z */
			((c >= 48) && (c <= 57)) ||   /* 0-9 */
			(c == 95)) {                  /* _ */

			continue;
		} else {
			str.erase(i, 1);
		}
	}
}


int main(int argc, const char *argv[]) {
	// Needs 3 arguments
	if (argc != 4) {
		std::cout << "Usage: <manifest>[in] <c++ file>[out] <header>[out]" << std::endl;
		return 1;
	}

	std::cout << "Manifest: " << argv[1] << "  CPP: " << argv[2] << "  Header: " << argv[3] << std::endl;

	std::ifstream file_manifest(argv[1]);
	std::ofstream file_cpp(argv[2]);
	std::ofstream file_h(argv[3]);

	// If the manifest has a path use that as the base for finding files.
	std::string manifest(argv[1]);
	std::string path_base;
	std::string::size_type pos = manifest.rfind('/');
	if (pos != std::string::npos) {
		path_base = manifest.substr(0, pos+1);
	}


	file_cpp << "#include \"libresrc.h\"" << std::endl;

	std::string file;	// File for array.
	while (file_manifest) {
		std::getline(file_manifest, file);
		if (file.empty()) continue;

		std::ifstream ifp((path_base + file).c_str(), std::ios_base::in|std::ios_base::binary);

		if (!ifp.is_open()) {
			std::cout << "Error opening file: " << file << std::endl;
			return 1;
		}

		// Identity used in C/Header files.
		std::string ident(file);
		clean(ident);

		file_cpp << "const unsigned char " << ident << "[] = {";

		/// Create byte-array.
		std::istreambuf_iterator<char> ifp_i(ifp);
		std::istreambuf_iterator<char> eof;
		int length = 0;

		while (ifp_i != eof) {
			if (length > 0) file_cpp << ",";
			file_cpp << (int)*ifp_i;

			++ifp_i;
			length++;
		}

		// Finish
		file_cpp << "};" << std::endl;

		// Prototype.
		file_h << "extern const unsigned char " << ident << "[" << length << "];" << std::endl;

	}

	return 0;
}
