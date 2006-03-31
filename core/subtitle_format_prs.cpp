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
#include <wx/mstream.h>
#include "subtitle_format_prs.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_override.h"
#include "avisynth_wrap.h"
#include "video_box.h"
#include "video_display.h"
#include "video_provider.h"
#include "main.h"
#include "frame_main.h"
#include "vfr.h"
#include "utils.h"
#include "md5.h"
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

	// Get range
	std::vector<int> frames = GetFrameRanges();

	// Render all frames that were detected to contain subtitles
	PClip clip1 = script1.AsClip();
	PClip clip2 = script2.AsClip();
	int totalFrames = frames.size();
	int id = 0;
	PRSDisplay *lastDisplay = NULL;
	for (int framen=0;framen<totalFrames;framen++) {
		// Render it?
		if (frames[framen] == 0) continue;

		// Read its image
		PVideoFrame frame1 = clip1->GetFrame(framen,env1);
		PVideoFrame frame2 = clip2->GetFrame(framen,env2);

		// Convert to PNG (the block is there to force it to dealloc bmp earlier)
		int x=0,y=0;
		wxMemoryOutputStream stream;
		{
			wxImage bmp = CalculateAlpha(frame1->GetReadPtr(),frame2->GetReadPtr(),frame1->GetRowSize(),frame1->GetHeight(),frame1->GetPitch(),&x,&y);
			if (!bmp.Ok()) continue;
			bmp.SaveFile(stream,wxBITMAP_TYPE_PNG);
			//bmp.SaveFile(filename + wxString::Format(_T("%i.png"),id),wxBITMAP_TYPE_PNG);
		}

		// Create PRSImage
		PRSImage *img = new PRSImage;
		img->id = id;
		img->dataLen = stream.GetSize();
		img->data = new char[img->dataLen];
		img->imageType = PNG_IMG;
		stream.CopyTo(img->data,img->dataLen);
		
		// Hash the PRSImage data
		md5_state_t state;
		md5_init(&state);
		md5_append(&state,(md5_byte_t*)img->data,img->dataLen);
		md5_finish(&state,(md5_byte_t*)img->md5);

		// Check for duplicates
		PRSImage *dupe = file.FindDuplicateImage(img);
		int useID = id;

		// Dupe found, use that instead
		if (dupe) {
			useID = dupe->id;
			delete img;
			img = NULL;
		}

		// Frame is all OK, add it to file
		else {
			file.AddEntry(img);
			id++;
		}

		// Find start and end times
		int startf = framen;
		while (++framen<totalFrames && frames[framen] == 1);
		int endf = --framen;
		int start = VFR_Output.GetTimeAtFrame(startf,true);
		int end = VFR_Output.GetTimeAtFrame(endf,false);

		// Set blend data
		unsigned char alpha = 255;
		unsigned char blend = 0;

		// Check if it's just an extension of last display
		if (lastDisplay && lastDisplay->id == useID && lastDisplay->endFrame == startf-1 &&
			lastDisplay->x == x && lastDisplay->y == y && lastDisplay->alpha == alpha && lastDisplay->blend == blend)
		{
			lastDisplay->end = start;
			lastDisplay->endFrame = startf;
		}

		// It isn't; needs a new display command
		else {
			// Create PRSDisplay
			PRSDisplay *display = new PRSDisplay;
			display->start = start;
			display->end = end;
			display->startFrame = startf;
			display->endFrame = endf;
			display->id = useID;
			display->x = x;
			display->y = y;
			display->alpha = alpha;
			display->blend = blend;
			lastDisplay = display;

			// Insert into list
			file.AddEntry(display);
		}
	}

	// Save file
	file.Save((const char*)filename.mb_str(wxConvLocal));
#endif
}


////////////////////
// Get frame ranges
std::vector<int> PRSSubtitleFormat::GetFrameRanges() {
	// Loop through subtitles in file
	AssFile *ass = AssFile::top;
	std::vector<int> frames;
	for (entryIter cur=ass->Line.begin();cur!=ass->Line.end();cur++) {
		AssDialogue *diag = AssEntry::GetAsDialogue(*cur);

		// Dialogue found
		if (diag) {
			// Parse tags
			diag->ParseASSTags();

			// Check if there is any animation tag, to flag the line as animated, forcing storage of every frame
			// THIS NEEDS OPTIMIZATION!
			// Currently it will redraw whole line, even if only some time of it is animated.
			// This is later hopefully removed by duplicate checker, but it would be faster if done here
			bool hasAnimation = false;
			int blocks = diag->Blocks.size();
			AssDialogueBlockOverride *block;
			for (int i=0;i<blocks;i++) {
				block = AssDialogueBlock::GetAsOverride(diag->Blocks[i]);
				if (block) {
					// Found an override block, see if it contains any animation tags
					int tags = block->Tags.size();
					for (int j=0;j<tags;j++) {
						wxString tagName = block->Tags[j]->Name;
						if (tagName == _T("\\t") || tagName == _T("\\move") || tagName == _T("\\k") || tagName == _T("\\K") ||
							tagName == _T("\\kf") || tagName == _T("\\ko") || tagName == _T("\\fad") || tagName == _T("\\fade")) {
							hasAnimation = true;
						}
					}
				}
			}

			// Calculate start and end times
			size_t start = VFR_Output.GetFrameAtTime(diag->Start.GetMS(),true);
			size_t end = VFR_Output.GetFrameAtTime(diag->End.GetMS(),false);

			// Ensure that the vector is long enough
			if (frames.size() <= end) frames.resize(end+1);

			// Fill data
			// 2 = Store this frame
			// 1 = Repeat last frame
			// 0 = Don't store this frame
			bool lastOn = false;
			for (size_t i=start;i<=end;i++) {
				// Put a keyframe at the very start, or everywhere if it's animated
				if (i == start || hasAnimation) frames[i] = 2;

				else {
					// Already set to 1 or 2, meaning that another subtitle is here already
					if (frames[i] != 0) lastOn = true;

					// Set to 0, so nothing is here
					else {
						// Just came out of a subtitle end, put a keyframe here
						if (lastOn) {
							frames[i] = 2;
							lastOn = false;
						}

						// Otherwise, just repeat
						frames[i] = 1;
					}
				}
			}

			// Clean up
			diag->ClearBlocks();
		}
	}

	// Done
	return frames;
}





