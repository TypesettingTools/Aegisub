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

#include <iterator>
#include <fstream>
#include <iostream>
#include <string>

/// Clean a filename for use as an identity.
/// @param str[in] String containing filename.
void clean(std::string &str) {
	// Chop extension
	auto pos = str.rfind('.');
	if (pos != std::string::npos)
		str.erase(pos, str.size() - pos);

	// Remove path
	pos = str.rfind('/');
	if (pos != std::string::npos)
		str.erase(0, pos + 1);
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

	if (!file_manifest.good()) {
		std::cout << "Failed to open manifest" << std::endl;
		return 1;
	}

	if (!file_cpp.good()) {
		std::cout << "Failed to open output CPP file" << std::endl;
		return 1;
	}

	if (!file_h.good()) {
		std::cout << "Failed to open output H file" << std::endl;
		return 1;
	}

	// If the manifest has a path use that as the base for finding files.
	std::string manifest(argv[1]);
	std::string path_base;
	std::string::size_type pos = manifest.rfind('/');
	if (pos != std::string::npos) {
		path_base = manifest.substr(0, pos+1);
	}

	file_cpp << "#include \"libresrc.h\"\n";

	std::string file;
	while (std::getline(file_manifest, file)) {
		if (file.empty()) continue;

		std::ifstream ifp((path_base + file).c_str(), std::ios_base::binary);

		if (!ifp.is_open()) {
			std::cout << "Error opening file: " << file << std::endl;
			return 1;
		}

		clean(file);
		file_cpp << "const unsigned char " << file << "[] = {";

		size_t length = 0;
		for (std::istreambuf_iterator<char> it(ifp), end; it != end; ++it) {
			if (length > 0) file_cpp << ",";
			file_cpp << (unsigned int)(unsigned char)*it;
			++length;
		}

		file_cpp << "};\n";

		file_h << "extern const unsigned char " << file << "[" << length << "];\n";
	}

	return 0;
}
