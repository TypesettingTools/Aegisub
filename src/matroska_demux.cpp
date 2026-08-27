// Copyright (c) 2026 Aegisub contributors
// Native Matroska demux implementation backed by libebml and libmatroska.
#include <cstddef>
#include "matroska_demux.h"

#include <ebml/EbmlBinary.h>
#include <ebml/EbmlFloat.h>
#include <ebml/EbmlHead.h>
#include <ebml/EbmlMaster.h>
#include <ebml/EbmlStream.h>
#include <ebml/EbmlString.h>
#include <ebml/EbmlUInteger.h>
#include <ebml/EbmlUnicodeString.h>
#include <ebml/IOCallback.h>
#include <matroska/KaxBlock.h>
#include <matroska/KaxBlockData.h>
#include <matroska/KaxCluster.h>
#include <matroska/KaxSegment.h>
#include <matroska/KaxSemantic.h>
#include <matroska/KaxTracks.h>
#include <zlib.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace libebml;
using namespace libmatroska;

namespace {
class InputCallback final : public IOCallback {
	InputStream *in;
	uint64_t pos = 0;

  public:
	explicit InputCallback(InputStream *input, uint64_t base) : in(input), pos(base) {}
	uint32 read(void *buffer, size_t size) override {
		if (!in || !in->read)
			throw std::runtime_error("Matroska input has no read callback");
		if (size > INT_MAX)
			size = INT_MAX;
		int got = in->read(in, pos, buffer, static_cast<int>(size));
		if (got < 0)
			throw std::runtime_error(in->geterror ? in->geterror(in) : "input error");
		if (static_cast<size_t>(got) > size)
			throw std::runtime_error("Matroska input returned too much data");
		pos += got;
		return got;
	}
	void setFilePointer(int64 offset, seek_mode mode) override {
		int64 next = offset;
		if (mode == seek_current)
			next += pos;
		else if (mode == seek_end)
			next += in->getfilesize(in);
		if (next < 0)
			throw std::runtime_error("invalid negative seek");
		pos = static_cast<uint64_t>(next);
	}
	size_t write(void const *, size_t) override {
		throw std::runtime_error("read-only input");
	}
	uint64 getFilePointer() override {
		return pos;
	}
	void close() override {}
};

template <class T> T *child(EbmlMaster &m) {
	return static_cast<T *>(m.FindFirstElt(EBML_INFO(T), false));
}
template <class T> uint64_t uint_value(EbmlMaster &m, uint64_t fallback = 0) {
	if (auto *v = child<T>(m))
		return static_cast<uint64_t>(*v);
	return fallback;
}
template <class T> std::string string_value(EbmlMaster &m, std::string fallback = {}) {
	if (auto *v = child<T>(m))
		return v->GetValue();
	return fallback;
}
template <class T> std::string unicode_value(EbmlMaster &m) {
	if (auto *v = child<T>(m))
		return v->GetValue().GetUTF8();
	return {};
}

struct TrackRecord {
	TrackInfo info{};
	std::string name, codec, language = "und";
	std::vector<unsigned char> priv, comp_private;
	KaxTrackEntry *entry{};
};
struct Frame {
	unsigned track{};
	uint64_t start{}, end{}, pos{};
	unsigned size{}, flags{};
};
struct AttachmentRecord {
	Attachment info{};
	std::string name, description, mime;
};
} // namespace

struct MatroskaFile {
	InputStream *input{};
	std::unique_ptr<InputCallback> io;
	std::unique_ptr<EbmlStream> stream;
	std::unique_ptr<EbmlElement> segment;
	std::vector<std::unique_ptr<EbmlElement>> top;
	SegmentInfo info{};
	std::vector<TrackRecord> tracks;
	std::vector<Frame> frames;
	std::vector<AttachmentRecord> attachments;
	std::vector<Attachment> attachment_view;
	std::unordered_map<uint64_t, unsigned> track_by_number;
	size_t cursor{};
	unsigned mask{};
	uint64_t segment_top{};
	std::string error;
};

