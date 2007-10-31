#include "IEHtmlWin.h"
//#include "App/EdApp.h"
#include <wx/msw/ole/oleutils.h>
#include <oleidl.h>
#include <winerror.h>
#include <exdispid.h>
#include <olectl.h>
#include <strsafe.h>

BEGIN_EVENT_TABLE(IEHtmlWin, wxWindow)
	EVT_SIZE(IEHtmlWin::OnSize)
	//EVT_MOVE(IEHtmlWin::OnMove)
	EVT_SET_FOCUS(IEHtmlWin::OnSetFocus)
	EVT_PAINT(IEHtmlWin::OnPaint)
	EVT_MOUSE_EVENTS(IEHtmlWin::OnMouse)
	EVT_CHAR(IEHtmlWin::OnChar)
END_EVENT_TABLE()

void wxLogTrace(const wchar_t *lol) {}

class IEHtmlWin;
class FS_IOleInPlaceFrame;
class FS_IOleInPlaceSiteWindowless;
class FS_IOleClientSite;
class FS_IOleControlSite;
class FS_IOleCommandTarget;
class FS_IOleItemContainer;
class FS_IDispatch;
class FS_DWebBrowserEvents2;
class FS_IAdviseSink2;
class FS_IAdviseSinkEx;

class FrameSite : public IUnknown
{
	friend class IEHtmlWin;
	friend class FS_IOleInPlaceFrame;
	friend class FS_IOleInPlaceSiteWindowless;
	friend class FS_IOleClientSite;
	friend class FS_IOleControlSite;
	friend class FS_IOleCommandTarget;
	friend class FS_IOleItemContainer;
	friend class FS_IDispatch;
	friend class FS_DWebBrowserEvents2;
	friend class FS_IAdviseSink2;
	friend class FS_IAdviseSinkEx;
public:
	FrameSite(IEHtmlWin * win);
	~FrameSite();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

protected:
	int m_cRef;

	FS_IOleInPlaceFrame * m_IOleInPlaceFrame;
	FS_IOleInPlaceSiteWindowless * m_IOleInPlaceSiteWindowless;
	FS_IOleClientSite * m_IOleClientSite;
	FS_IOleControlSite * m_IOleControlSite;
	FS_IOleCommandTarget * m_IOleCommandTarget;
	FS_IOleItemContainer * m_IOleItemContainer;
	FS_IDispatch * m_IDispatch;
	FS_DWebBrowserEvents2 * m_DWebBrowserEvents2;
	FS_IAdviseSink2 * m_IAdviseSink2;
	FS_IAdviseSinkEx * m_IAdviseSinkEx;
	
	IEHtmlWin * m_window;

	HDC m_hDCBuffer;
	HWND m_hWndParent;

	bool m_bSupportsWindowlessActivation;
	bool m_bInPlaceLocked;
	bool m_bInPlaceActive;
	bool m_bUIActive;
	bool m_bWindowless;

	LCID m_nAmbientLocale;
	COLORREF m_clrAmbientForeColor;
	COLORREF m_clrAmbientBackColor;
	bool m_bAmbientShowHatching;
	bool m_bAmbientShowGrabHandles;
	bool m_bAmbientUserMode;
	bool m_bAmbientAppearance;
};

class FS_IOleInPlaceFrame : public IOleInPlaceFrame
{
public:
	FS_IOleInPlaceFrame(FrameSite* fs) { m_fs = fs; }
	~FS_IOleInPlaceFrame() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IOleWindow
	STDMETHODIMP GetWindow(HWND*);
	STDMETHODIMP ContextSensitiveHelp(BOOL);
	//IOleInPlaceUIWindow
	STDMETHODIMP GetBorder(LPRECT);
	STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS);
	STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS);
	STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject*, LPCOLESTR);
	//IOleInPlaceFrame
	STDMETHODIMP InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS);
	STDMETHODIMP SetMenu(HMENU, HOLEMENU, HWND);
	STDMETHODIMP RemoveMenus(HMENU);
	STDMETHODIMP SetStatusText(LPCOLESTR);
	STDMETHODIMP EnableModeless(BOOL);
	STDMETHODIMP TranslateAccelerator(LPMSG, WORD);
protected:
	FrameSite * m_fs;
};

