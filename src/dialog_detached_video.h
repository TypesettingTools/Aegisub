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

/// @file dialog_detached_video.h
/// @see dialog_detached_video.cpp
/// @ingroup main_ui
///

#include <memory>
#include <wx/dialog.h>

#include <libaegisub/signal.h>

namespace agi { struct Context; }
class AsyncVideoProvider;
class PersistLocation;
class VideoBox;
class VideoDisplay;

class DialogDetachedVideo final : public wxDialog {
	agi::Context *context;
	VideoDisplay *old_display;
	wxWindow *old_slider;
	agi::signal::Connection video_open;
	std::unique_ptr<PersistLocation> persist;

	void OnClose(wxCloseEvent &);
	/// Minimize event handler to hack around a wx bug
	void OnMinimize(wxIconizeEvent &evt);
	void OnKeyDown(wxKeyEvent &evt);
	void OnVideoOpen(AsyncVideoProvider *new_provider);

public:
	/// @brief Constructor
	/// @param context Project context
	DialogDetachedVideo(agi::Context *context);
	/// Destructor
	~DialogDetachedVideo();
};
