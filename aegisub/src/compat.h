#ifndef AGI_PRE
#include <string>

#include <wx/colour.h>
#include <wx/arrstr.h>
#include <wx/string.h>
#endif

#include <libaegisub/color.h>

#define STD_STR(x) std::string((x).utf8_str())

inline wxColour to_wx(agi::Color color) { return wxColour(color.r, color.g, color.b, 255 - color.a); }
inline wxString to_wx(std::string const& str) { return wxString(str.c_str(), wxConvUTF8); }

inline agi::Color from_wx(wxColour color) { return agi::Color(color.Red(), color.Green(), color.Blue(), 255 - color.Alpha()); }
inline std::string from_wx(wxString const& str) { return std::string(str.utf8_str()); }

inline wxString lagi_wxString(const std::string &str) { return wxString(str.c_str(), wxConvUTF8); }
wxArrayString lagi_MRU_wxAS(const wxString &list);
