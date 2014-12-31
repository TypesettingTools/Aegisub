#pragma once

#include <string>
#include <vector>

#include <wx/colour.h>
#include <wx/arrstr.h>
#include <wx/string.h>

#include <libaegisub/color.h>

wxColour to_wx(agi::Color color);
wxString to_wx(std::string const& str);
wxArrayString to_wx(std::vector<std::string> const& vec);

agi::Color from_wx(wxColour color);
std::string from_wx(wxString const& str);

wxArrayString lagi_MRU_wxAS(const char *list);
