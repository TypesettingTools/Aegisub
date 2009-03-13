// Copyright (c) 2007, Niels Martin Hansen
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

// Scripting engine for legacy Automation 3 compatibility

#pragma once

#ifndef _AUTO4_AUTO3_H
#define _AUTO4_AUTO3_H

#include <wx/thread.h>
#include <wx/event.h>
#include "auto4_base.h"
#include "libauto3/auto3.h"
#include "ass_file.h"
#include "ass_entry.h"
#include "ass_dialogue.h"
#include "ass_style.h"

namespace Automation4 {

	class Auto3ProgressSink : public ProgressSink {
	private:
		Auto3Interpreter *script;

		static void SetStatus(void *cbdata, const char *msg);
		static void OutputDebug(void *cbdata, const char *msg);
		static void ReportProgress(void *cbdata, float progress);

	public:
		Auto3ProgressSink(Auto3Interpreter *_script, wxWindow *parent);
		virtual ~Auto3ProgressSink();
	};


	class Auto3ConfigDialog : public ScriptConfigDialog {
	private:
		Auto3ConfigOption *options;

		struct Control {
			wxStaticText *label;
			wxControl *control;
			Auto3ConfigOption *option;
			Control() : label(0), control(0), option(0) {}
		};
		std::vector<Control> controls;

	protected:
		wxWindow* CreateWindow(wxWindow *parent);

	public:
		Auto3ConfigDialog(Auto3Interpreter *script);
		virtual ~Auto3ConfigDialog();

		void ReadBack(); // from auto4 base

		wxString Serialise(); // make a string from the option name+value pairs
		void Unserialise(const wxString &settings); // set the option values from a serialized string
	};


	class Auto3Filter : public FeatureFilter {
	private:
		Auto3ConfigDialog *config;
		AssFile *_file;
		Auto3Interpreter *script;

	protected:
		ScriptConfigDialog* GenerateConfigDialog(wxWindow *parent);

		void Init();
	public:
		Auto3Filter(const wxString &_name, const wxString &_description, Auto3Interpreter *_script);

		void ProcessSubs(AssFile *subs, wxWindow *export_dialog);

		virtual ~Auto3Filter() { }
	};


	class Auto3ThreadedProcessor : public wxThread {
	private:
		Auto3Interpreter *script;
		AssFile *file;
		Auto3ConfigDialog *config;
		Auto3ProgressSink *sink;

		// Iterators used for read/write callbacks
		std::list<AssEntry*>::iterator style_pointer;
		std::list<AssEntry*>::iterator subs_pointer;

		// Char buffers holding data used in callbacks
		wxCharBuffer stylename, stylefont, stylecolor[4], diagstyle, diagactor, diageffect, diagtext;

		// Read/write callback functions
		static void ResetStylePointer(void *cbdata);
		static void ResetSubsPointer(void *cbdata);
		static void GetMetaInfo(void *cbdata, int *res_x, int *res_y);
		static int GetNextStyle(
			void *cbdata, char **name, char **fontname, int *fontsize, char **color1, char **color2, char **color3, char **color4,
			int *bold, int *italic, int *underline, int *strikeout, float *scale_x, float *scale_y, float *spacing, float *angle,
			int *borderstyle, float *outline, float *shadow, int *align, int *margin_l, int *margin_r, int *margin_v, int *encoding);
		static int GetNextSub(void *cbdata, int *layer, int *start_time, int *end_time, char **style, char **actor,
			int *margin_l, int *margin_r, int *margin_v, char **effect, char **text, int *comment);
		static void StartSubsWrite(void *cbdata);
		static void WriteSub(void *cbdata, int layer, int start_time, int end_time, const char *style, const char *actor,
			int margin_l, int margin_r, int margin_v, const char *effect, const char *text, int comment);

	public:
		Auto3ThreadedProcessor(Auto3Interpreter *_script, AssFile *_file, Auto3ConfigDialog *_config, Auto3ProgressSink *_sink);
		virtual ExitCode Entry();
	};


	class Auto3Script : public Script {
	private:
		Auto3Filter *filter;
		Auto3Interpreter *script;

		static filename_t ResolveInclude(void *cbdata, const char *incname);
		static void TextExtents(void *cbdata, const char *text, const char *fontname, int fontsize, int bold, int italic,
			int spacing, float scale_x, float scale_y, int encoding,
			float *out_width, float *out_height, float *out_descent, float *out_extlead);
		static int FrameFromMs(void *cbdata, int ms);
		static int MsFromFrame(void *cbdata, int frame);

		void Create();
		void Destroy();

	public:
		Auto3Script(const wxString &filename);
		virtual ~Auto3Script();

		virtual void Reload();
	};

};

#endif
