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


#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "libaegisub/log.h"
#include "libaegisub/mutex.h"

namespace agi {
	namespace log {

/// Global log sink.
LogSink *log = new LogSink();


SinkMessage::SinkMessage(const char *section, Severity severity, const char *file,
			const char *func, int line, timeval tv):
			section(section),
			severity(severity),
			file(file),
			func(func),
			line(line),
			tv(tv) {
}

SinkMessage::~SinkMessage() {
///@todo Memory cleanup
}


LogSink::LogSink(): emit(0) {
	sink = new Sink();
}

LogSink::~LogSink() {
/// @todo This needs to flush all log data to disk on quit.
	if (emit) {
		for (int i = emitters.size()-1; i >= 0; i--) {
			delete emitters[i];
		}
	}
}


void LogSink::log(SinkMessage *sm) {
	sink->push_back(sm);

	if (emit) {
		for (int i = emitters.size()-1; i >= 0; i--) {
			emitters[i]->log(sm);
		}
	}
}


Emitter::~Emitter() {
}


Emitter::Emitter() {
}

void EmitSTDOUT::log(SinkMessage *sm) {
	tm tmtime;
	localtime_r(&sm->tv.tv_sec, &tmtime);

	printf("%c %d-%02d-%02d %02d:%02d:%02d %ld %s %s %s:%d %s\n",
		Severity_ID[sm->severity],
		tmtime.tm_year+1900,
		tmtime.tm_mon,
		tmtime.tm_mday,
		tmtime.tm_hour,
		tmtime.tm_min,
		tmtime.tm_sec,
		sm->tv.tv_usec,
		sm->section,
		sm->file,
		sm->func,
		sm->line,
		strndup(sm->message,
		sm->len));
}


int LogSink::Subscribe(Emitter &em) {
	emitters.push_back(&em);
	emit = 1;

}


void LogSink::Unsubscribe(const int &id) {
	emitters.erase((emitters.begin()-1)+id);
	if (emitters.size() == 0)
		emit = 0;
}


Message::Message(const char *section,
				Severity severity,
				const char *file,
				const char *func,
				int line):
				len(1024) {
	buf = new char[len];
	msg = new std::ostrstream(buf, len);
	timeval tv;
	gettimeofday(&tv, (struct timezone *)NULL);
	sm = new SinkMessage(section, severity, file, func, line, tv);
}


Message::~Message() {
	sm->message = msg->str();
	sm->len = msg->pcount();
	agi::log::log->log(sm);
	delete[] buf;
	delete msg;
}


void Emitter::Enable() {
	id = agi::log::log->Subscribe(*(this));
}


void Emitter::Disable() {
	agi::log::log->Unsubscribe(id);
}


	} // namespace log
} // namespace agi
