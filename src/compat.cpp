#include "compat.h"

#include "options.h"

#include <algorithm>

wxArrayString lagi_MRU_wxAS(const char *list) {
	auto const& vec = *config::mru->Get(list);
	wxArrayString ret;
	ret.reserve(vec.size());
	transform(vec.begin(), vec.end(), std::back_inserter(ret),
		[](std::filesystem::path const& p) { return p.wstring(); });
	return ret;
}

wxArrayString to_wx(std::span<const std::string> vec) {
	wxArrayString ret;
	ret.reserve(vec.size());
	transform(vec.begin(), vec.end(), std::back_inserter(ret),
	          (wxString (*)(std::string const&))to_wx);
	return ret;
}

wxArrayString to_wx(std::span<const std::string_view> vec) {
	wxArrayString ret;
	ret.reserve(vec.size());
	transform(vec.begin(), vec.end(), std::back_inserter(ret),
	          (wxString (*)(std::string_view))to_wx);
	return ret;
}

wxColour to_wx(agi::Color color) {
	return wxColour(color.r, color.g, color.b, 255 - color.a);
}

wxString to_wx(std::string const& str) {
	return wxString::FromUTF8Unchecked(str);
}

wxString to_wx(std::string_view str) {
	return wxString::FromUTF8Unchecked(str.data(), str.size());
}

wxString to_wx(const char *str) {
	return wxString::FromUTF8Unchecked(str);
}

agi::Color from_wx(wxColour color) {
	return agi::Color(color.Red(), color.Green(), color.Blue(), 255 - color.Alpha());
}

std::string from_wx(wxString const& str) {
	return std::string(str.utf8_str());
}