namespace {
void parse_track(MatroskaFile &file, KaxTrackEntry &entry) {
	TrackRecord track;
	track.entry = &entry;
	track.info.Number = static_cast<unsigned char>(uint_value<KaxTrackNumber>(entry));
	track.info.UID = uint_value<KaxTrackUID>(entry);
	track.info.Type = static_cast<unsigned char>(uint_value<KaxTrackType>(entry));
	track.info.Enabled = uint_value<KaxTrackFlagEnabled>(entry, 1);
	track.info.Default = uint_value<KaxTrackFlagDefault>(entry, 1);
	track.info.Lacing = uint_value<KaxTrackFlagLacing>(entry, 1);
	track.info.MinCache = uint_value<KaxTrackMinCache>(entry);
	track.info.MaxCache = uint_value<KaxTrackMaxCache>(entry);
	track.info.DefaultDuration = uint_value<KaxTrackDefaultDuration>(entry);
	track.info.TimecodeScale =
		child<KaxTrackTimecodeScale>(entry) ? static_cast<double>(*child<KaxTrackTimecodeScale>(entry)) : 1.0;
	track.name = unicode_value<KaxTrackName>(entry);
	track.codec = string_value<KaxCodecID>(entry);
	track.language = string_value<KaxTrackLanguage>(entry, "eng");
	if (auto *codec_private = child<KaxCodecPrivate>(entry))
		track.priv.assign(codec_private->GetBuffer(), codec_private->GetBuffer() + codec_private->GetSize());
	if (auto *encodings = child<KaxContentEncodings>(entry))
		if (auto *encoding = child<KaxContentEncoding>(*encodings)) {
			if (uint_value<KaxContentEncodingType>(*encoding, 0) != 0)
				throw std::runtime_error("encrypted tracks are unsupported");
			if (auto *compression = child<KaxContentCompression>(*encoding)) {
				track.info.CompEnabled = 1;
				track.info.CompMethod = static_cast<unsigned>(uint_value<KaxContentCompAlgo>(*compression));
				if (auto *settings = child<KaxContentCompSettings>(*compression))
					track.comp_private.assign(settings->GetBuffer(), settings->GetBuffer() + settings->GetSize());
			}
		}
	file.track_by_number[track.info.Number] = file.tracks.size();
	file.tracks.push_back(std::move(track));
}

void parse_attachment(MatroskaFile &file, KaxAttached &element) {
	AttachmentRecord attachment;
	attachment.name = unicode_value<KaxFileName>(element);
	attachment.description = unicode_value<KaxFileDescription>(element);
	attachment.mime = string_value<KaxMimeType>(element);
	attachment.info.UID = uint_value<KaxFileUID>(element);

	if (auto *data = child<KaxFileData>(element)) {
		// File data is not copied. The legacy API exposes its location in the input stream.
		attachment.info.Position =
			data->GetElementPosition() + EBML_ID_LENGTH(static_cast<EbmlId const &>(*data)) + data->GetSizeLength();
		attachment.info.Length = data->GetSize();
	}

	file.attachments.push_back(std::move(attachment));
}

void parse_block(MatroskaFile &file, EbmlMaster &parent, KaxCluster *cluster, KaxInternalBlock &block) {
	auto track = file.track_by_number.find(block.TrackNum());
	if (track == file.track_by_number.end())
		return;

	auto track_index = track->second;
	uint64_t frame_duration = file.tracks[track_index].info.DefaultDuration;
	if (auto *group = dynamic_cast<KaxBlockGroup *>(&parent))
		if (auto *duration = child<KaxBlockDuration>(*group))
			frame_duration = static_cast<uint64_t>(*duration) * file.info.TimecodeScale;

	auto frame_count = block.NumberFrames();
	if (frame_count && frame_duration && dynamic_cast<KaxBlockGroup *>(&parent))
		frame_duration /= frame_count;

	int64_t cluster_ticks = cluster ? static_cast<int64_t>(uint_value<KaxClusterTimecode>(*cluster)) : 0;
	int64_t frame_ticks = cluster_ticks + block.GetRelativeTimestamp();
	uint64_t first_frame_time = frame_ticks < 0 ? 0 : static_cast<uint64_t>(frame_ticks) * file.info.TimecodeScale;

	bool is_keyframe = true;
	if (auto *simple_block = dynamic_cast<KaxSimpleBlock *>(&block))
		is_keyframe = simple_block->IsKeyframe();
	else if (auto *group = dynamic_cast<KaxBlockGroup *>(&parent))
		is_keyframe = child<KaxReferenceBlock>(*group) == nullptr;

	for (unsigned frame_index = 0; frame_index < frame_count; ++frame_index) {
		Frame frame;
		frame.track = track_index;
		frame.start = first_frame_time + frame_index * frame_duration;
		frame.end = frame.start + frame_duration;
		frame.pos = block.GetDataPosition(frame_index);
		frame.size = static_cast<unsigned>(block.GetFrameSize(frame_index));
		frame.flags = is_keyframe ? FRAME_KF : 0;
		file.frames.push_back(frame);
	}
}

void parse_master(MatroskaFile &file, EbmlMaster &master, KaxCluster *cluster = nullptr) {
	for (auto *element : master.GetElementList()) {
		if (auto *entry = dynamic_cast<KaxTrackEntry *>(element))
			parse_track(file, *entry);
		else if (auto *attachment = dynamic_cast<KaxAttached *>(element))
			parse_attachment(file, *attachment);
		else if (auto *block = dynamic_cast<KaxInternalBlock *>(element))
			parse_block(file, master, cluster, *block);

		if (auto *nested = dynamic_cast<EbmlMaster *>(element))
			parse_master(file, *nested, cluster ? cluster : dynamic_cast<KaxCluster *>(nested));
	}
}

void finalize(MatroskaFile &f) {
	for (auto &r : f.tracks) {
		r.info.Name = r.name.empty() ? nullptr : r.name.data();
		r.info.CodecID = r.codec.data();
		std::memset(r.info.Language, 0, 4);
		std::memcpy(r.info.Language, r.language.data(), std::min<size_t>(3, r.language.size()));
		r.info.CodecPrivate = r.priv.empty() ? nullptr : r.priv.data();
		r.info.CodecPrivateSize = r.priv.size();
		r.info.CompMethodPrivate = r.comp_private.empty() ? nullptr : r.comp_private.data();
		r.info.CompMethodPrivateSize = r.comp_private.size();
	}
	for (auto &r : f.attachments) {
		r.info.Name = r.name.data();
		r.info.Description = r.description.empty() ? nullptr : r.description.data();
		r.info.MimeType = r.mime.data();
		f.attachment_view.push_back(r.info);
	}
	std::stable_sort(f.frames.begin(), f.frames.end(), [](auto &a, auto &b) { return a.start < b.start; });
	if (!f.info.Duration)
		for (auto const &frame : f.frames)
			f.info.Duration = std::max(f.info.Duration, frame.end);
}
} // namespace

