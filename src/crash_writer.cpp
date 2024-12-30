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

#include "crash_writer.h"

#include "version.h"

#include <libaegisub/format.h>
#include <libaegisub/fs.h>
#include <libaegisub/util.h>

#include <fstream>
#include <wx/string.h>
#include <wx/stackwalk.h>

using namespace agi;

namespace {
agi::fs::path crashlog_path;

#if wxUSE_STACKWALKER == 1
class StackWalker : public wxStackWalker {
	std::ofstream fp;

public:
	StackWalker(std::string const& cause)
	: fp(crashlog_path, std::ios::app)
	{
		if (!fp.good()) return;

		fp << util::strftime("--- %y-%m-%d %H:%M:%S ------------------\n");
		fp << agi::format("VER - %s\n", GetAegisubLongVersionString());
		fp << agi::format("FTL - Beginning stack dump for \"%s\": \n", cause);
	}

	~StackWalker() {
		if (!fp.good()) return;

		fp << "End of stack dump.\n";
		fp << "----------------------------------------\n\n";
	}

	void OnStackFrame(wxStackFrame const& frame) override final {
		if (!fp.good()) return;

		fp << agi::format("%03u - %p: %s", frame.GetLevel(), frame.GetAddress(), frame.GetName().utf8_str().data());
		if (frame.HasSourceLocation())
			fp << agi::format(" on %s:%u", frame.GetFileName().utf8_str().data(), frame.GetLine());

		fp << "\n";
	}
};
#endif
}

namespace crash_writer {
void Initialize(agi::fs::path const& path) {
	crashlog_path = path / "crashlog.txt";
}

void Cleanup() { }

void Write() {
#if wxUSE_STACKWALKER == 1
	StackWalker walker("Fatal exception");
	walker.WalkFromException();
#endif
}

void Write(std::string const& error) {
	std::ofstream file(crashlog_path, std::ios::app);
	if (file.is_open()) {
		file << util::strftime("--- %y-%m-%d %H:%M:%S ------------------\n");
		file << agi::format("VER - %s\n", GetAegisubLongVersionString());
		file << agi::format("EXC - Aegisub has crashed with unhandled exception \"%s\".\n", error);
		file << "----------------------------------------\n\n";
		file.close();
	}
}
}
