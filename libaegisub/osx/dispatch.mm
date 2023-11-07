// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <dispatch/dispatch.h>
#include <mutex>

namespace {
using namespace agi::dispatch;
std::function<void (Thunk)> invoke_main;

struct OSXQueue : Queue {
    virtual void DoSync(Thunk thunk)=0;
};

struct MainQueue final : OSXQueue {
    void DoInvoke(Thunk thunk) override { invoke_main(thunk); }

    void DoSync(Thunk thunk) override {
        std::mutex m;
        std::condition_variable cv;
        std::unique_lock<std::mutex> l(m);
        std::exception_ptr e;
        bool done = false;
        invoke_main([&]{
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
};

struct GCDQueue final : OSXQueue {
    dispatch_queue_t queue;
    GCDQueue(dispatch_queue_t queue) : queue(queue) { }
    ~GCDQueue() { dispatch_release(queue); }

    void DoInvoke(Thunk thunk) override {
        dispatch_async(queue, ^{
            try {
                thunk();
            }
            catch (...) {
                auto e = std::current_exception();
                invoke_main([=,  this] { std::rethrow_exception(e); });
            }
        });
    }

    void DoSync(Thunk thunk) override {
        std::exception_ptr e;
        std::exception_ptr *e_ptr = &e;
        dispatch_sync(queue, ^{
            try {
                thunk();
            }
            catch (...) {
                *e_ptr = std::current_exception();
            }
        });
        if (e) std::rethrow_exception(e);
    }
};
}

namespace agi { namespace dispatch {
void Init(std::function<void (Thunk)> invoke_main) {
    ::invoke_main = std::move(invoke_main);
}

void Queue::Async(Thunk thunk) { DoInvoke(std::move(thunk)); }
void Queue::Sync(Thunk thunk) { static_cast<OSXQueue *>(this)->DoSync(std::move(thunk)); }

Queue& Main() {
    static MainQueue q;
    return q;
}

Queue& Background() {
    static GCDQueue q(dispatch_get_global_queue(0, DISPATCH_QUEUE_PRIORITY_DEFAULT));
    return q;
}

std::unique_ptr<Queue> Create() {
    return std::unique_ptr<Queue>(new GCDQueue(dispatch_queue_create("Aegisub worker queue",
                                                                     DISPATCH_QUEUE_SERIAL)));
}
} }
