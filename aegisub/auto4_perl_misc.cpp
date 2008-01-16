// Copyright (c) 2008, Simone Cociancich
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
// Contact: mailto:jiifurusu@gmail.com
//


#ifdef WITH_PERL


#include "auto4_perl.h"


namespace Automation4 {


  void xs_perl_misc(pTHX)
  {
	newXS("Aegisub::warn", log_warning, __FILE__);
	newXS("Aegisub::text_extents", text_extents, __FILE__);
  }


/////////////
// PerlLog
//

  XS(log_warning)
  {
	dXSARGS;
	wxString buffer;
	if(items >= 1) {
	  buffer = wxString(SvPV_nolen(ST(0)), wx2pl);
	  for(I32 i = 1; i < items; i++) {
		buffer << _T(" ") << wxString(SvPV_nolen(ST(i)), wx2pl);
	  }
	}
	wxLogWarning(buffer);
  }


////////////
// Others
//

  XS(text_extents)
  {
	/* TODO badly: rewrite this shit */
	dXSARGS;

	// Read the parameters
	SV *style; wxString text;
	if(items >= 2) {
	  // Enough of them
	  style = sv_mortalcopy(ST(0));
	  text = wxString(SvPV_nolen(ST(1)), pl2wx);
	}
	else {
	  // We needed 2 parameters at least!
	  XSRETURN_UNDEF;
	}

	// Get the AssStyle
	AssStyle *s;
	if(SvROK(style)) {
	  // Create one from the hassh
	  s = PerlAss::MakeAssStyle((HV*)SvRV(style));
	}
	else {
	  // It's the name of the style
	  wxString sn(SvPV_nolen(style), pl2wx);
	  // We get it from the AssFile::top
	  s = AssFile::top->GetStyle(sn);
	  /* TODO: make it dig from the current hassh's styles */
	}

	// The return parameters
	double width, height, descent, extlead;
	// The actual calculation
	if(!CalculateTextExtents(s, text, width, height, descent, extlead)) {
	  /* TODO: diagnose error */
	  XSRETURN_EMPTY;
	}

	// Returns
	switch(GIMME_V) {
	case G_ARRAY:
	  // List context
	  EXTEND(SP, 4);
	  XST_mNV(0, width);
	  XST_mNV(1, height);
	  XST_mNV(2, descent);
	  XST_mNV(3, extlead);
	  XSRETURN(4);
	  break;
	case G_SCALAR:
	  // Scalar context
	  XSRETURN_NV(width);
	}
  }

  
};


#endif //WITH_PERL
