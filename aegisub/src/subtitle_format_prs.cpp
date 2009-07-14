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
#include "config.h"

#if USE_PRS == 1
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/filename.h>
#include <wx/docview.h>
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
#include "dialog_progress.h"
#include "charset_conv.h"
#include "../prs/prs.h"


//////////////////////
// Can write to file?
bool PRSSubtitleFormat::CanWriteFile(wxString filename) {
#ifdef __WINDOWS__
	return false;
	//return (filename.Right(4).Lower() == _T(".prs"));
#else
	return false;
#endif
}


////////////
// Get name
wxString PRSSubtitleFormat::GetName() {
	return _T("Pre-Rendered Subtitles");
}


///////////////////////
// Get write wildcards
wxArrayString PRSSubtitleFormat::GetWriteWildcards() {
	wxArrayString formats;
	formats.Add(_T("prs"));
	return formats;
}


//////////////
// Write file
void PRSSubtitleFormat::WriteFile(wxString filename,wxString encoding) {
#ifdef __WINDOWS__
	// Video loaded?
	if (VideoContext::Get()->IsLoaded()) throw _T("Video not loaded!");

	// Create the PRS file
	PRSFile file;

	// Create the temporary .ass file
	wxString tempFile1 = wxFileName::CreateTempFileName(_T("aegisub"));
	wxRemoveFile(tempFile1);
	wxString tempFile = tempFile1 + _T(".ass");
	GetAssFile()->Save(tempFile,false,false);

	// Open two Avisynth environments
	AviSynthWrapper avs1,avs2;
	IScriptEnvironment *env1 = avs1.GetEnv();
	IScriptEnvironment *env2 = avs2.GetEnv();

	// Prepare the Avisynth environments, that is, generate blank clips and hardsub into them
	wxString val = wxString::Format(_T("BlankClip(pixel_type=\"RGB32\",length=%i,width=%i,height=%i,fps=%f"),VideoContext::Get()->GetLength(),VideoContext::Get()->GetWidth(),VideoContext::Get()->GetHeight(),VideoContext::Get()->GetFPS());
	AVSValue script1 = env1->Invoke("Eval",AVSValue(wxString(val + _T(",color=$000000)")).mb_str(wxConvUTF8)));
	AVSValue script2 = env2->Invoke("Eval",AVSValue(wxString(val + _T(",color=$FFFFFF)")).mb_str(wxConvUTF8)));
	char temp[512];
	strcpy(temp,tempFile.mb_str(csConvLocal));
	AVSValue args1[2] = { script1.AsClip(), temp };
	AVSValue args2[2] = { script2.AsClip(), temp };
	try {
		script1 = env1->Invoke("TextSub", AVSValue(args1,2));
		script2 = env2->Invoke("TextSub", AVSValue(args2,2));
	}
	catch (AvisynthError &err) {
		throw _T("AviSynth error: ") + wxString(err.msg,csConvLocal);
	}
	PClip clip1 = script1.AsClip();
	PClip clip2 = script2.AsClip();

	// Get range
	std::vector<int> frames = GetFrameRanges();
	int totalFrames = frames.size();
	int toDraw = 0;
	for (int i=0;i<totalFrames;i++) {
		if (frames[i] == 2) toDraw++;
	}

	// Set variables
	id = 0;
	lastDisplay = NULL;
	optimizer = 1;			// 0 = none, 1 = optipng, 2 = pngout

	// Progress
	bool canceled = false;
	DialogProgress *progress = new DialogProgress(NULL,_("Exporting PRS"),&canceled,_("Writing file"),0,toDraw);
	progress->Show();
	progress->SetProgress(0,toDraw);

	// Render all frames that were detected to contain subtitles
	int lastFrameDrawn = 0;
	int drawn = 0;
	for (int framen=0;framen<totalFrames;framen++) {
		// Canceled?
		if (canceled) break;

		// Is this frame supposed to be rendered?
		if (frames[framen] == 0) continue;

		// Update progress
		progress->SetProgress(drawn,toDraw);
		progress->SetText(wxString::Format(_T("Writing PRS file. Line: %i/%i (%.2f%%)"),framen,totalFrames,MIN(float(drawn)*100/toDraw,100.0)));
		if (frames[framen] == 2) drawn++;

		// Read the frame image
		PVideoFrame frame1 = clip1->GetFrame(framen,env1);
		PVideoFrame frame2 = clip2->GetFrame(framen,env2);

		// Prepare to get wxImage
		int x=0,y=0;
		int maxalpha=0;

		// Get wxImage
		wxImage bmp = CalculateAlpha(frame1->GetReadPtr(),frame2->GetReadPtr(),frame1->GetRowSize(),frame1->GetHeight(),frame1->GetPitch(),&x,&y,&maxalpha);
		if (!bmp.Ok()) continue;
		lastFrameDrawn = framen;

		// Get the list of rectangles
		std::vector<wxRect> rects;
		GetSubPictureRectangles(bmp,rects);

		// Add each sub-image to file
		int nrects = rects.size();
		int useFrameN;
		for (int i=0;i<nrects;i++) {
			// Pick either full image or subimage, as appropriate
			wxImage curImage;
			if (rects[i].x == 0 && rects[i].y == 0 && rects[i].width == bmp.GetWidth() && rects[i].height == bmp.GetHeight()) curImage = bmp;
			else curImage = SubImageWithAlpha(bmp,rects[i]);
			if (!curImage.Ok()) continue;

			// Optimize image
			OptimizeImage(curImage);

			// Insert the image
			useFrameN = framen;
			InsertFrame(file,useFrameN,frames,curImage,x+rects[i].x,y+rects[i].y,maxalpha);
		}
		framen = useFrameN;
	}

	// Destroy progress bar
	if (!canceled) progress->Destroy();
	else return;

	// Save file
	file.Save((const char*)filename.mb_str(csConvLocal));
	wxString filename2 = filename + _T(".prsa");
	file.SaveText((const char*)filename2.mb_str(csConvLocal));

	// Delete temp file
	wxRemoveFile(tempFile);
#endif
}


