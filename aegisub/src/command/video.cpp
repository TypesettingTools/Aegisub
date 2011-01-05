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

/// @file video.cpp
/// @brief video/ commands.
/// @ingroup command
///

#include "config.h"

#ifndef AGI_PRE
#endif

#include "command.h"

#include "aegisub/context.h"
#include "video_context.h"
#include "main.h"
#include "utils.h"
#include "frame_main.h"
#include "video_display.h"
#include "dialog_detached_video.h"
#include "dialog_video_details.h"
#include "video_slider.h"
#include "dialog_dummy_video.h"
#include "compat.h"
#include "dialog_jumpto.h"

namespace cmd {
/// @defgroup cmd-video Video commands.
/// @{


/// Forces video to 2.35 aspect ratio.
class video_aspect_cinematic: public Command {
public:
	CMD_NAME("video/aspect/cinematic")
	STR_MENU("&Cinematic (2.35)")
	STR_DISP("Cinematic (235)")
	STR_HELP("Forces video to 2.35 aspect ratio.")

	void operator()(agi::Context *c) {
	VideoContext::Get()->Stop();
	VideoContext::Get()->SetAspectRatio(3);
	wxGetApp().frame->SetDisplayMode(1,-1);
	}
};


/// Forces video to a custom aspect ratio.
class video_aspect_custom: public Command {
public:
	CMD_NAME("video/aspect/custom")
	STR_MENU("Custom..")
	STR_DISP("Custom")
	STR_HELP("Forces video to a custom aspect ratio.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();

		wxString value = wxGetTextFromUser(_("Enter aspect ratio in either:\n  decimal (e.g. 2.35)\n  fractional (e.g. 16:9)\n  specific resolution (e.g. 853x480)"),_("Enter aspect ratio"),AegiFloatToString(VideoContext::Get()->GetAspectRatioValue()));
		if (value.IsEmpty()) return;

		value.MakeLower();

		// Process text
		double numval;
		if (value.ToDouble(&numval)) {
			//Nothing to see here, move along
		}
		else {
			double a,b;
			int pos=0;
			bool scale=false;

			//Why bloat using Contains when we can just check the output of Find?
			pos = value.Find(':');
			if (pos==wxNOT_FOUND) pos = value.Find('/');
			if (pos==wxNOT_FOUND&&value.Contains(_T('x'))) {
				pos = value.Find('x');
				scale=true;
			}

			if (pos>0) {
				wxString num = value.Left(pos);
				wxString denum = value.Mid(pos+1);
				if (num.ToDouble(&a) && denum.ToDouble(&b) && b!=0) {
					numval = a/b;
					if (scale) c->videoBox->videoDisplay->SetZoom(b / VideoContext::Get()->GetHeight());
				}
			}
			else numval = 0.0;
		}

		// Sanity check
		if (numval < 0.5 || numval > 5.0) wxMessageBox(_("Invalid value! Aspect ratio must be between 0.5 and 5.0."),_("Invalid Aspect Ratio"),wxICON_ERROR|wxOK);

		// Set value
		else {
			VideoContext::Get()->SetAspectRatio(4,numval);
			wxGetApp().frame->SetDisplayMode(1,-1);
		}
	}
};



/// Leave video on original aspect ratio.
class video_aspect_default: public Command {
public:
	CMD_NAME("video/aspect/default")
	STR_MENU("&Default")
	STR_DISP("Default")
	STR_HELP("Leave video on original aspect ratio.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		VideoContext::Get()->SetAspectRatio(0);
		wxGetApp().frame->SetDisplayMode(1,-1);
	}
};



/// Forces video to 4:3 aspect ratio.
class video_aspect_full: public Command {
public:
	CMD_NAME("video/aspect/full")
	STR_MENU("&Fullscreen (4:3)")
	STR_DISP("Fullscreen (4:3)")
	STR_HELP("Forces video to 4:3 aspect ratio.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		VideoContext::Get()->SetAspectRatio(1);
		wxGetApp().frame->SetDisplayMode(1,-1);
	}
};


/// Forces video to 16:9 aspect ratio.
class video_aspect_wide: public Command {
public:
	CMD_NAME("video/aspect/wide")
	STR_MENU("&Widescreen (16:9)")
	STR_DISP("Widescreen (16:9)")
	STR_HELP("Forces video to 16:9 aspect ratio.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		VideoContext::Get()->SetAspectRatio(2);
		wxGetApp().frame->SetDisplayMode(1,-1);
	}
};


/// Closes the currently open video file.
class video_close: public Command {
public:
	CMD_NAME("video/close")
	STR_MENU("&Close Video")
	STR_DISP("Close Video")
	STR_HELP("Closes the currently open video file.")

