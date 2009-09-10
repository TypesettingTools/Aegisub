// Copyright (c) 2007-2008, Niels Martin Hansen
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

/// @file audio_provider_pcm.cpp
/// @brief PCM WAV and WAV64 audio provider
/// @ingroup audio_input
///


#include "config.h"

#ifndef AGI_PRE
#include <assert.h>
#include <stdint.h>
#ifndef __WINDOWS__
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/log.h>
#endif

#include "aegisub_endian.h"
#include "audio_provider_pcm.h"
#include "utils.h"


/// @brief DOCME
/// @param filename 
///
PCMAudioProvider::PCMAudioProvider(const wxString &filename)
{
#ifdef _WINDOWS

	file_handle = CreateFile(
		filename.c_str(),
		FILE_READ_DATA,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS,
		0);

	if (file_handle == INVALID_HANDLE_VALUE) {
		wxLogWarning(_T("PCM audio provider: Could not open audio file for reading (%d)"), GetLastError());
		throw _T("PCM audio provider: Could not open audio file for reading");
	}

	LARGE_INTEGER li_file_size = {0};
	if (!GetFileSizeEx(file_handle, &li_file_size)) {
		CloseHandle(file_handle);
		throw _T("PCM audio provider: Failed getting file size");
	}
	file_size = li_file_size.QuadPart;

	file_mapping = CreateFileMapping(
		file_handle,
		0,
		PAGE_READONLY,
		0, 0,
		0);

	if (file_mapping == 0) {
		CloseHandle(file_handle);
		throw _T("PCM audio provider: Failed creating file mapping");
	}

	current_mapping = 0;

#else

	file_handle = open(filename.mb_str(*wxConvFileName), O_RDONLY);

	if (file_handle == -1) {
		throw _T("PCM audio provider: Could not open audio file for reading");
	}

	struct stat filestats;
	memset(&filestats, 0, sizeof(filestats));
	if (fstat(file_handle, &filestats)) {
		close(file_handle);
		throw _T("PCM audio provider: Could not stat file to get size");
	}
	file_size = filestats.st_size;

	current_mapping = 0;

#endif
}



/// @brief DOCME
///
PCMAudioProvider::~PCMAudioProvider()
{
#ifdef _WINDOWS

	if (current_mapping) {
		UnmapViewOfFile(current_mapping);
	}

	CloseHandle(file_mapping);
	CloseHandle(file_handle);

#else

	if (current_mapping) {
		munmap(current_mapping, mapping_length);
	}

	close(file_handle);

#endif
}



