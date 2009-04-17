// Copyright (c) 2005, 2006, 2007, Niels Martin Hansen
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
// Contact: mailto:jiifurusu@gmail.com
//


#include "config.h"

#ifdef WITH_AUTOMATION
#ifdef WITH_AUTO3

#include <wx/spinctrl.h>
#include <wx/tokenzr.h>
#include "auto4_auto3.h"
#include "auto4_auto3_factory.h"
#include "libauto3/auto3.h"
#include "options.h"
#include "string_codec.h"
#include "vfr.h"
#include "ass_override.h"

namespace Automation4 {

	// Auto3ProgressSink

	void Auto3ProgressSink::SetStatus(void *cbdata, const char *msg)
	{
		Auto3ProgressSink *ps = (Auto3ProgressSink*)cbdata;
		ps->SetTask(wxString(msg, wxConvUTF8));
	}

	void Auto3ProgressSink::OutputDebug(void *cbdata, const char *msg)
	{
		Auto3ProgressSink *ps = (Auto3ProgressSink*)cbdata;
		ps->AddDebugOutput(wxString(msg, wxConvUTF8));
		ps->AddDebugOutput(_T("\n"));
	}

	void Auto3ProgressSink::ReportProgress(void *cbdata, float progress)
	{
		Auto3ProgressSink *ps = (Auto3ProgressSink*)cbdata;
		ps->SetProgress(progress);
	}

	Auto3ProgressSink::Auto3ProgressSink(Auto3Interpreter *_script, wxWindow *parent)
		: ProgressSink(parent)
		, script(_script)
	{
		script->cb.logdata = this;
		script->cb.log_error = OutputDebug;
		script->cb.log_message = OutputDebug;
		script->cb.set_progress = ReportProgress;
		script->cb.set_status = SetStatus;
	}

	Auto3ProgressSink::~Auto3ProgressSink()
	{
		script->cb.logdata = 0;
		script->cb.log_error = 0;
		script->cb.log_message = 0;
		script->cb.set_progress = 0;
		script->cb.set_status = 0;
	}


	// Auto3ConfigDialog

	wxWindow* Auto3ConfigDialog::CreateWindow(wxWindow *parent)
	{
		if (options->name == 0)
			return 0;

		wxPanel *res = new wxPanel(parent, -1);

		wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 5, 5);

		for (Auto3ConfigOption *opt = options; opt->name; opt++) {
			if (opt->kind == COK_INVALID)
				continue;

			Control control;
			control.option = opt;

			switch (opt->kind) {
				case COK_LABEL:
					control.control = new wxStaticText(res, -1, wxString(opt->label, wxConvUTF8));
					break;

				case COK_TEXT:
					control.control = new wxTextCtrl(res, -1, wxString(opt->value.stringval, wxConvUTF8));
					break;

				case COK_INT:
					control.control = new wxSpinCtrl(res, -1);
					if (opt->min.valid && opt->max.valid) {
						((wxSpinCtrl*)control.control)->SetRange(opt->min.intval, opt->max.intval);
					} else if (opt->min.valid) {
						((wxSpinCtrl*)control.control)->SetRange(opt->min.intval, 0x7fff);
					} else if (opt->max.valid) {
						((wxSpinCtrl*)control.control)->SetRange(-0x7fff, opt->max.intval);
					} else {
						((wxSpinCtrl*)control.control)->SetRange(-0x7fff, 0x7fff);
					}
					((wxSpinCtrl*)control.control)->SetValue(opt->value.intval);
					break;

				case COK_FLOAT:
					control.control = new wxTextCtrl(res, -1, wxString::Format(_T("%f"), opt->value.floatval));
					break;

				case COK_BOOL:
					control.control = new wxCheckBox(res, -1, wxString(opt->label, wxConvUTF8));
					((wxCheckBox*)control.control)->SetValue(!!opt->value.intval);
					break;

				case COK_COLOUR:
					// *FIXME* what to do here?
					// just put a stupid edit box for now
					control.control = new wxTextCtrl(res, -1, wxString(opt->value.stringval, wxConvUTF8));
					break;

				case COK_STYLE:
					control.control = new wxChoice(res, -1, wxDefaultPosition, wxDefaultSize, AssFile::top->GetStyles());
					((wxChoice*)control.control)->Insert(_T(""), 0);
					break;

				case COK_INVALID:
					break;
			}

			if (opt->kind != COK_LABEL && opt->kind != COK_BOOL) {
				control.label = new wxStaticText(res, -1, wxString(opt->label, wxConvUTF8));
				sizer->Add(control.label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
			} else {
				control.label = 0;
				sizer->AddSpacer(0);
			}
			control.control->SetToolTip(wxString(opt->hint, wxConvUTF8));
			sizer->Add(control.control, 1, wxEXPAND);

			controls.push_back(control);
		}

		res->SetSizerAndFit(sizer);

		return res;
	}

