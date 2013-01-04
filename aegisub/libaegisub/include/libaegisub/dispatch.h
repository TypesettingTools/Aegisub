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

#include <functional>
#include <memory>

namespace agi {
	namespace dispatch {
		typedef std::function<void()> Thunk;

		class Queue {
			virtual void DoInvoke(Thunk thunk)=0;
		public:
			virtual ~Queue() { }

			/// Invoke the thunk on this processing queue, returning immediately
			void Async(Thunk thunk);

			/// Invoke the thunk on this processing queue, returning only when
			/// it's complete
			void Sync(Thunk thunk);
		};

		/// Initialize the dispatch thread pools
		/// @param invoke_main A function which invokes the thunk on the GUI thread
		void Init(std::function<void (Thunk)> invoke_main);

		/// Get the main queue, which runs on the GUI thread
		Queue& Main();

		/// Get the generic background queue, which runs thunks in parallel
		Queue& Background();

		/// Create a new serial queue
		std::unique_ptr<Queue> Create();
	}
}
