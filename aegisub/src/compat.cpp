#include "compat.h"
#include "main.h"

#ifndef AGI_PRE
#include <algorithm>
#endif

wxArrayString lagi_MRU_wxAS(const wxString &list) {
	const agi::MRUManager::MRUListMap *map = config::mru->Get(STD_STR(list));
	wxArrayString work;
	work.reserve(map->size());
	transform(map->begin(), map->end(), std::back_inserter(work), lagi_wxString);
	return work;
}
