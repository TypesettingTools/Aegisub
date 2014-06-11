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

#import <libaegisub/spellchecker.h>

#import <libaegisub/option_value.h>
#import <libaegisub/signal.h>

#import <AppKit/AppKit.h>

namespace {
std::vector<std::string> array_to_vector(NSArray *arr) {
    std::vector<std::string> ret;
    ret.reserve(arr.count);
    for (NSString *str in arr)
        ret.push_back(str.UTF8String);
    return ret;
}

struct releaser {
    void operator()(id obj) { [obj release]; };
};

template<typename T>
using objc_ptr = std::unique_ptr<T, releaser>;

class CocoaSpellChecker final : public agi::SpellChecker {
    objc_ptr<NSString> language;
    agi::signal::Connection lang_listener;

    void OnLanguageChanged(agi::OptionValue const& opt) {
        language.reset([[NSString alloc] initWithCString:opt.GetString().c_str()
                                                encoding:NSUTF8StringEncoding]);
    }

public:
    CocoaSpellChecker(agi::OptionValue *opt)
    : lang_listener(opt->Subscribe(&CocoaSpellChecker::OnLanguageChanged, this))
    {
        OnLanguageChanged(*opt);
    }

    std::vector<std::string> GetLanguageList() override {
        return array_to_vector([[NSSpellChecker sharedSpellChecker] availableLanguages]);
    }

    bool CheckWord(std::string const& word) override {
        auto str = [NSString stringWithUTF8String:word.c_str()];
        auto range = [NSSpellChecker.sharedSpellChecker checkSpellingOfString:str
                                                                   startingAt:0
                                                                     language:language.get()
                                                                         wrap:NO
                                                       inSpellDocumentWithTag:0
                                                                    wordCount:nullptr];
        return range.location == NSNotFound;
    }

    std::vector<std::string> GetSuggestions(std::string const& word) override {
        auto str = [NSString stringWithUTF8String:word.c_str()];
        auto range = [NSSpellChecker.sharedSpellChecker checkSpellingOfString:str
                                                                   startingAt:0
                                                                     language:language.get()
                                                                         wrap:NO
                                                       inSpellDocumentWithTag:0
                                                                    wordCount:nullptr];
        return array_to_vector([NSSpellChecker.sharedSpellChecker guessesForWordRange:range
                                                                             inString:str
                                                                             language:language.get()
                                                               inSpellDocumentWithTag:0]);
    }

    bool CanAddWord(std::string const&) override {
        return true;
    }

    bool CanRemoveWord(std::string const& str) override {
        return [NSSpellChecker.sharedSpellChecker hasLearnedWord:[NSString stringWithUTF8String:str.c_str()]];
    }

    void AddWord(std::string const& word) override {
        NSSpellChecker.sharedSpellChecker.language = language.get();
        [NSSpellChecker.sharedSpellChecker learnWord:[NSString stringWithUTF8String:word.c_str()]];
    }

    void RemoveWord(std::string const& word) override {
        NSSpellChecker.sharedSpellChecker.language = language.get();
        [NSSpellChecker.sharedSpellChecker unlearnWord:[NSString stringWithUTF8String:word.c_str()]];
    }
};
}

namespace agi {
std::unique_ptr<SpellChecker> CreateCocoaSpellChecker(OptionValue *opt) {
    return std::unique_ptr<SpellChecker>(new CocoaSpellChecker(opt));
}
}