	Auto3ConfigDialog::Auto3ConfigDialog(Auto3Interpreter *script)
	{
		options = script->config;
	}

	Auto3ConfigDialog::~Auto3ConfigDialog()
	{
		// Nothing to do here
	}

	void Auto3ConfigDialog::ReadBack()
	{
		for (std::vector<Control>::iterator ctl = controls.begin(); ctl != controls.end(); ctl++) {
			switch (ctl->option->kind) {
				case COK_TEXT:
					Auto3Free(ctl->option->value.stringval);
					ctl->option->value.stringval = Auto3Strdup(((wxTextCtrl*)ctl->control)->GetValue().mb_str(wxConvUTF8));
					break;

				case COK_INT:
					ctl->option->value.intval = ((wxSpinCtrl*)ctl->control)->GetValue();
					break;

				case COK_FLOAT: {
					double v;
					if (!((wxTextCtrl*)ctl->control)->GetValue().ToDouble(&v)) {
						wxLogWarning(
							_T("The value entered for field '%s' (%s) could not be converted to a floating-point number. Default value (%f) substituted for the entered value."),
							wxString(ctl->option->label, wxConvUTF8).c_str(),
							((wxTextCtrl*)ctl->control)->GetValue().c_str(),
							ctl->option->default_val.floatval);
						ctl->option->value.floatval = ctl->option->default_val.floatval;
					}
					ctl->option->value.floatval = v;
					break; }

				case COK_BOOL:
					ctl->option->value.intval = (int)((wxCheckBox*)ctl->control)->GetValue();
					break;

				case COK_COLOUR:
					// *FIXME* needs to be updated to use a proper color control
					Auto3Free(ctl->option->value.stringval);
					ctl->option->value.stringval = Auto3Strdup(((wxTextCtrl*)ctl->control)->GetValue().mb_str(wxConvUTF8));
					break;

				case COK_STYLE:
					Auto3Free(ctl->option->value.stringval);
					ctl->option->value.stringval = Auto3Strdup(((wxChoice*)ctl->control)->GetStringSelection().mb_str(wxConvUTF8));
					break;
					
				case COK_LABEL:
				case COK_INVALID:
					break;
			}
		}
	}

	wxString Auto3ConfigDialog::Serialise()
	{
		if (options->name == 0)
			return _T("");

		wxString result;
		for (Auto3ConfigOption *opt = options; opt->name; opt++) {
			wxString optname(opt->name, wxConvUTF8);
			switch (opt->kind) {
				case COK_TEXT:
				case COK_STYLE:
				case COK_COLOUR: {
					wxString optstrval(opt->value.stringval, wxConvUTF8);
					result << wxString::Format(_T("%s:%s|"), optname.c_str(), inline_string_encode(optstrval).c_str());
					break; }
				case COK_INT:
				case COK_BOOL:
					result << wxString::Format(_T("%s:%d|"), optname.c_str(), opt->value.intval);
					break;
				case COK_FLOAT:
					result << wxString::Format(_T("%s:%e|"), optname.c_str(), opt->value.floatval);
					break;
				default:
					// The rest aren't stored
					break;
			}
		}
		if (!result.IsEmpty() && result.Last() == _T('|'))
			result.RemoveLast();
		return result;
	}

