/*
* Copyright (c) 2007, ai-chan
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the ASSDraw3 Team nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY AI-CHAN ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL AI-CHAN BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "_common.hpp"
#include <wx/html/htmlwin.h>
#include <wx/timer.h>


class ASSDrawFrame;

class ASSDrawSrcTxtCtrl : public wxTextCtrl
{
public:
	ASSDrawSrcTxtCtrl(wxWindow *parent, ASSDrawFrame *frame);
	virtual void CustomOnChar(wxKeyEvent &event);
	virtual void CustomOnText(wxCommandEvent &event);

protected:
	ASSDrawFrame *m_frame;
	DECLARE_EVENT_TABLE()
};

struct EightDouble
{
	double f1, f2, f3, f4, f5, f6, f7, f8;
};

class ASSDrawTransformDlg : public wxDialog
{

public:
	ASSDrawTransformDlg(ASSDrawFrame* parent);
	void OnTemplatesCombo(wxCommandEvent &event);
	void EndModal(int retCode);

	ASSDrawFrame* m_frame;
	wxComboBox* combo_templates;
	wxTextCtrl* txtctrl_m11;
	wxTextCtrl* txtctrl_m12;
	wxTextCtrl* txtctrl_m21;
	wxTextCtrl* txtctrl_m22;
	wxTextCtrl* txtctrl_mx;
	wxTextCtrl* txtctrl_my;
	wxTextCtrl* txtctrl_nx;
	wxTextCtrl* txtctrl_ny;
	EightDouble xformvals;

	static wxString combo_templatesStrings[];
	static int combo_templatesCount;
	static EightDouble combo_templatesValues[];

	DECLARE_EVENT_TABLE()

};

class ASSDrawAboutDlg : public wxDialog
{
public:
	ASSDrawAboutDlg(ASSDrawFrame *parent, unsigned timeout = 0);
	virtual ~ASSDrawAboutDlg();
	virtual void OnURL(wxHtmlLinkEvent &event);
	virtual int ShowModal();
	virtual void OnTimeout(wxTimerEvent& event);
	virtual void OnMouseEnterWindow(wxMouseEvent& event);

protected:
	wxTimer timer;
	wxHtmlWindow *htmlwin;
	const unsigned time_out;
};

class BigStaticBitmapCtrl : public wxPanel
{
public:
	BigStaticBitmapCtrl(wxWindow *parent, wxBitmap bmap, wxColour bgcol, wxWindow *todrag = NULL);
	virtual ~BigStaticBitmapCtrl();
	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnMouseLeftDown(wxMouseEvent &event);
	virtual void OnMouseLeftUp(wxMouseEvent &event);
	virtual void OnMouseMove(wxMouseEvent &event);

protected:
	wxBitmap bitmap;
	wxBrush bgbrush;
	wxWindow *window_to_drag;
	wxPoint dragpoint, wndpos;

	DECLARE_EVENT_TABLE()
};
