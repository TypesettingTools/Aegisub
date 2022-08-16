#pragma once

#include <span>
#include <string_view>
#include <vector>

#include <wx/colour.h>
#include <wx/arrstr.h>
#include <wx/string.h>

#include <libaegisub/color.h>

wxColour to_wx(agi::Color color);
wxString to_wx(std::string_view str);
wxString to_wx(std::string const& str);
wxString to_wx(const char *str);
wxArrayString to_wx(std::span<const std::string> vec);
wxArrayString to_wx(std::span<const std::string_view> vec);
template<typename T> wxArrayString to_wx(std::vector<T> const& vec) { return to_wx(std::span(vec)); }

agi::Color from_wx(wxColour color);
std::string from_wx(wxString const& str);

wxArrayString lagi_MRU_wxAS(const char *list);
