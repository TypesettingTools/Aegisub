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

#pragma once

#include "athenastring.h"
#include <exception>

namespace Athenasub {

	// Exception class
	class Exception : public std::exception {
	public:
		enum ExceptionList {
			Unknown,
			No_Format_Handler,
			Invalid_ActionList,
			Section_Already_Exists,
			Unknown_Format,
			Parse_Error,
			Unsupported_Format_Feature,
			Invalid_Token,
			Out_Of_Range,
			Invalid_Section,
			Internal_Error,
			TODO
		};

		Exception(ExceptionList code,String message="",const char* file=NULL,const long line=0);

		int GetCode() { return code; }

	private:
		static std::string GetMessageChar(int code);
		static std::string GetExceptionMessage(int code,String message,const char *file,long line);

		ExceptionList code;
	};

}

#ifndef THROW_ATHENA_EXCEPTION
#ifdef _MSC_VER
#define THROW_ATHENA_EXCEPTION(code) throw Athenasub::Exception(code,"",__FILE__,__LINE__)
#define THROW_ATHENA_EXCEPTION_MSG(code,message) throw Athenasub::Exception(code,message,__FILE__,__LINE__)
#else
#define THROW_ATHENA_EXCEPTION(code) throw Athenasub::Exception(code)
#define THROW_ATHENA_EXCEPTION_MSG(code,message) throw Athenasub::Exception(code,message)
#endif
#endif