class FS_IOleInPlaceSiteWindowless : public IOleInPlaceSiteWindowless
{
public:
	FS_IOleInPlaceSiteWindowless(FrameSite* fs) { m_fs = fs; }
	~FS_IOleInPlaceSiteWindowless() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IOleWindow
	STDMETHODIMP GetWindow(HWND* h)
	{ return m_fs->m_IOleInPlaceFrame->GetWindow(h); }
	STDMETHODIMP ContextSensitiveHelp(BOOL b)
	{ return m_fs->m_IOleInPlaceFrame->ContextSensitiveHelp(b); }
	//IOleInPlaceSite
	STDMETHODIMP CanInPlaceActivate();
	STDMETHODIMP OnInPlaceActivate();
	STDMETHODIMP OnUIActivate();
	STDMETHODIMP GetWindowContext(IOleInPlaceFrame**, IOleInPlaceUIWindow**, 
		LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
	STDMETHODIMP Scroll(SIZE);
	STDMETHODIMP OnUIDeactivate(BOOL);
	STDMETHODIMP OnInPlaceDeactivate();
	STDMETHODIMP DiscardUndoState();
	STDMETHODIMP DeactivateAndUndo();
	STDMETHODIMP OnPosRectChange(LPCRECT);
	//IOleInPlaceSiteEx
	STDMETHODIMP OnInPlaceActivateEx(BOOL*, DWORD);
	STDMETHODIMP OnInPlaceDeactivateEx(BOOL);
	STDMETHODIMP RequestUIActivate();
	//IOleInPlaceSiteWindowless
	STDMETHODIMP CanWindowlessActivate();
	STDMETHODIMP GetCapture();
	STDMETHODIMP SetCapture(BOOL);
	STDMETHODIMP GetFocus();
	STDMETHODIMP SetFocus(BOOL);
	STDMETHODIMP GetDC(LPCRECT, DWORD, HDC*);
	STDMETHODIMP ReleaseDC(HDC);
	STDMETHODIMP InvalidateRect(LPCRECT, BOOL);
	STDMETHODIMP InvalidateRgn(HRGN, BOOL);
	STDMETHODIMP ScrollRect(INT, INT, LPCRECT, LPCRECT);
	STDMETHODIMP AdjustRect(LPRECT);
	STDMETHODIMP OnDefWindowMessage(UINT, WPARAM, LPARAM, LRESULT*);
protected:
	FrameSite * m_fs;
};

class FS_IOleClientSite : public IOleClientSite
{
public:
	FS_IOleClientSite(FrameSite* fs) { m_fs = fs; }
	~FS_IOleClientSite() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IOleClientSite
	STDMETHODIMP SaveObject();
	STDMETHODIMP GetMoniker(DWORD, DWORD, IMoniker**);
	STDMETHODIMP GetContainer(LPOLECONTAINER FAR*);
	STDMETHODIMP ShowObject();
	STDMETHODIMP OnShowWindow(BOOL);
	STDMETHODIMP RequestNewObjectLayout();
protected:
	FrameSite * m_fs;
};

class FS_IOleControlSite : public IOleControlSite
{
public:
	FS_IOleControlSite(FrameSite* fs) { m_fs = fs; }
	~FS_IOleControlSite() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IOleControlSite
	STDMETHODIMP OnControlInfoChanged();
	STDMETHODIMP LockInPlaceActive(BOOL);
	STDMETHODIMP GetExtendedControl(IDispatch**);
	STDMETHODIMP TransformCoords(POINTL*, POINTF*, DWORD);
	STDMETHODIMP TranslateAccelerator(LPMSG, DWORD);
	STDMETHODIMP OnFocus(BOOL);
	STDMETHODIMP ShowPropertyFrame();
protected:
	FrameSite * m_fs;
};

class FS_IOleCommandTarget : public IOleCommandTarget
{
public:
	FS_IOleCommandTarget(FrameSite* fs) { m_fs = fs; }
	~FS_IOleCommandTarget() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IOleCommandTarget
	STDMETHODIMP QueryStatus(const GUID*, ULONG, OLECMD[], OLECMDTEXT*);
	STDMETHODIMP Exec(const GUID*, DWORD, DWORD, VARIANTARG*, VARIANTARG*);
protected:
	FrameSite * m_fs;
};

class FS_IOleItemContainer : public IOleItemContainer
{
public:
	FS_IOleItemContainer(FrameSite* fs) { m_fs = fs; }
	~FS_IOleItemContainer() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IParseDisplayName
	STDMETHODIMP ParseDisplayName(IBindCtx*, LPOLESTR, ULONG*, IMoniker**);
	//IOleContainer
	STDMETHODIMP EnumObjects(DWORD, IEnumUnknown**);
	STDMETHODIMP LockContainer(BOOL);
	//IOleItemContainer
	STDMETHODIMP GetObjectW(LPOLESTR, DWORD, IBindCtx*, REFIID, void**);
	STDMETHODIMP GetObjectStorage(LPOLESTR, IBindCtx*, REFIID, void**);
	STDMETHODIMP IsRunning(LPOLESTR);
protected:
	FrameSite * m_fs;
};

class FS_IDispatch : public IDispatch
{
public:
	FS_IDispatch(FrameSite* fs) { m_fs = fs; }
	~FS_IDispatch() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IDispatch
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR**, unsigned int, LCID, DISPID*);
	STDMETHODIMP GetTypeInfo(unsigned int, LCID, ITypeInfo**);
	STDMETHODIMP GetTypeInfoCount(unsigned int*);
	STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*);
protected:
	FrameSite * m_fs;
};

class FS_DWebBrowserEvents2 : public DWebBrowserEvents2
{
public:
	FS_DWebBrowserEvents2(FrameSite* fs) { m_fs = fs; }
	~FS_DWebBrowserEvents2() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IDispatch
	STDMETHODIMP GetIDsOfNames(REFIID r, OLECHAR** o, unsigned int i, LCID l, DISPID* d)
	{ return m_fs->m_IDispatch->GetIDsOfNames(r, o, i, l, d); }
	STDMETHODIMP GetTypeInfo(unsigned int i, LCID l, ITypeInfo** t)
	{ return m_fs->m_IDispatch->GetTypeInfo(i, l, t); }
	STDMETHODIMP GetTypeInfoCount(unsigned int* i)
	{ return m_fs->m_IDispatch->GetTypeInfoCount(i); }
	STDMETHODIMP Invoke(DISPID d, REFIID r, LCID l, WORD w, DISPPARAMS* dp, 
		VARIANT* v, EXCEPINFO* e, UINT* u)
	{ return m_fs->m_IDispatch->Invoke(d, r, l, w, dp, v, e, u); }
protected:
	FrameSite * m_fs;
};

class FS_IAdviseSink2 : public IAdviseSink2
{
public:
	FS_IAdviseSink2(FrameSite* fs) { m_fs = fs; }
	~FS_IAdviseSink2() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IAdviseSink
	void STDMETHODCALLTYPE OnDataChange(FORMATETC*, STGMEDIUM*);
	void STDMETHODCALLTYPE OnViewChange(DWORD, LONG);
	void STDMETHODCALLTYPE OnRename(IMoniker*);
	void STDMETHODCALLTYPE OnSave();
	void STDMETHODCALLTYPE OnClose();
	//IAdviseSink2
	void STDMETHODCALLTYPE OnLinkSrcChange(IMoniker*);
protected:
	FrameSite * m_fs;
};

class FS_IAdviseSinkEx : public IAdviseSinkEx
{
public:
	FS_IAdviseSinkEx(FrameSite* fs) { m_fs = fs; }
	~FS_IAdviseSinkEx() {}

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }
	//IAdviseSink
	void STDMETHODCALLTYPE OnDataChange(FORMATETC* f, STGMEDIUM* s)
	{ m_fs->m_IAdviseSink2->OnDataChange(f, s); }
	void STDMETHODCALLTYPE OnViewChange(DWORD d, LONG l)
	{ m_fs->m_IAdviseSink2->OnViewChange(d, l); }
	void STDMETHODCALLTYPE OnRename(IMoniker* i)
	{ m_fs->m_IAdviseSink2->OnRename(i); }
	void STDMETHODCALLTYPE OnSave()
	{ m_fs->m_IAdviseSink2->OnSave(); }
	void STDMETHODCALLTYPE OnClose()
	{ m_fs->m_IAdviseSink2->OnClose(); }
	//IAdviseSinkEx
	void STDMETHODCALLTYPE OnViewStatusChange(DWORD);
protected:
	FrameSite * m_fs;
};

class wxIStream : public IStream {
	wxIStream(const wxString &src) : source(src) {
		_refcount = 1;
		pos = 0;
	}