extern "C" {
MatroskaFile *mkv_Open(InputStream *io, char *msg, unsigned n) {
	return mkv_OpenEx(io, 0, 0, msg, n);
}
MatroskaFile *mkv_OpenEx(InputStream *input, uint64_t base, unsigned, char *msg, unsigned n) {
	try {
		if (!input || !input->read || !input->getfilesize)
			throw std::runtime_error("invalid Matroska input stream");
		auto input_size = input->getfilesize(input);
		if (input_size < 0 || base > static_cast<uint64_t>(input_size))
			throw std::runtime_error("invalid Matroska input size");
		auto f = std::make_unique<MatroskaFile>();
		f->input = input;
		f->io = std::make_unique<InputCallback>(input, base);
		f->stream = std::make_unique<EbmlStream>(*f->io);
		int upper = 0;
		std::unique_ptr<EbmlElement> head(
			f->stream->FindNextID(EBML_INFO(EbmlHead), std::numeric_limits<uint64_t>::max()));
		if (!head)
			throw std::runtime_error("EBML header not found");
		head->SkipData(*f->stream, EBML_CONTEXT(head.get()));
		std::unique_ptr<EbmlElement> e(
			f->stream->FindNextID(EBML_INFO(KaxSegment), std::numeric_limits<uint64_t>::max()));
		if (!e)
			throw std::runtime_error("Matroska segment not found");
		f->segment_top = e->GetElementPosition();
		if (std::strcmp(EBML_NAME(e.get()), "Segment"))
			throw std::runtime_error(std::string("expected Segment, got ") + EBML_NAME(e.get()));
		auto *segment = static_cast<KaxSegment *>(e.get());
		f->segment = std::move(e);
		f->info.TimecodeScale = 1000000;
		EbmlElement *next =
			f->stream->FindNextElement(EBML_CONTEXT(segment), upper, std::numeric_limits<uint64_t>::max(), true);
		while (next && upper <= 0) {
			std::unique_ptr<EbmlElement> current(next);
			next = nullptr;
			bool subtitle_frames =
				std::any_of(f->tracks.begin(), f->tracks.end(), [](auto const &t) { return t.info.Type == TT_SUB; });
			if (dynamic_cast<KaxCluster *>(current.get()) && !subtitle_frames)
				break;
			bool wanted = dynamic_cast<KaxInfo *>(current.get()) || dynamic_cast<KaxTracks *>(current.get()) ||
						  (dynamic_cast<KaxCluster *>(current.get()) && subtitle_frames) ||
						  dynamic_cast<KaxAttachments *>(current.get());
			if (wanted) {
				auto *master = dynamic_cast<EbmlMaster *>(current.get());
				master->Read(*f->stream, EBML_CONTEXT(current.get()), upper, next, true, SCOPE_ALL_DATA);
				if (auto *info = dynamic_cast<KaxInfo *>(master)) {
					f->info.TimecodeScale = uint_value<KaxTimecodeScale>(*info, 1000000);
					if (auto *d = child<KaxDuration>(*info))
						f->info.Duration = static_cast<uint64_t>(static_cast<double>(*d) * f->info.TimecodeScale);
				}
				parse_master(*f, *master, dynamic_cast<KaxCluster *>(master));
				f->top.push_back(std::move(current));
			} else
				current->SkipData(*f->stream, EBML_CONTEXT(segment));
			if (!next)
				next = f->stream->FindNextElement(EBML_CONTEXT(segment), upper, std::numeric_limits<uint64_t>::max(),
												  true);
		}
		finalize(*f);
		return f.release();
	} catch (std::exception const &e) {
		if (msg && n) {
			std::strncpy(msg, e.what(), n - 1);
			msg[n - 1] = 0;
		}
		return nullptr;
	} catch (...) {
		if (msg && n) {
			std::strncpy(msg, "unknown Matroska parsing error", n - 1);
			msg[n - 1] = 0;
		}
		return nullptr;
	}
}
void mkv_Close(MatroskaFile *f) {
	delete f;
}
const char *mkv_GetLastError(MatroskaFile *f) {
	return f ? f->error.c_str() : "invalid Matroska handle";
}
SegmentInfo *mkv_GetFileInfo(MatroskaFile *f) {
	return &f->info;
}
unsigned mkv_GetNumTracks(MatroskaFile *f) {
	return f->tracks.size();
}
TrackInfo *mkv_GetTrackInfo(MatroskaFile *f, unsigned i) {
	return i < f->tracks.size() ? &f->tracks[i].info : nullptr;
}
void mkv_GetAttachments(MatroskaFile *f, Attachment **a, unsigned *n) {
	*a = f->attachment_view.empty() ? nullptr : f->attachment_view.data();
	*n = f->attachment_view.size();
}
void mkv_GetChapters(MatroskaFile *, Chapter **c, unsigned *n) {
	*c = nullptr;
	*n = 0;
}
void mkv_GetTags(MatroskaFile *, Tag **t, unsigned *n) {
	*t = nullptr;
	*n = 0;
}
uint64_t mkv_GetSegmentTop(MatroskaFile *f) {
	return f->segment_top;
}
void mkv_SetTrackMask(MatroskaFile *f, unsigned m) {
	f->mask = m;
	f->cursor = 0;
}
int mkv_ReadFrame(MatroskaFile *f, unsigned mask, unsigned *track, uint64_t *start, uint64_t *end, uint64_t *pos,
				  unsigned *size, unsigned *flags) {
	while (f->cursor < f->frames.size()) {
		auto &r = f->frames[f->cursor++];
		if (((mask | f->mask) >> r.track) & 1)
			continue;
		*track = r.track;
		*start = r.start;
		*end = r.end;
		*pos = r.pos;
		*size = r.size;
		*flags = r.flags;
		return 0;
	}
	return -1;
}
void mkv_Seek(MatroskaFile *f, uint64_t t, unsigned) {
	f->cursor =
		std::lower_bound(f->frames.begin(), f->frames.end(), t, [](auto &a, uint64_t b) { return a.start < b; }) -
		f->frames.begin();
}
void mkv_SkipToKeyframe(MatroskaFile *) {}
uint64_t mkv_GetLowestQTimecode(MatroskaFile *f) {
	return f->cursor < f->frames.size() ? f->frames[f->cursor].start : 0;
}
int mkv_TruncFloat(MKFLOAT v) {
	return static_cast<int>(v);
}
}

