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


///////////
// Headers
#include "gecko_controller.h"
#include "gecko_display.h"
#include "gecko/nsEmbedCID.h"
#include "gecko/nsCOMPtr.h"
#include "gecko/nsIProfile.h"
#include "gecko/nsServiceManagerUtils.h"
#include "gecko/nsXPCOM.h"
#include "gecko/nsIComponentRegistrar.h"
#include "gecko/nsComponentManagerUtils.h"
#include "gecko/nsIWebBrowserSetup.h"
#include "gecko/nsIInterfaceRequestor.h"
#include "gecko/nsIBaseWindow.h"
#include "gecko/nsIWidget.h"


/////////////
// Libraries
#if __VISUALC__ >= 1200
#pragma comment(lib,"nspr4.lib")
#pragma comment(lib,"plds4.lib")
#pragma comment(lib,"plc4.lib")
#pragma comment(lib,"xpcom.lib")
#pragma comment(lib,"embed_base_s.lib")
#pragma comment(lib,"xpcomglue_s.lib")
#pragma comment(lib,"xpcomglue.lib")
#endif


///////////
// Statics
int GeckoController::controllers = 0;


///////////////
// Constructor
GeckoController::GeckoController(GeckoDisplay *_display,const wxString _path)
{
	try {
		// Setup
		nsresult rv;
		display = _display;

		// Gecko Controller count
		controllers++;

		// Initialize Gecko
		if (controllers == 1) {
			// Get folder
			nsCOMPtr<nsILocalFile> file;
			wxString path = _path;
			path = path.Left(path.Length()-1);
			rv = NS_NewLocalFile(nsString(path.c_str()),false,getter_AddRefs(file));
			if (NS_FAILED(rv)) throw rv;

			// Initialize embedding
			rv = NS_InitEmbedding(file,nsnull);
			// If at first you don't succeed... (and you won't)
			if (NS_FAILED(rv)) {
				// ...the second does seem to work, though. lol, gecko.
				rv = NS_InitEmbedding(file,nsnull);
				if (NS_FAILED(rv)) throw rv;
			}

			// Register factories... I think
			if (NS_FAILED(rv)) throw rv;
			nsCOMPtr<nsIComponentRegistrar> registrar;
			rv = NS_GetComponentRegistrar(getter_AddRefs(registrar));
			if (NS_FAILED(rv)) throw rv;
			rv = registrar->AutoRegister(nsnull);
		}

		AddRef();

		// Create browser
		nsWebBrowser = do_CreateInstance(NS_WEBBROWSER_CONTRACTID,&rv);
		if (NS_FAILED(rv)) throw rv;

		// Get pointer to navigation
		rv = NS_OK;
		nsNav = do_QueryInterface(nsWebBrowser, &rv);
		if (NS_FAILED(rv)) throw rv;

		// Set container window
		rv = nsWebBrowser->SetContainerWindow(NS_STATIC_CAST(nsIWebBrowserChrome*, this));
		if (NS_FAILED(rv)) throw rv;

		// Setup
		nsCOMPtr<nsIWebBrowserSetup> setup (do_QueryInterface(nsWebBrowser));
		if (setup) setup->SetProperty(nsIWebBrowserSetup::SETUP_IS_CHROME_WRAPPER,PR_TRUE);
		else throw NS_ERROR_FAILURE;

		// Create the base window
		rv = NS_OK;
		mBaseWindow = do_QueryInterface(nsWebBrowser, &rv);
		if (NS_FAILED(rv)) throw rv;

		// Initialize the window
		wxSize size = display->GetClientSize();
		rv = mBaseWindow->InitWindow(nsNativeWidget(display->GetHandle()), nsnull,0, 0, size.GetWidth(),size.GetHeight());
		if (NS_FAILED(rv)) throw rv;
		rv = mBaseWindow->Create();
		if (NS_FAILED(rv)) throw rv;

		// Set listener
		//nsCOMPtr<nsIWebProgressListener> listener(NS_STATIC_CAST(nsIWebProgressListener*, this));
		//nsCOMPtr<nsIWeakReference> thisListener(do_GetWeakReference(listener));
		//rv = nsWebBrowser->AddWebBrowserListener(thisListener, NS_GET_IID(nsIWebProgressListener));
		nsWeakPtr weakling (dont_AddRef(NS_GetWeakReference(NS_STATIC_CAST(nsIWebProgressListener*, this))));
		rv = nsWebBrowser->AddWebBrowserListener(weakling, NS_GET_IID(nsIWebProgressListener));
		if (NS_FAILED(rv)) throw rv;

		// Show window
		rv = mBaseWindow->SetVisibility(PR_TRUE);
		if (NS_FAILED(rv)) throw rv;

		// Navigate somewhere for shits and giggles
		rv = nsNav->LoadURI(L"http://www.aegisub.net",nsIWebNavigation::LOAD_FLAGS_NONE,nsnull,nsnull,nsnull);
		if (NS_FAILED(rv)) throw rv;
	}

	// Failed
	catch (...) {
		NS_TermEmbedding();
	}
}


//////////////
// Destructor
GeckoController::~GeckoController()
{
	controllers--;
	if (controllers == 0) NS_TermEmbedding();
}


