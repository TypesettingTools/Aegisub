#include "compat.h"
#include "main.h"

wxArrayString lagi_MRU_wxAS(const wxString &list) {
	wxArrayString work;

	const agi::MRUManager::MRUListMap *map_list = AegisubApp::Get()->mru->Get(STD_STR(list));

	for (agi::MRUManager::MRUListMap::const_iterator i_lst = map_list->begin(); i_lst != map_list->end(); ++i_lst) {
		work.Add(wxString(i_lst->second.c_str(), wxConvUTF8));
	}

	return work;
}
