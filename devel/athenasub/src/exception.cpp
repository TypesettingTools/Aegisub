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
// AEGISUB/ATHENASUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "exception.h"
using namespace Athenasub;


///////////////////////
// Stack walker helper
#include <wx/stackwalk.h>
#include <wx/filename.h>

class Stack : public wxStackWalker {
	std::string &str;
public:
	Stack(std::string &dest) : str(dest) {}

	void OnStackFrame(const wxStackFrame& frame)
	{
		wxString line = wxString::Format(_T("\n\t%03i - "),frame.GetLevel()) + frame.GetName();
		if (frame.HasSourceLocation()) {
			wxFileName fn(frame.GetFileName());
			line += _T(" (") + fn.GetFullName() + wxString::Format(_T(":%i)"),frame.GetLine());
		}
		str += line.mb_str(wxConvUTF8);
	}
};


////////////////
// Constructors
Exception::Exception(ExceptionList _code,String message,const char* file,const long line)
: std::exception(GetExceptionMessage(_code,message,file,line).c_str())
{
	code = _code;
}


//////////////////////
// Get message string
std::string Exception::GetMessageChar(int code)
{
	switch (code) {
		case Unknown: return "Unknown.";
		case No_Format_Handler: return "Could not find a suitable format handler.";
		case Invalid_ActionList: return "Invalid manipulator.";
		case Section_Already_Exists: return "The specified section already exists in this model.";
		case Unknown_Format: return "The specified file format is unknown.";
		case Parse_Error: return "Parse error.";
		case Unsupported_Format_Feature: return "This feature is not supported by this format.";
		case Invalid_Token: return "Invalid type for this token.";
		case Out_Of_Range: return "Out of range.";
		case Invalid_Section: return "Invalid section.";
		case Internal_Error: return "Internal error.";
		case TODO: return "TODO";
	}
	return "Invalid code.";
}


///////////////////////////////////////////
// Get the message string for the filename
std::string Exception::GetExceptionMessage(int code,String message,const char *file,long line)
{
	std::string str = GetMessageChar(code);

	// Append macro filename
	if (file != NULL) {
		str = str + " (" + file + ":";	// Yes, there's an actual reason why that's not += (operator overloading)
		char buffer[16];
		_itoa_s(line,buffer,10);
		str = str + buffer + ")";
	}

	// Append stack trace
	Stack stack(str);
	stack.Walk(2);

	// Append extra message
	if (!message.IsEmpty()) {
		str = str + "\nExtra message: " + message;
	}

	return str;
}
