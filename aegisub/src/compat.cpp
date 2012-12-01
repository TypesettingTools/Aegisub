#include "compat.h"
#include "main.h"

#include <algorithm>

wxArrayString lagi_MRU_wxAS(const wxString &list) {
	const agi::MRUManager::MRUListMap *map = config::mru->Get(STD_STR(list));
	wxArrayString work;
	work.reserve(map->size());
	transform(map->begin(), map->end(), std::back_inserter(work), lagi_wxString);
	return work;
}

wxArrayString to_wx(std::vector<std::string> const& vec) {
	wxArrayString ret;
	ret.reserve(vec.size());
	transform(vec.begin(), vec.end(), std::back_inserter(ret), lagi_wxString);
	return ret;
}