	~wxIStream() {
	}

public:
	HRESULT static OpenString(const wxString &str, IStream ** ppStream) {
		*ppStream = new wxIStream(str);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) { 
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IStream) || iid == __uuidof(ISequentialStream)) {
			*ppvObject = static_cast<IStream*>(this);
			AddRef();
			return S_OK;
		}
		else return E_NOINTERFACE; 
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) { 
		return (ULONG)InterlockedIncrement(&_refcount); 
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) {
		ULONG res = (ULONG) InterlockedDecrement(&_refcount);
		if (res == 0) delete this;
		return res;
	}

	// ISequentialStream Interface
public:
	virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead) {
		memcpy(pv,&source[pos],cb*2);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten) {
		return E_NOTIMPL;
	}

	// IStream Interface
public:
	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) { 
		return E_NOTIMPL;   
	}

	virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) { 
		return E_NOTIMPL;   
	}

	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD)	{ 
		return E_NOTIMPL;   
	}

	virtual HRESULT STDMETHODCALLTYPE Revert(void) { 
		return E_NOTIMPL;   
	}

	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)	{ 
		return E_NOTIMPL;   
	}

	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { 
		return E_NOTIMPL;   
	}

	virtual HRESULT STDMETHODCALLTYPE Clone(IStream **) { 
		return E_NOTIMPL;   
	}

	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin,ULARGE_INTEGER* lpNewFilePointer) { 
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg, DWORD grfStatFlag) {
		return E_NOTIMPL;
	}

private:
	const wxString &source;
	int pos;
	LONG _refcount;
};

IEHtmlWin::IEHtmlWin(wxWindow * parent, wxWindowID id)
	: wxWindow(parent, id)
{
	m_oleObject = NULL;
	m_oleInPlaceObject = NULL;
	m_webBrowser = NULL;

	m_currentUrl = _T("");
	m_specificallyOpened = false;

	CreateBrowser();
}

static const CLSID CLSID_MozillaBrowser =
{ 0x1339B54C, 0x3453, 0x11D2,
  { 0x93, 0xB9, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00 } };

void IEHtmlWin::CreateBrowser()
{
	SetTransparent(255);

	HRESULT hret;

	IUnknown *p;			
	// Get IUnknown Interface
	hret = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_ALL, IID_IUnknown, (void**)(&p));
	wxASSERT(SUCCEEDED(hret));

	// Get IOleObject interface
	hret = p->QueryInterface(IID_IViewObject, (void**)(&m_viewObject));
	wxASSERT(SUCCEEDED(hret));
	hret = p->QueryInterface(IID_IOleObject, (void**)(&m_oleObject));
	wxASSERT(SUCCEEDED(hret));

	FrameSite * c = new FrameSite(this);
	c->AddRef();

	DWORD dwMiscStatus;
	m_oleObject->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);
	bool m_bSetClientSiteFirst = false;
	if (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
	{
		m_bSetClientSiteFirst = true;
	}
	bool m_bVisibleAtRuntime = true;
	if (dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME)
	{
		m_bVisibleAtRuntime = false;
	}

	if (m_bSetClientSiteFirst) m_oleObject->SetClientSite(c->m_IOleClientSite);

	IPersistStreamInit * psInit = NULL;
	hret = p->QueryInterface(IID_IPersistStreamInit, (void**)(&psInit));
	if (SUCCEEDED(hret) && psInit != NULL) {
		hret = psInit->InitNew();
		wxASSERT(SUCCEEDED(hret));
	}

	// Get IOleInPlaceObject interface
	hret = p->QueryInterface(IID_IOleInPlaceObject, (void**)(&m_oleInPlaceObject));
	assert(SUCCEEDED(hret));

	hret = m_oleInPlaceObject->GetWindow(&m_oleObjectHWND);
	wxASSERT(SUCCEEDED(hret));

	::SetActiveWindow(m_oleObjectHWND);
	
	int w, h;
	GetSize(&w, &h);
	RECT posRect;
	posRect.left = 0;
	posRect.top = 0;
	posRect.right = w;
	posRect.bottom = h;
	
	m_oleInPlaceObject->SetObjectRects(&posRect, &posRect);

	if (m_bVisibleAtRuntime) {
		hret = m_oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, 
			c->m_IOleClientSite, 0, (HWND)GetHWND(), &posRect);
	}
	
	if (!m_bSetClientSiteFirst) m_oleObject->SetClientSite(c->m_IOleClientSite);

	// Get IWebBrowser2 Interface
	hret = p->QueryInterface(IID_IWebBrowser2, (void**)(&m_webBrowser));
	assert(SUCCEEDED(hret));

	IConnectionPointContainer * cpContainer;
	hret = p->QueryInterface(IID_IConnectionPointContainer, (void**)(&cpContainer));
	assert(SUCCEEDED(hret));
	hret = cpContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_connectionPoint);
	assert(SUCCEEDED(hret));
	m_connectionPoint->Advise(c->m_DWebBrowserEvents2, &m_adviseCookie);
	cpContainer->Release();

	p->Release();

	m_webBrowser->put_MenuBar(VARIANT_FALSE);
	m_webBrowser->put_AddressBar(VARIANT_FALSE);
	m_webBrowser->put_StatusBar(VARIANT_FALSE);
	m_webBrowser->put_ToolBar(VARIANT_FALSE);

	m_webBrowser->put_RegisterAsBrowser(VARIANT_TRUE);
	m_webBrowser->put_RegisterAsDropTarget(VARIANT_TRUE);
}

IEHtmlWin::~IEHtmlWin()
{
	if (m_oleInPlaceObject) {
		m_oleInPlaceObject->InPlaceDeactivate();
		m_oleInPlaceObject->UIDeactivate();
		m_oleInPlaceObject->Release();
	}
	if (m_connectionPoint) {
		m_connectionPoint->Unadvise(m_adviseCookie);
		m_connectionPoint->Release();
	}
	if (m_oleObject) {
		m_oleObject->Close(OLECLOSE_NOSAVE);
		m_oleObject->SetClientSite(NULL);
		m_oleObject->Release();
	}
	if (m_viewObject) {
		m_viewObject->Release();
	}
	if (m_webBrowser) {
		m_webBrowser->Release();
	}
}

void IEHtmlWin::OnSize(wxSizeEvent& event)
{
	int w, h;
	GetSize(&w, &h);

	if (m_webBrowser) {
		m_webBrowser->put_Height(h);
		m_webBrowser->put_Width(w);
	}

	RECT posRect;
	posRect.left = 0;
	posRect.top = 0;
	posRect.right = w;
	posRect.bottom = h;
	
	//::SetWindowPos((HWND)GetHWND(), m_oleObjectHWND, 0, 0, w, h, SWP_NOMOVE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
	if (m_oleInPlaceObject) {
		m_oleInPlaceObject->SetObjectRects(&posRect, &posRect);
	}

}

