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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file mkv_wrap.cpp
/// @brief High-level interface for obtaining various data from Matroska files
/// @ingroup video_input
///

#include "config.h"

#ifndef AGI_PRE
#include <errno.h>

#include <algorithm>
#include <iterator>

#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.
#endif

#include "ass_file.h"
#include "ass_time.h"
#include "dialog_progress.h"
#include <libaegisub/vfr.h>
#include "mkv_wrap.h"

/// DOCME
MatroskaWrapper MatroskaWrapper::wrapper;

/// DOCME
#define	CACHESIZE     65536

/// @brief Constructor 
///
MatroskaWrapper::MatroskaWrapper() {
	file = NULL;
}

/// @brief Destructor 
///
MatroskaWrapper::~MatroskaWrapper() {
	Close();
}

/// @brief Open file 
/// @param filename 
/// @param parse    
///
void MatroskaWrapper::Open(wxString filename,bool parse) {
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

		// Parse
		if (parse) Parse();
	}

	// Failed opening
	else {
		delete input;
		throw _T("Unable to open Matroska file for parsing.");
	}
}

/// @brief Close file 
/// @return 
///
void MatroskaWrapper::Close() {
	if (file) {
		mkv_Close(file);
		file = NULL;
		delete input;
	}
	keyFrames.clear();
	timecodes.clear();
}

/// @brief Return keyframes 
/// @return 
///
std::vector<int> MatroskaWrapper::GetKeyFrames() {
	return keyFrames;
}

/// @brief Comparison operator 
/// @param t1 
/// @param t2 
/// @return 
///
bool operator < (MkvFrame &t1, MkvFrame &t2) { 
	return t1.time < t2.time;
}

/// @brief Actually parse 
///
void MatroskaWrapper::Parse() {
	// Clear keyframes and timecodes
	keyFrames.clear();
	bytePos.Clear();
	timecodes.clear();
	frames.clear();
	rawFrames.clear();
	bytePos.Clear();

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
			//CompressedStream *cs = NULL;

			// Timecode scale
			longlong timecodeScale = mkv_TruncFloat(trackInfo->TimecodeScale) * segInfo->TimecodeScale;

			// Mask other tracks away
			mkv_SetTrackMask(file, ~(1 << track));

			// Progress bar
			int totalTime = int(double(segInfo->Duration) / timecodeScale);
			volatile bool canceled = false;
			DialogProgress *progress = new DialogProgress(NULL,_("Parsing Matroska"),&canceled,_("Reading keyframe and timecode data from Matroska file."),0,totalTime);
			progress->Show();
			progress->SetProgress(0,1);

			// Read frames
			register int frameN = 0;
			while (mkv_ReadFrame(file,0,&rt,&startTime,&endTime,&filePos,&frameSize,&frameFlags) == 0) {
				// Read value
				double curTime = double(startTime) / 1000000.0;
				frames.push_back(MkvFrame((frameFlags & FRAME_KF) != 0,curTime,filePos));
				frameN++;

				// Cancelled?
				if (canceled) {
					Close();
					throw agi::UserCancelException("Canceled");
				}

				// Identical to (frameN % 2048) == 0,
				// but much faster.
				if ((frameN & (2048 - 1)) == 0)
					// Update progress
					progress->SetProgress(int(curTime),totalTime);
			}

			// Clean up progress
			if (!canceled) progress->Destroy();

			break;
		}
	}

	rawFrames.reserve(frames.size());
	std::copy(frames.begin(), frames.end(), std::back_inserter(rawFrames));

	// Process timecodes and keyframes
	frames.sort();
	int i = 0;
	for (std::list<MkvFrame>::iterator cur=frames.begin();cur!=frames.end();cur++) {
		if (cur->isKey) keyFrames.push_back(i);
		bytePos.Add(cur->filePos);
		timecodes.push_back(cur->time);
		i++;
	}
}

static int mkv_round(double num) {
	return (int)(num + .5);
}

