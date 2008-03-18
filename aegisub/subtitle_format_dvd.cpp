// Copyright (c) 2008, Rodrigo Braz Monteiro
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
#include "subtitle_format_dvd.h"
#include "video_provider_dummy.h"
#include "subtitles_provider_manager.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#ifdef _OPENMP
#include <omp.h>
#endif
#include <wx/file.h>


///////////////
// Format name
wxString DVDSubtitleFormat::GetName() {
	return _T("DVD Subpictures");
}


//////////////
// Extensions
wxArrayString DVDSubtitleFormat::GetWriteWildcards() {
	wxArrayString results;
	results.Add(_T("sup"));
	return results;
}


/////////////
// Can write
bool DVDSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Lower().EndsWith(_T(".sup")));
}


///////////////////////
// Get subpicture list
void DVDSubtitleFormat::GetSubPictureList(std::vector<SubPicture> &pics) {
	// Create video frame
	int w = 720;
	int h = 480;
	VideoProvider *video = new DummyVideoProvider(10,1,w,h,wxColour(255,0,0),false);
	AegiVideoFrame srcFrame = video->GetFrame(0,FORMAT_RGB32);
	delete video;

	// Count and index lines
	using std::list;
	int count = 0;
	std::vector<AssDialogue*> diags;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = AssEntry::GetAsDialogue(*cur);
		if (current) {
			diags.push_back(current);
			count++;
		}
	}
	pics.resize(count);

	SubtitlesProvider *provider = NULL;
	provider = SubtitlesProviderFactoryManager::GetProvider();
	provider->LoadSubtitles(GetAssFile());

	// Write lines
	int i;
#ifdef _OPENMP
	#pragma omp parallel for shared(diags,pics,provider) private(i)
#endif
	for (i=0;i<count;i++) {
		// Dialogue
		AssDialogue *current = diags[i];

		// Get the image
		AegiVideoFrame dst;
		dst.CopyFrom(srcFrame);
		double time = (current->Start.GetMS()/1000.0 + current->End.GetMS()/1000.0)/2.0;
#ifdef _OPENMP
		#pragma omp critical
#endif
		{
			provider->DrawSubtitles(dst,time);
		}
		wxImage img = dst.GetImage();
		dst.Clear();

		// Perform colour reduction on image
		unsigned char r,g,b;
		unsigned char *data = img.GetData();
		const unsigned char *dataRead = data;
		unsigned char *dataWrite = data;
		int startY = 0;
		int endY = 0;
		int startX = w;
		int endX = 0;

		// For each line
		for (int y=h;--y>=0;) {
			bool hasData = false;
			int lineStartX = 0;
			int lineEndX = 0;

			// Scan line
			for (int x=w;--x>=0;) {
				// Read
				r = *(dataRead++);
				g = *(dataRead++);
				b = *(dataRead++);

				// Background
				if (r > 127 && g < 20) {
					r = 200;
					g = 0;
					b = 0;
				}

				// Text
				else {
					// Mark coordinates
					hasData = true;
					if (lineStartX == 0) lineStartX = w-x-1;
					lineEndX = w-x-1;

					// Set colour
					if (r > 170 && g > 170) {
						r = 255;
						g = 255;
						b = 255;
					}
					else if (r > 85 && g > 85) {
						r = 127;
						g = 127;
						b = 127;
					}
					else {
						r = 0;
						g = 0;
						b = 0;
					}
				}

				// Write
				*(dataWrite++) = r;
				*(dataWrite++) = g;
				*(dataWrite++) = b;
			}

			// Mark as last found so far
			if (hasData) {
				if (startY == 0) startY = h-y-1;
				endY = h-y-1;
				if (lineStartX < startX) startX = lineStartX;
				if (lineEndX > endX) endX = lineEndX;
			}
		}
		
		// Save image data
		if (startX > endX) endX = startX;
		if (startY > endY) endY = startY;
		int sw = endX-startX+1;
		int sh = endY-startY+1;
		pics[i].x = startX;
		pics[i].y = startY;
		pics[i].w = sw;
		pics[i].h = sh;
		pics[i].start = current->Start.GetMS();
		pics[i].end = current->End.GetMS();

		// RLE to memory
		for (int j=0;j<2;j++) {
			int curCol = -1;
			int col;
			int temp;
			int len = 0;
			//wxImage subPic = img.GetSubImage(wxRect(startX,startY,sw,sh));
			dataRead = data + ((startY+j)*w+startX)*3;
			//dataRead = subPic.GetData();
			std::vector<RLEGroup> groups;
			groups.reserve(1024);

			// Read this scanline
			for (int y=startY+j;y<=endY;y+=2) {
				for (int x=startX;x<=endX;x++) {
					// Read current pixel colour
					temp = *dataRead;
					if (temp == 200) col = 0;
					else if (temp == 255) col = 1;
					else if (temp == 0) col = 2;
					else col = 3;

					// See if it matches
					if (col == curCol) {
						len++;
						if (len == 255) {
							groups.push_back(RLEGroup(curCol,len,false));
							len = 0;
						}
					}
					else {
						if (len) groups.push_back(RLEGroup(curCol,len,false));
						len = 1;
						curCol = col;
					}

					dataRead += 3;
				}

				// Flush
				if (len) groups.push_back(RLEGroup(curCol,0,true));
				else {
					groups.back().len = 0;
					groups.back().eol = true;
				}
				curCol = -1;
				len = 0;

				// Advance
				dataRead += (2*w-sw)*3;
				//dataRead += sw*3;
			}

			// Encode into subpicture format
			int nibble[4];
			nibble[3] = 0;
			bool off = false;
			std::vector<unsigned char> &data = pics[i].data[j];
			unsigned char last = 0;
			for (size_t m=0;m<groups.size();m++) {
				unsigned char len = groups[m].len;
				int nibbles;

				// End of line, write b000000cc
				if (groups[m].eol) nibbles = 4;

				// Get proper nibble count
				else {
					if (len < 4) nibbles = 1;
					else if (len < 16) nibbles = 2;
					else if (len < 64) nibbles = 3;
					else nibbles = 4;
				}

				// Write nibbles
				nibble[0] = groups[m].col | ((len & 0x3) << 2);
				nibble[1] = (len & 0x3C) >> 2;
				nibble[2] = (len & 0xC0) >> 6;
				for (int n=nibbles;--n>=0;) {
					wxASSERT(nibble[n] >= 0 && nibble[n] < 16);
					wxASSERT(n >= 0 && n < 4);
					if (!off) {
						last = nibble[n] << 4;
						data.push_back(last);
					}
					else data.back() = nibble[n] | last;
					off = !off;

					// Check if just wrote end of line
					if (len == 0 && n == 0) {
						last = 0;
						off = false;
					}
				}
			}
			data.resize(data.size());
		}
	}

	// Clean up
	delete provider;
	srcFrame.Clear();
}


