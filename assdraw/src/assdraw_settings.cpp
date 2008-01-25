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


#include "assdraw.hpp"

void ASSDrawFrame::InitializeDefaultSettings()
{
	colors.canvas_bg = wxColour(0xFF, 0xFF, 0xFF);
	colors.canvas_shape_normal = wxColour(0x0, 0x0, 0xFF, 0x99);
	colors.canvas_shape_preview = wxColour(0x0, 0x0, 0xFF);
	colors.canvas_shape_outline = wxColour(0x0, 0x0, 0x0);
	colors.canvas_shape_guideline = wxColour(0x66, 0x66, 0x66);
	colors.canvas_shape_mainpoint = wxColour(0xFF, 0x0, 0x0, 0xCC);
	colors.canvas_shape_controlpoint = wxColour(0x0, 0xFF, 0x0, 0xCC);
	colors.canvas_shape_selectpoint = wxColour(0x0, 0x0, 0xCC);
	colors.library_shape = wxColour(0x0, 0x66, 0x99);
	colors.library_libarea = wxColour(0xFF, 0xFF, 0x99);
	colors.origin = wxColour(0xFF, 0x0, 0x0);
	colors.ruler_h = wxColour(0x0, 0x0, 0x66);
	colors.ruler_v = wxColour(0x66, 0x0, 0x0);

	alphas.canvas_shape_normal = 128;
	alphas.canvas_shape_preview = 255;
	alphas.canvas_shape_outline = 255;
	alphas.canvas_shape_guideline = 255;
	alphas.canvas_shape_mainpoint = 128;
	alphas.canvas_shape_controlpoint = 128;
	alphas.canvas_shape_selectpoint = 255;

	sizes.origincross = 2;

	behaviors.capitalizecmds = false;
	behaviors.autoaskimgopac = true;
	behaviors.parse_spc = false;
	behaviors.nosplashscreen = false;
	behaviors.confirmquit = true;
}

void ASSDrawFrame::ApplySettings()
{
	wxColourSetAlpha(colors.canvas_shape_normal, alphas.canvas_shape_normal);
	wxColourSetAlpha(colors.canvas_shape_preview, alphas.canvas_shape_preview);
	wxColourSetAlpha(colors.canvas_shape_outline, alphas.canvas_shape_outline);
	wxColourSetAlpha(colors.canvas_shape_guideline, alphas.canvas_shape_guideline);
	wxColourSetAlpha(colors.canvas_shape_mainpoint, alphas.canvas_shape_mainpoint);
	wxColourSetAlpha(colors.canvas_shape_controlpoint, alphas.canvas_shape_controlpoint);
	wxColourSetAlpha(colors.canvas_shape_selectpoint, alphas.canvas_shape_selectpoint);

	wxColourToAggRGBA(colors.canvas_shape_normal, m_canvas->rgba_shape_normal);
	wxColourToAggRGBA(colors.canvas_shape_preview, m_canvas->rgba_shape);
	wxColourToAggRGBA(colors.canvas_shape_outline, m_canvas->rgba_outline);
	wxColourToAggRGBA(colors.canvas_shape_guideline, m_canvas->rgba_guideline);
	wxColourToAggRGBA(colors.canvas_shape_mainpoint, m_canvas->rgba_mainpoint);
	wxColourToAggRGBA(colors.canvas_shape_controlpoint, m_canvas->rgba_controlpoint);
	wxColourToAggRGBA(colors.canvas_shape_selectpoint, m_canvas->rgba_selectpoint);
	wxColourToAggRGBA(colors.origin, m_canvas->rgba_origin);
	wxColourToAggRGBA(colors.ruler_h, m_canvas->rgba_ruler_h);
	wxColourToAggRGBA(colors.ruler_v, m_canvas->rgba_ruler_v);

	m_canvas->color_bg.r = colors.canvas_bg.Red();
	m_canvas->color_bg.g = colors.canvas_bg.Green();
	m_canvas->color_bg.b = colors.canvas_bg.Blue();
	m_canvas->color_bg.a = colors.canvas_bg.Alpha();
	m_canvas->PrepareBackgroundBitmap(-1.0);
	m_canvas->Refresh();

	shapelib->libarea->SetBackgroundColour(colors.library_libarea);
	typedef std::vector< ASSDrawShapePreview *> PrevVec;
	PrevVec shapes = shapelib->GetShapePreviews();
	int n = shapes.size();
	for (int i = 0; i < n; i++)
		wxColourToAggRGBA(colors.library_shape, shapes[i]->rgba_shape);
	shapelib->libarea->Refresh();
	
	m_canvas->SetDrawCmdSet(behaviors.parse_spc? _T("m n l b s p c _"):_T("m n l b _"));

	UpdateASSCommandStringToSrcTxtCtrl(m_canvas->GenerateASS());
}

