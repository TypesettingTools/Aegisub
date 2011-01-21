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

#include "../config.h"

#ifndef AGI_PRE
#endif

#include "command.h"

#include "../ass_dialogue.h"
#include "../ass_time.h"
#include "../compat.h"
#include "../frame_main.h"
#include "../main.h"
#include "../include/aegisub/context.h"
#include "../dialog_detached_video.h"
#include "../dialog_dummy_video.h"
#include "../dialog_jumpto.h"
#include "../dialog_video_details.h"
#include "../subs_grid.h"
#include "../utils.h"
#include "../video_box.h"
#include "../video_context.h"
#include "../video_display.h"
#include "../video_slider.h"

namespace cmd {
/// @defgroup cmd-video Video commands.
/// @{


/// Forces video to 2.35 aspect ratio.
struct video_aspect_cinematic : public Command {
	CMD_NAME("video/aspect/cinematic")
	STR_MENU("&Cinematic (2.35)")
	STR_DISP("Cinematic (235)")
	STR_HELP("Forces video to 2.35 aspect ratio.")

	void operator()(agi::Context *c) {
	c->videoController->Stop();
	c->videoController->SetAspectRatio(3);
	wxGetApp().frame->SetDisplayMode(1,-1);
	}
};


/// Forces video to a custom aspect ratio.
struct video_aspect_custom : public Command {
	CMD_NAME("video/aspect/custom")
	STR_MENU("Custom..")
	STR_DISP("Custom")
	STR_HELP("Forces video to a custom aspect ratio.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();

		wxString value = wxGetTextFromUser(_("Enter aspect ratio in either:\n  decimal (e.g. 2.35)\n  fractional (e.g. 16:9)\n  specific resolution (e.g. 853x480)"),_("Enter aspect ratio"),AegiFloatToString(c->videoController->GetAspectRatioValue()));
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
					if (scale) c->videoBox->videoDisplay->SetZoom(b / c->videoController->GetHeight());
				}
			}
			else numval = 0.0;
		}

		// Sanity check
		if (numval < 0.5 || numval > 5.0) wxMessageBox(_("Invalid value! Aspect ratio must be between 0.5 and 5.0."),_("Invalid Aspect Ratio"),wxICON_ERROR|wxOK);

		// Set value
		else {
			c->videoController->SetAspectRatio(4,numval);
			wxGetApp().frame->SetDisplayMode(1,-1);
		}
	}
};



/// Leave video on original aspect ratio.
struct video_aspect_default : public Command {
	CMD_NAME("video/aspect/default")
	STR_MENU("&Default")
	STR_DISP("Default")
	STR_HELP("Leave video on original aspect ratio.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->videoController->SetAspectRatio(0);
		wxGetApp().frame->SetDisplayMode(1,-1);
	}
};



/// Forces video to 4:3 aspect ratio.
struct video_aspect_full : public Command {
	CMD_NAME("video/aspect/full")
	STR_MENU("&Fullscreen (4:3)")
	STR_DISP("Fullscreen (4:3)")
	STR_HELP("Forces video to 4:3 aspect ratio.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->videoController->SetAspectRatio(1);
		wxGetApp().frame->SetDisplayMode(1,-1);
	}
};


/// Forces video to 16:9 aspect ratio.
struct video_aspect_wide : public Command {
	CMD_NAME("video/aspect/wide")
	STR_MENU("&Widescreen (16:9)")
	STR_DISP("Widescreen (16:9)")
	STR_HELP("Forces video to 16:9 aspect ratio.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->videoController->SetAspectRatio(2);
		wxGetApp().frame->SetDisplayMode(1,-1);
	}
};


/// Closes the currently open video file.
struct video_close : public Command {
	CMD_NAME("video/close")
	STR_MENU("&Close Video")
	STR_DISP("Close Video")
	STR_HELP("Closes the currently open video file.")

	void operator()(agi::Context *c) {
		c->videoController->SetVideo("");
	}
};