/// @brief Set target to timecodes 
/// @param target 
/// @return 
///
void MatroskaWrapper::SetToTimecodes(agi::vfr::Framerate &target) {
	if (timecodes.size() <= 1) return;

	std::vector<int> times;
	times.reserve(timecodes.size());
	std::transform(timecodes.begin(), timecodes.end(), std::back_inserter(times), &mkv_round);
	target = agi::vfr::Framerate(times);
}

/// @brief Get subtitles 
/// @param target 
///
void MatroskaWrapper::GetSubtitles(AssFile *target) {
	// Get info
	int tracks = mkv_GetNumTracks(file);
	TrackInfo *trackInfo;
	//SegmentInfo *segInfo = mkv_GetFileInfo(file);
	wxArrayInt tracksFound;
	wxArrayString tracksNames;
	int trackToRead = -1;

	// Haali's library variables
	ulonglong startTime, endTime, filePos;
	unsigned int rt, frameSize, frameFlags;
	//CompressedStream *cs = NULL;

	// Find tracks
	for (int track=0;track<tracks;track++) {
		trackInfo = mkv_GetTrackInfo(file,track);

		// Subtitle track
		if (trackInfo->Type == 0x11) {
			wxString CodecID = wxString(trackInfo->CodecID,*wxConvCurrent);
			wxString TrackName = wxString(trackInfo->Name,*wxConvCurrent);
			wxString TrackLanguage = wxString(trackInfo->Language,*wxConvCurrent);
			
			// Known subtitle format
			if (CodecID == _T("S_TEXT/SSA") || CodecID == _T("S_TEXT/ASS") || CodecID == _T("S_TEXT/UTF8")) {
				tracksFound.Add(track);
				tracksNames.Add(wxString::Format(_T("%i ("),track) + CodecID + _T(" ") + TrackLanguage + _T("): ") + TrackName);
			}
		}
	}

	// No tracks found
	if (tracksFound.Count() == 0) {
		target->LoadDefault(true);
		Close();
		throw _T("File has no recognised subtitle tracks.");
	}

	// Only one track found
	else if (tracksFound.Count() == 1) {
		trackToRead = tracksFound[0];
	}

	// Pick a track
	else {
		int choice = wxGetSingleChoiceIndex(_T("Choose which track to read:"),_T("Multiple subtitle tracks found"),tracksNames);
		if (choice == -1) {
			target->LoadDefault(true);
			Close();
			throw agi::UserCancelException("cancelled");
		}
		trackToRead = tracksFound[choice];
	}

	// Picked track
	if (trackToRead != -1) {
		// Get codec type (0 = ASS/SSA, 1 = SRT)
		trackInfo = mkv_GetTrackInfo(file,trackToRead);
		wxString CodecID = wxString(trackInfo->CodecID,*wxConvCurrent);
		int codecType = 0;
		if (CodecID == _T("S_TEXT/UTF8")) codecType = 1;

		// Read private data if it's ASS/SSA
		if (codecType == 0) {
			// Read raw data
			trackInfo = mkv_GetTrackInfo(file,trackToRead);
			unsigned int privSize = trackInfo->CodecPrivateSize;
			char *privData = new char[privSize+1];
			memcpy(privData,trackInfo->CodecPrivate,privSize);
			privData[privSize] = 0;
			wxString privString(privData,wxConvUTF8);
			delete[] privData;

			// Load into file
			wxString group = _T("[Script Info]");
			int version = 1;
			if (CodecID == _T("S_TEXT/SSA")) version = 0;
			wxStringTokenizer token(privString,_T("\r\n"),wxTOKEN_STRTOK);
			while (token.HasMoreTokens()) {
				wxString next = token.GetNextToken();
				if (next[0] == _T('[')) group = next;
				target->AddLine(next,group,version,&group);
			}

			// Insert "[Events]"
			//target->AddLine(_T(""),group,lasttime,version,&group);
			//target->AddLine(_T("[Events]"),group,lasttime,version,&group);
			//target->AddLine(_T("Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text"),group,lasttime,version,&group);
		}

		// Load default if it's SRT
		else {
			target->LoadDefault(false);
		}

		// Read timecode scale
		SegmentInfo *segInfo = mkv_GetFileInfo(file);
		longlong timecodeScale = mkv_TruncFloat(trackInfo->TimecodeScale) * segInfo->TimecodeScale;

		// Prepare STD vector to get lines inserted
		std::vector<wxString> subList;
		long int order = -1;

		// Progress bar
		int totalTime = int(double(segInfo->Duration) / timecodeScale);
		volatile bool canceled = false;
		DialogProgress *progress = new DialogProgress(NULL,_("Parsing Matroska"),&canceled,_("Reading subtitles from Matroska file."),0,totalTime);
		progress->Show();
		progress->SetProgress(0,1);

		// Load blocks
		mkv_SetTrackMask(file, ~(1 << trackToRead));
		while (mkv_ReadFrame(file,0,&rt,&startTime,&endTime,&filePos,&frameSize,&frameFlags) == 0) {
			// Canceled			
			if (canceled) {
				target->LoadDefault(true);
				Close();
				throw agi::UserCancelException("cancelled");
			}

			// Read to temp
			char *tmp = new char[frameSize+1];
			fseek(input->fp, filePos, SEEK_SET);
			fread(tmp,1,frameSize,input->fp);
			tmp[frameSize] = 0;
			wxString blockString(tmp,wxConvUTF8);
			delete[] tmp;

			// Get start and end times
			//longlong timecodeScaleLow = timecodeScale / 100;
			longlong timecodeScaleLow = 1000000;
			AssTime subStart,subEnd;
			subStart.SetMS(startTime / timecodeScaleLow);
			subEnd.SetMS(endTime / timecodeScaleLow);
			//wxLogMessage(subStart.GetASSFormated() + _T("-") + subEnd.GetASSFormated() + _T(": ") + blockString);

			// Process SSA/ASS
			if (codecType == 0) {
				// Get order number
				int pos = blockString.Find(_T(","));
				wxString orderString = blockString.Left(pos);
				orderString.ToLong(&order);
				blockString = blockString.Mid(pos+1);

				// Get layer number
				pos = blockString.Find(_T(","));
				long int layer = 0;
				if (pos) {
					wxString layerString = blockString.Left(pos);
					layerString.ToLong(&layer);
					blockString = blockString.Mid(pos+1);
				}

				// Assemble final
				blockString = wxString::Format(_T("Dialogue: %i,"),layer) + subStart.GetASSFormated() + _T(",") + subEnd.GetASSFormated() + _T(",") + blockString;
			}

			// Process SRT
			else {
				blockString = wxString(_T("Dialogue: 0,")) + subStart.GetASSFormated() + _T(",") + subEnd.GetASSFormated() + _T(",Default,,0000,0000,0000,,") + blockString;
				blockString.Replace(_T("\r\n"),_T("\\N"));
				blockString.Replace(_T("\r"),_T("\\N"));
				blockString.Replace(_T("\n"),_T("\\N"));
				order++;
			}

			// Insert into vector
			if (subList.size() == (unsigned int)order) subList.push_back(blockString);
			else {
				if ((signed)(subList.size()) < order+1) subList.resize(order+1);
				subList[order] = blockString;
			}

			// Update progress bar
			progress->SetProgress(int(double(startTime) / 1000000.0),totalTime);
		}

		// Insert into file
		wxString group = _T("[Events]");
		int version = (CodecID == _T("S_TEXT/SSA"));
		for (unsigned int i=0;i<subList.size();i++) {
			target->AddLine(subList[i],group,version,&group);
		}

		// Close progress bar
		if (!canceled) progress->Destroy();
	}

	// No track to load
	else {
		target->LoadDefault(true);
	}
}

