// Copyright (c) 2004-2006, Rodrigo Braz Monteiro, Mike Matsnev
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
#include <algorithm>
#include <errno.h>
#include "mkv_wrap.h"
#include "dialog_progress.h"


////////////
// Instance
MatroskaWrapper MatroskaWrapper::wrapper;


///////////
// Defines
#define	CACHESIZE     65536


///////////////
// Constructor
MatroskaWrapper::MatroskaWrapper() {
	file = NULL;
}


//////////////
// Destructor
MatroskaWrapper::~MatroskaWrapper() {
	Close();
}


/////////////
// Open file
void MatroskaWrapper::Open(wxString filename) {
	// Make sure it's closed first
	Close();

	// Open
	char err[2048];
	input = new MkvStdIO(filename);
	if (input->fp) {
		file = mkv_Open(input,err,sizeof(err));

		// Failed parsing
		if (!file) {
			delete input;
			throw wxString(_T("MatroskaParser error: ") + wxString(err,wxConvUTF8)).c_str();
		}

		Parse();
	}

	// Failed opening
	else {
		delete input;
		throw _T("Unable to open Matroska file for parsing.");
	}
}


//////////////
// Close file
void MatroskaWrapper::Close() {
	if (file) {
		mkv_Close(file);
		file = NULL;
		fclose(input->fp);
		delete input;
	}
}


////////////////////
// Return keyframes
wxArrayInt MatroskaWrapper::GetKeyFrames() {
	return keyFrames;
}


///////////////////////
// Comparison operator
bool operator < (MkvFrame &t1, MkvFrame &t2) { 
	return t1.time < t2.time;
}


//////////////////
// Actually parse
void MatroskaWrapper::Parse() {
	// Clear keyframes and timecodes
	keyFrames.Clear();
	bytePos.Clear();
	timecodes.clear();

	// Get info
	int tracks = mkv_GetNumTracks(file);
	TrackInfo *trackInfo;
	SegmentInfo *segInfo = mkv_GetFileInfo(file);

	// Parse tracks
	for (int track=0;track<tracks;track++) {
		trackInfo = mkv_GetTrackInfo(file,track);

		// Video track
		if (trackInfo->Type == 1) {
			// Variables
			ulonglong startTime, endTime, filePos;
			unsigned int rt, frameSize, frameFlags;
			CompressedStream *cs = NULL;

			// Timecode scale
			__int64 timecodeScale = mkv_TruncFloat(trackInfo->TimecodeScale) * segInfo->TimecodeScale;

			// Mask other tracks away
			mkv_SetTrackMask(file, ~(1 << track));

			// Progress bar
			int totalTime = double(segInfo->Duration) / timecodeScale;
			volatile bool canceled = false;
			DialogProgress *progress = new DialogProgress(NULL,_("Parsing Matroska"),&canceled,_("Reading keyframe and timecode data from Matroska file."),0,totalTime);
			progress->Show();
			progress->SetProgress(0,1);

			// Read frames
			int frameN = 0;
			while (mkv_ReadFrame(file,0,&rt,&startTime,&endTime,&filePos,&frameSize,&frameFlags) == 0) {
				// Read value
				double curTime = double(startTime) / 1000000.0;
				frames.push_back(MkvFrame((frameFlags & FRAME_KF) != 0,curTime,filePos));
				frameN++;

				// Cancelled?
				if (canceled) {
					Close();
					throw _T("Canceled");
				}

				// Update progress
				progress->SetProgress(curTime,totalTime);
			}

			// Clean up progress
			if (!canceled) progress->Destroy();

			break;
		}
	}

	// Copy raw
	for (std::list<MkvFrame>::iterator cur=frames.begin();cur!=frames.end();cur++) {
		rawFrames.push_back(*cur);
	}

	// Process timecodes and keyframes
	frames.sort();
	MkvFrame curFrame(false,0,0);
	int i = 0;
	for (std::list<MkvFrame>::iterator cur=frames.begin();cur!=frames.end();cur++) {
		curFrame = *cur;
		if (curFrame.isKey) keyFrames.Add(i);
		bytePos.Add(curFrame.filePos);
		timecodes.push_back(curFrame.time);
		i++;
	}
}


