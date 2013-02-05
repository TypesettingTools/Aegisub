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

#include <atomic>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace {
	boost::asio::io_service *service;
	std::function<void (agi::dispatch::Thunk)> invoke_main;
	std::atomic<uint_fast32_t> threads_running;

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

	struct IOServiceThreadPool {
		boost::asio::io_service io_service;
		std::unique_ptr<boost::asio::io_service::work> work;
		std::vector<std::thread> threads;

		IOServiceThreadPool() : work(new boost::asio::io_service::work(io_service)) { }
		~IOServiceThreadPool() {
			work.reset();
#ifndef _WIN32
			for (auto& thread : threads) thread.join();
#else
			// Calling join() after main() returns deadlocks
			// https://connect.microsoft.com/VisualStudio/feedback/details/747145
			for (auto& thread : threads) thread.detach();
			while (threads_running) std::this_thread::yield();
#endif
		}
	};
}

namespace agi { namespace dispatch {

void Init(std::function<void (Thunk)> invoke_main) {
	static IOServiceThreadPool thread_pool;
	::service = &thread_pool.io_service;
	::invoke_main = invoke_main;

	thread_pool.threads.reserve(std::max<unsigned>(4, std::thread::hardware_concurrency()));
	for (size_t i = 0; i < thread_pool.threads.capacity(); ++i) {
		thread_pool.threads.emplace_back([]{
			++threads_running;
			agi::util::SetThreadName("Dispatch Worker");
			service->run();
			--threads_running;
		});
	}
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
