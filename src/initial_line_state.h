// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <libaegisub/signal.h>

#include <string>

namespace agi { struct Context; }
class AssDialogue;

class InitialLineState {
	agi::signal::Connection active_line_connection;
	std::string initial_text;
	int line_id;

	agi::signal::Signal<std::string const&> InitialStateChanged;
	void OnActiveLineChanged(AssDialogue *new_line);

public:
	InitialLineState(agi::Context *c);

	std::string const& GetInitialText() const { return initial_text; }
	DEFINE_SIGNAL_ADDERS(InitialStateChanged, AddChangeListener)
};
