// Copyright (c) 2009, Karl Blomster
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

#include "video_provider_quicktime.h"

#ifdef WITH_QUICKTIME
#include "aegisub_endian.h"

// this function has a different name on win32 because the original name
// conflicts with a windows api function
#ifndef WIN32
#define MacOffsetRect OffsetRect
#endif


QuickTimeVideoProvider::QuickTimeVideoProvider(wxString filename) {
	in_dataref = NULL;
	movie	= NULL;
	gw		= NULL;
	w		= 0;
	h		= 0;
	cur_fn	= -1;
	num_frames = 0;
	keyframes.clear();

	qt_err = noErr;
	errmsg = _T("QuickTime video provider: ");

	// try to init quicktime
	try {
		InitQuickTime();
	}
	catch (wxString temp) {
		errmsg.Append(temp);
		throw errmsg;
	}
	catch (...) {
		throw;
	}

	// try loading the file
	try {
		LoadVideo(filename);
	}
	catch (wxString temp) {
		Close();
		errmsg.Append(temp);
		throw errmsg;
	}
	catch (...) {
		Close();
		throw;
	}
}


QuickTimeVideoProvider::~QuickTimeVideoProvider() {
	Close();
	DeInitQuickTime();
}


void QuickTimeVideoProvider::Close() {
	if (movie)
		DisposeMovie(movie);
	movie = NULL;
	if (gw)
		DisposeGWorld(gw);
	gw = NULL;
	if (in_dataref)
		DisposeHandle(in_dataref);
	in_dataref = NULL;

	keyframes.Clear();
	qt_timestamps.clear();
}





void QuickTimeVideoProvider::LoadVideo(const wxString _filename) {
	Close();

	OSType in_dataref_type;
	wxStringToDataRef(_filename, &in_dataref, &in_dataref_type);

	// verify that file is openable
	if (!CanOpen(in_dataref, in_dataref_type))
		throw wxString(_T("QuickTime cannot open file as video"));

	// actually open file
	short res_id = 0;
	qt_err = NewMovieFromDataRef(&movie, newMovieActive, &res_id, in_dataref, in_dataref_type);
	QTCheckError(qt_err, wxString(_T("Failed to open file")));

	// disable automagic screen rendering
	qt_err = SetMovieVisualContext(movie, NULL);
	QTCheckError(qt_err, wxString(_T("Failed to disable visual context")));

	// set offscreen buffer size to actual movie dimensions
	Rect m_box;
	GetMovieBox(movie, &m_box);
	// make sure its top left corner is at (0,0)
	MacOffsetRect(&m_box, -m_box.left, -m_box.top);
	SetMovieBox(movie, &m_box);
	w = m_box.right;
	h = m_box.bottom;
	// allocate a new offscreen rendering buffer with the correct dimensions
	QDErr qd_err = NewGWorld(&gw, 32, &m_box, NULL, NULL, keepLocal);
	if (qd_err != noErr)
		throw wxString(_T("Failed to initialize offscreen drawing buffer"));
	
	// select our new offscreen render target
	SetMovieGWorld(movie, gw, NULL);

	// get timestamps, keyframes and framecount
	std::vector<int> timecodes = IndexFile();
	if (timecodes.size() == 0)
		throw wxString(_T("QuickTime video provider: failed to index file"));

	// ask about vfr override etc
	vfr_fps.SetVFR(timecodes);
	int override_tc = wxYES;
	if (VFR_Output.IsLoaded()) {
		override_tc = wxMessageBox(_("You already have timecodes loaded. Would you like to replace them with timecodes from the video file?"), _("Replace timecodes?"), wxYES_NO | wxICON_QUESTION);
		if (override_tc == wxYES) {
			VFR_Input.SetVFR(timecodes);
			VFR_Output.SetVFR(timecodes);
		}
	} else { // no timecodes loaded, go ahead and apply
		VFR_Input.SetVFR(timecodes);
		VFR_Output.SetVFR(timecodes);
	}

	// set assumed "cfr" fps (dunno if this is actually used anywhere)
	double len_s = (double)GetMovieDuration(movie) / (double)GetMovieTimeScale(movie);
	assumed_fps = (double)num_frames / len_s;

	cur_fn = 0;
}


