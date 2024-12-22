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

#include "libaegisub/dispatch.h"

#include "libaegisub/util.h"

#include <atomic>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace {
	boost::asio::io_context *service;
	std::function<void (agi::dispatch::Thunk)> invoke_main;
	std::atomic<uint_fast32_t> threads_running;

	class MainQueue final : public agi::dispatch::Queue {
		void DoInvoke(agi::dispatch::Thunk&& thunk) override {
			invoke_main(thunk);
		}
	};

	class BackgroundQueue final : public agi::dispatch::Queue {
		void DoInvoke(agi::dispatch::Thunk&& thunk) override {
			boost::asio::post(*service, std::move(thunk));
		}
	};

	class SerialQueue final : public agi::dispatch::Queue {
		boost::asio::io_context::strand strand;

		void DoInvoke(agi::dispatch::Thunk&& thunk) override {
			boost::asio::post(strand, std::move(thunk));
		}
	public:
		SerialQueue() : strand(*service) { }
	};

	struct IOServiceThreadPool {
		boost::asio::io_context io_context;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;
		std::vector<std::thread> threads;

		IOServiceThreadPool() : work_guard(boost::asio::make_work_guard(io_context)) { }
		~IOServiceThreadPool() {
			work_guard.reset();
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

namespace agi::dispatch {

void Init(std::function<void (Thunk)>&& invoke_main) {
	static IOServiceThreadPool thread_pool;
	::service = &thread_pool.io_context;
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

void Queue::Async(Thunk&& thunk) {
	DoInvoke([=] {
		try {
			thunk();
		}
		catch (...) {
			auto e = std::current_exception();
			invoke_main([=] { std::rethrow_exception(e); });
		}
	});
}

void Queue::Sync(Thunk&& thunk) {
	std::mutex m;
	std::condition_variable cv;
	std::unique_lock<std::mutex> l(m);
	std::exception_ptr e;
	bool done = false;
	DoInvoke([&]{
		std::unique_lock<std::mutex> l(m);
		try {
			thunk();
		}
		catch (...) {
			e = std::current_exception();
		}
		done = true;
		cv.notify_all();
	});
	cv.wait(l, [&]{ return done; });
	if (e) std::rethrow_exception(e);
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

}
