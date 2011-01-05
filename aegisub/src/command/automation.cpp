// Copyright (c) 2005-2010, Niels Martin Hansen
// Copyright (c) 2005-2010, Rodrigo Braz Monteiro
// Copyright (c) 2010, Amar Takhar
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//	 this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//	 this list of conditions and the following disclaimer in the documentation
//	 and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//	 may be used to endorse or promote products derived from this software
//	 without specific prior written permission.
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

/// @file automation.cpp
/// @brief am/ (automation) commands
/// @ingroup command
///

#include "config.h"

#ifndef AGI_PRE
#endif

#include "command.h"

#include "main.h"
#include "aegisub/context.h"
#include "dialog_automation.h"
#include "auto4_base.h"
#include "video_context.h"
#include "frame_main.h"

namespace cmd {
/// @defgroup cmd-am Automation commands
/// @{


/// Open automation manager.
class am_manager: public Command {
public:
	CMD_NAME("am/manager")
	STR_MENU("&Automation..")
	STR_DISP("Automation")
	STR_HELP("Open automation manager.")

	void operator()(agi::Context *c) {
#ifdef WITH_AUTOMATION
#ifdef __APPLE__
		if (wxGetMouseState().CmdDown()) {
#else
			if (wxGetMouseState().ControlDown()) {
#endif
				wxGetApp().global_scripts->Reload();
				if (wxGetMouseState().ShiftDown()) {
					const std::vector<Automation4::Script*> scripts = c->local_scripts->GetScripts();
					for (size_t i = 0; i < scripts.size(); ++i) {
					try {
						scripts[i]->Reload();
					} catch (const wchar_t *e) {
						wxLogError(e);
					} catch (...) {
						wxLogError(_T("An unknown error occurred reloading Automation script '%s'."), scripts[i]->GetName().c_str());
					}
				}

				wxGetApp().frame->StatusTimeout(_("Reloaded all Automation scripts"));
			} else {
				wxGetApp().frame->StatusTimeout(_("Reloaded autoload Automation scripts"));
			}
		} else {
			VideoContext::Get()->Stop();
			DialogAutomation dlg(c->parent, c->local_scripts);
			dlg.ShowModal();
		}
#endif
	}
};

/// @}

/// Init am/ commands. (automation)
void init_automation(CommandManager *cm) {
	cm->reg(new am_manager());
}


} // namespace cmd
