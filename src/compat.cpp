#include "compat.h"

#include "options.h"

#include <algorithm>

wxArrayString lagi_MRU_wxAS(const char* list) {
	auto const& vec = *config::mru->Get(list);
	wxArrayString ret;
	ret.reserve(vec.size());
	transform(vec.begin(), vec.end(), std::back_inserter(ret),
	          [](agi::fs::path const& p) { return p.wstring(); });
	return ret;
}

wxArrayString to_wx(std::vector<std::string> const& vec) {
	wxArrayString ret;
	ret.reserve(vec.size());
	transform(vec.begin(), vec.end(), std::back_inserter(ret),
	          (wxString(*)(std::string const&))to_wx);
	return ret;
}

wxColour to_wx(agi::Color color) {
	return wxColour(color.r, color.g, color.b, 255 - color.a);
}

wxString to_wx(std::string const& str) {
	return wxString(str.c_str(), wxConvUTF8);
}

agi::Color from_wx(wxColour color) {
	return agi::Color(color.Red(), color.Green(), color.Blue(), 255 - color.Alpha());
}

std::string from_wx(wxString const& str) {
	return std::string(str.utf8_str());
}
