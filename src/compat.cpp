#include "compat.h"

#include "options.h"

#include <algorithm>

wxArrayString lagi_MRU_wxAS(std::string const& list) {
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
	transform(vec.begin(), vec.end(), std::back_inserter(ret), (wxString (*)(std::string const&))to_wx);
	return ret;
}