struct CompressedStream {
	MatroskaFile *file;
	unsigned track;
	uint64_t frame_position{};
	unsigned frame_size{};
	std::vector<unsigned char> data;
	size_t cursor{};
	std::string error;
};

namespace {
std::vector<unsigned char> read_frame(CompressedStream const &stream) {
	std::vector<unsigned char> frame(stream.frame_size);
	auto bytes_read =
		stream.file->input->read(stream.file->input, stream.frame_position, frame.data(), stream.frame_size);
	if (bytes_read != static_cast<int>(stream.frame_size))
		throw std::runtime_error("short frame read");
	return frame;
}

std::vector<unsigned char> inflate_frame(std::vector<unsigned char> const &input) {
	z_stream zstream{};
	if (inflateInit(&zstream) != Z_OK)
		throw std::runtime_error("zlib initialization failed");

	struct EndInflate {
		z_stream *stream;
		~EndInflate() {
			inflateEnd(stream);
		}
	} end_inflate{&zstream};

	zstream.next_in = const_cast<unsigned char *>(input.data());
	zstream.avail_in = input.size();

	std::vector<unsigned char> output;
	unsigned char buffer[4096];
	int result;
	do {
		zstream.next_out = buffer;
		zstream.avail_out = sizeof buffer;
		result = inflate(&zstream, Z_NO_FLUSH);
		output.insert(output.end(), buffer, buffer + sizeof buffer - zstream.avail_out);
	} while (result == Z_OK);

	if (result != Z_STREAM_END)
		throw std::runtime_error("invalid zlib frame");
	return output;
}

void decode_frame(CompressedStream &stream) {
	auto raw_frame = read_frame(stream);
	auto const &track = stream.file->tracks[stream.track];

	if (!track.info.CompEnabled)
		stream.data = std::move(raw_frame);
	else if (track.info.CompMethod == COMP_PREPEND) {
		stream.data = track.comp_private;
		stream.data.insert(stream.data.end(), raw_frame.begin(), raw_frame.end());
	} else if (track.info.CompMethod == COMP_ZLIB)
		stream.data = inflate_frame(raw_frame);
	else
		throw std::runtime_error("unsupported Matroska compression method");
}
} // namespace

extern "C" {
CompressedStream *cs_Create(MatroskaFile *file, unsigned track, char *message, unsigned message_size) {
	if (!file || track >= file->tracks.size()) {
		if (message && message_size)
			std::strncpy(message, "invalid track", message_size);
		return nullptr;
	}
	return new CompressedStream{file, track, 0, 0, {}, {}, {}};
}
void cs_Destroy(CompressedStream *c) {
	delete c;
}
void cs_NextFrame(CompressedStream *stream, uint64_t position, unsigned size) {
	stream->frame_position = position;
	stream->frame_size = size;
	stream->data.clear();
	stream->cursor = 0;
}
int cs_ReadData(CompressedStream *stream, char *output, unsigned capacity) {
	try {
		if (stream->data.empty() && stream->frame_size)
			decode_frame(*stream);

		size_t bytes_to_copy = std::min<size_t>(capacity, stream->data.size() - stream->cursor);
		std::memcpy(output, stream->data.data() + stream->cursor, bytes_to_copy);
		stream->cursor += bytes_to_copy;
		return bytes_to_copy;
	} catch (std::exception const &e) {
		stream->error = e.what();
		return -1;
	}
}
const char *cs_GetLastError(CompressedStream *c) {
	return c->error.c_str();
}
}
