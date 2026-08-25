#pragma once

#include "options.h"
#include <wx/window.h>

namespace Theme {
	inline bool IsDarkModeEnabled() { return OPT_GET("App/Dark Mode")->GetBool(); }
	inline int BorderRaised() { return IsDarkModeEnabled() ? wxBORDER_SIMPLE : wxBORDER_RAISED; }
	inline int BorderSunken() { return IsDarkModeEnabled() ? wxBORDER_SIMPLE : wxBORDER_SUNKEN; }
	inline int BorderStaticOrRaised() { return IsDarkModeEnabled() ? wxBORDER_STATIC : wxRAISED_BORDER; }
}