std::vector<int> QuickTimeVideoProvider::IndexFile() {
	TimeScale scale = GetMovieTimeScale(movie);
	OSType v_type[1];
	v_type[0] = VisualMediaCharacteristic;
	std::vector<int> timecodes;
	std::map<TimeValue,int> timestamp_map;	// TODO: just do a binary search instead

	int framecount = 1;
	TimeValue cur_timeval = 0;

	// get the first frame
	GetMovieNextInterestingTime(movie, nextTimeMediaSample + nextTimeEdgeOK, 1, v_type, cur_timeval, 0, &cur_timeval, NULL);

	// first, find timestamps and count frames
	while (cur_timeval >= 0) {
		qt_timestamps.push_back(cur_timeval);
		timestamp_map.insert(std::pair<TimeValue,int>(cur_timeval, framecount));
		timecodes.push_back((cur_timeval * 1000) / scale);
		framecount++;
		GetMovieNextInterestingTime(movie, nextTimeMediaSample, 1, v_type, cur_timeval, 0, &cur_timeval, NULL);
	}
	// GetMovieNextInterestingTime() returns -1 when there are no more interesting times,
	// so we incremented framecount once too much
	num_frames = --framecount; 


	// next, find keyframes
	cur_timeval = 0;
	keyframes.push_back(0); // interesting assumption?
	while (cur_timeval >= 0) {
		GetMovieNextInterestingTime(movie, nextTimeSyncSample, 1, v_type, cur_timeval, 0, &cur_timeval, NULL);
		keyframes.push_back(timestamp_map[cur_timeval]);
	}

	return timecodes;
}


const AegiVideoFrame QuickTimeVideoProvider::GetFrame(int n) {
	if (n < 0)
		n = 0;
	if (n >= num_frames)
		n = num_frames-1;
	cur_fn = n;

	// seek
	SetMovieTimeValue(movie, qt_timestamps[n]);
	qt_err = GetMoviesError();
	QTCheckError(qt_err, wxString::Format(_T("QuickTime video provider: failed to seek to TimeValue %d"), qt_timestamps[n]));

	// render to offscreen buffer
	qt_err = UpdateMovie(movie);
	QTCheckError(qt_err, wxString(_T("QuickTime video provider: failed to render frame")));
	MoviesTask(movie, 0L);
	qt_err = GetMoviesError();
	QTCheckError(qt_err, wxString(_T("QuickTime video provider: failed to render frame")));

	// set up destination
	AegiVideoFrame dst_frame;
	dst_frame.format = FORMAT_RGB32;
	dst_frame.w = w;
	dst_frame.h = h;
	dst_frame.invertChannels = true;
	dst_frame.flipped = false;
	dst_frame.pitch[0] = w * 4; // 4 bytes per sample
	dst_frame.Allocate();

	// copy data from offscreen buffer
	Ptr src_ptr8 = GetPixBaseAddr(GetGWorldPixMap(gw));
	uint32_t *src_ptr = reinterpret_cast<uint32_t *>(src_ptr8);
	uint32_t *dst_ptr = reinterpret_cast<uint32_t *>(dst_frame.data[0]);
	for (int i=0; i<(w*h); i++)
		// swap endian if needed; quickdraw always renders big-endian
		*dst_ptr++ = Endian::BigToMachine(*src_ptr++); // fun with pointers!

	return dst_frame;
}




///////////////
// Utility functions
int QuickTimeVideoProvider::GetWidth() {
	return w;
}

int QuickTimeVideoProvider::GetHeight() {
	return h;
}

int QuickTimeVideoProvider::GetFrameCount() {
	return num_frames;
}

int QuickTimeVideoProvider::GetPosition() {
	return cur_fn;
}

double QuickTimeVideoProvider::GetFPS() {
	return assumed_fps;
}

bool QuickTimeVideoProvider::AreKeyFramesLoaded() { 
	if (keyframes.GetCount() > 0)
		return true;
	else
		return false;
}

wxArrayInt QuickTimeVideoProvider::GetKeyFrames() {
	return keyframes;
}

FrameRate QuickTimeVideoProvider::GetTrueFrameRate() {
	return vfr_fps; 
}


#endif /* WITH_QUICKTIME */