/// Detach video, displaying it in a separate Window.
struct video_detach : public Command {
	CMD_NAME("video/detach")
	STR_MENU("Detach Video")
	STR_DISP("Detach Video")
	STR_HELP("Detach video, displaying it in a separate Window.")

	void operator()(agi::Context *c) {
		wxGetApp().frame->DetachVideo(!c->detachedVideo);
	}
};


/// Shows video details.
struct video_details : public Command {
	CMD_NAME("video/details")
	STR_MENU("Show Video Details..")
	STR_DISP("Show Video Details")
	STR_HELP("Shows video details.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogVideoDetails(c).ShowModal();
	}
};


/// 
struct video_focus_seek : public Command {
	CMD_NAME("video/focus_seek")
	STR_MENU("XXX: no idea")
	STR_DISP("XXX: no idea")
	STR_HELP("XXX: no idea")

	void operator()(agi::Context *c) {
		wxWindow *curFocus = wxWindow::FindFocus();
		if (curFocus == c->videoBox->videoSlider) {
			if (c->previousFocus) c->previousFocus->SetFocus();
		}
		else {
			c->previousFocus = curFocus;
			c->videoBox->videoSlider->SetFocus();
		}
	}
};


/// Seek to the next frame.
struct video_frame_next : public Command {
	CMD_NAME("video/frame/next")
	STR_MENU("Next Frame")
	STR_DISP("Next Frame")
	STR_HELP("Seek to the next frame.")

	void operator()(agi::Context *c) {
		c->videoController->NextFrame();
	}
};

/// Seek to the next subtitle boundary.
struct video_frame_next_boundary : public Command {
	CMD_NAME("video/frame/next/boundary")
	STR_MENU("Next Boundary")
	STR_DISP("Next Boundary")
	STR_HELP("Seek to the next subtitle boundary.")

	void operator()(agi::Context *c) {
		AssDialogue *active_line = c->selectionController->GetActiveLine();
		if (!active_line) return;

		int target = c->videoController->FrameAtTime(active_line->Start.GetMS(), agi::vfr::START);
		if (target > c->videoController->GetFrameN()) {
			c->videoController->JumpToFrame(target);
			return;
		}

		target = c->videoController->FrameAtTime(active_line->End.GetMS(), agi::vfr::END);
		if (target > c->videoController->GetFrameN()) {
			c->videoController->JumpToFrame(target);
			return;
		}

		c->selectionController->NextLine();
		AssDialogue *new_line = c->selectionController->GetActiveLine();
		if (new_line != active_line)
		c->videoController->JumpToTime(new_line->Start.GetMS());
	}
};

/// Seek to the next keyframe.
struct video_frame_next_keyframe : public Command {
	CMD_NAME("video/frame/next/keyframe")
	STR_MENU("Next Keyframe")
	STR_DISP("Next Keyframe")
	STR_HELP("Seek to the next keyframe.")

	void operator()(agi::Context *c) {
		std::vector<int> const& kf = c->videoController->GetKeyFrames();
		std::vector<int>::const_iterator pos = lower_bound(kf.begin(), kf.end(), c->videoController->GetFrameN());
		if (pos != kf.end()) ++pos;

		c->videoController->JumpToFrame(pos == kf.end() ? c->videoController->GetFrameN() - 1 : *pos);
	}
};

/// Fast jump forward
struct video_frame_next_large : public Command {
	CMD_NAME("video/frame/next/large")
	STR_MENU("Fast jump forward")
	STR_DISP("Fast jump forward")
	STR_HELP("Fast jump forward.")

	void operator()(agi::Context *c) {
		c->videoController->JumpToFrame(
			c->videoController->GetFrameN() +
			OPT_GET("Video/Slider/Fast Jump Step")->GetInt());
	}
};

/// Seek to the previous frame.
struct video_frame_prev : public Command {
	CMD_NAME("video/frame/prev")
	STR_MENU("Previous Frame")
	STR_DISP("Previous Frame")
	STR_HELP("Seek to the previous frame.")

	void operator()(agi::Context *c) {
		c->videoController->PrevFrame();
	}
};

