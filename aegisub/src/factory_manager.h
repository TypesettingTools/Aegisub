// Copyright (c) 2007-2008, Rodrigo Braz Monteiro
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file factory_manager.h
/// @brief Template/base-class for factory classes
/// @ingroup utility
///


#pragma once


///////////
// Headers
#include <map>
#include <wx/string.h>
#include <wx/arrstr.h>


/////////////////
// Factory class
template <class T>
class FactoryManager {
protected:
	// Static map of all factories
	static std::map<wxString,T*> *factories;

	static void ClearFactories() { 
		if (factories && !factories->empty()) {
			typename std::map<wxString,T*>::iterator iter;
			for (iter = factories->begin(); iter != factories->end(); iter++) {
				delete iter->second;
			}
			factories->clear(); 
		}
		delete factories;
	}

	// Register one factory type (with possible subtypes)
	static void RegisterFactory(T* factory,wxString name, wxArrayString subTypes=wxArrayString()) {
		// Create factories if it doesn't exist
		if (factories == NULL) factories = new std::map<wxString,T*>;

		// Prepare subtypes
		if (subTypes.GetCount() == 0) subTypes.Add(_T(""));
		else {
			for (unsigned int i=0;i<subTypes.GetCount();i++) {
				subTypes[i] = _T("/") + subTypes[i];
			}
		}

		// Insert each subtype
		for (unsigned int i=0;i<subTypes.GetCount();i++) {
			factories->insert(std::make_pair(name.Lower() + subTypes[i],factory));
		}
	}

	// Get a factory with name
	static T *GetFactory(wxString name) {
		// No factories
		if (factories == NULL) {
			factories = new std::map<wxString,T*>;
			return NULL;
		}

		// Search for factory that matches
		typename std::map<wxString,T*>::iterator cur;
		for (cur = factories->begin();cur != factories->end();cur++) {
			if (cur->first.StartsWith(name)) return cur->second;
		}

		// None found
		return NULL;
	}

public:
	// Virtual destructor
	virtual ~FactoryManager() {
		ClearFactories();
	};

	// Get list of all factories, with favourite as first
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
};

