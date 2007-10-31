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
//   * Neither the name of the TrayDict Group nor the names of its contributors
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
// TRAYDICT
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#pragma once


///////////
// Headers
#include <wx/wxprec.h>
#include "gecko/nsStringAPI.h"
#include "gecko/nsEmbedAPI.h"
#include "gecko/nsIWebBrowserChrome.h"
#include "gecko/nsIEmbeddingSiteWindow.h"
#include "gecko/nsIWebProgressListener.h"
#include "gecko/nsWeakReference.h"
#include "gecko/nsIWebNavigation.h"
#include "gecko/nsIWebBrowser.h"
#include "gecko/nsIBaseWindow.h"


//////////////
// Prototypes
class GeckoDisplay;


////////////////////
// Gecko Controller
class GeckoController : public nsIWebBrowserChrome,
						public nsIEmbeddingSiteWindow,
						public nsIWebProgressListener,
						public nsSupportsWeakReference
{
private:
	int refCount;
	static int controllers;
	GeckoDisplay *display;

	nsCOMPtr<nsIWebNavigation> nsNav;
	nsCOMPtr<nsIWebBrowser> nsWebBrowser;
	nsCOMPtr<nsIBaseWindow> mBaseWindow;
	unsigned int mChromeFlags;

public:
	GeckoController(GeckoDisplay *_display,const wxString _path);
	~GeckoController();

	void SetSize(wxSize &size);

	NS_DECL_ISUPPORTS
	NS_DECL_NSIWEBBROWSERCHROME
	NS_DECL_NSIEMBEDDINGSITEWINDOW
	NS_DECL_NSIWEBPROGRESSLISTENER
	//NS_DECL_NSISUPPORTSWEAKREFERENCE
};
