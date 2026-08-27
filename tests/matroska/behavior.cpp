#include <cstddef>
#include "MatroskaParser.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct FileInput final : InputStream {
	FILE *file{};
	std::string error;
	uint64_t size{};
	explicit FileInput(char const *name) {
		file = std::fopen(name, "rb");
		if (!file)
			return;
		std::fseek(file, 0, SEEK_END);
		size = std::ftell(file);
		std::fseek(file, 0, SEEK_SET);
		read = [](InputStream *s, uint64_t p, void *b, int n) {
			auto &i = *static_cast<FileInput *>(s);
			if (p >= i.size)
				return 0;
			n = std::min<uint64_t>(n, i.size - p);
			if (std::fseek(i.file, p, SEEK_SET))
				return -1;
			return (int)std::fread(b, 1, n, i.file);
		};
		scan = [](InputStream *s, uint64_t p, unsigned sig) -> int64_t {
			unsigned v = 0;
			unsigned char c;
			for (; static_cast<FileInput *>(s)->read(s, p, &c, 1) == 1; ++p) {
				v = (v << 8) | c;
				if (v == sig)
					return p - 3;
			}
			return -1;
		};
		getcachesize = [](InputStream *) { return 1U << 20; };
		geterror = [](InputStream *s) { return static_cast<FileInput *>(s)->error.c_str(); };
		memalloc = [](InputStream *, size_t n) { return std::malloc(n); };
		memrealloc = [](InputStream *, void *p, size_t n) { return std::realloc(p, n); };
		memfree = [](InputStream *, void *p) { std::free(p); };
		progress = [](InputStream *, uint64_t, uint64_t) { return 1; };
		getfilesize = [](InputStream *s) { return (int64_t)static_cast<FileInput *>(s)->size; };
	}
	~FileInput() {
		if (file)
			std::fclose(file);
	}
};
static uint64_t hash(FileInput &input, uint64_t position, unsigned size) {
	uint64_t h = 1469598103934665603ULL;
	std::vector<unsigned char> data(size);
	if (size && input.read(&input, position, data.data(), size) != static_cast<int>(size))
		return 0;
	for (auto byte : data) {
		h ^= byte;
		h *= 1099511628211ULL;
	}
	return h;
}
static uint64_t decoded_hash(MatroskaFile *file, unsigned track, uint64_t position, unsigned size) {
	char error[256]{};
	auto *stream = cs_Create(file, track, error, sizeof error);
	if (!stream)
		return 0;
	cs_NextFrame(stream, position, size);
	uint64_t h = 1469598103934665603ULL;
	char data[256];
	for (;;) {
		int bytes_read = cs_ReadData(stream, data, sizeof data);
		if (bytes_read <= 0)
			break;
		for (int i = 0; i < bytes_read; ++i) {
			h ^= static_cast<unsigned char>(data[i]);
			h *= 1099511628211ULL;
		}
	}
	cs_Destroy(stream);
	return h;
}
int main(int argc, char **argv) {
	if (argc != 2)
		return 2;
	FileInput input(argv[1]);
	char error[2048]{};
	auto *file = mkv_Open(&input, error, sizeof error);
	if (!file) {
		std::cout << "error\t" << error << '\n';
		return 1;
	}
	auto *segment = mkv_GetFileInfo(file);
	std::cout << "segment\t" << segment->TimecodeScale << '\t' << segment->Duration << '\n';
	for (unsigned track_index = 0; track_index < mkv_GetNumTracks(file); ++track_index) {
		auto *track = mkv_GetTrackInfo(file, track_index);
		std::cout << "track\t" << track_index << '\t' << unsigned(track->Number) << '\t' << unsigned(track->Type)
				  << '\t' << (track->CodecID ? track->CodecID : "") << '\t' << track->Language << '\t'
				  << (track->Name ? track->Name : "") << '\t' << track->CodecPrivateSize << '\t' << track->CompEnabled
				  << '\t' << track->CompMethod << '\n';
	}
	Attachment *attachments;
	unsigned attachment_count;
	mkv_GetAttachments(file, &attachments, &attachment_count);
	for (unsigned i = 0; i < attachment_count; ++i)
		std::cout << "attachment\t" << (attachments[i].Name ? attachments[i].Name : "") << '\t'
				  << (attachments[i].MimeType ? attachments[i].MimeType : "") << '\t' << attachments[i].Length << '\t'
				  << std::hex << hash(input, attachments[i].Position, attachments[i].Length) << std::dec << '\n';
	unsigned track, size, flags;
	uint64_t start, end, position;
	while (!mkv_ReadFrame(file, 0, &track, &start, &end, &position, &size, &flags)) {
		std::cout << "frame\t" << track << '\t' << start << '\t' << end << '\t' << size << '\t' << flags << '\t'
				  << std::hex << hash(input, position, size);
		if (mkv_GetTrackInfo(file, track)->CompEnabled)
			std::cout << '\t' << decoded_hash(file, track, position, size);
		std::cout << std::dec << '\n';
	}
	mkv_Close(file);
}