	void operator()(agi::Context *c) {
		wxGetApp().frame->LoadVideo(_T(""));
	}
};



/// Detach video, displaying it in a separate Window.
class video_detach: public Command {
public:
	CMD_NAME("video/detach")
	STR_MENU("Detach Video")
	STR_DISP("Detach Video")
	STR_HELP("Detach video, displaying it in a separate Window.")

	void operator()(agi::Context *c) {
		wxGetApp().frame->DetachVideo(!c->detachedVideo);
	}
};


/// Shows video details.
class video_details: public Command {
public:
	CMD_NAME("video/details")
	STR_MENU("Show Video Details..")
	STR_DISP("Show Video Details")
	STR_HELP("Shows video details.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		DialogVideoDetails videodetails(c->parent);
		videodetails.ShowModal();
	}
};


/// 
class video_focus_seek: public Command {
public:
	CMD_NAME("video/focus_seek")
	STR_MENU("XXX: no idea")
	STR_DISP("XXX: no idea")
	STR_HELP("XXX: no idea")

	void operator()(agi::Context *c) {
		wxWindow *curFocus = wxWindow::FindFocus();
		if (curFocus == c->videoBox->videoSlider) {
			if (c->PreviousFocus) c->PreviousFocus->SetFocus();
		}
		else {
			c->PreviousFocus = curFocus;
			c->videoBox->videoSlider->SetFocus();
		}
	}
};


/// Seek to the next frame.
class video_frame_next: public Command {
public:
	CMD_NAME("video/frame/next")
	STR_MENU("Next Frame")
	STR_DISP("Next Frame")
	STR_HELP("Seek to the next frame.")

	void operator()(agi::Context *c) {
		c->videoBox->videoSlider->NextFrame();
	}
};


/// Play video.
class video_frame_play: public Command {
public:
	CMD_NAME("video/frame/play")
	STR_MENU("Play")
	STR_DISP("Play")
	STR_HELP("Play video.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Play();
	}
};


/// Seek to the previous frame.
class video_frame_prev: public Command {
public:
	CMD_NAME("video/frame/prev")
	STR_MENU("Previous Frame")
	STR_DISP("Previous Frame")
	STR_HELP("Seek to the previous frame.")

	void operator()(agi::Context *c) {
		c->videoBox->videoSlider->PrevFrame();
	}
};


/// Jump to frame or time.
class video_jump: public Command {
public:
	CMD_NAME("video/jump")
	STR_MENU("&Jump to..")
	STR_DISP("Jump to")
	STR_HELP("Jump to frame or time.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		if (VideoContext::Get()->IsLoaded()) {
			DialogJumpTo JumpTo(c->parent);
			JumpTo.ShowModal();
			c->videoBox->videoSlider->SetFocus();
		}
	}
};


/// Jumps the video to the end frame of current subtitle.
class video_jump_end: public Command {
public:
	CMD_NAME("video/jump/end")
	STR_MENU("Jump Video to End")
	STR_DISP("Jump Video to End")
	STR_HELP("Jumps the video to the end frame of current subtitle.")

	void operator()(agi::Context *c) {
		c->SubsGrid->SetVideoToSubs(false);
	}
};


/// Jumps the video to the start frame of current subtitle.
class video_jump_start: public Command {
public:
	CMD_NAME("video/jump/start")
	STR_MENU("Jump Video to Start")
	STR_DISP("Jump Video to Start")
	STR_HELP("Jumps the video to the start frame of current subtitle.")

	void operator()(agi::Context *c) {
		c->SubsGrid->SetVideoToSubs(true);
	}
};


/// Opens a video file.
class video_open: public Command {
public:
	CMD_NAME("video/open")
	STR_MENU("&Open Video..")
	STR_DISP("Open Video")
	STR_HELP("Opens a video file.")

