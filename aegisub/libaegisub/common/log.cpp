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

/// @file log.cpp
/// @brief Logging
/// @ingroup libaegisub

#include "../config.h"

#include "libaegisub/log.h"

#include "libaegisub/cajun/elements.h"
#include "libaegisub/cajun/writer.h"
#include "libaegisub/io.h"
#include "libaegisub/util.h"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/range/algorithm.hpp>
#include <cstring>
#include <fstream>
#include <functional>

namespace agi {
	namespace log {

/// Global log sink.
LogSink *log;

/// Short Severity ID
/// Keep this ordered the same as Severity
const char *Severity_ID = "EAWID";

SinkMessage::SinkMessage(const char *section, Severity severity, const char *file, const char *func, int line, timeval tv)
: section(section)
, severity(severity)
, file(file)
, func(func)
, line(line)
, tv(tv)
{
}

/// @todo The log files need to be trimmed after N amount.
LogSink::~LogSink() {
	// The destructor for emitters may try to log messages, so disable all the
	// emitters before destructing any
	std::vector<Emitter*> emitters_temp;
	swap(emitters_temp, emitters);
	util::delete_clear(emitters_temp);

	util::delete_clear(sink);
}

void LogSink::log(SinkMessage *sm) {
	sink.push_back(sm);
	boost::for_each(emitters, [=](Emitter *em) { em->log(sm); });
}

void LogSink::Subscribe(Emitter *em) {
	LOG_D("agi/log/emitter/subscribe") << "Subscribe: " << this;
	emitters.push_back(em);
}

void LogSink::Unsubscribe(Emitter *em) {
	emitters.erase(remove(emitters.begin(), emitters.end(), em), emitters.end());
	delete em;
	LOG_D("agi/log/emitter/unsubscribe") << "Un-Subscribe: " << this;
}

Message::Message(const char *section, Severity severity, const char *file, const char *func, int line)
: msg(nullptr, 1024)
{
	sm = new SinkMessage(section, severity, file, func, line, util::time_log());
}

Message::~Message() {
	sm->message = std::string(msg.str(), (std::string::size_type)msg.pcount());
	agi::log::log->log(sm);
	msg.freeze(false);
}

JsonEmitter::JsonEmitter(fs::path const& directory)
: fp(new boost::filesystem::ofstream(unique_path(directory/util::strftime("%Y-%m-%d-%H-%M-%S-%%%%%%%%.json"))))
{
	WriteTime("open");
}

JsonEmitter::~JsonEmitter() {
	WriteTime("close");
}

void JsonEmitter::log(SinkMessage *sm) {
	json::Object entry;
	entry["sec"]      = (int64_t)sm->tv.tv_sec;
	entry["usec"]     = (int64_t)sm->tv.tv_usec;
	entry["severity"] = sm->severity;
	entry["section"]  = sm->section;
	entry["file"]     = sm->file;
	entry["func"]     = sm->func;
	entry["line"]     = sm->line;
	entry["message"]  = sm->message;
	json::Writer::Write(entry, *fp);
	fp->flush();
}

void JsonEmitter::WriteTime(const char *key) {
	auto time = util::time_log();
	json::Object obj;
	json::Array &timeval = obj[key];
	timeval.push_back((int64_t)time.tv_sec);
	timeval.push_back((int64_t)time.tv_usec);
	json::Writer::Write(obj, *fp);
}

	} // namespace log
} // namespace agi
