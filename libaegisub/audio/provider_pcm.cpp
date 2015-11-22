// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "libaegisub/audio/provider.h"

#include "libaegisub/file_mapping.h"
#include "libaegisub/fs.h"
#include "libaegisub/make_unique.h"

#include <array>
#include <vector>

namespace {
using namespace agi;

struct IndexPoint {
	uint64_t start_byte;
	uint64_t num_samples;
};

struct file_ended {};

class PCMAudioProvider : public AudioProvider {
	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto write_buf = static_cast<char *>(buf);
		auto bps = bytes_per_sample * channels;
		uint64_t pos = 0;

		for (auto ip : index_points) {
			if (count == 0) break;
			if (pos + ip.num_samples <= (uint64_t)start) {
				pos += ip.num_samples;
				continue;
			}

			auto read_offset = start - pos;
			auto read_count = std::min<size_t>(count, ip.num_samples - read_offset);
			auto bytes = read_count * bps;
			memcpy(write_buf, file.read(ip.start_byte + read_offset * bps, bytes), bytes);

			write_buf += bytes;
			count -= read_count;
			start += read_count;
			pos += ip.num_samples;
		}
	}

protected:
	mutable read_file_mapping file;
	uint64_t file_pos = 0;

	PCMAudioProvider(fs::path const& filename) : file(filename) { }

	template<typename T, typename UInt>
	T Read(UInt *data_left) {
		if (*data_left < sizeof(T)) throw file_ended();
		if (file.size() - file_pos < sizeof(T)) throw file_ended();

		auto data = file.read(file_pos, sizeof(T));
		file_pos += sizeof(T);
		*data_left -= sizeof(T);
		T ret;
		memcpy(&ret, data, sizeof(T));
		return ret;
	}

	std::vector<IndexPoint> index_points;
};

struct FourCC {
	std::array<char, 4> data;
	bool operator!=(const char *cmp) const {
		return data[0] != cmp[0] || data[1] != cmp[1]
			|| data[2] != cmp[2] || data[3] != cmp[3];
	}
	bool operator==(const char *cmp) const { return !(*this != cmp); }
};

// Overview of RIFF WAV: <http://www.sonicspot.com/guide/wavefiles.html>
struct RiffWav {
	using DataSize = uint32_t;
	using ChunkId = FourCC;

	static const char *riff_id() { return "RIFF"; }
	static const char *wave_id() { return "WAVE"; }
	static const char *fmt_id()  { return "fmt "; }
	static const char *data_id() { return "data "; }

	static const int alignment = 1;

	static uint32_t data_size(uint32_t size) { return size; }
	static uint32_t chunk_size(uint32_t size) { return size; }
};

typedef std::array<uint8_t, 16> GUID;

static const GUID w64GuidRIFF = {{
	// {66666972-912E-11CF-A5D6-28DB04C10000}
	0x72, 0x69, 0x66, 0x66, 0x2E, 0x91, 0xCF, 0x11, 0xA5, 0xD6, 0x28, 0xDB, 0x04, 0xC1, 0x00, 0x00
}};

static const GUID w64GuidWAVE = {{
	// {65766177-ACF3-11D3-8CD1-00C04F8EDB8A}
	0x77, 0x61, 0x76, 0x65, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A
}};

static const GUID w64Guidfmt = {{
	// {20746D66-ACF3-11D3-8CD1-00C04F8EDB8A}
	0x66, 0x6D, 0x74, 0x20, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A
}};

static const GUID w64Guiddata = {{
	// {61746164-ACF3-11D3-8CD1-00C04F8EDB8A}
	0x64, 0x61, 0x74, 0x61, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A
}};

// http://www.vcs.de/fileadmin/user_upload/MBS/PDF/Whitepaper/Informations_about_Sony_Wave64.pdf
struct Wave64 {
	using DataSize = uint64_t;
	using ChunkId = GUID;

