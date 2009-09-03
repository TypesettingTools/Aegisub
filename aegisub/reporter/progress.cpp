// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @@file progress.cpp
/// @brief Progress bar when uploading data.

#include "progress.h"

/// @brief Constructor
/// @param frame Main frame.
Progress::Progress(wxWindow *frame)
    : wxProgressDialog(_("Progress"), _("Sending data, please wait."), 100, frame, wxPD_CAN_ABORT|wxPD_ELAPSED_TIME|wxPD_ESTIMATED_TIME|wxPD_REMAINING_TIME|wxPD_AUTO_HIDE) {
}

/// @brief Cancel the transfer.
///        This closes the dialog.
void Progress::Cancel(wxCommandEvent& WXUNUSED(event)) {
	printf("close");
	Destroy();
}

BEGIN_EVENT_TABLE(Progress, wxProgressDialog)
    EVT_BUTTON(wxID_CLOSE, Progress::Cancel)
    EVT_BUTTON(wxID_CANCEL, Progress::Cancel)
	EVT_CLOSE(Progress::Close)
END_EVENT_TABLE()

