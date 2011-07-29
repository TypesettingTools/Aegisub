// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file dialog_detached_video.h
/// @see dialog_detached_video.cpp
/// @ingroup main_ui
///

#ifndef AGI_PRE
#include <wx/dialog.h>
#endif

#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>

namespace agi { struct Context; }
class PersistLocation;
class VideoBox;

/// DOCME
/// @class DialogDetachedVideo
/// @brief DOCME
///
/// DOCME
class DialogDetachedVideo : public wxDialog {
	agi::Context *context;
	agi::signal::Connection video_open;
	agi::scoped_ptr<PersistLocation> persist;

	void OnClose(wxCloseEvent &);
	/// Minimize event handler to hack around a wx bug
	void OnMinimize(wxIconizeEvent &evt);
	void OnKeyDown(wxKeyEvent &evt);
	void OnVideoOpen();

public:
	/// @brief Constructor
	/// @param context Project context
	/// @param initialDisplaySize Initial size of the window
	DialogDetachedVideo(agi::Context *context, const wxSize &initialDisplaySize);
	/// Destructor
	~DialogDetachedVideo();
};