	void Auto3ConfigDialog::Unserialise(const wxString &settings)
	{
		wxStringTokenizer toker(settings, _T("|"), wxTOKEN_STRTOK);
		while (toker.HasMoreTokens()) {
			// get the parts of this setting
			wxString setting = toker.GetNextToken();

			wxString optname = setting.BeforeFirst(_T(':'));
			wxString optval = setting.AfterFirst(_T(':'));

			// find the setting in the list loaded from the script
			Auto3ConfigOption *opt = options;
			while (opt->name && wxString(opt->name, wxConvUTF8) != optname)
				opt ++;

			if (opt->name) {
				// ok, found the option!
				switch (opt->kind) {
					case COK_TEXT:
					case COK_STYLE:
					case COK_COLOUR:
						Auto3Free(opt->value.stringval);
						opt->value.stringval = Auto3Strdup(inline_string_decode(optval).mb_str(wxConvUTF8));
						break;

					case COK_INT:
					case COK_BOOL: {
						long n;
						optval.ToLong(&n, 10);
						opt->value.intval = n;
						break; }

					case COK_FLOAT: {
						double v;
						optval.ToDouble(&v);
						opt->value.floatval = (float)v;
						break; }

					case COK_LABEL:
					case COK_INVALID:
						break;
				}
			}
		}
	}


	// Auto3Filter

	Auto3Filter::Auto3Filter(const wxString &_name, const wxString &_description, Auto3Interpreter *_script)
		: Feature(SCRIPTFEATURE_FILTER, _name)
		, FeatureFilter(_name, _description, 0)
		, script(_script)
	{
		// Nothing more to do
	}

	ScriptConfigDialog* Auto3Filter::GenerateConfigDialog(wxWindow *parent)
	{
		config = new Auto3ConfigDialog(script);
		return config;
	}

	void Auto3Filter::Init()
	{
		// Nothing to do here
	}

	void Auto3Filter::ProcessSubs(AssFile *subs, wxWindow *export_dialog)
	{
		Auto3ProgressSink *sink = new Auto3ProgressSink(script, export_dialog);
		sink->SetTitle(GetName());
		Auto3ThreadedProcessor thread(script, subs, config, sink);

		sink->ShowModal();
		thread.Wait();

		delete sink;
	}


	// Auto3ThreadedProcessor

	Auto3ThreadedProcessor::Auto3ThreadedProcessor(Auto3Interpreter *_script, AssFile *_file, Auto3ConfigDialog *_config, Auto3ProgressSink *_sink)
		: wxThread(wxTHREAD_JOINABLE)
		, script(_script)
		, file(_file)
		, config(_config)
		, sink(_sink)
	{
		// Pure copypasta
		int prio = Options.AsInt(_T("Automation Thread Priority"));
		if (prio == 0) prio = 50; // normal
		else if (prio == 1) prio = 30; // below normal
		else if (prio == 2) prio = 10; // lowest
		else prio = 50; // fallback normal
		Create();
		SetPriority(prio);
		Run();
	}

	wxThread::ExitCode Auto3ThreadedProcessor::Entry()
	{
		script->cb.rwdata = this;
		script->cb.reset_style_pointer = ResetStylePointer;
		script->cb.reset_subs_pointer = ResetSubsPointer;
		script->cb.get_meta_info = GetMetaInfo;
		script->cb.get_next_style = GetNextStyle;
		script->cb.get_next_sub = GetNextSub;
		script->cb.start_subs_write = StartSubsWrite;
		script->cb.write_sub = WriteSub;

		int res = RunAuto3Script(script);

		script->cb.rwdata = 0;
		script->cb.reset_style_pointer = 0;
		script->cb.reset_subs_pointer = 0;
		script->cb.get_meta_info = 0;
		script->cb.get_next_style = 0;
		script->cb.get_next_sub = 0;
		script->cb.start_subs_write = 0;
		script->cb.write_sub = 0;

		sink->script_finished = true;
		wxWakeUpIdle();

		if (res) return (wxThread::ExitCode) 1;
		else return 0;
	}


