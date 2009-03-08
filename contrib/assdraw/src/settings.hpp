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
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

class ASSDrawFrame;

DECLARE_EVENT_TYPE(wxEVT_SETTINGS_CHANGED, -1)

class ASSDrawSettingsDialog : public wxPanel
{
public:

	ASSDrawFrame* m_frame;

	ASSDrawSettingsDialog( wxWindow *parent, ASSDrawFrame *frame, int id = wxID_ANY );
	virtual ~ASSDrawSettingsDialog();

	virtual void Init();
	virtual void OnSettingsApplyButtonClicked(wxCommandEvent &event);
	virtual void OnSettingsRevertButtonClicked(wxCommandEvent &event);
	virtual void RefreshSettingsDisplay();

	wxPGId colors_canvas_bg_pgid;
	wxPGId colors_canvas_shape_normal_pgid;
	wxPGId colors_canvas_shape_preview_pgid;
	wxPGId colors_canvas_shape_outline_pgid;
	wxPGId colors_canvas_shape_guideline_pgid;
	wxPGId colors_canvas_shape_mainpoint_pgid;
	wxPGId colors_canvas_shape_controlpoint_pgid;
	wxPGId colors_canvas_shape_selectpoint_pgid;
	wxPGId colors_library_shape_pgid;
	wxPGId colors_library_libarea_pgid;
	wxPGId colors_origin_pgid;
	wxPGId colors_ruler_h_pgid;
	wxPGId colors_ruler_v_pgid;
	wxPGId alphas_canvas_shape_normal_pgid;
	wxPGId alphas_canvas_shape_preview_pgid;
	wxPGId alphas_canvas_shape_outline_pgid;
	wxPGId alphas_canvas_shape_guideline_pgid;
	wxPGId alphas_canvas_shape_mainpoint_pgid;
	wxPGId alphas_canvas_shape_controlpoint_pgid;
	wxPGId alphas_canvas_shape_selectpoint_pgid;
	wxPGId sizes_origincross_pgid;
	wxPGId behaviors_capitalizecmds_pgid;
	wxPGId behaviors_autoaskimgopac_pgid;
	wxPGId behaviors_parse_spc_pgid;
	wxPGId behaviors_nosplashscreen_pgid;
	wxPGId behaviors_confirmquit_pgid;

	wxPropertyGrid *propgrid;
	//DECLARE_EVENT_TABLE()

};

class wxLongPropertyValidator : public wxValidator
{
public:

    wxLongPropertyValidator( const int min, const int max )
        : wxValidator()
    {
		m_min = min, m_max = max;
    }

    virtual wxObject* Clone() const
    {
        return new wxLongPropertyValidator( m_min, m_max );
    }

    virtual bool Validate(wxWindow* WXUNUSED(parent))
	{
	    wxTextCtrl* tc = wxDynamicCast(GetWindow(), wxTextCtrl);
	    wxCHECK_MSG(tc, true, wxT("validator window must be wxTextCtrl"));

	    wxString val = tc->GetValue();

		long valint = 0;
		val.ToLong(&valint);

		if (valint < m_min) valint = m_min;
		if (valint > m_max) valint = m_max;

		tc->SetValue(wxString::Format(_T("%d"), valint));

	    return true;
	}

private:
    int m_min, m_max;
};