///////////////////////////
// Set target to timecodes
void MatroskaWrapper::SetToTimecodes(FrameRate &target) {
	// Enough frames?
	int frames = timecodes.size();
	if (frames <= 1) return;

	// Sort
	//std::sort<std::vector<double>::iterator>(timecodes.begin(),timecodes.end());

	// Check if it's CFR
	bool isCFR = true;
	double estimateCFR = timecodes.back() / timecodes.size()-1;
	double curTime = 0;
	for (int i=0;i<frames;i++) {
		int delta = int(curTime - timecodes[i]);
		if (abs(delta > 1)) {
			isCFR = false;
			break;
		}
		curTime += estimateCFR;
	}

	// Constant framerate
	if (isCFR) {
		if (abs(estimateCFR - 23.976) < 0.01) estimateCFR = 23.976;
		if (abs(estimateCFR - 29.97) < 0.01) estimateCFR = 29.97;
		target.SetCFR(estimateCFR);
	}

	// Variable framerate
	else {
		std::vector<int> times;
		for (int i=0;i<frames;i++) times.push_back(int(timecodes[i]+0.5));
		target.SetVFR(times);
	}
}





////////////////////////////// LOTS OF HAALI C CODE DOWN HERE ///////////////////////////////////////

///////////////
// STDIO class
int StdIoRead(InputStream *_st, ulonglong pos, void *buffer, int count) {
  MkvStdIO *st = (MkvStdIO *) _st;
  size_t  rd;
  if (fseek(st->fp, pos, SEEK_SET)) {
    st->error = errno;
    return -1;
  }
  rd = fread(buffer, 1, count, st->fp);
  if (rd == 0) {
    if (feof(st->fp))
      return 0;
    st->error = errno;
    return -1;
  }
  return rd;
}

/* scan for a signature sig(big-endian) starting at file position pos
 * return position of the first byte of signature or -1 if error/not found
 */
longlong StdIoScan(InputStream *_st, ulonglong start, unsigned signature) {
  MkvStdIO *st = (MkvStdIO *) _st;
  int	      c;
  unsigned    cmp = 0;
  FILE	      *fp = st->fp;

  if (fseek(fp, start, SEEK_SET))
    return -1;

  while ((c = getc(fp)) != EOF) {
    cmp = ((cmp << 8) | c) & 0xffffffff;
    if (cmp == signature)
      return ftell(fp) - 4;
  }

  return -1;
}

/* return cache size, this is used to limit readahead */
unsigned StdIoGetCacheSize(InputStream *_st) {
  return CACHESIZE;
}

/* return last error message */
const char *StdIoGetLastError(InputStream *_st) {
  MkvStdIO *st = (MkvStdIO *) _st;
  return strerror(st->error);
}

/* memory allocation, this is done via stdlib */
void  *StdIoMalloc(InputStream *_st, size_t size) {
  return malloc(size);
}

void  *StdIoRealloc(InputStream *_st, void *mem, size_t size) {
  return realloc(mem,size);
}

void  StdIoFree(InputStream *_st, void *mem) {
  free(mem);
}

int   StdIoProgress(InputStream *_st, ulonglong cur, ulonglong max) {
  return 1;
}

MkvStdIO::MkvStdIO(wxString filename) {
	read = StdIoRead;
	scan = StdIoScan;
	getcachesize = StdIoGetCacheSize;
	geterror = StdIoGetLastError;
	memalloc = StdIoMalloc;
	memrealloc = StdIoRealloc;
	memfree = StdIoFree;
	progress = StdIoProgress;
	fp = fopen(filename.mb_str(),"rb");
	if (fp) {
		setvbuf(fp, NULL, _IOFBF, CACHESIZE);
	}
}