void ASSDrawFrame::wxColourToAggRGBA(wxColour &colour, agg::rgba &rgba)
{
	rgba.r = (double) colour.Red() / 255.0;
	rgba.g = (double) colour.Green() / 255.0;
	rgba.b = (double) colour.Blue() / 255.0;
	rgba.a = (double) colour.Alpha() / 255.0;
}

void ASSDrawFrame::wxColourSetAlpha(wxColour &colour, long alpha)
{
	colour.Set(colour.Red(), colour.Green(), colour.Blue(), alpha);
}

void ASSDrawFrame::OnSettingsChanged(wxCommandEvent& event)
{
	ApplySettings();
}

void ASSDrawFrame::LoadSettings()
{
	#define CFGREADCOLOR(color) if (config->Read(wxString(#color,wxConvUTF8), &tmpstr)) color.Set(tmpstr);
	#define CFGREAD(var) config->Read(wxString(#var,wxConvUTF8), &var);
	config->SetPath(_T("settings"));
	wxString tmpstr;
	CFGREADCOLOR(colors.canvas_bg)
	CFGREADCOLOR(colors.canvas_shape_normal)
	CFGREADCOLOR(colors.canvas_shape_preview)
	CFGREADCOLOR(colors.canvas_shape_controlpoint)
	CFGREADCOLOR(colors.canvas_shape_guideline)
	CFGREADCOLOR(colors.canvas_shape_mainpoint)
	CFGREADCOLOR(colors.canvas_shape_outline)
	CFGREADCOLOR(colors.canvas_shape_selectpoint)
	CFGREADCOLOR(colors.library_libarea)
	CFGREADCOLOR(colors.library_shape)
	CFGREADCOLOR(colors.origin)
	CFGREADCOLOR(colors.ruler_h)
	CFGREADCOLOR(colors.ruler_v)
	CFGREAD(alphas.canvas_shape_normal)
	CFGREAD(alphas.canvas_shape_preview)
	CFGREAD(alphas.canvas_shape_controlpoint)
	CFGREAD(alphas.canvas_shape_guideline)
	CFGREAD(alphas.canvas_shape_mainpoint)
	CFGREAD(alphas.canvas_shape_outline)
	CFGREAD(alphas.canvas_shape_selectpoint)
	CFGREAD(sizes.origincross)
	CFGREAD(behaviors.autoaskimgopac)
	CFGREAD(behaviors.capitalizecmds)
	CFGREAD(behaviors.parse_spc)
	CFGREAD(behaviors.nosplashscreen)
	CFGREAD(behaviors.confirmquit)
	config->SetPath(_T(".."));
}

void ASSDrawFrame::SaveSettings()
{
	#define CFGWRITE(var) config->Write(wxString(#var,wxConvUTF8), var);
	#define CFGWRITECOLOR(color) config->Write(wxString(#color,wxConvUTF8), color.GetAsString(wxC2S_HTML_SYNTAX));
	config->SetPath(_T("settings"));
	CFGWRITECOLOR(colors.canvas_bg)
	CFGWRITECOLOR(colors.canvas_shape_normal)
	CFGWRITECOLOR(colors.canvas_shape_preview)
	CFGWRITECOLOR(colors.canvas_shape_controlpoint)
	CFGWRITECOLOR(colors.canvas_shape_guideline)
	CFGWRITECOLOR(colors.canvas_shape_mainpoint)
	CFGWRITECOLOR(colors.canvas_shape_outline)
	CFGWRITECOLOR(colors.canvas_shape_selectpoint)
	CFGWRITECOLOR(colors.library_libarea)
	CFGWRITECOLOR(colors.library_shape)
	CFGWRITECOLOR(colors.origin)
	CFGWRITECOLOR(colors.ruler_h)
	CFGWRITECOLOR(colors.ruler_v)
	CFGWRITE(alphas.canvas_shape_normal)
	CFGWRITE(alphas.canvas_shape_preview)
	CFGWRITE(alphas.canvas_shape_controlpoint)
	CFGWRITE(alphas.canvas_shape_guideline)
	CFGWRITE(alphas.canvas_shape_mainpoint)
	CFGWRITE(alphas.canvas_shape_outline)
	CFGWRITE(alphas.canvas_shape_selectpoint)
	CFGWRITE(sizes.origincross)
	CFGWRITE(behaviors.autoaskimgopac)
	CFGWRITE(behaviors.capitalizecmds)
	CFGWRITE(behaviors.parse_spc)
	CFGWRITE(behaviors.nosplashscreen)
	CFGWRITE(behaviors.confirmquit)
	config->SetPath(_T(".."));
}
