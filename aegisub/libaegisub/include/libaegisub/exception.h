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

#pragma once

#include <string>
#ifdef _WIN32
#include <memory>
#else
#include <tr1/memory>
#endif

/// @see aegisub.h
namespace agi {


	/// @class Exception
	/// @brief Base class for all exceptions in Aegisub.
	///
	/// All exceptions thrown by Aegisub should derive from this class.
	/// It is incorrect to throw anything that is not a subclass of this.
	///
	/// However, there are no public constructors for this class, it should
	/// not be instantiated and thrown directly. Throw instances of a
	/// relevant sub class, declare a new one if necessary. It is allowed to
	/// declare sub classes of Exception and derivates in private headers
	/// and even inside source files, as long as a caller has a chance to
	/// catch the exception thrown.
	///
	/// When throwing exceptions, throw temporaries, not heap allocated
	/// objects. (C++ FAQ Lite 17.6.) I.e. this is correct:
	/// @code
	/// throw agi::SomeException("Message for exception");
	/// @endcode
	/// This is wrong:
	/// @code
	/// throw new agi::SomeException("Remember this is the wrong way!");
	/// @endcode
	/// Exceptions must not be allocated on heap, because of the risks of
	/// leaking memory that way. (C++ FAQ Lite 17.8.)
	///
	/// When catching exceptions, make sure you catch them by reference,
	/// otherwise polymorphism will not work. (The C++ Programming
	/// Language Special Edition 14.2.1, C++ FAQ Lite 17.7.)
	///
	/// Catch like this:
	/// @code
	/// try {
	///     /* ... */
	/// }
	/// catch (agi::UserCancelException &e) {
	///     /* handle the fact that the user cancelled */
	/// }
	/// catch (agi::VideoInputException &e) {
	///     /* handle the video provider failing */
	/// }
	/// @endcode
	/// Don't always handle all exceptions the code you're protected might
	/// throw, sometimes it's better to let an exception slip through and
	/// let code further out handle it. Sometimes you might want to catch and
	/// package an exception into something else, for example to represent
	/// cases such as "subtitle file could not be read @e because the file
	/// could not be opened for reading". This is the purpose of the "inner"
	/// exceptions.
	class Exception {

		/// The error message
		std::string message;

		/// An inner exception, the cause of this exception
		std::tr1::shared_ptr<Exception> inner;

	protected:

		/// @brief Protected constructor initialising members
		/// @param msg The error message
		/// @param inr The inner exception, optional
		///
		/// Deriving classes should always use this constructor for initialising
		/// the base class.
		Exception(const std::string &msg, const Exception *inr = 0)
			: message(msg)
		{
			if (inr)
				inner.reset(inr->Copy());
		}

		/// @brief Default constructor, not implemented
		///
		/// The default constructor is not implemented because it must not be used,
		/// as it leaves the members un-initialised.
		Exception();

	public:
		/// @brief Destructor
		virtual ~Exception()
		{
		}

		/// @brief Get the outer exception error message
		/// @return Error message
		virtual std::string GetMessage() const { return message; }

		/// @brief Get error messages for chained exceptions
		/// @return Chained error message
		///
		/// If there is an inner exception, prepend its chained error message to
		/// our error message, with a CRLF between. Returns our own error message
		/// alone if there is no inner exception.
		std::string GetChainedMessage() const { if (inner.get()) return inner->GetChainedMessage() + "\r\n" + GetMessage(); else return GetMessage(); }
		
		/// @brief Exception class printable name
		///
		/// Sub classes should implement this to return a constant character string
		/// naming their exception in a hierarchic manner.
		///
		/// Exception classes inheriting directly from Exception define a top-level
		/// name for their sub-tree, further sub-classes add further levels, each
		/// level is separated by a slash. Characters allowed in the name for a
		/// level are [a-z0-9_].
		virtual const char * GetName() const = 0;


		/// @brief Convert to char array as the error message
		/// @return The error message
		operator const char * () { return GetMessage().c_str(); }

		/// @brief Convert to std::string as the error message
		/// @return The error message
		operator std::string () { return GetMessage(); }

		/// @brief Create a copy of the exception allocated on the heap
		/// @return A heap-allocated exception object
		///
		/// All deriving classes must implement this explicitly to avoid losing
		/// information in the duplication.
		virtual Exception *Copy() const = 0;
	};



/// @brief Convenience macro to include the current location in code
///
/// Intended for use in error messages where it can sometimes be convenient to
/// indicate the exact position the error occurred at.
#define AG_WHERE " (at " __FILE__ ":" #__LINE__ ")"



/// @brief Convenience macro for declaring exceptions with no support for inner exception
/// @param classname   Name of the exception class to declare
/// @param baseclass   Class to derive from
/// @param displayname The printable name of the exception (return of GetName())
///
/// This macro covers most cases of exception classes where support for inner
/// exceptions is not relevant/wanted.
#define DEFINE_SIMPLE_EXCEPTION_NOINNER(classname,baseclass,displayname)             \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const std::string &msg) : baseclass(msg) { }                          \
		const char * GetName() const { return displayname; }                   \
		Exception * Copy() const { return new classname(*this); }                    \
	};