///////////////////////////////////////////////////////////////////////////////////////////////////
// TODO!! MOVE THE TWO FUNCTIONS BELOW INTO A CLASS OF THEIR OWN, THEY MIGHT FIND USE ELSEWHERE. //
// Obvious choice would be the subtitles_rasterizer.h derivation for vsfilter, when that exists. //
///////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////
// Generates a 32-bit wxImage from two frames
// ------------------------------------------
// Frame 1 should have the image on a BLACK background
// Frame 2 should have the same image on a WHITE background
wxImage PRSSubtitleFormat::CalculateAlpha(const unsigned char* frame1, const unsigned char* frame2, int w, int h, int pitch, int *dstx, int *dsty) {
	// Allocate image data
	unsigned char *data = (unsigned char*) malloc(sizeof(unsigned char)*w*h*3);
	unsigned char *alpha = (unsigned char*) malloc(sizeof(unsigned char)*w*h);

	// Pointers
	const unsigned char *src1 = frame1;
	const unsigned char *src2 = frame2;
	unsigned char *dst = data + ((h-1)*w*3/4);
	unsigned char *dsta = alpha + ((h-1)*w/4);

	// Boundaries
	int minx = w;
	int miny = h;
	int maxx = 0;
	int maxy = 0;

	// Process
	unsigned char r1,g1,b1,r2,g2,b2;
	unsigned char r,g,b,a;
	for (int y=h;--y>=0;) {
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
				// Update range
				if (x < minx) minx = x;
				else if (x > maxx) maxx = x;
				if (y < miny) miny = y;
				else if (y > maxy) maxy = y;

				// Calculate colour components
				int mod = MAX(0,128/a-1);
				r = MAX(0,r1-mod)*255 / a;
				g = MAX(0,g1-mod)*255 / a;
				b = MAX(0,b1-mod)*255 / a;
			}

			// Write to destination
			*(dst++) = r;
			*(dst++) = g;
			*(dst++) = b;
			*(dsta++) = a;
		}

		// Roll back dst
		dst -= w*3/2;
		dsta -= w/2;
	}

	// Calculate sizes
	minx /= 4;
	maxx /= 4;
	if (dstx) *dstx = minx;
	if (dsty) *dsty = miny;
	int width = maxx-minx+1;
	int height = maxy-miny+1;
	if (width <= 0 || height <= 0) return wxImage();

	// Create the actual image
	wxImage img(w/4,h,data,false);
	img.SetAlpha(alpha,false);

	// Return subimage
	wxImage subimg = SubImageWithAlpha(img,wxRect(minx,miny,width,height));
	return subimg;
}


////////////////////////////////////////////////
// Creates a sub image preserving alpha channel
// Modified from wx's source
wxImage PRSSubtitleFormat::SubImageWithAlpha (wxImage source,const wxRect &rect) {
    wxImage image;
    wxCHECK_MSG(source.Ok(), image, wxT("invalid image") );
    wxCHECK_MSG((rect.GetLeft()>=0) && (rect.GetTop()>=0) && (rect.GetRight()<=source.GetWidth()) && (rect.GetBottom()<=source.GetHeight()), image, wxT("invalid subimage size") );

    int subwidth=rect.GetWidth();
    const int subheight=rect.GetHeight();

    image.Create(subwidth, subheight, false);

	image.SetAlpha();
    unsigned char *subdata = image.GetData();
	unsigned char *data = source.GetData();
    unsigned char *subalpha = image.GetAlpha();
	unsigned char *alpha = source.GetAlpha();

    wxCHECK_MSG(subdata, image, wxT("unable to create image"));

    const int subleft=3*rect.GetLeft();
    const int width=3*source.GetWidth();
	const int afullwidth=source.GetWidth();
	int awidth = subwidth;
    subwidth*=3;

    data+=rect.GetTop()*width+subleft;
	alpha+=rect.GetTop()*afullwidth+rect.GetLeft();

    for (long j = 0; j < subheight; ++j) {
        memcpy(subdata, data, subwidth);
		memcpy(subalpha, alpha, awidth);
        subdata+=subwidth;
		subalpha+=awidth;
        data+=width;
		alpha+=afullwidth;
    }

    return image;
}
