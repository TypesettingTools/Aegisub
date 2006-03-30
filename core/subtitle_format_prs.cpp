// Copyright (c) 2006, Rodrigo Braz Monteiro
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
#include <wx/image.h>
#include "subtitle_format_prs.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "avisynth_wrap.h"
#include "video_box.h"
#include "video_display.h"
#include "video_provider.h"
#include "main.h"
#include "frame_main.h"
#include "vfr.h"
#include "../prs/prs_file.h"
#include "../prs/prs_image.h"
#include "../prs/prs_display.h"


//////////////////////
// Can write to file?
bool PRSSubtitleFormat::CanWriteFile(wxString filename) {
#ifdef __WINDOWS__
	return (filename.Right(4).Lower() == _T(".prs"));
#else
	return false;
#endif
}


//////////////
// Write file
void PRSSubtitleFormat::WriteFile(wxString filename,wxString encoding) {
#ifdef __WINDOWS__
	// Video loaded?
	VideoDisplay *display = ((AegisubApp*)wxTheApp)->frame->videoBox->videoDisplay;

	// Create file
	PRSFile file;

	// Open two Avisynth environments
	AviSynthWrapper avs1,avs2;
	IScriptEnvironment *env1 = avs1.GetEnv();
	IScriptEnvironment *env2 = avs2.GetEnv();

	// Prepare environments
	wxString val = wxString::Format(_T("BlankClip(pixel_type=\"RGB32\",length=%i,width=%i,height=%i,fps=%f"),display->provider->GetFrameCount(),display->provider->GetSourceWidth(),display->provider->GetSourceHeight(),display->provider->GetFPS());
	AVSValue script1 = env1->Invoke("Eval",AVSValue(wxString(val + _T(",color=$000000)")).mb_str(wxConvUTF8)));
	AVSValue script2 = env2->Invoke("Eval",AVSValue(wxString(val + _T(",color=$FFFFFF)")).mb_str(wxConvUTF8)));
	char temp[512];
	strcpy(temp,display->GetTempWorkFile().mb_str(wxConvLocal));
	AVSValue args1[2] = { script1.AsClip(), temp };
	AVSValue args2[2] = { script2.AsClip(), temp };
	try {
		script1 = env1->Invoke("TextSub", AVSValue(args1,2));
		script2 = env2->Invoke("TextSub", AVSValue(args2,2));
	}
	catch (AvisynthError &err) {
		throw _T("AviSynth error: ") + wxString(err.msg,wxConvLocal);
	}

	// Loop through subtitles in file
	AssFile *ass = AssFile::top;
	AssDialogue *diag = NULL;
	PClip clip1 = script1.AsClip();
	PClip clip2 = script2.AsClip();
	int id = 0;
	for (entryIter cur=ass->Line.begin();cur!=ass->Line.end();cur++) {
		diag = AssEntry::GetAsDialogue(*cur);

		// Dialogue found
		if (diag) {
			// Read its image
			int framen = VFR_Output.GetFrameAtTime(diag->Start.GetMS(),true);
			PVideoFrame frame1 = clip1->GetFrame(framen,env1);
			PVideoFrame frame2 = clip2->GetFrame(framen,env2);

			// Convert to PNG
			wxImage bmp = CalculateAlpha(frame1->GetReadPtr(),frame2->GetReadPtr(),frame1->GetRowSize(),frame1->GetHeight(),frame1->GetPitch());
			bmp.SaveFile(filename + wxString::Format(_T("%i.png"),id),wxBITMAP_TYPE_PNG);

			// Create PRSImage
			PRSImage *img = new PRSImage;
			img->id = id;
			img->dataLen = 0;
			img->data = NULL;

			// Create PRSDisplay
			PRSDisplay *display = new PRSDisplay;
			display->start = diag->Start.GetMS();
			display->end = diag->End.GetMS();
			display->id = id;
			display->x = 0;
			display->y = 0;
			display->alpha = 255;
			display->blend = 0;

			// Insert into list
			file.AddEntry(img);
			file.AddEntry(display);
			id++;
		}
	}

	// Save file
	file.Save(filename.mb_str(wxConvLocal));
#endif
}


