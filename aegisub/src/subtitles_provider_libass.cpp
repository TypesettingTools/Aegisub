// Copyright (c) 2006-2007, Rodrigo Braz Monteiro, Evgeniy Stepanov
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
// Contact: mailto:zeratul@cellosoft.com
//

///////////
// Headers
#include "config.h"


#ifdef WITH_LIBASS

#include "subtitles_provider_libass.h"
#include "ass_file.h"
#include "video_context.h"
#include "utils.h"
#include "standard_paths.h"
#include <wx/filefn.h>
#include <wx/thread.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/utils.h>

#ifdef __APPLE__
extern "C" {
#include "libosxutil/libosxutil.h"
#include <sys/param.h>
}
#endif



DEFINE_EVENT_TYPE(AEGISUB_LIBASS_FONTUPDATE_FINISH)

class AegisubLibassFontupdateFinishEvent : public wxEvent {
public:
	AegisubLibassFontupdateFinishEvent()
		: wxEvent(0, AEGISUB_LIBASS_FONTUPDATE_FINISH)
	{ }
	virtual wxEvent *Clone() const {
		return new AegisubLibassFontupdateFinishEvent(*this);
	}
};

class LibassFontUpdateThread : public wxThread {
	wxEvtHandler *completion_event_handler;
	ASS_Renderer *renderer;

public:
	LibassFontUpdateThread(wxEvtHandler *completion_event_handler, ASS_Renderer *renderer)
		: wxThread(wxTHREAD_JOINABLE)
		, completion_event_handler(completion_event_handler)
		, renderer(renderer)
	{
		Create();
		Run();
	}

	ExitCode Entry() {
		ass_fonts_update(renderer);
		completion_event_handler->AddPendingEvent(AegisubLibassFontupdateFinishEvent());
		return 0;
	}
};


class LibassFontUpdateStatusDialog : public wxDialog {
	void OnDoneUpdating(AegisubLibassFontupdateFinishEvent &evt) {
		Destroy();
	}
	
	void OnTimer(wxTimerEvent &evt) {
		Show();
	}
	
	wxTimer timer;

public:
	LibassFontUpdateStatusDialog()
		: wxDialog(0, -1, _T(""), wxDefaultPosition, wxDefaultSize, 0)
		, timer(this, -1)
	{
		wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

		main_sizer->Add(new wxStaticText(this, -1, _("Please wait, caching fonts...")), 0, wxEXPAND|wxALL, 12);

		wxGauge *gauge = new wxGauge(this, -1, 1, wxDefaultPosition, wxSize(350, -1));
		gauge->Pulse();
		main_sizer->Add(gauge, 0, wxEXPAND|wxALL&~wxTOP, 12);
		
		Connect(-1, -1, AEGISUB_LIBASS_FONTUPDATE_FINISH,
			(wxObjectEventFunction)&LibassFontUpdateStatusDialog::OnDoneUpdating,
			0, this);
		Connect(timer.GetId(), -1, wxEVT_TIMER,
			(wxObjectEventFunction)&LibassFontUpdateStatusDialog::OnTimer,
			0, this);
		
		timer.Start(500, true);

		SetSizerAndFit(main_sizer);
		CentreOnScreen();
	}
	
	void WaitForUpdatingFinish(wxThread *thread) {
		wxWindowDisabler disabler(this);
		wxApp &app = *wxTheApp;
		while (thread->IsAlive()) {
			while (app.Pending())
				app.Dispatch();
			wxYield();
		}
		thread->Wait();
	}
};