void IEHtmlWin::OnMove(wxMoveEvent& event)
{
	int x, y;
	GetPosition(&x, &y);

	if (m_webBrowser) {
		m_webBrowser->put_Left(0/*x*/);
		m_webBrowser->put_Top(0/*y*/);
	}
}

bool IEHtmlWin::Show(bool shown)
{
	bool ret;
	ret = wxWindow::Show(shown);
	//ret = ::ShowWindow(m_oleObjectHWND, (shown) ? SW_SHOW : SW_HIDE);
	if (m_webBrowser) {
		m_webBrowser->put_Visible(shown);
	}
	return ret;
}

void IEHtmlWin::OnSetFocus(wxFocusEvent& event)
{
	if (m_webBrowser) {
		m_webBrowser->put_Visible(TRUE);
		::PostMessage(m_oleObjectHWND, WM_SETFOCUS, (WPARAM)GetHWND(), NULL);
	}
	if (m_oleInPlaceObject) {
		//::PostMessage(m_oleObjectHWND, WM_SETFOCUS, (WPARAM)GetHWND(), NULL);
	}
}

void IEHtmlWin::OnPaint(wxPaintEvent& event)
{
	wxLogTrace(_T("repainting html win"));
	wxPaintDC dc(this);
	int w, h;
	GetSize(&w, &h);
	RECT posRect;
	posRect.left = 0;
	posRect.top = 0;
	posRect.right = w;
	posRect.bottom = h;

	// Draw only when control is windowless or deactivated
	if (m_viewObject)
	{
		::RedrawWindow(m_oleObjectHWND, NULL, NULL, RDW_INTERNALPAINT);
		{
			RECTL *prcBounds = (RECTL *) &posRect;
			m_viewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, 
				(HDC)dc.GetHDC(), prcBounds, NULL, NULL, 0);
		}
	}
	else
	{
		dc.SetBrush(*wxRED_BRUSH);
		dc.DrawRectangle(0, 0, w, h);
		dc.SetBrush(wxNullBrush);
	}
}

void IEHtmlWin::OnMouse(wxMouseEvent& event)
{
	wxLogTrace(_T("mouse event"));
	UINT msg = 0;
	WPARAM wParam = 0;
	LPARAM lParam = 0;
	LRESULT lResult = 0;

	if (event.m_metaDown) wParam |= MK_CONTROL;
	if (event.m_shiftDown) wParam |= MK_SHIFT;
	if (event.m_leftDown) wParam |= MK_LBUTTON;
	if (event.m_middleDown) wParam |= MK_MBUTTON;
	if (event.m_rightDown) wParam |= MK_RBUTTON;
	lParam = event.m_x << 16;
	lParam |= event.m_y;

	if (event.LeftDown()) {
		msg = WM_LBUTTONDOWN;
		SetFocus();
	}
	else if (event.LeftDClick()) msg = WM_LBUTTONDBLCLK;
	else if (event.LeftUp()) msg = WM_LBUTTONUP;
	else if (event.MiddleDown()) {
		msg = WM_MBUTTONDOWN;
		SetFocus();
	}
	else if (event.MiddleDClick()) msg = WM_MBUTTONDBLCLK;
	else if (event.MiddleUp()) msg = WM_MBUTTONUP;
	else if (event.RightDown()) {
		msg = WM_RBUTTONDOWN;
		SetFocus();
	}
	else if (event.RightDClick()) msg = WM_RBUTTONDBLCLK;
	else if (event.RightUp()) msg = WM_RBUTTONUP;
	else if (event.Moving() || event.Dragging()) msg = WM_MOUSEMOVE;

	wxString log;
	if (msg == 0) { wxLogTrace(_T("no message")); event.Skip(); return; }
	if (m_oleInPlaceObject == NULL) { wxLogTrace(_T("no oleInPlaceObject")); event.Skip(); return; }
	if (!::SendMessage(m_oleObjectHWND, msg, wParam, lParam)) { wxLogTrace(_T("msg not delivered")); event.Skip(); return; }
	wxLogTrace(_T("msg sent"));
}

void IEHtmlWin::OnChar(wxKeyEvent& event)
{
}

bool IEHtmlWin::OnStartURL(wxString& url)
{
	wxLogTrace(_T("loading url:"));
	wxLogTrace(url.c_str());

	m_currentUrl = url;
	if (m_specificallyOpened) {
		m_specificallyOpened = false;
		return true;
	}

	// should we open this url?
	// you should subclass IEHtmlWin and provide your own
	// implementation of when to load the specificed url

	return true;
}

void IEHtmlWin::OnProgressURL(long current, long maximum)
{
	//wxString log;
	//log.Printf(_T("url progress: %li/%li", current, maximum);
	//wxLogTrace(log.c_str());
}

void IEHtmlWin::OnFinishURL(wxString& url)
{
	wxLogTrace(_T("loaded url:"));
	wxLogTrace(url.c_str());

	m_currentUrl = url;
}

void IEHtmlWin::OpenURL(const wxString& url)
{
	VARIANTARG navFlag, targetFrame, postData, headers;
	navFlag.vt = VT_EMPTY; 
	//navFlag.vt = VT_I2;
	//navFlag.iVal = navNoReadFromCache;
	targetFrame.vt = VT_EMPTY;
	postData.vt = VT_EMPTY;
	headers.vt = VT_EMPTY;

	m_specificallyOpened = true;

	HRESULT hret;
	hret = m_webBrowser->Navigate((BSTR)(const wchar_t*)url.wc_str(wxConvLibc), 
		&navFlag, &targetFrame, &postData, &headers);	
}

HRESULT LoadWebBrowserFromStream(IWebBrowser* pWebBrowser, IStream* pStream)
{
	HRESULT hr;
	IDispatch* pHtmlDoc = NULL;
	IPersistStreamInit* pPersistStreamInit = NULL;

	// Retrieve the document object.
	hr = pWebBrowser->get_Document( &pHtmlDoc );
	if ( SUCCEEDED(hr) )
	{
		// Query for IPersistStreamInit.
		hr = pHtmlDoc->QueryInterface( IID_IPersistStreamInit,  (void**)&pPersistStreamInit );
		if ( SUCCEEDED(hr) )
		{
			// Initialize the document.
			hr = pPersistStreamInit->InitNew();
			if ( SUCCEEDED(hr) )
			{
				// Load the contents of the stream.
				hr = pPersistStreamInit->Load( pStream );
			}
			pPersistStreamInit->Release();
		}
	}
	return S_OK;
}

