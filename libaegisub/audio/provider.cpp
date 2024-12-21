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

#include "libaegisub/fs.h"
#include "libaegisub/io.h"
#include "libaegisub/log.h"
#include "libaegisub/util.h"

namespace agi {
void AudioProvider::GetAudioWithVolume(void *buf, int64_t start, int64_t count, double volume) const {
	GetAudio(buf, start, count);

	if (volume == 1.0) return;
	if (bytes_per_sample != 2)
		throw agi::InternalError("GetAudioWithVolume called on unconverted audio stream");

	auto buffer = static_cast<int16_t *>(buf);
	for (size_t i = 0; i < (size_t)count; ++i)
		buffer[i] = util::mid(-0x8000, static_cast<int>(buffer[i] * volume + 0.5), 0x7FFF);
}

void AudioProvider::ZeroFill(void *buf, int64_t count) const {
	if (bytes_per_sample == 1)
		// 8 bit formats are usually unsigned with bias 128
		memset(buf, 128, count * channels);
	else // While everything else is signed
		memset(buf, 0, count * bytes_per_sample * channels);
}

void AudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
	if (start < 0) {
		ZeroFill(buf, std::min(-start, count));
		buf = static_cast<char *>(buf) + -start * bytes_per_sample * channels;
		count += start;
		start = 0;
	}

	if (start + count > num_samples) {
		int64_t zero_count = std::min(count, start + count - num_samples);
		count -= zero_count;
		ZeroFill(static_cast<char *>(buf) + count * bytes_per_sample * channels, zero_count);
	}

	if (count <= 0) return;

	try {
		FillBuffer(buf, start, count);
	}
	catch (AudioDecodeError const& e) {
		// We don't have any good way to report errors here, so just log the
		// failure and return silence
		LOG_E("audio_provider") << e.GetMessage();
		ZeroFill(buf, count);
		return;
	}
	catch (...) {
		LOG_E("audio_provider") << "Unknown audio decoding error";
		ZeroFill(buf, count);
		return;
	}
}

namespace {
class writer {
	io::Save outfile;
	std::ostream& out;

public:
	writer(agi::fs::path const& filename) : outfile(filename, true), out(outfile.Get()) { }

	template<int N>
	void write(const char(&str)[N]) {
		out.write(str, N - 1);
	}

	void write(std::vector<char> const& data) {
		out.write(data.data(), data.size());
	}

	template<typename Dest, typename Src>
	void write(Src v) {
		auto converted = static_cast<Dest>(v);
		out.write(reinterpret_cast<char *>(&converted), sizeof(Dest));
	}
};
}

void SaveAudioClip(AudioProvider const& provider, fs::path const& path, int start_time, int end_time) {
	const auto max_samples = provider.GetNumSamples();
	const auto start_sample = std::min(max_samples, ((int64_t)start_time * provider.GetSampleRate() + 999) / 1000);
	const auto end_sample = util::mid(start_sample, ((int64_t)end_time * provider.GetSampleRate() + 999) / 1000, max_samples);

	const size_t bytes_per_sample = provider.GetBytesPerSample() * provider.GetChannels();
	const size_t bufsize = (end_sample - start_sample) * bytes_per_sample;

	writer out{path};
	out.write("RIFF");
	out.write<int32_t>(bufsize + 36);

	out.write("WAVEfmt ");
	out.write<int32_t>(16); // Size of chunk
	out.write<int16_t>(1);  // compression format (PCM)
	out.write<int16_t>(provider.GetChannels());
	out.write<int32_t>(provider.GetSampleRate());
	out.write<int32_t>(provider.GetSampleRate() * provider.GetChannels() * provider.GetBytesPerSample());
	out.write<int16_t>(provider.GetChannels() * provider.GetBytesPerSample());
	out.write<int16_t>(provider.GetBytesPerSample() * 8);

	out.write("data");
	out.write<int32_t>(bufsize);

	// samples per read
	size_t spr = 65536 / bytes_per_sample;
	std::vector<char> buf;
	for (int64_t i = start_sample; i < end_sample; i += spr) {
		spr = std::min<size_t>(spr, end_sample - i);
		buf.resize(spr * bytes_per_sample);
		provider.GetAudio(&buf[0], i, spr);
		out.write(buf);
	}
}
}
