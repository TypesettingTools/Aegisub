// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/

#include "config.h"

#include "libaegisub/dispatch.h"

#include "libaegisub/util.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace {
	boost::asio::io_service *service;
	std::function<void (agi::dispatch::Thunk)> invoke_main;

	class MainQueue : public agi::dispatch::Queue {
		void DoInvoke(agi::dispatch::Thunk thunk) {
			invoke_main(thunk);
		}
	};

	class BackgroundQueue : public agi::dispatch::Queue {
		void DoInvoke(agi::dispatch::Thunk thunk) {
			service->post(thunk);
		}
	};

	class SerialQueue : public agi::dispatch::Queue {
		boost::asio::io_service::strand strand;

		void DoInvoke(agi::dispatch::Thunk thunk) {
			strand.post(thunk);
		}
	public:
		SerialQueue() : strand(*service) { }
	};
}

namespace agi { namespace dispatch {

void Init(std::function<void (Thunk)> invoke_main) {
	static boost::asio::io_service service;
	static boost::asio::io_service::work work(service);
	::service = &service;
	::invoke_main = invoke_main;

	for (unsigned i = 0; i < std::max<unsigned>(1, std::thread::hardware_concurrency()); ++i)
		std::thread([]{
			util::SetThreadName("Dispatch Worker");
			::service->run();
		}).detach();
}

void Queue::Async(Thunk thunk) {
	DoInvoke(thunk);
}

void Queue::Sync(Thunk thunk) {
	std::mutex m;
	std::condition_variable cv;
	std::unique_lock<std::mutex> l(m);
	bool done = false;
	DoInvoke([&]{
		std::unique_lock<std::mutex> l(m);
		thunk();
		done = true;
		cv.notify_all();
	});
	cv.wait(l, [&]{ return done; });
}

Queue& Main() {
	static MainQueue q;
	return q;
}

Queue& Background() {
	static BackgroundQueue q;
	return q;
}

std::unique_ptr<Queue> Create() {
	return std::unique_ptr<Queue>(new SerialQueue);
}

} }
