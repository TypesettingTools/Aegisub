#ifndef _IEHTMLWIN_H_
#define _IEHTMLWIN_H_

#include <wx/setup.h>
#include <wx/wx.h>
#include <exdisp.h>

class IEHtmlWin : public wxWindow {
public:
	IEHtmlWin(wxWindow * parent, wxWindowID id = -1);
	virtual ~IEHtmlWin();

	void OpenURL(const wxString&);
	void LoadData(const wxString&);

	bool Show(bool shown = true);

	void CreateBrowser();

	void OnPaint(wxPaintEvent&);
	void OnSize(wxSizeEvent&);
	void OnMove(wxMoveEvent&);
	void OnSetFocus(wxFocusEvent&);
	void OnMouse(wxMouseEvent&);
	void OnChar(wxKeyEvent&);

	virtual bool OnStartURL(wxString& url);
	virtual void OnFinishURL(wxString& url);
	virtual void OnProgressURL(long current, long maximum);

	wxString& GetOpenedPage() { return m_currentUrl; }

	DECLARE_EVENT_TABLE();

protected:

	wxString m_currentUrl;
	bool m_specificallyOpened;

	IWebBrowser2 * m_webBrowser;
	IOleObject * m_oleObject;
	IOleInPlaceObject * m_oleInPlaceObject;
	IViewObject * m_viewObject;
	IConnectionPoint * m_connectionPoint;
	HWND m_oleObjectHWND;

	DWORD m_adviseCookie;
};

#endif /* _IEHTMLWIN_H_ */
