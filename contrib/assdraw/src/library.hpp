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

#include "engine.hpp"
#include <wx/scrolwin.h>

class ASSDrawFrame;
class ASSDrawShapeLibrary;

enum LIBLAYOUT { VERTICAL, HORIZONTAL };

enum {
	MENU_RANGE_START = 450,
	MENU_LOAD,
	MENU_COPYCLIPBOARD,
	MENU_SAVECANVAS,
	MENU_DELETE,
	MENU_RANGE_END,
	TOOL_SAVE,
	TOOL_CHECK,
	TOOL_UNCHECK,
	TOOL_DELETE
};

class ASSDrawShapePreview : public ASSDrawEngine
{
protected:
	ASSDrawShapePreview( wxWindow *parent, ASSDrawShapeLibrary *shapelib, wxString initialcmds = _T(""));
	virtual void OnSize(wxSizeEvent& event);

	ASSDrawShapeLibrary *shapelib;
	wxCheckBox *cb;
	DECLARE_EVENT_TABLE()
	friend class ASSDrawShapeLibrary;
};

class ASSDrawShapeLibrary : public wxScrolledWindow
{
public:
	ASSDrawShapeLibrary( wxWindow *parent, ASSDrawFrame *frame );
	virtual ASSDrawShapePreview* AddShapePreview(wxString cmds, bool addtotop = false);
	virtual void OnSize(wxSizeEvent& event);
	virtual void OnMouseLeftDClick(wxMouseEvent &event);
	virtual void OnMouseRightClick(wxMouseEvent &event);
	virtual void OnPopupMenuClicked(wxCommandEvent &event);
	virtual void SaveShapeFromCanvas(wxCommandEvent& WXUNUSED(event));
	virtual void CheckUncheckAllPreviews(wxCommandEvent &event);
	virtual void DeleteChecked(wxCommandEvent& WXUNUSED(event));
	virtual void UpdatePreviewDisplays();
	virtual std::vector< ASSDrawShapePreview *> GetShapePreviews();
	virtual void LoadToCanvas(ASSDrawShapePreview *preview);
	
	wxScrolledWindow* libarea;
	wxFlexGridSizer *libsizer;
	wxSizer* sizer;
	LIBLAYOUT layout;
protected:
	ASSDrawFrame *m_frame;
	ASSDrawShapePreview *activepreview;

	DECLARE_EVENT_TABLE()
};