	void Auto3ThreadedProcessor::ResetStylePointer(void *cbdata)
	{
		Auto3ThreadedProcessor *self = (Auto3ThreadedProcessor*)cbdata;
		self->style_pointer = self->file->Line.begin();
	}


	void Auto3ThreadedProcessor::ResetSubsPointer(void *cbdata)
	{
		Auto3ThreadedProcessor *self = (Auto3ThreadedProcessor*)cbdata;
		self->subs_pointer = self->file->Line.begin();
	}


	void Auto3ThreadedProcessor::GetMetaInfo(void *cbdata, int *res_x, int *res_y)
	{
		Auto3ThreadedProcessor *self = (Auto3ThreadedProcessor*)cbdata;
		self->file->GetResolution(*res_x, *res_y);
	}


	int Auto3ThreadedProcessor::GetNextStyle(
		void *cbdata, char **name, char **fontname, int *fontsize, char **color1, char **color2, char **color3, char **color4,
		int *bold, int *italic, int *underline, int *strikeout, float *scale_x, float *scale_y, float *spacing, float *angle,
		int *borderstyle, float *outline, float *shadow, int *align, int *margin_l, int *margin_r, int *margin_v, int *encoding)
	{
		Auto3ThreadedProcessor *self = (Auto3ThreadedProcessor*)cbdata;

		while (self->style_pointer != self->file->Line.end()) {
			AssStyle *style = AssEntry::GetAsStyle(*self->style_pointer);
			// Increase iterator before we test for and return style data, since it needs to be done either way
			// The iterator should always point to the next line to be examined
			self->style_pointer++;
			if (style) {
				// Put strings into buffers
				self->stylename = style->name.mb_str(wxConvUTF8);
				self->stylefont = style->font.mb_str(wxConvUTF8);
				self->stylecolor[0] = style->primary.GetASSFormatted(true, false, true).mb_str(wxConvUTF8);
				self->stylecolor[1] = style->secondary.GetASSFormatted(true, false, true).mb_str(wxConvUTF8);
				self->stylecolor[2] = style->outline.GetASSFormatted(true, false, true).mb_str(wxConvUTF8);
				self->stylecolor[3] = style->shadow.GetASSFormatted(true, false, true).mb_str(wxConvUTF8);

				// Store data to lib
				*name = self->stylename.data();
				*fontname = self->stylefont.data();
				*fontsize = (int)style->fontsize;
				*color1 = self->stylecolor[0].data();
				*color2 = self->stylecolor[1].data();
				*color3 = self->stylecolor[2].data();
				*color4 = self->stylecolor[3].data();
				*bold = (int)style->bold;
				*italic = (int)style->italic;
				*underline = (int)style->italic;
				*strikeout = (int)style->strikeout;
				*scale_x = style->scalex;
				*scale_y = style->scaley;
				*spacing = style->spacing;
				*angle = style->angle;
				*borderstyle = style->borderstyle;
				*outline = style->outline_w;
				*shadow = style->shadow_w;
				*align = style->alignment;
				*margin_l = style->Margin[0];
				*margin_r = style->Margin[1];
				*margin_v = style->Margin[2];
				*encoding = style->encoding;

				// and return success
				return 1;
			}
		}

		return 0;
	}


	int Auto3ThreadedProcessor::GetNextSub(void *cbdata, int *layer, int *start_time, int *end_time, char **style, char **actor,
		int *margin_l, int *margin_r, int *margin_v, char **effect, char **text, int *comment)
	{
		Auto3ThreadedProcessor *self = (Auto3ThreadedProcessor*)cbdata;

		while (self->subs_pointer != self->file->Line.end()) {
			AssDialogue *dia = AssEntry::GetAsDialogue(*self->subs_pointer);
			self->subs_pointer++;
			if (dia) {
				// Put strings into buffers
				self->diagstyle = dia->Style.mb_str(wxConvUTF8);
				self->diagactor = dia->Actor.mb_str(wxConvUTF8);
				self->diageffect = dia->Effect.mb_str(wxConvUTF8);
				self->diagtext = dia->Text.mb_str(wxConvUTF8);

				// Store data to lib
				*comment = (int)dia->Comment;
				*layer = dia->Layer;
				*start_time = dia->Start.GetMS()/10;
				*end_time = dia->End.GetMS()/10;
				*style = self->diagstyle.data();
				*actor = self->diagactor.data();
				*margin_l = dia->Margin[0];
				*margin_r = dia->Margin[1];
				*margin_v = dia->Margin[2];
				*effect = self->diageffect.data();
				*text = self->diagtext.data();

				// return success
				return 1;
			}
		}
		
		return 0;
	}