/// @brief DOCME
/// @param range_start  
/// @param range_length 
/// @return 
///
char * PCMAudioProvider::EnsureRangeAccessible(int64_t range_start, int64_t range_length)
{
	if (range_start + range_length > file_size) {
		throw _T("PCM audio provider: Attempted to map beyond end of file");
	}

	// Check whether the requested range is already visible
	if (!current_mapping || range_start < mapping_start || range_start+range_length > mapping_start+(int64_t)mapping_length) {

		// It's not visible, change the current mapping
		
		if (current_mapping) {
#ifdef _WINDOWS
			UnmapViewOfFile(current_mapping);
#else
			munmap(current_mapping, mapping_length);
#endif
		}

		// Align range start on a 1 MB boundary and go 16 MB back
		mapping_start = (range_start & ~0xFFFFF) - 0x1000000;
		if (mapping_start < 0) mapping_start = 0;

		if (sizeof(void*) > 4)
			// Large address space, use a 2 GB mapping
			mapping_length = 0x80000000;
		else
			// Small (32 bit) address space, use a 256 MB mapping
			mapping_length = 0x10000000;

		// Make sure to always make a mapping at least as large as the requested range
		if ((int64_t)mapping_length < range_length) {
			if (range_length > (int64_t)(~(size_t)0))
				throw _T("PCM audio provider: Requested range larger than max size_t, cannot create view of file");
			else
				mapping_length = range_length;
		}
		// But also make sure we don't try to make a mapping larger than the file
		if (mapping_start + (int64_t)mapping_length > file_size)
			mapping_length = (size_t)(file_size - mapping_start);
		// We already checked that the requested range doesn't extend over the end of the file
		// Hopefully this should ensure that small files are always mapped in their entirety

#ifdef _WINDOWS
		LARGE_INTEGER mapping_start_li;
		mapping_start_li.QuadPart = mapping_start;
		current_mapping = MapViewOfFile(
			file_mapping,	// Mapping handle
			FILE_MAP_READ,	// Access type
			mapping_start_li.HighPart,	// Offset high-part
			mapping_start_li.LowPart,	// Offset low-part
			mapping_length);	// Length of view
#else
		current_mapping = mmap(0, mapping_length, PROT_READ, MAP_PRIVATE, file_handle, mapping_start);
#endif

		if (!current_mapping) {
			throw _T("PCM audio provider: Failed mapping a view of the file");
		}

	}

	assert(current_mapping);
	assert(range_start >= mapping_start);

	// Difference between actual current mapping start and requested range start
	ptrdiff_t rel_ofs = (ptrdiff_t)(range_start - mapping_start);

	// Calculate a pointer into current mapping for the requested range
	return ((char*)current_mapping) + rel_ofs;
}



/// @brief DOCME
/// @param buf   
/// @param start 
/// @param count 
///
void PCMAudioProvider::GetAudio(void *buf, int64_t start, int64_t count)
{
	// Read blocks from the file
	size_t index = 0;
	while (count > 0 && index < index_points.size()) {
		// Check if this index contains the samples we're looking for
		IndexPoint &ip = index_points[index];
		if (ip.start_sample <= start && ip.start_sample+ip.num_samples > start) {

			// How many samples we can maximum take from this block
			int64_t samples_can_do = ip.num_samples - start + ip.start_sample;
			if (samples_can_do > count) samples_can_do = count;

			// Read as many samples we can
			char *src = EnsureRangeAccessible(
				ip.start_byte + (start - ip.start_sample) * bytes_per_sample * channels,
				samples_can_do * bytes_per_sample * channels);
			memcpy(buf, src, samples_can_do * bytes_per_sample * channels);

			// Update data
			buf = (char*)buf + samples_can_do * bytes_per_sample * channels;
			start += samples_can_do;
			count -= samples_can_do;

		}

		index++;
	}

	// If we exhausted all sample sections zerofill the rest
	if (count > 0) {
		if (bytes_per_sample == 1)
			// 8 bit formats are usually unsigned with bias 127
			memset(buf, 127, count*channels);
		else
			// While everything else is signed
			memset(buf, 0, count*bytes_per_sample*channels);
	}
}


// RIFF WAV PCM provider
// Overview of RIFF WAV: <http://www.sonicspot.com/guide/wavefiles.html>


/// DOCME
/// @class RiffWavPCMAudioProvider
/// @brief DOCME
///
/// DOCME
class  RiffWavPCMAudioProvider : public PCMAudioProvider {
private:

	/// DOCME
	struct ChunkHeader {

		/// DOCME
		char type[4];

		/// DOCME
		uint32_t size;
	};

	/// DOCME
	struct RIFFChunk {

		/// DOCME
		ChunkHeader ch;

		/// DOCME
		char format[4];
	};

	/// DOCME
	struct fmtChunk {

		/// DOCME
		uint16_t compression; // compression format used -- 0x0001 = PCM

		/// DOCME
		uint16_t channels;

		/// DOCME
		uint32_t samplerate;

		/// DOCME
		uint32_t avg_bytes_sec; // can't always be trusted

		/// DOCME
		uint16_t block_align;

