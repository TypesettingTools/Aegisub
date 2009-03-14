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
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#include <wx/wxprec.h>
#include <fstream>
#include <iostream>
#include "utils.h"
#include "../../../aegisub/src/md5.h"


bool AreFilesIdentical(std::string file1, std::string file2)
{
	std::string f1 = GetFileMD5(file1);
	std::string f2 = GetFileMD5(file2);
	return f1 == f2;
}

std::string GetFileMD5(std::string file) {
	md5_state_s md5;
	md5_byte_t digest[16];
	const size_t toRead = 512;
	md5_byte_t data[toRead];

	std::ifstream fp(file.c_str());
	if (fp.is_open()) {
		md5_init(&md5);

		while (!fp.eof()) {
			fp.read((char*)data,toRead);
			size_t n = fp.gcount();
			md5_append(&md5,data,n);
		}

		md5_finish(&md5,digest);
		unsigned int *res = (unsigned int*) digest;
		return std::string(wxString::Format(_T("%08X%08X%08X%08X"),res[0],res[1],res[2],res[3]).mb_str(wxConvUTF8));
	}

	else {
		return "";
	}
}
