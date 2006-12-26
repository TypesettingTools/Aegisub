// Copyright (c) 2005-2006, Rodrigo Braz Monteiro, Fredrik Mellbin
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
#include <wx/filename.h>
#include "dialog_progress.h"
#include "audio_provider_hd.h"
#include "main.h"
#include "options.h"
#include "utils.h"


///////////////
// Constructor
HDAudioProvider::HDAudioProvider(AudioProvider *source) {
	// Copy parameters
	bytes_per_sample = source->GetBytesPerSample();
	num_samples = source->GetNumSamples();
	channels = source->GetChannels();
	sample_rate = source->GetSampleRate();
	filename = source->GetFilename();

	// Check free space
	wxLongLong freespace;
	if (wxGetDiskSpace(DiskCachePath(), NULL, &freespace)) {
		if (num_samples * channels * bytes_per_sample > freespace) {
			throw wxString(_T("Not enough free diskspace in "))+DiskCachePath()+wxString(_T(" to cache the audio"));
		}
	}

	// Open output file
	std::ofstream file;
	char filename[512];
	strcpy(filename,DiskCacheName().mb_str(wxConvLocal));
	file.open(filename,std::ios::binary | std::ios::out | std::ios::trunc);

	// Start progress
	volatile bool canceled = false;
	DialogProgress *progress = new DialogProgress(NULL,_T("Load audio"),&canceled,_T("Reading to Hard Disk cache"),0,num_samples);
	progress->Show();

	// Write to disk
	int block = 4096;
	char *temp = new char[block * channels * bytes_per_sample];
	for (__int64 i=0;i<num_samples && !canceled; i+=block) {
		if (block+i > num_samples) block = num_samples - i;
		source->GetAudio(temp,i,block);
		file.write(temp,block * channels * bytes_per_sample);
		progress->SetProgress(i,num_samples);
	}
	file.close();

	// Finish
	if (!canceled) {
		progress->Destroy();
		file_cache.open(filename,std::ios::binary | std::ios::in);
	}
	else {
		throw wxString(_T("Audio loading cancelled by user"));
	}
}


//////////////
// Destructor
HDAudioProvider::~HDAudioProvider() {
	file_cache.close();
	wxRemoveFile(DiskCacheName());
}


/////////////
// Get audio
void HDAudioProvider::GetAudio(void *buf, __int64 start, __int64 count) {
	// Requested beyond the length of audio
	if (start+count > num_samples) {
		__int64 oldcount = count;
		count = num_samples-start;
		if (count < 0) count = 0;

		// Fill beyond with zero
		if (bytes_per_sample == 1) {
			char *temp = (char *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
		if (bytes_per_sample == 2) {
			short *temp = (short *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
	}

	if (count) {
		wxMutexLocker disklock(diskmutex);
		file_cache.seekg(start*bytes_per_sample);
		file_cache.read((char*)buf,count*bytes_per_sample*channels);
	}
}


///////////////////////////
// Get disk cache path
wxString HDAudioProvider::DiskCachePath() {
	// Default
	wxString path = Options.AsText(_T("Audio HD Cache Location"));
	if (path == _T("default")) return AegisubApp::folderName;

	// Specified
	return DecodeRelativePath(path,AegisubApp::folderName);
}


///////////////////////////
// Get disk cache filename
wxString HDAudioProvider::DiskCacheName() {
	return DiskCachePath() + _T("audio.tmp");
}
