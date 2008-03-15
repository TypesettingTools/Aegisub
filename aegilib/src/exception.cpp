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
// AEGISUB/GORGONSUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "exception.h"
using namespace Gorgonsub;


////////////////
// Constructors
Exception::Exception(ExceptionList _code)
: std::exception(GetMessage(_code).mb_str(wxConvLocal))
{
	code = _code;
}

Exception::Exception(ExceptionList _code,const char *file,const long line)
: std::exception(GetMessageFile(_code,file,line).mb_str(wxConvLocal))
{
	code = _code;
}


//////////////////////
// Get message string
String Exception::GetMessage(int code)
{
	switch (code) {
		case Unknown: return L"Unknown.";
		case No_Format_Handler: return L"Could not find a suitable format handler.";
		case Invalid_ActionList: return L"Invalid manipulator.";
		case Section_Already_Exists: return L"The specified section already exists in this model.";
		case Unknown_Format: return L"The specified file format is unknown.";
		case Parse_Error: return L"Parse error.";
		case Unsupported_Format_Feature: return L"This feature is not supported by this format.";
		case Invalid_Token: return L"Invalid type for this token.";
		case TODO: return L"TODO";
	}
	return L"Invalid code.";
}


///////////////////////////////////
// Insert file and line on message
String Exception::GetMessageFile(int code,const char *file,long line)
{
	return GetMessage(code) + _T(" (") + wxString(file,wxConvLocal) + wxString::Format(_T(":%i)."),line);
}


////////////
// Get code
int Exception::GetCode()
{
	return code;
}