/// Seek to the previous subtitle boundary.
struct video_frame_prev_boundary : public Command {
	CMD_NAME("video/frame/prev/boundary")
	STR_MENU("Previous Boundary")
	STR_DISP("Previous Boundary")
	STR_HELP("Seek to the previous subtitle boundary.")

	void operator()(agi::Context *c) {
		AssDialogue *active_line = c->selectionController->GetActiveLine();
		if (!active_line) return;

		int target = c->videoController->FrameAtTime(active_line->End.GetMS(), agi::vfr::END);
		if (target < c->videoController->GetFrameN()) {
			c->videoController->JumpToFrame(target);
			return;
		}

		target = c->videoController->FrameAtTime(active_line->Start.GetMS(), agi::vfr::START);
		if (target < c->videoController->GetFrameN()) {
			c->videoController->JumpToFrame(target);
			return;
		}

		c->selectionController->PrevLine();
		AssDialogue *new_line = c->selectionController->GetActiveLine();
		if (new_line != active_line)
			c->videoController->JumpToTime(new_line->End.GetMS(), agi::vfr::END);
	}
};

/// Seek to the previous keyframe.
struct video_frame_prev_keyframe : public Command {
	CMD_NAME("video/frame/prev/keyframe")
	STR_MENU("Previous Keyframe")
	STR_DISP("Previous Keyframe")
	STR_HELP("Seek to the previous keyframe.")

	void operator()(agi::Context *c) {
		int frame = c->videoController->GetFrameN();
		std::vector<int> const& kf = c->videoController->GetKeyFrames();
		std::vector<int>::const_iterator pos = lower_bound(kf.begin(), kf.end(), frame);

		if (frame != 0 && (pos == kf.end() || *pos == frame))
			--pos;

		c->videoController->JumpToFrame(*pos);
	}
};

/// Fast jump backwards
struct video_frame_prev_large : public Command {
	CMD_NAME("video/frame/prev/large")
	STR_MENU("Fast jump backwards")
	STR_DISP("Fast jump backwards")
	STR_HELP("Fast jump backwards")

	void operator()(agi::Context *c) {
		c->videoController->JumpToFrame(
			c->videoController->GetFrameN() -
			OPT_GET("Video/Slider/Fast Jump Step")->GetInt());
	}
};

/// Jump to frame or time.
struct video_jump : public Command {
	CMD_NAME("video/jump")
	STR_MENU("&Jump to..")
	STR_DISP("Jump to")
	STR_HELP("Jump to frame or time.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		if (c->videoController->IsLoaded()) {
			DialogJumpTo(c).ShowModal();
			c->videoBox->videoSlider->SetFocus();
		}
	}
};


/// Jumps the video to the end frame of current subtitle.
struct video_jump_end : public Command {
	CMD_NAME("video/jump/end")
	STR_MENU("Jump Video to End")
	STR_DISP("Jump Video to End")
	STR_HELP("Jumps the video to the end frame of current subtitle.")

	void operator()(agi::Context *c) {
		c->subsGrid->SetVideoToSubs(false);
	}
};


/// Jumps the video to the start frame of current subtitle.
struct video_jump_start : public Command {
	CMD_NAME("video/jump/start")
	STR_MENU("Jump Video to Start")
	STR_DISP("Jump Video to Start")
	STR_HELP("Jumps the video to the start frame of current subtitle.")

	void operator()(agi::Context *c) {
		c->subsGrid->SetVideoToSubs(true);
	}
};


/// Opens a video file.
struct video_open : public Command {
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
			c->videoController->SetVideo(filename);
			OPT_SET("Path/Last/Video")->SetString(STD_STR(filename));
		}
	}
};


/// Opens a video clip with solid colour.
struct video_open_dummy : public Command {
	CMD_NAME("video/open/dummy")
	STR_MENU("Use Dummy Video...")
	STR_DISP("Use Dummy Video")
	STR_HELP("Opens a video clip with solid colour.")