	void operator()(agi::Context *c) {
		wxString path = lagi_wxString(OPT_GET("Path/Last/Video")->GetString());
		wxString str = wxString(_("Video Formats")) + _T(" (*.avi,*.mkv,*.mp4,*.avs,*.d2v,*.ogm,*.mpeg,*.mpg,*.vob,*.mov)|*.avi;*.avs;*.d2v;*.mkv;*.ogm;*.mp4;*.mpeg;*.mpg;*.vob;*.mov|")
					 + _("All Files") + _T(" (*.*)|*.*");
		wxString filename = wxFileSelector(_("Open video file"),path,_T(""),_T(""),str,wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (!filename.empty()) {
			wxGetApp().frame->LoadVideo(filename);
			OPT_SET("Path/Last/Video")->SetString(STD_STR(filename));
		}
	}
};


/// Opens a video clip with solid colour.
class video_open_dummy: public Command {
public:
	CMD_NAME("video/open/dummy")
	STR_MENU("Use Dummy Video..")
	STR_DISP("Use Dummy Video")
	STR_HELP("Opens a video clip with solid colour.")

	void operator()(agi::Context *c) {
		wxString fn;
		if (DialogDummyVideo::CreateDummyVideo(c->parent, fn)) {
			wxGetApp().frame->LoadVideo(fn);
		}
	}
};


/// Show a mask over the video.
class video_show_overscan: public Command {
public:
	CMD_NAME("video/show_overscan")
	STR_MENU("Show Overscan Mask")
	STR_DISP("Show Overscan Mask")
	STR_HELP("Show a mask over the video, indicating areas that might get cropped off by overscan on televisions.")

	void operator()(agi::Context *c) {
//XXX: Fix to not require using an event. (maybe)
//		OPT_SET("Video/Overscan Mask")->SetBool(event.IsChecked());
		VideoContext::Get()->Stop();
		c->videoBox->videoDisplay->Render();
	}
};


/// Set zoom to 100%.
class video_zoom_100: public Command {
public:
	CMD_NAME("video/zoom/100")
	STR_MENU("&100%")
	STR_DISP("100%")
	STR_HELP("Set zoom to 100%.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		c->videoBox->videoDisplay->SetZoom(1.);
	}
};



/// Set zoom to 200%.
class video_zoom_200: public Command {
public:
	CMD_NAME("video/zoom/200")
	STR_MENU("&200%")
	STR_DISP("200%")
	STR_HELP("Set zoom to 200%.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		c->videoBox->videoDisplay->SetZoom(2.);
	}
};


/// Set zoom to 50%.
class video_zoom_50: public Command {
public:
	CMD_NAME("video/zoom/50")
	STR_MENU("&50%")
	STR_DISP("50%")
	STR_HELP("Set zoom to 50%.")

	void operator()(agi::Context *c) {
		VideoContext::Get()->Stop();
		c->videoBox->videoDisplay->SetZoom(.5);
	}
};


/// Zoom video in.
class video_zoom_in: public Command {
public:
	CMD_NAME("video/zoom/in")
	STR_MENU("Zoom In")
	STR_DISP("Zoom In")
	STR_HELP("Zoom video in.")

	void operator()(agi::Context *c) {
		c->videoBox->videoDisplay->SetZoom(c->videoBox->videoDisplay->GetZoom() + .125);
	}
};


/// Zoom video out.
class video_zoom_out: public Command {
public:
	CMD_NAME("video/zoom/out")
	STR_MENU("Zoom Out")
	STR_DISP("Zoom Out")
	STR_HELP("Zoom video out.")

	void operator()(agi::Context *c) {
		c->videoBox->videoDisplay->SetZoom(c->videoBox->videoDisplay->GetZoom() - .125);
	}
};

/// @}

/// Init video/ commands.
void init_video(CommandManager *cm) {
	cm->reg(new video_aspect_cinematic());
	cm->reg(new video_aspect_custom());
	cm->reg(new video_aspect_default());
	cm->reg(new video_aspect_full());
	cm->reg(new video_aspect_wide());
	cm->reg(new video_close());
	cm->reg(new video_detach());
	cm->reg(new video_details());
	cm->reg(new video_focus_seek());
	cm->reg(new video_frame_next());
	cm->reg(new video_frame_play());
	cm->reg(new video_frame_prev());
	cm->reg(new video_jump());
	cm->reg(new video_jump_end());
	cm->reg(new video_jump_start());
	cm->reg(new video_open());
	cm->reg(new video_open_dummy());
	cm->reg(new video_show_overscan());
	cm->reg(new video_zoom_100());
	cm->reg(new video_zoom_200());
	cm->reg(new video_zoom_50());
	cm->reg(new video_zoom_in());
	cm->reg(new video_zoom_out());
}

} // namespace cmd
