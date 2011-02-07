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

/// @file progress.cpp
/// @brief Progress bars.
/// @ingroup libaegisub

#ifndef LAG_PRE
#include <assert.h>

#include <string>
#include <iomanip>
#include <iostream>
#include <cmath>
#endif

#include "libaegisub/progress.h"


namespace agi {


class NullProgressSink : public ProgressSink {
public:
	virtual void set_progress(int steps, int max) { }
	virtual void set_operation(const std::string &operation) { }
	virtual bool get_cancelled() const { return false; }
};

ProgressSink * NullProgressSinkFactory::create_progress_sink(const std::string &title) const {
	return new NullProgressSink;
}


class StdioProgressSink : public ProgressSink {
private:
	std::string operation;
	float progress;

	void print();

public:
	StdioProgressSink(const std::string &title);
	virtual ~StdioProgressSink();
	virtual void set_progress(int steps, int max);
	virtual void set_operation(const std::string &operation);
	virtual bool get_cancelled() const { return false; } // or maybe see if there is an ESC waiting or get a ^C sent flag
};

ProgressSink * StdioProgressSinkFactory::create_progress_sink(const std::string &title) const {
	return new StdioProgressSink(title);
}


StdioProgressSink::StdioProgressSink(const std::string &title)
: operation(title)
, progress(0) {
	std::cout << title << ": ";
	print();
}

StdioProgressSink::~StdioProgressSink()
{
	std::cout << "end" << std::endl;
}


void StdioProgressSink::set_progress(int steps, int max) {
	assert(steps >= 0);
	assert(steps <= max);

	float old_progress = progress;
	progress = (100.f * steps)/max;

	if (std::fabs(progress-old_progress) > 0.8)
	{
		print();
	}
}


void StdioProgressSink::set_operation(const std::string &operation) {
	this->operation = operation;
	print();
}


void StdioProgressSink::print() {
	std::cout << operation << ": " << std::setprecision(2) << progress << std::endl;
}



} // namespace agi
