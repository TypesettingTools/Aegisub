#include <string>
#include <vector>

#include <wx/colour.h>
#include <wx/arrstr.h>
#include <wx/string.h>

#include <libaegisub/color.h>

inline wxColour to_wx(agi::Color color) { return wxColour(color.r, color.g, color.b, 255 - color.a); }
inline wxString to_wx(std::string const& str) { return wxString(str.c_str(), wxConvUTF8); }
wxArrayString to_wx(std::vector<std::string> const& vec);

inline agi::Color from_wx(wxColour color) { return agi::Color(color.Red(), color.Green(), color.Blue(), 255 - color.Alpha()); }
inline std::string from_wx(wxString const& str) { return std::string(str.utf8_str()); }

wxArrayString lagi_MRU_wxAS(const wxString &list);
