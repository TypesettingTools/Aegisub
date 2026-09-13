#pragma once

#include <wx/intl.h>

/// Check whether two wx language descriptions refer to the same locale.
///
/// wxWidgets represents some catalog names using a base language (for example
/// "tr") while the system locale uses its regional alias ("tr_TR").  In that
/// case Language and CanonicalName differ, but CanonicalRef identifies the
/// locale the base language aliases.
inline bool LanguagesMatch(wxLanguageInfo const& left, wxLanguageInfo const& right) {
	auto canonical_locale = [](wxLanguageInfo const& info) -> wxString const& {
		return info.CanonicalRef.empty() ? info.CanonicalName : info.CanonicalRef;
	};

	return left.Language == right.Language || canonical_locale(left) == canonical_locale(right);
}
