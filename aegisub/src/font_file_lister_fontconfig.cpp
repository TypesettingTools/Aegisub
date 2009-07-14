// Copyright (c) 2007, David Lamparter, Rodrigo Braz Monteiro
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
#include "config.h"

#include <wx/wxprec.h>
#ifndef WIN32
#include "font_file_lister_fontconfig.h"
#include "charset_conv.h"


///////////////////////////////////
// Get files that contain the face
wxArrayString FontConfigFontFileLister::DoGetFilesWithFace(wxString facename) {
	wxArrayString results;

	// Code stolen from asa
	FcPattern *final, *tmp1, *tmp2;
	FcResult res;
	FcChar8 *filename,*gotfamily;
	int fontindex;
	char buffer[1024];
	strcpy(buffer,facename.mb_str(wxConvUTF8));

	// Get data from fconfig or something
	tmp1 = FcPatternBuild(NULL,FC_FAMILY, FcTypeString,buffer,NULL);
	if (!tmp1) return results;
	tmp2 = FcFontRenderPrepare(fontconf, tmp1, aux);
	FcPatternDestroy(tmp1);
	FcDefaultSubstitute(tmp2);
	FcConfigSubstitute(fontconf, tmp2, FcMatchPattern);
	final = FcFontMatch(fontconf, tmp2, &res);
	FcPatternDestroy(tmp2);
	if (!final) return results;
	if (FcPatternGetString(final, FC_FILE, 0, &filename) == FcResultMatch && FcPatternGetInteger(final, FC_INDEX, 0, &fontindex) == FcResultMatch) {
		FcPatternGetString(final, FC_FAMILY, fontindex, &gotfamily);
		if (strcmp(gotfamily,buffer) == 0) {
			results.Add(wxString((char*) filename,csConvLocal));
		}
	}
	FcPatternDestroy(final);

	return results;
}


///////////////
// Constructor
FontConfigFontFileLister::FontConfigFontFileLister()
: fontconf(0), aux(0)
{
}


//////////////
// Initialize
void FontConfigFontFileLister::DoInitialize() {
	fontconf = FcInitLoadConfigAndFonts();
	aux = FcPatternCreate();
}


////////////
// Clean up
void FontConfigFontFileLister::DoClearData() {
	if (aux) FcPatternDestroy(aux);
#ifdef HAVE_FCFINI
	FcFini();
#endif
}

#endif