	static GUID riff_id() { return w64GuidRIFF; }
	static GUID wave_id() { return w64GuidWAVE; }
	static GUID fmt_id()  { return w64Guidfmt; }
	static GUID data_id() { return w64Guiddata; }

	static const uint64_t alignment = 7ULL;

	// Wave 64 includes the size of the header in the chunk sizes
	static uint64_t data_size(uint64_t size) { return size - 16; }
	static uint64_t chunk_size(uint64_t size) { return size - 24; }
};

template<typename Impl>
class WavPCMAudioProvider : public PCMAudioProvider {
public:
	WavPCMAudioProvider(fs::path const& filename)
	: PCMAudioProvider(filename)
	{
		using DataSize = typename Impl::DataSize;
		using ChunkId = typename Impl::ChunkId;

		try {
			auto data_left = std::numeric_limits<DataSize>::max();
			if (Read<ChunkId>(&data_left) != Impl::riff_id())
				throw AudioDataNotFound("File is not a RIFF file");

			data_left = Impl::data_size(Read<DataSize>(&data_left));

			if (Read<ChunkId>(&data_left) != Impl::wave_id())
				throw AudioDataNotFound("File is not a RIFF WAV file");

			while (data_left) {
				auto chunk_fcc = Read<ChunkId>(&data_left);
				auto chunk_size = Impl::chunk_size(Read<DataSize>(&data_left));

				data_left -= chunk_size;

				if (chunk_fcc == Impl::fmt_id()) {
					if (channels || sample_rate || bytes_per_sample)
						throw AudioProviderError("Multiple 'fmt ' chunks not supported");

					auto compression = Read<uint16_t>(&chunk_size);
					if (compression != 1)
						throw AudioProviderError("File is not uncompressed PCM");

					channels = Read<uint16_t>(&chunk_size);
					sample_rate = Read<uint32_t>(&chunk_size);
					Read<uint32_t>(&chunk_size); // Average bytes per sample; meaningless
					Read<uint16_t>(&chunk_size); // Block align
					bytes_per_sample = (Read<uint16_t>(&chunk_size) + 7) / 8;
				}
				else if (chunk_fcc == Impl::data_id()) {
					if (!channels || !sample_rate || !bytes_per_sample)
						throw AudioProviderError("Found 'data' chunk without format being set.");
					index_points.emplace_back(IndexPoint{file_pos, chunk_size / bytes_per_sample / channels});
					num_samples += chunk_size / bytes_per_sample / channels;
				}
				// There's a bunch of other chunk types. They're all dumb.

				// blocks are aligned and the padding bytes are not included in
				// the size of the chunk
				file_pos += (chunk_size + Impl::alignment) & ~Impl::alignment;
			}

		}
		catch (file_ended) {
			if (!channels || !sample_rate || !bytes_per_sample)
				throw AudioDataNotFound("File ended before reaching format chunk");
			// Truncated files are fine otherwise
		}
		decoded_samples = num_samples;
	}
};
}

namespace agi {
std::unique_ptr<AudioProvider> CreatePCMAudioProvider(fs::path const& filename, BackgroundRunner *) {
	bool wrong_file_type = true;
	std::string msg;

	try {
		return make_unique<WavPCMAudioProvider<RiffWav>>(filename);
	}
	catch (AudioDataNotFound const& err) {
		msg = "RIFF PCM WAV audio provider: " + err.GetMessage();
	}
	catch (AudioProviderError const& err) {
		wrong_file_type = false;
		msg = "RIFF PCM WAV audio provider: " + err.GetMessage();
	}

	try {
		return make_unique<WavPCMAudioProvider<Wave64>>(filename);
	}
	catch (AudioDataNotFound const& err) {
		msg += "\nWave64 audio provider: " + err.GetMessage();
	}
	catch (AudioProviderError const& err) {
		wrong_file_type = false;
		msg += "\nWave64 audio provider: " + err.GetMessage();
	}

	if (wrong_file_type)
		throw AudioDataNotFound(msg);
	else
		throw AudioProviderError(msg);
}
}