bool MatroskaWrapper::HasSubtitles(wxString const& filename) {
	char err[2048];
	MkvStdIO input(filename);
	if (!input.fp) return false;

	MatroskaFile* file = mkv_Open(&input, err, sizeof(err));
	if (!file) return false;

	// Find tracks
	int tracks = mkv_GetNumTracks(file);
	for (int track = 0; track < tracks; track++) {
		TrackInfo *trackInfo = mkv_GetTrackInfo(file, track);

		if (trackInfo->Type == 0x11) {
			wxString CodecID = wxString(trackInfo->CodecID, *wxConvCurrent);
			if (CodecID == _T("S_TEXT/SSA") || CodecID == _T("S_TEXT/ASS") || CodecID == _T("S_TEXT/UTF8")) {
				mkv_Close(file);
				return true;
			}
		}
	}

	mkv_Close(file);
	return false;
}

////////////////////////////// LOTS OF HAALI C CODE DOWN HERE ///////////////////////////////////////

#ifdef __VISUALC__

/// DOCME
#define std_fread fread

/// DOCME
#define std_fseek _fseeki64

/// DOCME
#define std_ftell _ftelli64
#else

/// DOCME
#define std_fread fread

/// DOCME
#define std_fseek fseeko

/// DOCME
#define std_ftell ftello
#endif