void IEHtmlWin::LoadData(const wxString& data) {
	HRESULT hr;
	IUnknown* pUnkBrowser = NULL;
	IUnknown* pUnkDisp = NULL;
	IStream* pStream = NULL;
	HGLOBAL hHTMLText;
	wxCharBuffer buf = data.mb_str(wxConvUTF8);
	const char *szHTMLText = buf.data();

	// Is this the DocumentComplete event for the top frame window?
	// Check COM identity: compare IUnknown interface pointers.
	hr = m_webBrowser->QueryInterface( IID_IUnknown,  (void**)&pUnkBrowser );
	if ( SUCCEEDED(hr) )
	{
		hr = m_viewObject->QueryInterface( IID_IUnknown,  (void**)&pUnkDisp );
		if ( SUCCEEDED(hr) )
		{
			if ( pUnkBrowser == pUnkDisp ){
				size_t buffer = strlen(szHTMLText);
				hHTMLText = GlobalAlloc(GPTR, buffer+1);

				if ( hHTMLText ) {
					StringCchCopyA((char*)hHTMLText, buffer+1, szHTMLText);

					hr = CreateStreamOnHGlobal(hHTMLText, false, &pStream);
					if ( SUCCEEDED(hr) )
					{
						// Call the helper function to load the browser from the stream.
						LoadWebBrowserFromStream(m_webBrowser, pStream);
						pStream->Release();
					}
					GlobalFree(hHTMLText);
				}
			}
			pUnkDisp->Release();
		}
		pUnkBrowser->Release();
	}
}

FrameSite::FrameSite(IEHtmlWin * win)
{
	m_cRef = 0;

	m_window = win;
	m_bSupportsWindowlessActivation = true;
	m_bInPlaceLocked = false;
	m_bUIActive = false;
	m_bInPlaceActive = false;
	m_bWindowless = false;

	m_nAmbientLocale = 0;
	m_clrAmbientForeColor = ::GetSysColor(COLOR_WINDOWTEXT);
	m_clrAmbientBackColor = ::GetSysColor(COLOR_WINDOW);
	m_bAmbientUserMode = true;
	m_bAmbientShowHatching = true;
	m_bAmbientShowGrabHandles = true;
	m_bAmbientAppearance = true;
 
	m_hDCBuffer = NULL;
	m_hWndParent = (HWND)m_window->GetHWND();

	m_IOleInPlaceFrame = new FS_IOleInPlaceFrame(this);
	m_IOleInPlaceSiteWindowless = new FS_IOleInPlaceSiteWindowless(this);
	m_IOleClientSite = new FS_IOleClientSite(this);
	m_IOleControlSite = new FS_IOleControlSite(this);
	m_IOleCommandTarget = new FS_IOleCommandTarget(this);
	m_IOleItemContainer = new FS_IOleItemContainer(this);
	//m_IOleItemContainer = NULL;
	m_IDispatch = new FS_IDispatch(this);
	m_DWebBrowserEvents2 = new FS_DWebBrowserEvents2(this);
	m_IAdviseSink2 = new FS_IAdviseSink2(this);
	m_IAdviseSinkEx = new FS_IAdviseSinkEx(this);
}

FrameSite::~FrameSite()
{
	delete m_IAdviseSinkEx;
	delete m_IAdviseSink2;
	delete m_DWebBrowserEvents2;
	delete m_IDispatch;
	delete m_IOleItemContainer;
	delete m_IOleCommandTarget;
	delete m_IOleControlSite;
	delete m_IOleClientSite;
	delete m_IOleInPlaceSiteWindowless;
	delete m_IOleInPlaceFrame;
}

//IUnknown

STDMETHODIMP FrameSite::QueryInterface(REFIID riid, void **ppv)             
{
	if (ppv == NULL) return E_INVALIDARG;
	*ppv = NULL;
	if (riid == IID_IUnknown) 
		*ppv = this;
	else if (riid == IID_IOleWindow ||
		riid == IID_IOleInPlaceUIWindow ||
		riid == IID_IOleInPlaceFrame) 
		*ppv = m_IOleInPlaceFrame;
	else if (riid == IID_IOleInPlaceSite ||
		riid == IID_IOleInPlaceSiteEx ||
		riid == IID_IOleInPlaceSiteWindowless) 
		*ppv = m_IOleInPlaceSiteWindowless;
	else if (riid == IID_IOleClientSite) 
		*ppv = m_IOleClientSite;
	else if (riid == IID_IOleControlSite) 
		*ppv = m_IOleControlSite;
	else if (riid == IID_IOleCommandTarget) 
		*ppv = m_IOleCommandTarget;
	else if (riid == IID_IOleItemContainer ||
		riid == IID_IOleContainer ||
		riid == IID_IParseDisplayName) 
		*ppv = m_IOleItemContainer;
	else if (riid == IID_IDispatch) 
		*ppv = m_IDispatch;
	else if (riid == DIID_DWebBrowserEvents2) 
		*ppv = m_DWebBrowserEvents2;
	else if (riid == IID_IAdviseSink || 
		riid == IID_IAdviseSink2) 
		*ppv = m_IAdviseSink2;
	else if (riid == IID_IAdviseSinkEx) 
		*ppv = m_IAdviseSinkEx;
	
	if (*ppv == NULL) return (HRESULT) E_NOINTERFACE;                                         
	AddRef();
	return S_OK;
}                                                                           
                                                                              
STDMETHODIMP_(ULONG) FrameSite::AddRef()                                    
{                                                                           
	return ++m_cRef;                                                          
}                                                                           
                                                                              
STDMETHODIMP_(ULONG) FrameSite::Release()                                   
{                                                                           
	if ( --m_cRef == 0 ) {                                                    
		delete this;                                                            
		return 0;                                                               
	}                                                                         
	else return m_cRef;                                                          
}

//IDispatch

HRESULT FS_IDispatch::GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, unsigned int cNames,
								 LCID lcid, DISPID * rgDispId)
{
	wxLogTrace(_T("IDispatch::GetIDsOfNames"));
	return E_NOTIMPL;
}

HRESULT FS_IDispatch::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo ** ppTInfo)
{
	wxLogTrace(_T("IDispatch::GetTypeInfo"));
	return E_NOTIMPL;
}

HRESULT FS_IDispatch::GetTypeInfoCount(unsigned int * pcTInfo)
{
	wxLogTrace(_T("IDispatch::GetTypeInfoCount"));
	return E_NOTIMPL;
}

