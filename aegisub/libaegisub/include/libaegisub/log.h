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

/// @file log.h
/// @brief Logging
/// @ingroup libaegisub

#include <libaegisub/fs_fwd.h>
#include <libaegisub/time.h>

#include <ctime>
#include <deque>
#include <iosfwd>
#include <memory>
#include <vector>

#ifdef __DEPRECATED // Dodge GCC warnings
# undef __DEPRECATED
# include <strstream>
# define __DEPRECATED
#else
# include <strstream>
#endif

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
	SinkMessage(const char *section, Severity severity, const char *file, const char *func, int line, timeval tv);

	const char *section;	///< Section info eg "video/open" "video/seek" etc
	Severity severity;		///< Severity
	const char *file;		///< Source file
	const char *func;		///< Function name
	int line;				///< Source line
	agi_timeval tv;			///< Time at execution
	std::string message;	///< Formatted message
};

class Emitter;

/// Message sink for holding all messages
typedef std::deque<SinkMessage*> Sink;

/// Log sink, single destination for all messages
class LogSink {
	/// Log sink
	Sink sink;

	/// List of pointers to emitters
	std::vector<Emitter*> emitters;

public:
	/// Destructor
	~LogSink();

	/// Insert a message into the sink.
	void log(SinkMessage *sm);

	/// @brief Subscribe an emitter
	/// @param em Emitter to add
	///
	/// LogSink takes ownership of the passed emitter
	void Subscribe(Emitter *em);

	/// @brief Unsubscribe and delete an emitter
	/// @param em Emitter to delete
	void Unsubscribe(Emitter *em);

	/// @brief @get the complete (current) log.
	/// @return Const pointer to internal sink.
	const Sink* GetSink() const { return &sink; }
};

/// An emitter to produce human readable output for a log sink.
class Emitter {
public:
	/// Destructor
	virtual ~Emitter() { }

	/// Accept a single log entry
	virtual void log(SinkMessage *sm)=0;
};

/// A simple emitter which writes the log to a file in json format
class JsonEmitter : public Emitter {
	std::unique_ptr<std::ostream> fp;

	void WriteTime(const char *key);

public:
	/// Constructor
	/// @param directory Directory to write the log file in
	JsonEmitter(fs::path const& directory);
	/// Destructor
	~JsonEmitter();

	void log(SinkMessage *);
};

/// Generates a message and submits it to the log sink.
class Message {
	std::ostrstream msg;
	SinkMessage *sm;

public:
	Message(const char *section, Severity severity, const char *file, const char *func, int line);
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
