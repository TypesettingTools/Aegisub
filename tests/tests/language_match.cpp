#include "../../src/language_match.h"

#include <main.h>

#include <stdexcept>

namespace {
wxLanguageInfo const& LanguageInfo(wxLanguage language) {
	auto info = wxLocale::GetLanguageInfo(language);
	if (!info)
		throw std::runtime_error("wxWidgets has no information for the test language");
	return *info;
}

wxLanguageInfo const& LanguageInfo(char const *name) {
	auto info = wxLocale::FindLanguageInfo(name);
	if (!info)
		throw std::runtime_error("wxWidgets has no information for the test locale");
	return *info;
}
}

TEST(language_match, exact_language) {
	EXPECT_TRUE(LanguagesMatch(LanguageInfo(wxLANGUAGE_ENGLISH_US), LanguageInfo("en_US")));
}

TEST(language_match, canonical_reference_alias) {
	// Regression test for #211: on Windows the system locale is tr_TR, while
	// the translation catalog is exposed as tr by wxTranslations.
	auto const& system = LanguageInfo(wxLANGUAGE_TURKISH_TURKEY);
	auto const& catalog = LanguageInfo("tr");

	ASSERT_NE(system.Language, catalog.Language);
	ASSERT_NE(system.CanonicalName, catalog.CanonicalName);
	ASSERT_EQ(system.CanonicalName, catalog.CanonicalRef);
	EXPECT_TRUE(LanguagesMatch(system, catalog));
}

TEST(language_match, distinct_regional_locales) {
	EXPECT_FALSE(LanguagesMatch(LanguageInfo(wxLANGUAGE_ENGLISH_UK), LanguageInfo("en_US")));
}

TEST(language_match, unrelated_languages) {
	EXPECT_FALSE(LanguagesMatch(LanguageInfo(wxLANGUAGE_GERMAN), LanguageInfo("tr")));
}
