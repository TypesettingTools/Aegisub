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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_provider_hd.cpp
/// @brief Caching audio provider using a file for backing
/// @ingroup audio_input
///

#include "config.h"

#ifndef AGI_PRE
#endif

#include <libaegisub/io.h>

#include "audio_hd.h"


namespace media {

/// @brief Constructor 
/// @param source 
///
HDAudioProvider::HDAudioProvider(AudioProvider *src,  agi::ProgressSinkFactory *progress_factory) {
	std::auto_ptr<AudioProvider> source(src);
	// Copy parameters
	bytes_per_sample = source->GetBytesPerSample();
	num_samples = source->GetNumSamples();
	channels = source->GetChannels();
	sample_rate = source->GetSampleRate();
	filename = source->GetFilename();
	samples_native_endian = source->AreSamplesNativeEndian();

	// Check free space
	uint64_t freespace;
// XXX: fixme (add diskspace method to agi::util)
//	if (wxGetDiskSpace(DiskCachePath(), NULL, &freespace)) {
//		if (num_samples * channels * bytes_per_sample > freespace) {
//			throw AudioOpenError("Not enough free disk space in " + STD_STR(DiskCachePath()) + " to cache the audio");
//		}
//	}

	// Open output file
	diskCacheFilename = DiskCacheName();
//	file_cache.Create(diskCacheFilename,true,wxS_DEFAULT);
	io::Save file(diskCacheFilename);
	std::ofstream& file_cache = file.Get();
//	file_cache.Open(diskCacheFilename,wxFile::read_write);
	if (!file_cache.is_open()) throw AudioOpenError("Unable to write to audio disk cache.");

	// Start progress
	ProgressSink *progress = progress_factory->create_progress_sink("Reading to Hard Disk cache");

	volatile bool canceled = progress->get_cancelled();

	// Write to disk
	int block = 4096;
	data = new char[block * channels * bytes_per_sample];
	for (int64_t i=0;i<num_samples && !canceled; i+=block) {
		if (block+i > num_samples) block = num_samples - i;
		source->GetAudio(data,i,block);
		file_cache.write(data,block * channels * bytes_per_sample);
		progress->set_progress(i,num_samples);
	}
	file_cache.seekp(0);

	// Finish
	if (canceled) {
//		file_cache.Close();
		delete[] data;
		throw agi::UserCancelException("Audio loading cancelled by user");
	}

	delete progress;
}

/// @brief Destructor 
///
HDAudioProvider::~HDAudioProvider() {
//XXX	wxRemoveFile(diskCacheFilename);
	delete[] data;
}

/// @brief Get audio 
/// @param buf   
/// @param start 
/// @param count 
///
void HDAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
	// Requested beyond the length of audio
	if (start+count > num_samples) {
		int64_t oldcount = count;
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
		diskmutex.Lock();
		file_cache.seekp(start*bytes_per_sample);
		file_cache.read((char*)buf,count*bytes_per_sample*channels);
	}
}

/// @brief Get disk cache path 
/// @return 
///
std::string HDAudioProvider::DiskCachePath() {
	// Default
/*
	std::string path = lagi_wxString(OPT_GET("Audio/Cache/HD/Location")->GetString());
	if (path == _T("default")) return StandardPaths::DecodePath(_T("?temp/"));

	// Specified
	return DecodeRelativePath(path,StandardPaths::DecodePath(_T("?user/")));
*/
	return "XXX: fixme";
}

/// @brief Get disk cache filename 
///
std::string HDAudioProvider::DiskCacheName() {
/*
XXX: fixme
	// Get pattern
	std::string pattern = lagi_wxString(OPT_GET("Audio/Cache/HD/Name")->GetString());
	if (pattern.Find(_T("%02i")) == wxNOT_FOUND) pattern = _T("audio%02i.tmp");

	// Try from 00 to 99
	for (int i=0;i<100;i++) {
		// File exists?
		wxString curStringTry = DiskCachePath() + wxString::Format(pattern.c_str(),i);
		if (!wxFile::Exists(curStringTry)) return curStringTry;
	}
*/
	return "";
}

} // namespace media