HRESULT FS_IDispatch::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
						  WORD wFlags, DISPPARAMS * pDispParams,
						  VARIANT * pVarResult, EXCEPINFO * pExcepInfo,
						  unsigned int * puArgErr)
{
	wxLogTrace(_T("IDispatch::Invoke"));
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		if (pVarResult == NULL) return E_INVALIDARG;
		switch (dispIdMember)
		{
			case DISPID_AMBIENT_APPEARANCE:
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientAppearance;
				break;

			case DISPID_AMBIENT_FORECOLOR:
				pVarResult->vt = VT_I4;
				pVarResult->lVal = (long) m_fs->m_clrAmbientForeColor;
				break;

			case DISPID_AMBIENT_BACKCOLOR:
				pVarResult->vt = VT_I4;
				pVarResult->lVal = (long) m_fs->m_clrAmbientBackColor;
				break;

			case DISPID_AMBIENT_LOCALEID:
				pVarResult->vt = VT_I4;
				pVarResult->lVal = (long) m_fs->m_nAmbientLocale;
				break;

			case DISPID_AMBIENT_USERMODE:
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientUserMode;
				break;

			case DISPID_AMBIENT_SHOWGRABHANDLES:
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientShowGrabHandles;
				break;

			case DISPID_AMBIENT_SHOWHATCHING:
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientShowHatching;
				break;

			default:
				return DISP_E_MEMBERNOTFOUND;
		}
		return S_OK;
	}

	switch (dispIdMember)
	{
		case DISPID_BEFORENAVIGATE2:
		{
			VARIANT * vurl = pDispParams->rgvarg[5].pvarVal;
			wxString url;
			if (vurl->vt & VT_BYREF) url = *vurl->pbstrVal;
			else url = vurl->bstrVal;
			if (m_fs->m_window->OnStartURL(url)) {
				*pDispParams->rgvarg->pboolVal = VARIANT_FALSE;
			} else {
				*pDispParams->rgvarg->pboolVal = VARIANT_TRUE;
			}
			break;
		}
		case DISPID_PROGRESSCHANGE:
		{
			long current = pDispParams->rgvarg[1].lVal;
			long maximum = pDispParams->rgvarg[0].lVal;
			m_fs->m_window->OnProgressURL(current, maximum);
			break;
		}
		case DISPID_DOCUMENTCOMPLETE:
		{
			VARIANT * vurl = pDispParams->rgvarg[0].pvarVal;
			wxString url;
			if (vurl->vt & VT_BYREF) url = *vurl->pbstrVal;
			else url = vurl->bstrVal;
			m_fs->m_window->OnFinishURL(url);
			break;
		}
	}
	
	return S_OK;
}

//IOleWindow

HRESULT FS_IOleInPlaceFrame::GetWindow(HWND * phwnd)
{
	wxLogTrace(_T("IOleWindow::GetWindow"));
	if (phwnd == NULL) return E_INVALIDARG;
	(*phwnd) = m_fs->m_hWndParent;
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
	wxLogTrace(_T("IOleWindow::ContextSensitiveHelp"));
	return S_OK;
}

//IOleInPlaceUIWindow

HRESULT FS_IOleInPlaceFrame::GetBorder(LPRECT lprectBorder)
{
	wxLogTrace(_T("IOleInPlaceUIWindow::GetBorder"));
	if (lprectBorder == NULL) return E_INVALIDARG;
	return INPLACE_E_NOTOOLSPACE;
}

HRESULT FS_IOleInPlaceFrame::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
	wxLogTrace(_T("IOleInPlaceUIWindow::RequestBorderSpace"));
	if (pborderwidths == NULL) return E_INVALIDARG;
	return INPLACE_E_NOTOOLSPACE;
}

HRESULT FS_IOleInPlaceFrame::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
	wxLogTrace(_T("IOleInPlaceUIWindow::SetBorderSpace"));
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
	wxLogTrace(_T("IOleInPlaceUIWindow::SetActiveObject"));
	return S_OK;
}

//IOleInPlaceFrame