		/// DOCME
		uint16_t significant_bits_sample;
		// Here was supposed to be some more fields but we don't need them
		// and just skipping by the size of the struct wouldn't be safe
		// either way, as the fields can depend on the compression.
	};


	/// @brief DOCME
	/// @param str1[] 
	/// @param str2[] 
	/// @return 
	///
	static bool CheckFourcc(const char str1[], const char str2[])
	{
		assert(str1);
		assert(str2);
		return
			(str1[0] == str2[0]) &&
			(str1[1] == str2[1]) &&
			(str1[2] == str2[2]) &&
			(str1[3] == str2[3]);
	}

public:

	/// @brief DOCME
	/// @param _filename 
	///
	RiffWavPCMAudioProvider(const wxString &_filename)
		: PCMAudioProvider(_filename)
	{
		filename = _filename;

		// Read header
		// This should throw an exception if the mapping fails
		void *filestart = EnsureRangeAccessible(0, sizeof(RIFFChunk));
		assert(filestart);
		RIFFChunk &header = *(RIFFChunk*)filestart;

		// Check magic values
		if (!CheckFourcc(header.ch.type, "RIFF"))
			throw _T("RIFF PCM WAV audio provider: File is not a RIFF file");
		if (!CheckFourcc(header.format, "WAVE"))
			throw _T("RIFF PCM WAV audio provider: File is not a RIFF WAV file");

		// Count how much more data we can have in the entire file
		// The first 4 bytes are already eaten by the header.format field
		uint32_t data_left = Endian::LittleToMachine(header.ch.size) - 4;
		// How far into the file we have processed.
		// Must be incremented by the riff chunk size fields.
		uint32_t filepos = sizeof(header);

		bool got_fmt_header = false;

		// Inherited from AudioProvider
		num_samples = 0;

		// Continue reading chunks until out of data
		while (data_left) {
			ChunkHeader &ch = *(ChunkHeader*)EnsureRangeAccessible(filepos, sizeof(ChunkHeader));

			// Update counters
			data_left -= sizeof(ch);
			filepos += sizeof(ch);

			if (CheckFourcc(ch.type, "fmt ")) {
				if (got_fmt_header) throw _T("RIFF PCM WAV audio provider: Invalid file, multiple 'fmt ' chunks");
				got_fmt_header = true;

				fmtChunk &fmt = *(fmtChunk*)EnsureRangeAccessible(filepos, sizeof(fmtChunk));

				if (Endian::LittleToMachine(fmt.compression) != 1) throw _T("RIFF PCM WAV audio provider: Can't use file, not PCM encoding");

				// Set stuff inherited from the AudioProvider class
				sample_rate = Endian::LittleToMachine(fmt.samplerate);
				channels = Endian::LittleToMachine(fmt.channels);
				bytes_per_sample = (Endian::LittleToMachine(fmt.significant_bits_sample) + 7) / 8; // round up to nearest whole byte
			}

			else if (CheckFourcc(ch.type, "data")) {
				// This won't pick up 'data' chunks inside 'wavl' chunks
				// since the 'wavl' chunks wrap those.

				if (!got_fmt_header) throw _T("RIFF PCM WAV audio provider: Found 'data' chunk before 'fmt ' chunk, file is invalid.");

				int64_t samples = Endian::LittleToMachine(ch.size) / bytes_per_sample;
				int64_t frames = samples / channels;

				IndexPoint ip;
				ip.start_sample = num_samples;
				ip.num_samples = frames;
				ip.start_byte = filepos;
				index_points.push_back(ip);

				num_samples += frames;
			}

			// Support wavl (wave list) chunks too?

			// Update counters
			// Make sure they're word aligned
			data_left -= (Endian::LittleToMachine(ch.size) + 1) & ~1;
			filepos += (Endian::LittleToMachine(ch.size) + 1) & ~1;
		}
	}



