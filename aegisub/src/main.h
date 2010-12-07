// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file main.h
/// @see main.cpp
/// @ingroup main
///


///////////////////
// Include headers
#ifndef AGI_PRE
#include <wx/app.h>
#include <wx/file.h>
#include <wx/stackwalk.h>
#endif

#include "aegisublocale.h"
#include <libaegisub/mru.h>
#include <libaegisub/option.h>

//////////////
// Prototypes
class FrameMain;
class PluginManager;
#ifdef _DEBUG
namespace agi {
	namespace log {
		class EmitSTDOUT;
	}
}
#endif

/// For holding all configuration-related objects and values.
namespace config {
	extern agi::Options *opt; 		///< Options
	extern agi::MRUManager *mru;	///< Most Recently Used
}

/// DOCME
namespace Automation4 { class AutoloadScriptManager; }

/// Macro to get OptionValue object.
#define OPT_GET(x) const_cast<const agi::OptionValue*>(config::opt->Get(x))

/// Macro to set OptionValue object.
#define OPT_SET(x) config::opt->Get(x)

/// Macro to subscribe to OptionValue changes
#define OPT_SUB(x, ...) config::opt->Get(x)->Subscribe(__VA_ARGS__)

/// Macro to unsubscribe to OptionValue changes
#define OPT_UNSUB(x, ...) config::opt->Get(x)->Unsubscribe(__VA_ARGS__)


/// DOCME
/// @class AegisubApp
/// @brief DOCME
///
/// DOCME
class AegisubApp: public wxApp {
private:

	/// DOCME
	PluginManager *plugins;

	void OnMouseWheel(wxMouseEvent &event);
	void OnKey(wxKeyEvent &key);

#ifdef _DEBUG
	/// stdout log emitter
	agi::log::EmitSTDOUT *emit_stdout;
#endif

public:

	/// DOCME
	AegisubLocale locale;

	/// DOCME
	FrameMain *frame;

#ifdef WITH_AUTOMATION
	/// DOCME
	Automation4::AutoloadScriptManager *global_scripts;
#endif


	/// @brief DOCME
	/// @return 
	///
	static AegisubApp* Get() { return (AegisubApp*) wxTheApp; }
	static void OpenURL(wxString url);

	bool OnInit();
	int OnExit();
	int OnRun();

#ifdef __WXMAC__
	// Apple events
	virtual void MacOpenFile(const wxString &filename);
#endif

#if !defined(_DEBUG) || defined(WITH_EXCEPTIONS)
	void OnUnhandledException();
	void OnFatalException();
#endif

#if defined(wxUSE_EXCEPTIONS)
	// This function wraps all event handler calls anywhere in the application and is
	// our ticket to catch exeptions happening in event handlers.
    virtual void HandleEvent(wxEvtHandler *handler,
                             wxEventFunction func,
                             wxEvent& event) const;
#else
# error wxWidgets is compiled without exceptions support, Aegisub requires exceptions support in wxWidgets to run safely
#endif

	//int OnRun();
	DECLARE_EVENT_TABLE()
};

DECLARE_APP(AegisubApp)


#if wxUSE_STACKWALKER == 1
/// @class StackWalker
/// @brief DOCME
///
/// DOCME
class StackWalker: public wxStackWalker {
private:

	wxFile *crash_text;	// FP to the crash text file.
	wxFile *crash_xml;	// FP to the crash xml file.

public:
	StackWalker(wxString cause);
	~StackWalker();
	void OnStackFrame(const wxStackFrame& frame);
};
#endif // wxUSE_STACKWALKER
