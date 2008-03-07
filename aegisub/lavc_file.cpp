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



#ifdef WITH_FFMPEG
#include <wx/wxprec.h>
#include <wx/filename.h>
#include "lavc_file.h"

LAVCFile::Initializer LAVCFile::init;

LAVCFile::Initializer::Initializer()
{
	av_register_all();
}

LAVCFile::LAVCFile(Aegisub::String _filename)
{
	int result = 0;
	fctx = NULL;
	wxString filename = _filename.c_str();

#ifdef WIN32
	wxFileName fn(filename);
	filename = fn.GetShortPath();
#endif

	result = av_open_input_file(&fctx,filename.mb_str(wxConvLocal),NULL,0,NULL);
	if (result != 0) throw _T("Failed opening file.");

	// Get stream info
	result = av_find_stream_info(fctx);
	if (result < 0) {
		av_close_input_file(fctx);
		fctx = NULL;
		throw _T("Unable to read stream info");
	}
	refs = 1;
}

LAVCFile::~LAVCFile()
{
	if (fctx)
		av_close_input_file(fctx);
}


#endif // WITH_FFMPEG
