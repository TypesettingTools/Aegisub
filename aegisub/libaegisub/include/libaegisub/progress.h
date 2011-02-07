// Copyright (c) 2011, Niels Martin Hansen <nielsm@aegisub.org>
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
//
// $Id$

/// @file progress.h
/// @brief Progress bars.
/// @ingroup libaegisub

#ifndef LAG_PRE
#include <string>
#endif

namespace agi {

class ProgressSink;

class ProgressSinkFactory {
public:
	virtual ~ProgressSinkFactory() { }
	virtual ProgressSink * create_progress_sink(const std::string &title) const = 0;
};


class ProgressSink {
public:
	virtual ~ProgressSink() { }
	virtual void set_progress(int steps, int max) = 0;
	virtual void set_operation(const std::string &operation) = 0;
	virtual bool get_cancelled() const = 0;
};


class NullProgressSinkFactory : public ProgressSinkFactory {
public:
	virtual ProgressSink * create_progress_sink(const std::string &title) const;
};


class StdioProgressSinkFactory : public ProgressSinkFactory {
private:

public:
	StdioProgressSinkFactory();
	virtual ProgressSink * create_progress_sink(const std::string &title) const;
};

} // namespace agi
