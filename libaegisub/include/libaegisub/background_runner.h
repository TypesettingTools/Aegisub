// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

/// @file background_runner.h
/// @brief Background runner and progress sink interfaces
/// @ingroup libaegisub

#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace agi {
	/// @class ProgressSink
	/// @brief A receiver for progress updates sent from a worker function
	///
	/// Note that ProgressSinks are not required to be thread-safe. The only
	/// guarantee provided is that they can be used on the thread they are
	/// spawned on (which may or may not be the GUI thread).
	class ProgressSink {
	public:
		/// Virtual destructor so that things can safely inherit from this
		virtual ~ProgressSink() { }

		/// Set the progress to indeterminate
		virtual void SetIndeterminate()=0;

		/// Set the title of the running task
		virtual void SetTitle(std::string const& title)=0;
		/// Set an additional message associated with the task
		virtual void SetMessage(std::string const& msg)=0;
		/// Set the current task progress
		virtual void SetProgress(int64_t cur, int64_t max)=0;

		/// @brief Log a message
		///
		/// If any messages are logged then the dialog will not automatically close
		/// when the task finishes so that the user has the chance to read them.
		virtual void Log(std::string const& str)=0;

		/// Has the user asked the task to cancel?
		virtual bool IsCancelled()=0;
	};

	/// @class BackgroundRunner
	/// @brief A class which runs a function, providing it with a progress sink
	///
	/// Normally implementations of this interface will spawn a new thread to
	/// run the task on, but there are sensible implementations that may not.
	/// For example, an implementation which has no UI and simply writes the
	/// log output to a file would simply run on the main thread.
	class BackgroundRunner {
	public:
		/// Virtual destructor so that things can safely inherit from this
		virtual ~BackgroundRunner() { }

		/// @brief Run a function on a background thread
		/// @param task Function to run
		/// @param priority Thread priority or -1 for default
		/// @throws agi::UserCancelException on cancel
		///
		/// Blocks the calling thread until the task completes or is canceled.
		/// Progress updates sent to the progress sink passed to the task should
		/// be displayed to the user in some way, along with some way for the
		/// user to cancel the task.
		virtual void Run(std::function<void(ProgressSink *)> task, int priority=-1)=0;
	};
}
