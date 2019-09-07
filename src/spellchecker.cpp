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

#include "include/aegisub/spellchecker.h"
#include "spellchecker_hunspell.h"

#include "options.h"

#include <libaegisub/make_unique.h>
#include <libaegisub/spellchecker.h>

#ifdef __APPLE__
namespace agi {
class OptionValue;
std::unique_ptr<agi::SpellChecker> CreateCocoaSpellChecker(OptionValue *opt);
}
#endif

std::unique_ptr<agi::SpellChecker> SpellCheckerFactory::GetSpellChecker() {
#ifdef __APPLE__
	return agi::CreateCocoaSpellChecker(OPT_SET("Tool/Spell Checker/Language"));
#elif defined(WITH_HUNSPELL)
	return agi::make_unique<HunspellSpellChecker>();
#else
	return {};
#endif
}