//////////////////////////
// Insert frame into file
void PRSSubtitleFormat::InsertFrame(PRSFile &file,int &framen,std::vector<int> &frames,wxImage &bmp,int x,int y,int maxalpha) {
	// Generic data holder
	size_t datasize = 0;
	char *rawData = NULL;
	std::vector<char> data;
	//bmp.SaveFile(wxString::Format(_T("test_%i.png"),id),wxBITMAP_TYPE_PNG);

	// pngout/optipng optimize
	if (optimizer) {
		// Get temporary filename
		wxString tempFile = wxFileName::CreateTempFileName(_T("aegiprs"));
		wxString tempOut = tempFile + _T("out.png");

		// Prepare arrays to capture output
		wxArrayString output;
		wxArrayString errors;

		// Generate the temporary PNG and run the optimizer program on it
		if (optimizer == 1) {
			bmp.SaveFile(tempOut,wxBITMAP_TYPE_PNG);
			wxExecute(AegisubApp::folderName + _T("optipng.exe -zc9 -zm8 -zs0-3 -f0 ") + tempOut,output,errors);
		}
		if (optimizer == 2) {
			bmp.SaveFile(tempFile,wxBITMAP_TYPE_PNG);
			wxExecute(AegisubApp::folderName + _T("pngout.exe ") + tempFile + _T(" ") + tempOut + _T(" /f0 /y /q"),output,errors);
		}

		// Read file back
		FILE *fp = fopen(tempOut.mb_str(csConvLocal),"rb");
		fseek(fp,0,SEEK_END);
		datasize = ftell(fp);
		data.resize(datasize);
		rawData = &data[0];
		rewind(fp);
		fread(rawData,1,datasize,fp);
		fclose(fp);

		// Destroy temporary files
		wxRemoveFile(tempFile);
		wxRemoveFile(tempOut);
	}

	// No optimization (much faster)
	else {
		// Convert wxImage to PNG directly
		wxMemoryOutputStream stream;
		bmp.SaveFile(stream,wxBITMAP_TYPE_PNG);
		datasize = stream.GetSize();
		data.resize(datasize);
		rawData = &data[0];
		stream.CopyTo(rawData,datasize);
	}

	// Find start and end times
	int startf = framen;
	int totalFrames = frames.size();
	while (++framen<totalFrames && frames[framen] == 1);
	framen--; // need -1, otherwise all sub-images are extended by one frame
	int endf = framen;
	int start = VFR_Output.GetTimeAtFrame(startf,true);
	int end = VFR_Output.GetTimeAtFrame(endf,false);

	// Create PRSImage
	PRSImage *img = new PRSImage;
	img->id = id;
	img->imageType = PNG_IMG;
	img->w = bmp.GetWidth();
	img->h = bmp.GetHeight();
	img->maxAlpha = maxalpha;
	img->dataLen = datasize;
	img->data = new char[img->dataLen];
	memcpy(img->data,rawData,img->dataLen);

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

	// Set blend data
	unsigned char alpha = 255;
	unsigned char blend = 0;

	// Check if it's just an extension of last display
	if (lastDisplay && lastDisplay->id == (unsigned)useID && (signed)lastDisplay->endFrame == startf-1 &&
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


///////////////////////////////////
// Get rectangles of useful glyphs
void PRSSubtitleFormat::GetSubPictureRectangles(wxImage &image,std::vector<wxRect> &rects) {
	// Boundaries
	int w = image.GetWidth();
	int h = image.GetHeight();
	int startx = 0;
	int starty = 0;
	int endx = w-1;
	int endy = h-1;

	// Variables
	bool hasSubImage = false;
	bool isBlankRow = true;
	const unsigned char *data = image.GetAlpha();
	const unsigned char *src = data;
	unsigned char a;

	// For each row
	for (int y=0;y<=h;y++) {
		if (y < h) {
			// Reset row data
			isBlankRow = true;
			if (!hasSubImage) {
				startx = w;
				endx = -1;
			}

			// Check row
			for (int x=0;x<w;x++) {
				a = *src++;
				if (a) {
					isBlankRow = false;
					if (x < startx) startx = x;
					if (x > endx) endx = x;
				}
			}

			// Set sub image status
			if (!isBlankRow && !hasSubImage) {
				starty = y;
				hasSubImage = true;
			}
		}

		// If the processed row is totally blank and there is a subimage, separate them
		if ((isBlankRow && hasSubImage) || y == h) {
			// Insert rectangle
			endy = y-1;
			rects.push_back(wxRect(startx,starty,endx-startx+1,endy-starty+1));
			hasSubImage = false;
		}
	}
}


////////////////////
// Get frame ranges
std::vector<int> PRSSubtitleFormat::GetFrameRanges() {
	// Loop through subtitles in file
	std::vector<int> frames;
	for (entryIter cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *diag = AssEntry::GetAsDialogue(*cur);

		// Dialogue found
		if (diag && !diag->Comment) {
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
			// Yes, +1, this is an optimization for something below
			if (frames.size() <= end+1) frames.resize(end+2);

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

				// Ends right before another "1" block, so make this a "2"
				if (i == end && frames[i+1] == 1) frames[i] = 2;
			}

			// Clean up
			diag->ClearBlocks();
		}
	}

	// Done
	return frames;
}


/////////////////////////////////////////////
// Optimize the image by tweaking the colors
// First, a little macro to help the comparisons down there
#define IN_ERROR_MARGIN(col1,col2,error) ((col1 > col2 ? ((int)(col1-col2)) : ((int)(col2-col1))) <= (error))

// Now, since I don't expect anyone to be able to decypher this...
// This "flood fills" the image based on alpha, to make it easier to compress, without affecting visual quality
// e.g. if you have a pixel with 25% opacity, then a difference of as much as 3 in the color channels won't have a visual impact
void PRSSubtitleFormat::OptimizeImage(wxImage &image) {
	// Get the raw data
	unsigned char *data = (unsigned char*) image.GetData();
	unsigned char *alpha = (unsigned char*) image.GetAlpha();
	int w = image.GetWidth();
	int h = image.GetHeight();
	int len = w*h;

	// Create mask for status and fill with zeroes
	char *status = new char[len];
	for (int i=0;i<len;i++) status[i] = 0;

	// Find highest alpha
	unsigned char highAlpha = 0;
	for (int i=0;i<len;i++) {
		if (status[i] != 2 && alpha[i] > highAlpha) highAlpha = alpha[i];
	}

	// Fill mask of "correct" pixels with 2 on highAlpha pixels
	for (int i=0;i<len;i++) {
		if (alpha[i] == highAlpha) status[i] = 2;
	}

	// Alpha finding loop
	bool outerLoop = true;
	while (outerLoop) {
		// Loop through
		int totalModified = 0;
		bool doRepeat = true;
		while (doRepeat) {
			int modified = 0;
			unsigned char *cur = data;
			unsigned char c1,c2,c3;
			unsigned char d1,d2,d3;
			int error;
			for (int i=0;i<len;i++) {
				// Get colors
				c1 = *(cur++);
				c2 = *(cur++);
				c3 = *(cur++);

				// Check status
				if (status[i] != 0) continue;

				// Get error
				int a = alpha[i];
				if (a == 0) error = 255;
				else error = 1024/a;

				// Right pixel
				if (i != len-1 && status[i+1] == 2) {
					// Get colors
					d1 = *(cur);
					d2 = *(cur+1);
					d3 = *(cur+2);

					// Compare error
					if (IN_ERROR_MARGIN(d1,c1,error) && IN_ERROR_MARGIN(d2,c2,error) && IN_ERROR_MARGIN(d3,c3,error)) {
						*(cur-3) = d1;
						*(cur-2) = d2;
						*(cur-1) = d3;
						status[i] = 2;
						modified++;
						continue;
					}
				}

				// Left pixel
				if (i != 0 && status[i-1] == 2) {
					// Get colors
					d1 = *(cur-6);
					d2 = *(cur-5);
					d3 = *(cur-4);

					// Compare error
					if (IN_ERROR_MARGIN(d1,c1,error) && IN_ERROR_MARGIN(d2,c2,error) && IN_ERROR_MARGIN(d3,c3,error)) {
						*(cur-3) = d1;
						*(cur-2) = d2;
						*(cur-1) = d3;
						status[i] = 2;
						modified++;
						continue;
					}
				}

				// Top pixel
				if (i >= w && status[i-w] == 2) {
					// Get colors
					d1 = *(cur-w*3-3);
					d2 = *(cur-w*3-2);
					d3 = *(cur-w*3-1);

					// Compare error
					if (IN_ERROR_MARGIN(d1,c1,error) && IN_ERROR_MARGIN(d2,c2,error) && IN_ERROR_MARGIN(d3,c3,error)) {
						*(cur-3) = d1;
						*(cur-2) = d2;
						*(cur-1) = d3;
						status[i] = 2;
						modified++;
						continue;
					}
				}

				// Bottom pixel
				if (i < len-w && status[i+w] == 2) {
					// Get colors
					d1 = *(cur+w*3-3);
					d2 = *(cur+w*3-2);
					d3 = *(cur+w*3-1);

					// Compare error
					if (IN_ERROR_MARGIN(d1,c1,error) && IN_ERROR_MARGIN(d2,c2,error) && IN_ERROR_MARGIN(d3,c3,error)) {
						*(cur-3) = d1;
						*(cur-2) = d2;
						*(cur-1) = d3;
						status[i] = 2;
						modified++;
						continue;
					}
				}
			}

			// End repetion
			totalModified += modified;
			if (!modified) doRepeat = false;
		}

		// End outer loop
		if (!totalModified) outerLoop = false;

		// Copy values 1 to 2
		int changes = 0;
		for (int i=0;i<len;i++) {
			if (status[i] == 1) {
				status[i] = 2;
				changes++;
			}
		}
		if (!changes) outerLoop = false;
	}

	// Just for tests, fill alpha with 0xFF
	//for (int i=0;i<len;i++) alpha[i] = 0xFF;

	// Delete mask
	delete [] status;
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
wxImage PRSSubtitleFormat::CalculateAlpha(const unsigned char* frame1, const unsigned char* frame2, int w, int h, int pitch, int *dstx, int *dsty, int *maxalpha) {
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
	int maxA = 0;
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
				//int mod = MAX(0,128/a-1);
				//r = MAX(0,r1-mod)*255 / a;
				//g = MAX(0,g1-mod)*255 / a;
				//b = MAX(0,b1-mod)*255 / a;
				r = r1*255/a;
				g = g1*255/a;
				b = b1*255/a;
			}

			// Write to destination
			*(dst++) = r;
			*(dst++) = g;
			*(dst++) = b;
			*(dsta++) = a;

			// Store maximum alpha
			if (a > maxA) maxA = a;
		}

		// Roll back dst
		dst -= w*3/2;
		dsta -= w/2;
	}

	// Store maximum alpha
	if (maxalpha) *maxalpha = maxA;

	// Calculate sizes
	minx /= 4;
	maxx /= 4;
	if (dstx) *dstx = minx;
	if (dsty) *dsty = miny;
	int width = maxx-minx+1;
	int height = maxy-miny+1;

	// 100% transparent image; clean up and return an empty one
	if (width <= 0 || height <= 0) {
		delete [] data;
		delete [] alpha;
		return wxImage();
	}

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
wxImage PRSSubtitleFormat::SubImageWithAlpha (wxImage &source,const wxRect &rect) {
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

#endif
