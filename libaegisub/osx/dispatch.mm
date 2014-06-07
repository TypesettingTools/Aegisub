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

namespace {
std::function<void (agi::dispatch::Thunk)> invoke_main;

struct GCDQueue : agi::dispatch::Queue {
    dispatch_queue_t queue;
    GCDQueue(dispatch_queue_t queue) : queue(queue) { }
    void DoInvoke(agi::dispatch::Thunk) override final { }
};

struct OwningQueue final : GCDQueue {
    using GCDQueue::GCDQueue;
    ~OwningQueue() { dispatch_release(queue); }
};
}

namespace agi { namespace dispatch {
void Init(std::function<void (Thunk)> invoke_main) {
    ::invoke_main = std::move(invoke_main);
}

void Queue::Async(Thunk thunk) {
    dispatch_async(static_cast<GCDQueue *>(this)->queue, ^{
        try {
            thunk();
        }
        catch (...) {
            auto e = std::current_exception();
            invoke_main([=] { std::rethrow_exception(e); });
        }
    });
}

void Queue::Sync(Thunk thunk) {
    std::exception_ptr e;
    std::exception_ptr *e_ptr = &e;
    dispatch_sync(static_cast<GCDQueue *>(this)->queue, ^{
        try {
            thunk();
        }
        catch (...) {
            *e_ptr = std::current_exception();
        }
    });
    if (e) std::rethrow_exception(e);
}

Queue& Main() {
    static GCDQueue q(dispatch_get_main_queue());
    return q;
}

Queue& Background() {
    static GCDQueue q(dispatch_get_global_queue(0, DISPATCH_QUEUE_PRIORITY_DEFAULT));
    return q;
}

std::unique_ptr<Queue> Create() {
    return std::unique_ptr<Queue>(new OwningQueue(dispatch_queue_create("Aegisub worker queue",
                                                                        DISPATCH_QUEUE_SERIAL)));
}
} }