///////////////////////
// Actually write them
void DVDSubtitleFormat::WriteFile(wxString filename,wxString encoding) {
	// Prepare subtitles
	CreateCopy();
	SortLines();
	Merge(true,true,true,false);

	// Get subpictures
	std::vector<SubPicture> pics;
	GetSubPictureList(pics);

	// Open file for writing
	wxFile fp(filename,wxFile::write);
	if (!fp.IsOpened()) throw _T("Could not open file for writing.");

	// Write each subpicture
	size_t pos = 0;
	for (size_t i=0;i<pics.size();i++) {
		// Write sup packet data
		pos += fp.Write("SP",2);
		wxUint32 temp = wxUINT32_SWAP_ON_BE(pics[i].start * 90);
		pos += fp.Write(&temp,4);
		temp = 0;
		pos += fp.Write(&temp,4);

		// Calculate lengths
		size_t controlLen = 30;
		size_t packetLen = 4 + pics[i].data[0].size() + pics[i].data[1].size() + controlLen;
		size_t packetStart = pos;

		// Write position of the next packet and control
		unsigned short temp2;
		temp2 = wxUINT16_SWAP_ON_LE(packetLen);
		pos += fp.Write(&temp2,2);
		temp2 = wxUINT16_SWAP_ON_LE(packetLen-controlLen);
		pos += fp.Write(&temp2,2);

		// Write RLE data
		size_t line0pos = pos - packetStart;
		pos += fp.Write(&pics[i].data[0][0],pics[i].data[0].size());
		size_t line1pos = pos - packetStart;
		pos += fp.Write(&pics[i].data[1][0],pics[i].data[1].size());

		// Control group data
		size_t comm2add = packetLen - 4;
		unsigned char comm2_b1 = (comm2add & 0xFF00) >> 8;
		unsigned char comm2_b2 = comm2add & 0xFF;
		unsigned char pix0_b1 = (line0pos & 0xFF00) >> 8;
		unsigned char pix0_b2 = line0pos & 0xFF;
		unsigned char pix1_b1 = (line1pos & 0xFF00) >> 8;
		unsigned char pix1_b2 = line1pos & 0xFF;
		int delay = (pics[i].end - pics[i].start)*90/1024;
		unsigned char delay_b1 = (delay & 0xFF00) >> 8;
		unsigned char delay_b2 = delay & 0xFF;
		int sx = pics[i].x;
		int sy = pics[i].y;
		int ex = pics[i].w + sx - 1;
		int ey = pics[i].h + sy - 1;
		unsigned char dispx_b1 = (sx & 0xFF0) >> 4;
		unsigned char dispx_b2 = ((sx & 0x0F) << 4) | ((ex & 0xF00) >> 8);
		unsigned char dispx_b3 = (ex & 0xFF);
		unsigned char dispy_b1 = (sy & 0xFF0) >> 4;
		unsigned char dispy_b2 = ((sy & 0x0F) << 4) | ((ey & 0xF00) >> 8);
		unsigned char dispy_b3 = (ey & 0xFF);

		// Write control group
		unsigned char control[] = {
			0x00, 0x00,			// Delay
			comm2_b1, comm2_b2,	// Next command
			0x03, 0x01, 0x23,	// Set colours
			0x04, 0x0F, 0xFF,	// Alpha blend
			0x05, dispx_b1, dispx_b2, dispx_b3, dispy_b1, dispy_b2, dispy_b3, // Display area
			0x06, pix0_b1, pix0_b2, pix1_b1, pix1_b2, // Pixel pointers
			0x01,				// Start display
			0xFF,				// End block 1
			delay_b1, delay_b2,	// Delay
			comm2_b1, comm2_b2,	// This command
			0x02,				// Stop display
			0xFF				// End
		};
		pos += fp.Write(control,controlLen);
	}
}