/// @brief Convenience macro for declaring exceptions supporting inner exceptions
/// @param classname   Name of the exception class to declare
/// @param baseclass   Class to derive from
/// @param displayname The printable name of the exception (return of GetName())
///
/// This macro covers most cases of exception classes that should support
/// inner exceptions.
#define DEFINE_SIMPLE_EXCEPTION(classname,baseclass,displayname)                     \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const std::string &msg, Exception *inner) : baseclass(msg, inner) { } \
		const char * GetName() const { return displayname; }                   \
		Exception * Copy() const { return new classname(*this); }                    \
	};

/// @brief Macro for declaring non-instantiable exception base classes
/// @param classname Name of the exception class to declare
/// @param baseclass Class to derive from
///
/// Declares an exception class that does not implement the GetName() function
/// and as such (unless a base class implements it) is not constructable.
/// Classes declared by this macro do not support inner exceptions.
#define DEFINE_BASE_EXCEPTION_NOINNER(classname,baseclass)                           \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const std::string &msg) : baseclass(msg) { }                          \
	};

/// @brief Macro for declaring non-instantiable exception base classes with inner
///        exception support
/// @param classname Name of the exception class to declare
/// @param baseclass Class to derive from
///
/// Declares an exception class that does not implement the GetName() function
/// and as such (unless a base class implements it) is not constructable.
/// Classes declared by this macro do support inner exceptions.
#define DEFINE_BASE_EXCEPTION(classname,baseclass)                                   \
	class classname : public baseclass {                                             \
	public:                                                                          \
		classname(const std::string &msg, Exception *inner) : baseclass(msg, inner) { } \
	};


	/// @class agi::UserCancelException
	/// @extends agi::Exception
	/// @brief Exception for "user cancel" events
	///
	/// I.e. when we want to abort an operation because the user requested that we do so.
	/// Not actually an error and should not be handled as such.
	///
	/// This is intended to signal that an operation should be completely aborted at the
	/// request of the user, and should usually be handled as close to the main UI as
	/// possible, user cancel exceptions should unwind anything that was going on at the
	/// moment. For this to work, RAII methodology has to be used consequently in the
	/// code in question.
	DEFINE_SIMPLE_EXCEPTION_NOINNER(UserCancelException,Exception,"nonerror/user_cancel")


	/// @class agi::InternalError
	/// @extends agi:Exception
	/// @brief Errors that should never happen and point to some invalid assumption in the code
	///
	/// Throw an internal error when a sanity check fails, and the insanity should have
	/// been caught and handled at an earlier stage, i.e. when something seems to
	/// have become inconsistent. All internal errors are of the type "this should never
	/// happen", most often you'll want this kind of error unwind all the way past the main UI
	/// and eventually cause an abort().
	DEFINE_SIMPLE_EXCEPTION(InternalError, Exception, "internal_error")


	/// @class agi::FileSystemError
	/// @extends agi::Exception
	/// @brief Base class for errors related to the file system
	///
	/// This base class can not be instantiated.
	/// File system errors do not support inner exceptions, as they are always originating
	/// causes for errors.
	DEFINE_BASE_EXCEPTION_NOINNER(FileSystemError,Exception)

	/// @class agi::FileNotAccessibleError
	/// @extends agi::FileSystemError
	/// @brief A file can't be accessed for some reason
	DEFINE_SIMPLE_EXCEPTION_NOINNER(FileNotAccessibleError,FileSystemError,"filesystem/not_accessible")


	/// @class FileNotFoundError
	/// @brief A file can't be accessed because there's no file by the given name
	class FileNotFoundError : public FileNotAccessibleError {
	public:

		/// @brief Constructor, automatically builds the error message
		/// @param filename Name of the file that could not be found
		FileNotFoundError(const std::string &filename) : FileNotAccessibleError(std::string("File not found: ") + filename) { }

		// Not documented, see  agi::Exception class
		const char * GetName() const { return "filesystem/not_accessible/not_found"; }

		// Not documented, see  agi::Exception class
		Exception * Copy() const { return new FileNotFoundError(*this); }
	};


	/// @class agi::InvalidInputException
	/// @extends agi::Exception
	/// @brief Some input data were invalid and could not be processed
	DEFINE_BASE_EXCEPTION(InvalidInputException,Exception)

}