////////////
// Set size
void GeckoController::SetSize(wxSize &size)
{
	mBaseWindow->SetPositionAndSize(0,0,size.GetWidth(),size.GetHeight(),true);
	//mBaseWindow->SetPositionAndSize(32,32,size.GetWidth()-64,size.GetHeight()-64,true);
	mBaseWindow->SetVisibility(true);
}


///////////////
// nsISupports
NS_IMPL_ADDREF(GeckoController)
NS_IMPL_RELEASE(GeckoController)

NS_INTERFACE_MAP_BEGIN(GeckoController)
NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIWebBrowserChrome)
//NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
NS_INTERFACE_MAP_ENTRY(nsIWebBrowserChrome)
NS_INTERFACE_MAP_ENTRY(nsIEmbeddingSiteWindow)
NS_INTERFACE_MAP_ENTRY(nsIWebProgressListener)
NS_INTERFACE_MAP_ENTRY(nsSupportsWeakReference)
NS_INTERFACE_MAP_END


///////////////////////
// nsIWebBrowserChrome
nsresult GeckoController::SetStatus(PRUint32 statusType, const PRUnichar *status)
{
	return NS_OK;
}

nsresult GeckoController::DestroyBrowserWindow()
{
	return NS_OK;
}

nsresult GeckoController::SizeBrowserTo(PRInt32 aCX, PRInt32 aCY)
{
	display->SetClientSize(aCX,aCY);
	return NS_OK;
}

nsresult GeckoController::ShowAsModal()
{
	return NS_OK;
}

nsresult GeckoController::IsWindowModal(PRBool *_retval)
{
	*_retval = FALSE;
	return NS_ERROR_NOT_IMPLEMENTED;
}

nsresult GeckoController::ExitModalEventLoop(nsresult aStatus)
{
	return NS_OK;
}

nsresult GeckoController::SetWebBrowser(nsIWebBrowser *aWebBrowser)
{
	nsWebBrowser = aWebBrowser;
	return NS_OK;
}

nsresult GeckoController::GetWebBrowser(nsIWebBrowser **aWebBrowser)
{
	if (aWebBrowser) *aWebBrowser = nsWebBrowser;
	return NS_OK;
}

nsresult GeckoController::SetChromeFlags(PRUint32 aChromeFlags)
{
	mChromeFlags = aChromeFlags;
	return NS_OK;
}

nsresult GeckoController::GetChromeFlags(PRUint32 *aChromeFlags)
{
	if (aChromeFlags) *aChromeFlags = mChromeFlags;
	return NS_OK;
}


//////////////////////////
// nsIEmbeddingSiteWindow
nsresult GeckoController::SetDimensions(PRUint32 flags, PRInt32 x, PRInt32 y, PRInt32 cx, PRInt32 cy)
{
	return NS_OK;
}

nsresult GeckoController::GetDimensions(PRUint32 flags, PRInt32 *x, PRInt32 *y, PRInt32 *cx, PRInt32 *cy)
{
	wxRect pos = display->GetClientRect();
	if (flags & nsIEmbeddingSiteWindow::DIM_FLAGS_POSITION) {
		if (x) *x = pos.GetLeft();
		if (y) *y = pos.GetTop();
	}
	else if (flags & nsIEmbeddingSiteWindow::DIM_FLAGS_SIZE_INNER) {
		wxRect size = display->GetClientSize();
		if (cx) *cx = size.GetWidth();
		if (cy) *cy = size.GetHeight();
	}
	else if (flags & nsIEmbeddingSiteWindow::DIM_FLAGS_SIZE_OUTER) {
		wxRect size = display->GetSize();
		if (cx) *cx = size.GetWidth();
		if (cy) *cy = size.GetHeight();
	}
	return NS_OK;
}

nsresult GeckoController::SetFocus(void)
{
	display->SetFocus();
	return NS_OK;
}

nsresult GeckoController::GetVisibility(PRBool *aVisibility)
{
	*aVisibility = TRUE;
	return NS_OK;
}

nsresult GeckoController::SetVisibility(PRBool aVisibility)
{
	return NS_OK;
}

nsresult GeckoController::GetTitle(PRUnichar * *aTitle)
{
	if (aTitle) *aTitle = nsnull;
	return NS_ERROR_NOT_IMPLEMENTED;
}

nsresult GeckoController::SetTitle(const PRUnichar * aTitle)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

nsresult GeckoController::GetSiteWindow(void * *aSiteWindow)
{
	if (aSiteWindow) *aSiteWindow = (nativeWindow) display->GetHandle();
	return NS_OK;
}


//////////////////////////
// nsIWebProgressListener
nsresult GeckoController::OnStateChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
{
	return NS_OK;
}

nsresult GeckoController::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
	return NS_OK;
}

nsresult GeckoController::OnLocationChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsIURI *aLocation)
{
	return NS_OK;
}

nsresult GeckoController::OnStatusChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsresult aStatus, const PRUnichar *aMessage)
{
	return NS_OK;
}

nsresult GeckoController::OnSecurityChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aState)
{
	return NS_OK;
}