HRESULT FS_IOleInPlaceFrame::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	wxLogTrace(_T("IOleInPlaceFrame::InsertMenus"));
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	wxLogTrace(_T("IOleInPlaceFrame::SetMenu"));
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::RemoveMenus(HMENU hmenuShared)
{
	wxLogTrace(_T("IOleInPlaceFrame::RemoveMenus"));
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::SetStatusText(LPCOLESTR pszStatusText)
{
	wxLogTrace(_T("IOleInPlaceFrame::SetStatusText"));
	//((wxFrame*)wxGetApp()->GetTopWindow())->GetStatusBar()->SetStatusText(pszStatusText);
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::EnableModeless(BOOL fEnable)
{
	wxLogTrace(_T("IOleInPlaceFrame::EnableModeless"));
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
	wxLogTrace(_T("IOleInPlaceFrame::TranslateAccelerator"));
	// TODO: send an event with this id
	return E_NOTIMPL;
}

//IOleInPlaceSite

HRESULT FS_IOleInPlaceSiteWindowless::CanInPlaceActivate()
{
	wxLogTrace(_T("IOleInPlaceSite::CanInPlaceActivate"));
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceActivate()
{
	wxLogTrace(_T("IOleInPlaceSite::OnInPlaceActivate"));
	m_fs->m_bInPlaceActive = true;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnUIActivate()
{
	wxLogTrace(_T("IOleInPlaceSite::OnUIActivate"));
	m_fs->m_bUIActive = true;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetWindowContext(IOleInPlaceFrame **ppFrame,
									IOleInPlaceUIWindow **ppDoc,
									LPRECT lprcPosRect,
									LPRECT lprcClipRect,
									LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	wxLogTrace(_T("IOleInPlaceSite::GetWindowContext"));
	if (ppFrame == NULL || ppDoc == NULL || lprcPosRect == NULL ||
		lprcClipRect == NULL || lpFrameInfo == NULL)
	{
		if (ppFrame != NULL) (*ppFrame) = NULL;
		if (ppDoc != NULL) (*ppDoc) = NULL;
		return E_INVALIDARG;
	}

	(*ppDoc) = (*ppFrame) = m_fs->m_IOleInPlaceFrame;
	(*ppDoc)->AddRef();
	(*ppFrame)->AddRef();

	int w, h;
	m_fs->m_window->GetSize(&w, &h);
	lprcPosRect->left = lprcPosRect->top = 0;
	lprcPosRect->right = w;
	lprcPosRect->bottom = h;
	lprcClipRect->left = lprcClipRect->top = 0;
	lprcClipRect->right = w;
	lprcClipRect->bottom = h;

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = m_fs->m_hWndParent;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::Scroll(SIZE scrollExtent)
{
	wxLogTrace(_T("IOleInPlaceSite::Scroll"));
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnUIDeactivate(BOOL fUndoable)
{
	wxLogTrace(_T("IOleInPlaceSite::OnUIDeactivate"));
	m_fs->m_bUIActive = false;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceDeactivate()
{
	wxLogTrace(_T("IOleInPlaceSite::OnInPlaceDeactivate"));
	m_fs->m_bInPlaceActive = false;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::DiscardUndoState()
{
	wxLogTrace(_T("IOleInPlaceSite::DiscardUndoState"));
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::DeactivateAndUndo()
{
	wxLogTrace(_T("IOleInPlaceSite::DeactivateAndUndo"));
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnPosRectChange(LPCRECT lprcPosRect)
{
	wxLogTrace(_T("IOleInPlaceSite::OnPosRectChange"));
	return S_OK;
}

//IOleInPlaceSiteEx

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceActivateEx(BOOL * pfNoRedraw, DWORD dwFlags)
{
	wxLogTrace(_T("IOleInPlaceSiteEx::OnInPlaceActivateEx"));
	if (pfNoRedraw) (*pfNoRedraw) = FALSE;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceDeactivateEx(BOOL fNoRedraw)
{
	wxLogTrace(_T("IOleInPlaceSiteEx::OnInPlaceDeactivateEx"));
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::RequestUIActivate()
{
	wxLogTrace(_T("IOleInPlaceSiteEx::RequestUIActivate"));
	return S_FALSE;
}

//IOleInPlaceSiteWindowless

HRESULT FS_IOleInPlaceSiteWindowless::CanWindowlessActivate()
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::CanWindowlessActivate"));
	return (m_fs->m_bSupportsWindowlessActivation) ? S_OK : S_FALSE;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetCapture()
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::GetCapture"));
	return S_FALSE;
}

HRESULT FS_IOleInPlaceSiteWindowless::SetCapture(BOOL fCapture)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::SetCapture"));
	return S_FALSE;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetFocus()
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::GetFocus"));
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::SetFocus(BOOL fFocus)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::SetFocus"));
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetDC(LPCRECT pRect, DWORD grfFlags, HDC* phDC)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::GetDC"));
	if (phDC == NULL) return E_INVALIDARG;

	if (grfFlags & OLEDC_NODRAW) 
	{
		(*phDC) = m_fs->m_hDCBuffer;
		return S_OK;
	}

	if (m_fs->m_hDCBuffer != NULL) return E_UNEXPECTED;

	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::ReleaseDC(HDC hDC)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::ReleaseDC"));
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::InvalidateRect(LPCRECT pRect, BOOL fErase)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::InvalidateRect"));
	// Clip the rectangle against the object's size and invalidate it
	RECT rcI = { 0, 0, 0, 0 };
	RECT posRect;
	int w, h;
	m_fs->m_window->GetSize(&w, &h);
	posRect.left = 0;
	posRect.top = 0;
	posRect.right = w;
	posRect.bottom = h;
	if (pRect == NULL)
	{
		rcI = posRect;
	}
	else
	{
		IntersectRect(&rcI, &posRect, pRect);
	}
	::InvalidateRect(m_fs->m_hWndParent, &rcI, fErase);
 
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::InvalidateRgn(HRGN, BOOL)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::InvalidateRgn"));
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::ScrollRect(INT, INT, LPCRECT, LPCRECT)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::ScrollRect"));
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::AdjustRect(LPRECT)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::AdjustRect"));
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnDefWindowMessage(UINT, WPARAM, LPARAM, LRESULT*)
{
	wxLogTrace(_T("IOleInPlaceSiteWindowless::OnDefWindowMessage"));
	return E_NOTIMPL;
}

//IOleClientSite

HRESULT FS_IOleClientSite::SaveObject()
{
	wxLogTrace(_T("IOleClientSite::SaveObject"));
	return S_OK;
}

HRESULT FS_IOleClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
							  IMoniker ** ppmk)
{
	wxLogTrace(_T("IOleClientSite::GetMoniker"));
	return E_NOTIMPL;
}

HRESULT FS_IOleClientSite::GetContainer(LPOLECONTAINER * ppContainer)
{
	wxLogTrace(_T("IOleClientSite::GetContainer"));
	if (ppContainer == NULL) return E_INVALIDARG;
	this->QueryInterface(IID_IOleContainer, (void**)(ppContainer));
	return S_OK;
}

HRESULT FS_IOleClientSite::ShowObject()
{
	wxLogTrace(_T("IOleClientSite::ShowObject"));
	return S_OK;
}

HRESULT FS_IOleClientSite::OnShowWindow(BOOL fShow)
{
	wxLogTrace(_T("IOleClientSite::OnShowWindow"));
	return S_OK;
}

HRESULT FS_IOleClientSite::RequestNewObjectLayout()
{
	wxLogTrace(_T("IOleClientSite::RequestNewObjectLayout"));
	return E_NOTIMPL;
}

//IParseDisplayName

HRESULT FS_IOleItemContainer::ParseDisplayName(IBindCtx *pbc, LPOLESTR pszDisplayName,
									ULONG *pchEaten, IMoniker **ppmkOut)
{
	wxLogTrace(_T("IParseDisplayName::ParseDisplayName"));
	return E_NOTIMPL;
}

//IOleContainer

HRESULT FS_IOleItemContainer::EnumObjects(DWORD grfFlags, IEnumUnknown **ppenum)
{
	wxLogTrace(_T("IOleContainer::EnumObjects"));
	return E_NOTIMPL;
}

HRESULT FS_IOleItemContainer::LockContainer(BOOL fLock)
{
	wxLogTrace(_T("IOleContainer::LockContainer"));
	// TODO
	return S_OK;
}

//IOleItemContainer

HRESULT FS_IOleItemContainer::GetObjectW(LPOLESTR pszItem, DWORD dwSpeedNeeded, 
							 IBindCtx * pbc, REFIID riid, void ** ppvObject)
{
	wxLogTrace(_T("IOleItemContainer::GetObject"));
	if (pszItem == NULL) return E_INVALIDARG;
	if (ppvObject == NULL) return E_INVALIDARG;

	*ppvObject = NULL;
	return MK_E_NOOBJECT;
}

HRESULT FS_IOleItemContainer::GetObjectStorage(LPOLESTR pszItem, IBindCtx * pbc, 
									REFIID riid, void ** ppvStorage)
{
	wxLogTrace(_T("IOleItemContainer::GetObjectStorage"));
	if (pszItem == NULL) return E_INVALIDARG;
	if (ppvStorage == NULL) return E_INVALIDARG;

	*ppvStorage = NULL;
	return MK_E_NOOBJECT;
}

HRESULT FS_IOleItemContainer::IsRunning(LPOLESTR pszItem)
{
	wxLogTrace(_T("IOleItemContainer::IsRunning"));
	if (pszItem == NULL) return E_INVALIDARG;

	return MK_E_NOOBJECT;
}

//IOleControlSite

HRESULT FS_IOleControlSite::OnControlInfoChanged()
{
	wxLogTrace(_T("IOleControlSite::OnControlInfoChanged"));
	return S_OK;
}

HRESULT FS_IOleControlSite::LockInPlaceActive(BOOL fLock)
{
	wxLogTrace(_T("IOleControlSite::LockInPlaceActive"));
	m_fs->m_bInPlaceLocked = (fLock) ? true : false;
	return S_OK;
}

HRESULT FS_IOleControlSite::GetExtendedControl(IDispatch ** ppDisp)
{
	wxLogTrace(_T("IOleControlSite::GetExtendedControl"));
	return E_NOTIMPL;
}

HRESULT FS_IOleControlSite::TransformCoords(POINTL * pPtlHimetric, POINTF * pPtfContainer, DWORD dwFlags)
{
	wxLogTrace(_T("IOleControlSite::TransformCoords"));
	HRESULT hr = S_OK;

	if (pPtlHimetric == NULL)
	{
		return E_INVALIDARG;
	}
	if (pPtfContainer == NULL)
	{
		return E_INVALIDARG;
	}

	HDC hdc = ::GetDC(m_fs->m_hWndParent);
	::SetMapMode(hdc, MM_HIMETRIC);
	POINT rgptConvert[2];
	rgptConvert[0].x = 0;
	rgptConvert[0].y = 0;

	if (dwFlags & XFORMCOORDS_HIMETRICTOCONTAINER)
	{
		rgptConvert[1].x = pPtlHimetric->x;
		rgptConvert[1].y = pPtlHimetric->y;
		::LPtoDP(hdc, rgptConvert, 2);
		if (dwFlags & XFORMCOORDS_SIZE)
		{
			pPtfContainer->x = (float)(rgptConvert[1].x - rgptConvert[0].x);
			pPtfContainer->y = (float)(rgptConvert[0].y - rgptConvert[1].y);
		}
		else if (dwFlags & XFORMCOORDS_POSITION)
		{
			pPtfContainer->x = (float)rgptConvert[1].x;
			pPtfContainer->y = (float)rgptConvert[1].y;
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}
	else if (dwFlags & XFORMCOORDS_CONTAINERTOHIMETRIC)
	{
		rgptConvert[1].x = (int)(pPtfContainer->x);
		rgptConvert[1].y = (int)(pPtfContainer->y);
		::DPtoLP(hdc, rgptConvert, 2);
		if (dwFlags & XFORMCOORDS_SIZE)
		{
			pPtlHimetric->x = rgptConvert[1].x - rgptConvert[0].x;
			pPtlHimetric->y = rgptConvert[0].y - rgptConvert[1].y;
		}
		else if (dwFlags & XFORMCOORDS_POSITION)
		{
			pPtlHimetric->x = rgptConvert[1].x;
			pPtlHimetric->y = rgptConvert[1].y;
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	::ReleaseDC(m_fs->m_hWndParent, hdc); 
	return hr;
}

HRESULT FS_IOleControlSite::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
	wxLogTrace(_T("IOleControlSite::TranslateAccelerator"));
	// TODO: send an event with this id
	return E_NOTIMPL;
}

HRESULT FS_IOleControlSite::OnFocus(BOOL fGotFocus)
{
	wxLogTrace(_T("IOleControlSite::OnFocus"));
	return S_OK;
}

HRESULT FS_IOleControlSite::ShowPropertyFrame()
{
	wxLogTrace(_T("IOleControlSite::ShowPropertyFrame"));
	return E_NOTIMPL;
}

//IOleCommandTarget

HRESULT FS_IOleCommandTarget::QueryStatus(const GUID * pguidCmdGroup, ULONG cCmds, 
							   OLECMD * prgCmds, OLECMDTEXT * pCmdTet)
{
	wxLogTrace(_T("IOleCommandTarget::QueryStatus"));
	if (prgCmds == NULL) return E_INVALIDARG;
	bool bCmdGroupFound = false;

	for (ULONG nCmd = 0; nCmd < cCmds; nCmd++)
	{
		// unsupported by default
		prgCmds[nCmd].cmdf = 0;

		// TODO
	}

	if (!bCmdGroupFound) { OLECMDERR_E_UNKNOWNGROUP; }
	return S_OK;
}

HRESULT FS_IOleCommandTarget::Exec(const GUID * pguidCmdGroup, DWORD nCmdID, 
						DWORD nCmdExecOpt, VARIANTARG * pVaIn, 
						VARIANTARG * pVaOut)
{
	wxLogTrace(_T("IOleCommandTarget::Exec"));
	bool bCmdGroupFound = false;

	if (!bCmdGroupFound) { OLECMDERR_E_UNKNOWNGROUP; }
	return OLECMDERR_E_NOTSUPPORTED;
}

//IAdviseSink

void STDMETHODCALLTYPE FS_IAdviseSink2::OnDataChange(FORMATETC * pFormatEtc, STGMEDIUM * pgStgMed)
{
	wxLogTrace(_T("IAdviseSink::OnDataChange"));
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnViewChange(DWORD dwAspect, LONG lIndex)
{
	wxLogTrace(_T("IAdviseSink::OnViewChange"));
	// redraw the control
	m_fs->m_IOleInPlaceSiteWindowless->InvalidateRect(NULL, FALSE);
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnRename(IMoniker * pmk)
{
	wxLogTrace(_T("IAdviseSink::OnRename"));
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnSave()
{
	wxLogTrace(_T("IAdviseSink::OnSave"));
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnClose()
{
	wxLogTrace(_T("IAdviseSink::OnClose"));
}

//IAdviseSink2

void STDMETHODCALLTYPE FS_IAdviseSink2::OnLinkSrcChange(IMoniker * pmk)
{
	wxLogTrace(_T("IAdviseSink2::OnLinkSrcChange"));
}

//IAdviseSinkEx

void STDMETHODCALLTYPE FS_IAdviseSinkEx::OnViewStatusChange(DWORD dwViewStatus)
{
	wxLogTrace(_T("IAdviseSinkEx::OnViewStatusChange"));
}

