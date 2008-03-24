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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//


#include <wx/filename.h>
#include <wx/file.h>
#include "audio_provider_pcm.h"
#include "utils.h"
#include <stdint.h>

#ifndef _WINDOWS
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#endif



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

	struct stat filestats = {0};
	if (fstat(file_handle, &filestats)) {
		close(file_handle);
		throw _T("PCM audio provider: Could not stat file to get size");
	}
	file_size = filestats.st_size;

	current_mapping = 0;

#endif
}


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


char * PCMAudioProvider::EnsureRangeAccessible(int64_t range_start, int64_t range_length)
{
	if (range_start + range_length > file_size) {
		throw _T("PCM audio provider: Attempted to map beyond end of file");
	}

	// Check whether the requested range is already visible
	if (range_start < mapping_start || range_start+range_length > mapping_start+mapping_length) {

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
#ifdef _WINDOWS
# if defined(_M_X64) || defined(_M_IA64)
		// Make 2 GB mappings by default on 64 bit archs
		mapping_length = 0x80000000;
# else
		// Make 256 MB mappings by default on x86
		mapping_length = 0x10000000;
# endif
#else
		// 256 MB
		mapping_length = 0x10000000;
#endif
		if (mapping_length > file_size) mapping_length = (size_t)(file_size - mapping_start);
		// Hopefully this should ensure that small files are always mapped in their entirety

#ifdef _WINDOWS
		current_mapping = MapViewOfFile(
			file_mapping,
			FILE_MAP_READ,
			mapping_start >> 32, mapping_start & 0xFFFFFFFF,
			mapping_length);
#else
		current_mapping = mmap(0, mapping_length, PROT_READ, MAP_PRIVATE, file_handle, mapping_start);
#endif

		if (!current_mapping) {
			throw _T("PCM audio provider: Failed mapping a view of the file");
		}

	}

	assert(range_start >= mapping_start);

	ptrdiff_t rel_ofs = (ptrdiff_t)(range_start - mapping_start);

	return ((char*)current_mapping) + rel_ofs;
}


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

class  RiffWavPCMAudioProvider : public PCMAudioProvider {
private:
	struct ChunkHeader {
		char type[4];
		uint32_t size; // XXX: Assume we're compiling on little endian
	};
	struct RIFFChunk {
		ChunkHeader ch;
		char format[4];
	};
	struct fmtChunk {
		// Skip the chunk header here, it's processed separately
		uint16_t compression; // compression format used -- 0x01 = PCM
		uint16_t channels;
		uint32_t samplerate;
		uint32_t avg_bytes_sec; // can't always be trusted
		uint16_t block_align;
		uint16_t significant_bits_sample;
		// Here was supposed to be some more fields but we don't need them
		// and just skipping by the size of the struct wouldn't be safe
		// either way, as the fields can depend on the compression.
	};

public:
	RiffWavPCMAudioProvider(const wxString &_filename)
		: PCMAudioProvider(_filename)
	{
		filename = _filename;

		// Read header
		// Assume we won't get files smaller than 256 bytes
		void *filestart = EnsureRangeAccessible(0, sizeof(RIFFChunk));
		RIFFChunk &header = *(RIFFChunk*)filestart;

		// Check that it's good
		if (strncmp(header.ch.type, "RIFF", 4)) throw _T("RIFF PCM WAV audio provider: File is not a RIFF file");
		if (strncmp(header.format, "WAVE", 4)) throw _T("RIFF PCM WAV audio provider: File is not a RIFF WAV file");

		// Count how much more data we can have in the entire file
		// The first 4 bytes are already eaten by the header.format field
		uint32_t data_left = header.ch.size - 4;
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

			if (strncmp(ch.type, "fmt ", 4) == 0) {
				if (got_fmt_header) throw _T("RIFF PCM WAV audio provider: Invalid file, multiple 'fmt ' chunks");
				got_fmt_header = true;

				fmtChunk &fmt = *(fmtChunk*)EnsureRangeAccessible(filepos, sizeof(fmtChunk));

				if (fmt.compression != 1) throw _T("RIFF PCM WAV audio provider: Can't use file, not PCM encoding");

				// Set stuff inherited from the AudioProvider class
				sample_rate = fmt.samplerate;
				channels = fmt.channels;
				bytes_per_sample = (fmt.significant_bits_sample + 7) / 8; // round up to nearest whole byte
			}

			else if (strncmp(ch.type, "data", 4) == 0) {
				// This won't pick up 'data' chunks inside 'wavl' chunks
				// since the 'wavl' chunks wrap those.

				int64_t samples = ch.size / bytes_per_sample;
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
			data_left -= (ch.size + 1) & ~1;
			filepos += (ch.size + 1) & ~1;
		}
	}
};


// Mix down any number of channels to mono

class DownmixingAudioProvider : public AudioProvider {
private:
	AudioProvider *provider;
	int src_channels;

public:
	DownmixingAudioProvider(AudioProvider *source)
	{
		filename = source->GetFilename();
		channels = 1; // target
		src_channels = source->GetChannels();
		num_samples = source->GetNumSamples();
		bytes_per_sample = source->GetBytesPerSample();
		sample_rate = source->GetSampleRate();

		// We now own this
		provider = source;

		if (!(bytes_per_sample == 1 || bytes_per_sample == 2)) throw _T("Downmixing Audio Provider: Can only downmix 8 and 16 bit audio");
	}

	~DownmixingAudioProvider()
	{
		delete provider;
	}

	void GetAudio(void *buf, int64_t start, int64_t count)
	{
		if (count == 0) return;

		// We can do this ourselves
		if (start >= num_samples) {
			if (bytes_per_sample == 1)
				// 8 bit formats are usually unsigned with bias 127
				memset(buf, 127, count);
			else
				// While everything else is signed
				memset(buf, 0, count*bytes_per_sample);

			return;
		}

		// So alloc some temporary memory for this
		// Depending on use, this might be made faster by using
		// a pre-allocced block of memory...?
		char *tmp = new char[count*bytes_per_sample*src_channels];

		provider->GetAudio(tmp, start, count);

		// Now downmix
		// Just average the samples over the channels (really bad if they're out of phase!)
		if (bytes_per_sample == 1) {
			uint8_t *src = (uint8_t *)tmp;
			uint8_t *dst = (uint8_t *)buf;

			while (count > 0) {
				int sum = 0;
				for (int c = 0; c < src_channels; c++)
					sum += *(src++);
				*(dst++) = (uint8_t)(sum / src_channels);
				count--;
			}
		}
		else if (bytes_per_sample == 2) {
			int16_t *src = (int16_t *)tmp;
			int16_t *dst = (int16_t *)buf;

			while (count > 0) {
				int sum = 0;
				for (int c = 0; c < src_channels; c++)
					sum += *(src++);
				*(dst++) = (int16_t)(sum / src_channels);
				count--;
			}
		}

		// Done downmixing, free the work buffer
		delete[] tmp;
	}
};


AudioProvider *CreatePCMAudioProvider(const wxString &filename)
{
	AudioProvider *provider = 0;

	// Try Microsoft/IBM RIFF WAV first
	// XXX: This is going to blow up if built on big endian archs
	try {
		provider = new RiffWavPCMAudioProvider(filename);
	}
	catch (...) {
		provider = 0;
	}

	if (provider && provider->GetChannels() > 1) {
		// Can't feed non-mono audio to the rest of the program.
		// Create a downmixing proxy and if it fails, don't provide PCM.
		try {
			provider = new DownmixingAudioProvider(provider);
		}
		catch (...) {
			delete provider;
			provider = 0;
		}
	}

	return provider;
}