///////////////
// Constructor
LibassSubtitlesProvider::LibassSubtitlesProvider() {
	// Initialize library
	static bool first = true;
	if (first) {
		ass_library = ass_library_init();
		if (!ass_library) throw _T("ass_library_init failed");

		wxString fonts_dir = StandardPaths::DecodePath(_T("?user/libass_fonts/"));
		if (!wxDirExists(fonts_dir))
			// It's only one level below the user dir, and we assume the user dir already exists at this point.
			wxMkdir(fonts_dir);

		ass_set_fonts_dir(ass_library, fonts_dir.mb_str(wxConvFile));
		ass_set_extract_fonts(ass_library, 0);
		ass_set_style_overrides(ass_library, NULL);
		first = false;
	}

	// Initialize renderer
	ass_track = NULL;
	ass_renderer = ass_renderer_init(ass_library);
	if (!ass_renderer) throw _T("ass_renderer_init failed");
	ass_set_font_scale(ass_renderer, 1.);

#ifdef __APPLE__
	char config_path[MAXPATHLEN];
	char *config_dir;

	config_dir = OSX_GetBundleResourcesDirectory();
	snprintf(config_path, MAXPATHLEN, "%s/etc/fonts/fonts.conf", config_dir);
	free(config_dir);
#else
	const char *config_path = NULL;
#endif

	ass_set_fonts(ass_renderer, NULL, "Sans", 1, config_path, 0);

	LibassFontUpdateStatusDialog *fonts_update_dlg = new LibassFontUpdateStatusDialog;
	wxThread * update_thread = new LibassFontUpdateThread(fonts_update_dlg, ass_renderer);
	fonts_update_dlg->WaitForUpdatingFinish(update_thread);
}


//////////////
// Destructor
LibassSubtitlesProvider::~LibassSubtitlesProvider() {
}


//////////////////
// Load subtitles
void LibassSubtitlesProvider::LoadSubtitles(AssFile *subs) {
	// Prepare subtitles
	std::vector<char> data;
	subs->SaveMemory(data,_T("UTF-8"));
	delete subs;

	// Load file
	if (ass_track) ass_free_track(ass_track);
	ass_track = ass_read_memory(ass_library, &data[0], data.size(),"UTF-8");
	if (!ass_track) throw _T("libass failed to load subtitles.");
}


/////////////////////////////
// Macros to get the colours
#define _r(c)  ((c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>>8)&0xFF)
#define _a(c)  ((c)&0xFF)

//////////////////
// Draw subtitles
void LibassSubtitlesProvider::DrawSubtitles(AegiVideoFrame &frame,double time) {

	// libass doesn't like null tracks.
	if (!ass_track) return;

	// Set size
	ass_set_frame_size(ass_renderer, frame.w, frame.h);

	// Get frame
	ASS_Image* img = ass_render_frame(ass_renderer, ass_track, int(time * 1000), NULL);

	// libass actually returns several alpha-masked monochrome images.
	// Here, we loop through their linked list, get the colour of the current, and blend into the frame.
	// This is repeated for all of them.
	while (img) {
		// Get colours
		unsigned int opacity = 255 - ((unsigned int)_a(img->color));
		unsigned int r = (unsigned int)_r(img->color);
		unsigned int g = (unsigned int)_g(img->color);
		unsigned int b = (unsigned int) _b(img->color);

		// Prepare copy
		int src_stride = img->stride;
		int dst_stride = frame.pitch[0];
		int dst_delta = dst_stride - img->w*4;
		//int stride = MIN(src_stride,dst_stride);
		const unsigned char *src = img->bitmap;
		unsigned char *dst = frame.data[0] + (img->dst_y * dst_stride + img->dst_x * 4);
		unsigned int k,ck,t;

		// Copy image to destination frame
		for (int y=0;y<img->h;y++) {
			//memcpy(dst,src,stride);
			for (int x = 0; x < img->w; ++x) {
				k = ((unsigned)src[x]) * opacity / 255;
				ck = 255 - k;
				t = *dst;
				*dst++ = (k*b + ck*t) / 255;
				t = *dst;
				*dst++ = (k*g + ck*t) / 255;
				t = *dst;
				*dst++ = (k*r + ck*t) / 255;
				dst++;
			}

			dst += dst_delta;
			src += src_stride;
		}

		// Next image
		img = img->next;
	}
}


//////////
// Static
ASS_Library* LibassSubtitlesProvider::ass_library;


#endif // WITH_LIBASS
