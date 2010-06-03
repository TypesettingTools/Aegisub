// Copyright (c) 2010, Thomas Goyne
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file charset_conv.h
/// @see charset_conv.cpp
/// @ingroup utility
///

#ifndef AGI_PRE
#include <wx/string.h>
#include <wx/strconv.h>
#include <wx/thread.h>
#endif

#include "aegisub_endian.h"
#include <libaegisub/charset_conv.h>

/// @class AegisubCSConv
/// @brief wxMBConv implementation for converting to and from unicode
class AegisubCSConv : public wxMBConv {
public:

	// wxMBConv implementation; see strconv.h for usage details
	size_t ToWChar(wchar_t *dst, size_t dstLen, const char *src, size_t srcLen = wxNO_LEN) const;
	size_t FromWChar(char *dst, size_t dstLen, const wchar_t *src, size_t srcLen = wxNO_LEN) const;
	wxMBConv *Clone() const { return NULL; };

protected:
	AegisubCSConv();
private:
	AegisubCSConv(const AegisubCSConv&);
	AegisubCSConv& operator=(const AegisubCSConv&);
	wxString localCharset;

#if wxUSE_THREADS
	mutable wxMutex iconvMutex;
#endif

	// ToWChar and FromWChar are const in wxMBConv, but iconv can't be used
	// immutably
	mutable agi::charset::IconvWrapper conv;
};

extern AegisubCSConv& csConvLocal;