	void operator()(agi::Context *c) {
		wxString fn;
		if (DialogDummyVideo::CreateDummyVideo(c->parent, fn)) {
			c->videoController->SetVideo(fn);
		}
	}
};

/// Toggle autoscrolling video when the active line changes
struct video_opt_autoscroll : public Command {
	CMD_NAME("video/opt/autoscroll")
	STR_MENU("Toggle autoscroll of video")
	STR_DISP("Toggle autoscroll of video")
	STR_HELP("Toggle autoscroll of video")

	void operator()(agi::Context *c) {
		OPT_SET("Video/Subtitle Sync")->SetBool(!OPT_GET("Video/Subtitle Sync")->GetBool());
	}
};

/// Play video.
struct video_play : public Command {
	CMD_NAME("video/play")
	STR_MENU("Play")
	STR_DISP("Play")
	STR_HELP("Play video starting on this position")

	void operator()(agi::Context *c) {
		c->videoController->Play();
	}
};

/// Play video for the active line.
struct video_play_line : public Command {
	CMD_NAME("video/play/line")
	STR_MENU("Play line")
	STR_DISP("Play line")
	STR_HELP("Play current line")

	void operator()(agi::Context *c) {
		c->videoController->PlayLine();
	}
};

/// Show a mask over the video.
struct video_show_overscan : public Command {
	CMD_NAME("video/show_overscan")
	STR_MENU("Show Overscan Mask")
	STR_DISP("Show Overscan Mask")
	STR_HELP("Show a mask over the video, indicating areas that might get cropped off by overscan on televisions.")

	void operator()(agi::Context *c) {
//XXX: Fix to not require using an event. (maybe)
//		OPT_SET("Video/Overscan Mask")->SetBool(event.IsChecked());
		c->videoController->Stop();
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
		c->videoController->Stop();
		c->videoBox->videoDisplay->SetZoom(1.);
	}
};

/// Stop video playback
class video_stop: public Command {
public:
	CMD_NAME("video/stop")
	STR_MENU("Stop video")
	STR_DISP("Stop video")
	STR_HELP("Stop video playback")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
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
		c->videoController->Stop();
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
		c->videoController->Stop();
		c->videoBox->videoDisplay->SetZoom(.5);
	}
};


/// Zoom video in.
struct video_zoom_in : public Command {
	CMD_NAME("video/zoom/in")
	STR_MENU("Zoom In")
	STR_DISP("Zoom In")
	STR_HELP("Zoom video in.")

	void operator()(agi::Context *c) {
		c->videoBox->videoDisplay->SetZoom(c->videoBox->videoDisplay->GetZoom() + .125);
	}
};


/// Zoom video out.
struct video_zoom_out : public Command {
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
	cm->reg(new video_aspect_cinematic);
	cm->reg(new video_aspect_custom);
	cm->reg(new video_aspect_default);
	cm->reg(new video_aspect_full);
	cm->reg(new video_aspect_wide);
	cm->reg(new video_close);
	cm->reg(new video_detach);
	cm->reg(new video_details);
	cm->reg(new video_focus_seek);
	cm->reg(new video_frame_next);
	cm->reg(new video_frame_next_boundary);
	cm->reg(new video_frame_next_keyframe);
	cm->reg(new video_frame_next_large);
	cm->reg(new video_frame_prev);
	cm->reg(new video_frame_prev_boundary);
	cm->reg(new video_frame_prev_keyframe);
	cm->reg(new video_frame_prev_large);
	cm->reg(new video_jump);
	cm->reg(new video_jump_end);
	cm->reg(new video_jump_start);
	cm->reg(new video_open);
	cm->reg(new video_open_dummy);
	cm->reg(new video_opt_autoscroll);
	cm->reg(new video_play);
	cm->reg(new video_play_line);
	cm->reg(new video_show_overscan);
	cm->reg(new video_stop);
	cm->reg(new video_zoom_100);
	cm->reg(new video_zoom_200);
	cm->reg(new video_zoom_50);
	cm->reg(new video_zoom_in);
	cm->reg(new video_zoom_out);
}

} // namespace cmd
