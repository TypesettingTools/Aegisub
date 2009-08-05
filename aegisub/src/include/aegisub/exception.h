// Copyright (c) 2009, Niels Martin Hansen
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

/// @file exception.h
/// @brief Base exception classes for structured error handling
/// @ingroup main_headers
///


#include <wx/string.h>



/// @see aegisub.h
namespace Aegisub {


	/// @class Exception
	/// @brief DOCME
	///
	/// DOCME
	class Exception {

		/// DOCME
		wxString message;

		/// DOCME
		Exception *inner;

	protected:

		/// @brief DOCME
		/// @param msg 
		/// @param inr 
		///
		Exception(const wxString &msg, Exception *inr = 0) : message(msg), inner(inr) { }
		Exception(); // not implemented, not wanted

		/// @brief DOCME
		/// @return 
		///
		virtual ~Exception() { if (inner) delete inner; }

	public:

		/// @brief // Error message for outer exception
		/// @return 
		///
		virtual wxString GetMessage() const { return message; }

		/// @brief // Error message for outer exception, and chained message for inner exception
		/// @return 
		///
		wxString GetChainedMessage() const { if (inner) return inner->GetChainedMessage() + _T("\r\n") + GetMessage(); else return GetMessage(); }
		// Name of exception class, should only be implemented by specific classes
		virtual const wxChar * GetName() const = 0;


		/// @brief DOCME
		/// @return 
		///
		operator const wxChar * () { return GetMessage().c_str(); }

		/// @brief DOCME
		/// @return 
		///
		operator wxString () { return GetMessage(); }
	};



/// DOCME
#define AG_WHERE _T(" (at ") _T(__FILE__) _T(":") _T(#__LINE__) _T(")")



/// DOCME
#define DEFINE_SIMPLE_EXCEPTION_NOINNER(classname,baseclass,displayname)             \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const wxString &msg) : baseclass(msg) { }                          \
		const wxChar * GetName() const { return _T(displayname); }                   \
	};

/// DOCME
#define DEFINE_SIMPLE_EXCEPTION(classname,baseclass,displayname)                     \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const wxString &msg, Exception *inner) : baseclass(msg, inner) { } \
		const wxChar * GetName() const { return _T(displayname); }                   \
	};

/// DOCME
#define DEFINE_BASE_EXCEPTION_NOINNER(classname,baseclass)                           \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const wxString &msg) : baseclass(msg) { }                          \
	};

/// DOCME
#define DEFINE_BASE_EXCEPTION(classname,baseclass)                                   \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const wxString &msg, Exception *inner) : baseclass(msg, inner) { } \
	};


	// Exception for "user cancel" events
	// I.e. when we want to abort an operation because the user requested that we do so
	// Not actually an error and should not be handled as such
	DEFINE_SIMPLE_EXCEPTION_NOINNER(UserCancelException,Exception,"nonerror/user_cancel")


	// Errors that should never happen and point to some invalid assumption in the code
	DEFINE_SIMPLE_EXCEPTION(InternalError, Exception, "internal_error")


	// Some error related to the filesystem
	// These should always be original causes and as such do not support inner exceptions
	DEFINE_BASE_EXCEPTION_NOINNER(FileSystemError,Exception)

	// A file can't be accessed for some reason
	DEFINE_SIMPLE_EXCEPTION_NOINNER(FileNotAccessibleError,FileSystemError,"filesystem/not_accessible")


	/// DOCME
	/// @class FileNotFoundError
	/// @brief DOCME
	///
	/// DOCME
	class FileNotFoundError : public FileNotAccessibleError {
	public:

		/// @brief DOCME
		/// @param filename 
		/// @return 
		///
		FileNotFoundError(const wxString &filename) : FileNotAccessibleError(wxString(_T("File not found: ")) + filename) { }

		/// @brief DOCME
		///
		const wxChar * GetName() const { return _T("filesystem/not_accessible/not_found"); }
	};


	// A problem with some input data
	DEFINE_BASE_EXCEPTION(InvalidInputException,Exception)


	// There is no "generic exception" class, everything must be a specific one
	// Define new classes if none fit the error you're reporting

};
