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

#pragma once

#include <string>

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

  protected:
	/// @brief Protected constructor initialising members
	/// @param msg The error message
	///
	/// Deriving classes should always use this constructor for initialising
	/// the base class.
	Exception(std::string msg) : message(std::move(msg)) {}

  public:
	/// @brief Get the outer exception error message
	/// @return Error message
	std::string const& GetMessage() const { return message; }
};

/// @brief Convenience macro to include the current location in code
///
/// Intended for use in error messages where it can sometimes be convenient to
/// indicate the exact position the error occurred at.
#define AG_WHERE " (at " __FILE__ ":" #__LINE__ ")"

/// @brief Convenience macro for declaring exceptions
/// @param classname   Name of the exception class to declare
/// @param baseclass   Class to derive from
#define DEFINE_EXCEPTION(classname, baseclass) \
	class classname : public baseclass { \
	  public: \
		classname(std::string msg) : baseclass(std::move(msg)) {} \
	}

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
DEFINE_EXCEPTION(UserCancelException, Exception);

/// @class agi::InternalError
/// @extends agi:Exception
/// @brief Errors that should never happen and point to some invalid assumption in the code
///
/// Throw an internal error when a sanity check fails, and the insanity should have
/// been caught and handled at an earlier stage, i.e. when something seems to
/// have become inconsistent. All internal errors are of the type "this should never
/// happen", most often you'll want this kind of error unwind all the way past the main UI
/// and eventually cause an abort().
DEFINE_EXCEPTION(InternalError, Exception);

/// @class agi::EnvironmentError
/// @extends agi:Exception
/// @brief The execution environment is broken in some fundamental way
///
/// Throw an environment error when a call to the platform API has failed
/// in some way that should normally never happen or suggests that the
/// runtime environment is too insane to support.
DEFINE_EXCEPTION(EnvironmentError, Exception);

/// @class agi::InvalidInputException
/// @extends agi::Exception
/// @brief Some input data were invalid and could not be processed
DEFINE_EXCEPTION(InvalidInputException, Exception);
} // namespace agi