	/// @brief DOCME
	/// @return 
	///
	bool AreSamplesNativeEndian()
	{
		// 8 bit samples don't consider endianness
		if (bytes_per_sample < 2) return true;
		// Otherwise test whether we're little endian
		uint32_t testvalue = 0x008800ff;
		return testvalue == Endian::LittleToMachine(testvalue);
	}
};



// Sony Wave64 audio provider
// Specs obtained at: <http://www.vcs.de/fileadmin/user_upload/MBS/PDF/Whitepaper/Informations_about_Sony_Wave64.pdf>


/// DOCME
static const uint8_t w64GuidRIFF[16] = {
	// {66666972-912E-11CF-A5D6-28DB04C10000}
	0x72, 0x69, 0x66, 0x66, 0x2E, 0x91, 0xCF, 0x11, 0xA5, 0xD6, 0x28, 0xDB, 0x04, 0xC1, 0x00, 0x00
};


/// DOCME
static const uint8_t w64GuidWAVE[16] = {
	// {65766177-ACF3-11D3-8CD1-00C04F8EDB8A}
	0x77, 0x61, 0x76, 0x65, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A
};


/// DOCME
static const uint8_t w64Guidfmt[16] = {
	// {20746D66-ACF3-11D3-8CD1-00C04F8EDB8A}
	0x66, 0x6D, 0x74, 0x20, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A
};


/// DOCME
static const uint8_t w64Guiddata[16] = {
	// {61746164-ACF3-11D3-8CD1-00C04F8EDB8A}
	0x64, 0x61, 0x74, 0x61, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A
};


/// DOCME
/// @class Wave64AudioProvider
/// @brief DOCME
///
/// DOCME
class Wave64AudioProvider : public PCMAudioProvider {
private:
	// Here's some copy-paste from the FFmpegSource2 code


	/// DOCME
	struct WaveFormatEx { 

		/// DOCME
		uint16_t wFormatTag; 

		/// DOCME
		uint16_t nChannels; 

		/// DOCME
		uint32_t nSamplesPerSec; 

		/// DOCME
		uint32_t nAvgBytesPerSec; 

		/// DOCME
		uint16_t nBlockAlign; 

		/// DOCME
		uint16_t wBitsPerSample; 

		/// DOCME
		uint16_t cbSize; 
	};


	/// DOCME
	struct RiffChunk {

		/// DOCME
		uint8_t riff_guid[16];

		/// DOCME
		uint64_t file_size;

		/// DOCME
		uint8_t format_guid[16];
	};


	/// DOCME
	struct FormatChunk {

		/// DOCME
		uint8_t chunk_guid[16];

		/// DOCME
		uint64_t chunk_size;

		/// DOCME
		WaveFormatEx format;

		/// DOCME
		uint8_t padding[6];
	};


	/// DOCME
	struct DataChunk {

		/// DOCME
		uint8_t chunk_guid[16];

		/// DOCME
		uint64_t chunk_size;
	};


	/// @brief DOCME
	/// @param guid1 
	/// @param guid2 
	/// @return 
	///
	inline bool CheckGuid(const uint8_t *guid1, const uint8_t *guid2)
	{
		return memcmp(guid1, guid2, 16) == 0;
	}

public:

	/// @brief DOCME
	/// @param _filename 
	///
	Wave64AudioProvider(const wxString &_filename)
		: PCMAudioProvider(_filename)
	{
		filename = _filename;

		int64_t smallest_possible_file = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk);

		if (file_size < smallest_possible_file)
			throw _T("Wave64 audio provider: File is too small to be a Wave64 file");

		// Read header
		// This should throw an exception if the mapping fails
		void *filestart = EnsureRangeAccessible(0, sizeof(RiffChunk));
		assert(filestart);
		RiffChunk &header = *(RiffChunk*)filestart;

		// Check magic values
		if (!CheckGuid(header.riff_guid, w64GuidRIFF))
			throw _T("Wave64 audio provider: File is not a Wave64 RIFF file");
		if (!CheckGuid(header.format_guid, w64GuidWAVE))
			throw _T("Wave64 audio provider: File is not a Wave64 WAVE file");

