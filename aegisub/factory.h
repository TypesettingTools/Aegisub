// Copyright (c) 2007, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once


///////////
// Headers
#include <map>


/////////////////
// Factory class
template <class T>
class AegisubFactory {
protected:
	static std::map<wxString,T*> *factories;
	void RegisterFactory(wxString name) {
		if (factories == NULL) factories = new std::map<wxString,T*>;
		factories->insert(std::make_pair(name.Lower(),(T*)this));
	}
	static T *GetFactory(wxString name) {
		if (factories == NULL) {
			factories = new std::map<wxString,T*>;
			return NULL;
		}
		typename std::map<wxString,T*>::iterator res = factories->find(name.Lower());
		if (res != factories->end()) return res->second;
		return NULL;
	}

public:
	static wxArrayString GetFactoryList(wxString favourite=_T("")) {
		if (factories == NULL) factories = new std::map<wxString,T*>;
		wxArrayString list;
		favourite = favourite.Lower();
		for (typename std::map<wxString,T*>::iterator cur=factories->begin();cur!=factories->end();cur++) {
			if (cur->first == favourite) list.Insert(cur->first,0);
			else list.Add(cur->first);
		}
		return list;
	}
	virtual ~AegisubFactory() {}
};
