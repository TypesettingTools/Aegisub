#include "compat.h"
#include "options.h"

#include <algorithm>

template<typename T>
wxArrayString to_wxAS(T const& src) {
	wxArrayString ret;
	ret.reserve(src.size());
	transform(src.begin(), src.end(), std::back_inserter(ret), (wxString (*)(std::string const&))to_wx);
	return ret;
}

wxArrayString lagi_MRU_wxAS(const wxString &list) {
	return to_wxAS(*config::mru->Get(from_wx(list)));
}

wxArrayString to_wx(std::vector<std::string> const& vec) {
	return to_wxAS(vec);
}
