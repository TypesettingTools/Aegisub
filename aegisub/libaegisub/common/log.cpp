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

/// @file log.cpp
/// @brief Logging
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#endif

#include "libaegisub/cajun/elements.h"
#include "libaegisub/cajun/writer.h"
#include "libaegisub/io.h"
#include "libaegisub/log.h"
#include "libaegisub/types.h"
#include "libaegisub/util.h"

namespace agi {
	namespace log {

/// Global log sink.
LogSink *log;

/// Short Severity ID
/// Keep this ordered the same as Severity
const char *Severity_ID = "EAWID";

SinkMessage::SinkMessage(const char *section, Severity severity,
						 const char *file, const char *func, int line,
						 agi_timeval tv)
: section(section)
, severity(severity)
, file(file)
, func(func)
, line(line)
, tv(tv)
, message(NULL)
, len(0)
{
}

SinkMessage::~SinkMessage() {
	delete message;
}


LogSink::LogSink(const std::string dir_log): dir_log(dir_log) {
	util::time_log(time_start);
}

/// @todo The log files need to be trimed after N amount.
LogSink::~LogSink() {
	json::Object root;
	json::Array array;

	agi_timeval time_close;
	util::time_log(time_close);

	std::stringstream str;
	str << dir_log << time_start.tv_sec << ".json";
	io::Save file(str.str());

	for (unsigned int i=0; i < sink.size(); i++) {
		json::Object entry;
		entry["sec"]		= json::Number(sink[i]->tv.tv_sec);
		entry["usec"]		= json::Number(sink[i]->tv.tv_usec);
		entry["severity"]	= json::Number(sink[i]->severity),
		entry["section"]	= json::String(sink[i]->section);
		entry["file"]		= json::String(sink[i]->file);
		entry["func"]		= json::String(sink[i]->func);
		entry["line"]		= json::Number(sink[i]->line);
		entry["message"]	= json::String(std::string(sink[i]->message, sink[i]->len));

		array.Insert(entry);
	}

	json::Array timeval_open;
	timeval_open.Insert(json::Number(time_start.tv_sec));
	timeval_open.Insert(json::Number(time_start.tv_usec));
	root["timeval"]["open"] = timeval_open;

	json::Array timeval_close;
	timeval_close.Insert(json::Number(time_close.tv_sec));
	timeval_close.Insert(json::Number(time_close.tv_usec));
	root["timeval"]["close"] = timeval_close;


	root["log"] = array;

	json::Writer::Write(root, file.Get());

	agi::util::delete_clear(sink);
}


void LogSink::log(SinkMessage *sm) {
	sink.push_back(sm);

	std::for_each(
		emitters.begin(),
		emitters.end(),
		std::bind2nd(std::mem_fun(&Emitter::log), sm));
}

void LogSink::Subscribe(Emitter *em) {
	LOG_D("agi/log/emitter/subscribe") << "Subscribe: " << this;
	emitters.push_back(em);
}

void LogSink::Unsubscribe(Emitter *em) {
	emitters.erase(std::remove(emitters.begin(), emitters.end(), em), emitters.end());
	LOG_D("agi/log/emitter/unsubscribe") << "Un-Ssubscribe: " << this;
}

Message::Message(const char *section,
				Severity severity,
				const char *file,
				const char *func,
				int line)
: len(1024)
, buf(new char[len])
, msg(buf, len)
{
	agi_timeval tv;
	util::time_log(tv);
	sm = new SinkMessage(section, severity, file, func, line, tv);
}

Message::~Message() {
	sm->message = msg.str();
	sm->len = (size_t)msg.pcount();
	agi::log::log->log(sm);
}

Emitter::Emitter() {
}

Emitter::~Emitter() {
	Disable();
}

void Emitter::Enable() {
	agi::log::log->Subscribe(this);
}

void Emitter::Disable() {
	agi::log::log->Unsubscribe(this);
}


	} // namespace log
} // namespace agi
