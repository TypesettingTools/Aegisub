// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

/// @file log.h
/// @brief Logging
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <deque>
#include <memory>
#ifdef __DEPRECATED // Dodge GCC warnings
# undef __DEPRECATED
# include <strstream>
# define __DEPRECATED
#else
# include <strstream>
#endif
#include <vector>
#endif
//#include <libaegisub/exception.h>
#include <libaegisub/types.h>

// These macros below aren't a perm solution, it will depend on how annoying they are through
// actual usage, and also depends on msvc support.
#define LOG_SINK(section, severity) agi::log::Message(section, severity, __FILE__, __FUNCTION__, __LINE__).stream()
#define LOG_E(section) LOG_SINK(section, agi::log::Exception)
#define LOG_A(section) LOG_SINK(section, agi::log::Assert)
#define LOG_W(section) LOG_SINK(section, agi::log::Warning)
#define LOG_I(section) LOG_SINK(section, agi::log::Info)
#define LOG_D(section) LOG_SINK(section, agi::log::Debug)

#define LOG_W_IF(cond, section) if (cond) LOG_SINK(section, agi::log::Warning)
#define LOG_I_IF(cond, section) if (cond) LOG_SINK(section, agi::log::Info)
#define LOG_D_IF(cond, section) if (cond) LOG_SINK(section, agi::log::Debug)

namespace agi {
	namespace log {
class LogSink;

/// Severity levels
enum Severity {
	Exception,	///< Used when exceptions are thrown
	Assert,		///< Fatal and non-fatal assert logging
	Warning,	///< Warnings
	Info,		///< Information only
	Debug		///< Enabled by default when compiled in debug mode.
};

/// Short Severity ID
/// Set in common/log.cpp, keep this ordered the same as Severity.
extern const char *Severity_ID;

/// Global log sink.
extern LogSink *log;

/// Container to hold a single message
struct SinkMessage {
	/// @brief Constructor
	/// @param section  Section info
	/// @param severity Severity
	/// @param file     File name
	/// @param func     Function name
	/// @param line     Source line
	/// @param tv       Log time
	SinkMessage(const char *section, Severity severity, const char *file,
            const char *func, int line, agi_timeval tv);

	/// Destructor
	~SinkMessage();

	const char *section;	///< Section info eg "video/open" "video/seek" etc
	Severity severity;		///< Severity
	const char *file;		///< Source file
	const char *func;		///< Function name
	int line;				///< Source line
	agi_timeval tv;			///< Time at execution
	char *message;			///< Formatted message
	size_t len;				///< Message length
};

class Emitter;

/// Message sink for holding all messages
typedef std::deque<SinkMessage*> Sink;

/// Log sink, single destination for all messages
class LogSink {
	/// Size of current sink, this is only an estimate that is used for trimming.
	int64_t size;

	/// Log sink
	Sink sink;

	/// List of function pointers to emitters
	std::vector<Emitter*> emitters;

	/// Init time for log writing purposes.
	agi_timeval time_start;

	/// Directory to place logfiles.
	const std::string dir_log;


public:
	/// Constructor
	/// @param dir_log Directory to place log files.
	LogSink(const std::string dir_log);

	/// Destructor
	~LogSink();

	/// Insert a message into the sink.
	void log(SinkMessage *sm);

	/// @brief Subscribe an emitter.
	/// @param em Emitter to add
	void Subscribe(Emitter *em);

	/// @brief Unsubscribe an emitter.
	/// @param em Emitter to remove
	void Unsubscribe(Emitter *em);

	/// @brief @get the complete (current) log.
	/// @return Const pointer to internal sink.
	const Sink* GetSink() { return &sink; }
};

/// An emitter to produce human readable output for a log sink.
class Emitter {
public:
	/// Constructor
	Emitter();

	/// Destructor
	virtual ~Emitter();

	/// Enable (subscribe)
	void Enable();

	/// Disable (unsubscribe)
	void Disable();

	/// Accept a single log entry
	virtual void log(SinkMessage *sm)=0;
};


/// Generates a message and submits it to the log sink.
class Message {
	const int len;
	char *buf;
	std::ostrstream msg;
	SinkMessage *sm;

public:
	Message(const char *section,
			Severity severity,
			const char *file,
			const char *func,
			int line);
	~Message();
	std::ostream& stream() { return msg; }
};


/// Emit log entries to stdout.
class EmitSTDOUT: public Emitter {
public:
	void log(SinkMessage *sm);
};


	} // namespace log
} // namespace agi