//////////////////////////////////////////////
// Generates a 32-bit wxImage from two frames
// ------------------------------------------
// Frame 1 should have the image on a BLACK background
// Frame 2 should have the same image on a WHITE background
wxImage PRSSubtitleFormat::CalculateAlpha(const unsigned char* frame1, const unsigned char* frame2, int w, int h, int pitch) {
	//// Allocate image data
	//unsigned char *data = (unsigned char*) malloc(sizeof(unsigned char)*w*h*4);

	//// Pointers
	//const unsigned char *src1 = frame1;
	//const unsigned char *src2 = frame2;
	//unsigned char *dst = data + ((h-1)*w);

	//// Process
	//int r1,g1,b1,r2,g2,b2;
	//int r,g,b,a;
	//for (int y=0;y<h;y++) {
	//	for (int x=0;x<w;x+=4) {
	//		// Read pixels
	//		b1 = *(src1++);
	//		b2 = *(src2++);
	//		g1 = *(src1++);
	//		g2 = *(src2++);
	//		r1 = *(src1++);
	//		r2 = *(src2++);
	//		src1++;
	//		src2++;

	//		// Calculate new values
	//		a = 255 + r1 - r2;
	//		if (a == 0) {
	//			r = 0;
	//			g = 0;
	//			b = 0;
	//		}
	//		else {
	//			r = r1*255 / a;
	//			g = g1*255 / a;
	//			b = b1*255 / a;
	//		}

	//		// Write to destination
	//		*(dst++) = b;
	//		*(dst++) = g;
	//		*(dst++) = r;
	//		*(dst++) = a;
	//	}

	//	// Roll back dst
	//	dst -= 2*w;
	//}

	//// Create the actual image and return it
	////return wxImage(w/4,h,data,false);
	//wxBitmap bmp ((const char*)data,w/4,h,32);
	//wxImage img = bmp.ConvertToImage();
	//if (img.HasAlpha()) wxLogMessage(_T("Has alpha"));
	//else wxLogMessage(_T("oshit."));
	//return img;


	// Allocate image data
	unsigned char *data = (unsigned char*) malloc(sizeof(unsigned char)*w*h*3);
	unsigned char *alpha = (unsigned char*) malloc(sizeof(unsigned char)*w*h);

	// Pointers
	const unsigned char *src1 = frame1;
	const unsigned char *src2 = frame2;
	unsigned char *dst = data + ((h-1)*w*3/4);
	unsigned char *dsta = alpha + ((h-1)*w/4);
	int mina = 255;
	int maxa = 0;

	// Process
	int r1,g1,b1,r2,g2,b2;
	int r,g,b,a;
	for (int y=0;y<h;y++) {
		for (int x=0;x<w;x+=4) {
			// Read pixels
			b1 = *(src1++);
			b2 = *(src2++);
			g1 = *(src1++);
			g2 = *(src2++);
			r1 = *(src1++);
			r2 = *(src2++);
			src1++;
			src2++;

			// Calculate new values
			a = 255 + r1 - r2;
			if (a == 0) {
				r = 0;
				g = 0;
				b = 0;
			}
			else {
				r = r1*255 / a;
				g = g1*255 / a;
				b = b1*255 / a;
			}

			// Write to destination
			*(dst++) = r;
			*(dst++) = g;
			*(dst++) = b;
			*(dsta++) = a;

			if (a > maxa) maxa = a;
			if (a < mina) mina = a;
		}

		// Roll back dst
		dst -= w*3/2;
		dsta -= w/2;
	}

	wxLogMessage(wxString::Format(_T("Min: %i, Max: %i"),mina,maxa));

	// Create the actual image and return it
	wxImage img(w/4,h,data,false);
	img.SetAlpha(alpha,false);
	return img;
}