/// @brief STDIO class 
/// @param _st    
/// @param pos    
/// @param buffer 
/// @param count  
/// @return 
///
int StdIoRead(InputStream *_st, ulonglong pos, void *buffer, int count) {
  MkvStdIO *st = (MkvStdIO *) _st;
  size_t  rd;
  if (std_fseek(st->fp, pos, SEEK_SET)) {
    st->error = errno;
    return -1;
  }
  rd = std_fread(buffer, 1, count, st->fp);
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

/// @brief */
/// @param _st       
/// @param start     
/// @param signature 
/// @return 
///
longlong StdIoScan(InputStream *_st, ulonglong start, unsigned signature) {
  MkvStdIO *st = (MkvStdIO *) _st;
  int	      c;
  unsigned    cmp = 0;
  FILE	      *fp = st->fp;

  if (std_fseek(fp, start, SEEK_SET))
    return -1;

  while ((c = getc(fp)) != EOF) {
    cmp = ((cmp << 8) | c) & 0xffffffff;
    if (cmp == signature)
      return std_ftell(fp) - 4;
  }

  return -1;
}

/// @brief This is used to limit readahead.
/// @param _st 
/// @return Cache size
///
unsigned StdIoGetCacheSize(InputStream *_st) {
  return CACHESIZE;
}

/// @brief Get last error message
/// @param _st 
/// @return Last error message
///
const char *StdIoGetLastError(InputStream *_st) {
  MkvStdIO *st = (MkvStdIO *) _st;
  return strerror(st->error);
}

/// @brief Memory allocation, this is done via stdlib
/// @param _st  
/// @param size 
/// @return 
///
void  *StdIoMalloc(InputStream *_st, size_t size) {
  return malloc(size);
}

/// @brief DOCME
/// @param _st  
/// @param mem  
/// @param size 
/// @return 
///
void  *StdIoRealloc(InputStream *_st, void *mem, size_t size) {
  return realloc(mem,size);
}

/// @brief DOCME
/// @param _st 
/// @param mem 
///
void  StdIoFree(InputStream *_st, void *mem) {
  free(mem);
}

/// @brief DOCME
/// @param _st 
/// @param cur 
/// @param max 
/// @return 
///
int   StdIoProgress(InputStream *_st, ulonglong cur, ulonglong max) {
  return 1;
}

/// @brief DOCME
/// @param _st 
/// @return 
///
longlong StdIoGetFileSize(InputStream *_st) {
	MkvStdIO *st = (MkvStdIO *) _st;
	longlong epos = 0;
	longlong cpos = std_ftell(st->fp);
	std_fseek(st->fp, 0, SEEK_END);
	epos = std_ftell(st->fp);
	std_fseek(st->fp, cpos, SEEK_SET);
	return epos;
}

/// @brief DOCME
/// @param filename 
///
MkvStdIO::MkvStdIO(wxString filename) {
	read = StdIoRead;
	scan = StdIoScan;
	getcachesize = StdIoGetCacheSize;
	geterror = StdIoGetLastError;
	memalloc = StdIoMalloc;
	memrealloc = StdIoRealloc;
	memfree = StdIoFree;
	progress = StdIoProgress;
	getfilesize = StdIoGetFileSize;
	wxFileName fname(filename);
	fp = fopen(fname.GetShortPath().mb_str(wxConvUTF8),"rb");
	if (fp) {
		setvbuf(fp, NULL, _IOFBF, CACHESIZE);
	}
}

