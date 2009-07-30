// Copyright (c) 2005, Niels Martin Hansen
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

/// @file string_codec.h
/// @see string_codec.cpp
/// @ingroup utility
///
/// Functions for "inline string encoding" handling,
/// a simple encoding-form used for encoding strings that can't contain control codes and a few other special characters,
/// so they can be stored as part of a field in an ASS line
///
/// Even though the encoding will handle unicode strings, it can only encode ASCII characters.
/// This is not a problem, since only ASCII characters are used for the special purposes.
///
/// The encoding is based on an escape-character followed by a two-digit hexadecimal number, the number being the
/// ASCII code for the encoded character. The escape character is # (ASCII 0x23).
///
/// @verbatim
/// The following ASCII codes must be escaped:
/// 0x00 .. 0x1F -- Control codes (nonprintable characters, including linebreaks)
///         0x23 -- Sharp (the escape character itself must be escaped to appear in the literal)
///         0x2C -- Comma (used for field separator in standard ASS lines)
///         0x3A -- Colon (used in some custom list formats for name:value pairs)
///         0x7C -- Pipe (used in some custom lists, as item separator, eg. itemA|itemB)
/// @endverbatim
///
/// The encoded string should be usable in any kind of field in an ASS file.

#ifndef _STRING_CODEC_H

/// DOCME
#define _STRING_CODEC_H

#include <wx/wxprec.h>
#include <wx/string.h>

wxString inline_string_encode(const wxString &input);
wxString inline_string_decode(const wxString &input);

#endif