		// Count how much more data we can have in the entire file
		uint64_t data_left = Endian::LittleToMachine(header.file_size) - sizeof(RiffChunk);
		// How far into the file we have processed.
		// Must be incremented by the riff chunk size fields.
		uint64_t filepos = sizeof(header);

		bool got_fmt_header = false;

		// Inherited from AudioProvider
		num_samples = 0;

		// Continue reading chunks until out of data
		while (data_left) {
			uint8_t *chunk_guid = (uint8_t*)EnsureRangeAccessible(filepos, 16);
			uint64_t chunk_size = Endian::LittleToMachine(*(uint64_t*)EnsureRangeAccessible(filepos+16, sizeof(uint64_t)));

			if (CheckGuid(chunk_guid, w64Guidfmt)) {
				if (got_fmt_header)
					throw _T("Wave64 audio provider: Bad file, found more than one 'fmt' chunk");

				FormatChunk &fmt = *(FormatChunk*)EnsureRangeAccessible(filepos, sizeof(FormatChunk));
				got_fmt_header = true;

				if (Endian::LittleToMachine(fmt.format.wFormatTag) == 3)
					throw _T("Wave64 audio provider: File is IEEE 32 bit float format which isn't supported. Bug the developers if this matters.");
				if (Endian::LittleToMachine(fmt.format.wFormatTag) != 1)
					throw _T("Wave64 audio provider: Can't use file, not PCM encoding");

				// Set stuff inherited from the AudioProvider class
				sample_rate = Endian::LittleToMachine(fmt.format.nSamplesPerSec);
				channels = Endian::LittleToMachine(fmt.format.nChannels);
				bytes_per_sample = (Endian::LittleToMachine(fmt.format.wBitsPerSample) + 7) / 8; // round up to nearest whole byte
			}

			else if (CheckGuid(chunk_guid, w64Guiddata)) {
				if (!got_fmt_header)
					throw _T("Wave64 audio provider: Found 'data' chunk before 'fmt ' chunk, file is invalid.");

				int64_t samples = chunk_size / bytes_per_sample;
				int64_t frames = samples / channels;

				IndexPoint ip;
				ip.start_sample = num_samples;
				ip.num_samples = frames;
				ip.start_byte = filepos;
				index_points.push_back(ip);

				num_samples += frames;
			}

			// Update counters
			// Make sure they're 64 bit aligned
			data_left -= (chunk_size + 7) & ~7;
			filepos += (chunk_size + 7) & ~7;
		}
	}



	/// @brief DOCME
	/// @return 
	///
	bool AreSamplesNativeEndian()
	{
		// 8 bit samples don't consider endianness
		if (bytes_per_sample < 2) return true;
		// Otherwise test whether we're little endian
		uint32_t testvalue = 0x008800ff;
		return testvalue == Endian::LittleToMachine(testvalue);
	}
};



/// @brief DOCME
/// @param filename 
///
AudioProvider *CreatePCMAudioProvider(const wxString &filename)
{
	AudioProvider *provider = 0;

	// Try Microsoft/IBM RIFF WAV
	try {
		provider = new RiffWavPCMAudioProvider(filename);
		// don't bother trying with anything else if this works
		return provider;
	}
	catch (const wxChar *msg) {
		provider = 0;
		wxLogDebug(_T("Creating PCM WAV reader failed with message: %s\nProceeding to try other providers."), msg);
	}

	// Try Sony Wave64
	try {
		provider = new Wave64AudioProvider(filename);
		return provider;
	}
	catch (const wxChar *msg) {
		provider = 0;
		wxLogDebug(_T("Creating Wave64 reader failed with message: %s\nProceeding to try other providers."), msg);
	}

	// no providers could be created
	return NULL;
}