	void Auto3ThreadedProcessor::StartSubsWrite(void *cbdata)
	{
		Auto3ThreadedProcessor *self = (Auto3ThreadedProcessor*)cbdata;
		
		// clear all dialogue lines
		std::list<AssEntry*>::iterator line = self->file->Line.begin();
		while (line != self->file->Line.end()) {
			std::list<AssEntry*>::iterator cur = line;
			line++;
			if (AssEntry::GetAsDialogue(*cur)) {
				delete *cur;
				self->file->Line.erase(cur);
			}
		}
	}


	void Auto3ThreadedProcessor::WriteSub(void *cbdata, int layer, int start_time, int end_time, const char *style, const char *actor,
		int margin_l, int margin_r, int margin_v, const char *effect, const char *text, int comment)
	{
		Auto3ThreadedProcessor *self = (Auto3ThreadedProcessor*)cbdata;
		
		// Construct dialogue object
		AssDialogue *dia = new AssDialogue();
		dia->Comment = !!comment;
		dia->Layer = layer;
		dia->Start.SetMS(start_time*10);
		dia->End.SetMS(end_time*10);
		dia->Style = wxString(style, wxConvUTF8);
		dia->Actor = wxString(actor, wxConvUTF8);
		dia->Margin[0] = margin_l;
		dia->Margin[1] = margin_r;
		dia->Margin[2] = dia->Margin[3] = margin_v;
		dia->Effect = wxString(effect, wxConvUTF8);
		dia->Text = wxString(text, wxConvUTF8);
		
		// Append to file
		self->file->Line.push_back(dia);
	}


	// Auto3Script

	Auto3Script::Auto3Script(const wxString &filename)
		: Script(filename)
		, filter(0)
		, script(0)
	{
		try {
			Create();
		}
		catch (wxChar *e) {
			description = e;
			loaded = false;
		}
	}

	Auto3Script::~Auto3Script()
	{
		if (script) Destroy();
	}

	void Auto3Script::TextExtents(void *cbdata, const char *text, const char *fontname, int fontsize, int bold, int italic, int spacing, 
		float scale_x, float scale_y, int encoding, float *out_width, float *out_height, float *out_descent, float *out_extlead)
	{
		double resx, resy, resd, resl;

		wxString intext(text, wxConvUTF8);

		AssStyle st;

		st.font = wxString(fontname, wxConvUTF8);
		st.fontsize = fontsize;
		st.bold = !!bold;
		st.italic = !!italic;
		st.underline = false;
		st.strikeout = false;
		st.scalex = scale_x;
		st.scaley = scale_y;
		st.spacing = spacing;
		st.encoding = encoding;

		CalculateTextExtents(&st, intext, resx, resy, resd, resl);
		// So no error checking here... FIXME?

		*out_width = resx;
		*out_height = resy;
		*out_descent = resd;
		*out_extlead = resl;
	}

	filename_t Auto3Script::ResolveInclude(void *cbdata, const char *incname)
	{
		Auto3Script *s = (Auto3Script*)cbdata;

		wxString fnames(incname, wxConvUTF8);

		wxFileName fname(fnames);
		if (fname.GetDirCount() == 0) {
			// filename only
			fname = s->include_path.FindAbsoluteValidPath(fnames);
		} else if (fname.IsRelative()) {
			// relative path
			wxFileName sfname(s->GetFilename());
			fname.MakeAbsolute(sfname.GetPath(true));
		} else {
			// absolute path, do nothing
		}
		if (!fname.IsOk() || !fname.FileExists()) {
			return 0;
		}

#ifdef WIN32
		// Get number of widechars in filename string
		size_t wfnlen = wcslen(fname.GetFullPath().wc_str());
		// Alloc memory to hold string
		filename_t wfn = (filename_t)Auto3Malloc((wfnlen+1) * sizeof(wchar_t));
		// And copy string into memory
		wcsncpy(wfn, fname.GetFullPath().wc_str(), wfnlen);
		wfn[wfnlen] = 0;
		return wfn;
#else
		return Auto3Strdup(fname.GetFullPath().fn_str());
#endif
	}

	int Auto3Script::FrameFromMs(void *cbdata, int ms)
	{
		if (VFR_Output.IsLoaded()) {
			return VFR_Output.GetFrameAtTime(ms, true);
		} else {
			return 0;
		}
	}

	int Auto3Script::MsFromFrame(void *cbdata, int frame)
	{
		if (VFR_Output.IsLoaded()) {
			return VFR_Output.GetTimeAtFrame(frame, true);
		} else {
			return 0;
		}
	}

	void Auto3Script::Create()
	{
		Destroy();

		// Fill callbacks struct
		Auto3Callbacks cb;
		// Logging, implemented only during script execution
		cb.logdata = 0;
		cb.log_error = 0;
		cb.log_message = 0;
		cb.set_progress = 0;
		cb.set_status = 0;
		// Read/write, also only during execution
		cb.rwdata = 0;
		cb.get_meta_info = 0;
		cb.get_next_style = 0;
		cb.get_next_sub = 0;
		cb.reset_style_pointer = 0;
		cb.reset_subs_pointer = 0;
		cb.start_subs_write = 0;
		cb.write_sub = 0;
		// Misc, implemented all the time
		cb.rundata = this;
		cb.resolve_include = ResolveInclude;
		cb.text_extents = TextExtents;
		cb.frame_from_ms = FrameFromMs;
		cb.ms_from_frame = MsFromFrame;

		char *errormsg = 0;
		// Why oh why... GCC wants fn_str() to be dereffed with .data() but MSVC hates that...
		// If anyone can FIXME to something more sensible, please do so
#ifdef WIN32
		script = CreateAuto3Script((const filename_t)GetFilename().fn_str(), GetPrettyFilename().mb_str(wxConvUTF8), &cb, &errormsg);
#else
		script = CreateAuto3Script((const filename_t)GetFilename().fn_str().data(), GetPrettyFilename().mb_str(wxConvUTF8).data(), &cb, &errormsg);
#endif

		if (script) {
			assert(errormsg == 0);
			loaded = true;

			name = wxString(script->name, wxConvUTF8);
			description = wxString(script->description, wxConvUTF8);
			author = _T("");
			version = _T("");

			filter = new Auto3Filter(name, description, script);

		} else {
			loaded = false;
			name = GetPrettyFilename();
			if (errormsg) {
				description = wxString(errormsg, wxConvUTF8);
				Auto3Free(errormsg);
			} else {
				description = _T("Unknown error (auto3 library returned NULL error message)");
			}
		}
	}

	void Auto3Script::Destroy()
	{
		if (!script) return;

		if (filter) {
			delete filter;
			filter = 0;
		}

		DestroyAuto3Script(script);
		script = 0;

		loaded = false;
	}

	void Auto3Script::Reload()
	{
		Destroy();
		Create();
	}

	Auto3ScriptFactory::Auto3ScriptFactory()
	{
		engine_name = _T("Legacy Automation 3");
		filename_pattern = _T("*.auto3");
		Register(this);
	}

	Auto3ScriptFactory::~Auto3ScriptFactory() { }

	Script* Auto3ScriptFactory::Produce(const wxString &filename) const
	{
		if (filename.Right(6).Lower() == _T(".auto3")) {
			return new Auto3Script(filename);
		} else {
			return 0;
		}
	}


};

#endif // WITH_AUTO3
#endif // WITH_AUTOMATION
