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
#include <climits>
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
	uint64_t number{};
	std::string name, codec, language = "und";
	std::vector<unsigned char> priv, comp_private;
	KaxTrackEntry *entry{};
};
struct Frame {
	unsigned track{};
	uint64_t start{}, end{}, pos{};
	unsigned size{}, flags{};
};
struct ClusterLocation {
	uint64_t data{}, size{};
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
	std::vector<ClusterLocation> clusters;
	std::vector<AttachmentRecord> attachments;
	std::vector<Attachment> attachment_view;
	std::unordered_map<uint64_t, unsigned> track_by_number;
	size_t cursor{};
	size_t cluster_cursor{};
	unsigned mask{};
	uint64_t segment_top{};
	std::string error;
};

namespace {
void parse_track(MatroskaFile &file, KaxTrackEntry &entry) {
	TrackRecord track;
	track.entry = &entry;
	track.number = uint_value<KaxTrackNumber>(entry);
	track.info.Number = static_cast<unsigned char>(track.number);
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
	if (!file.track_by_number.emplace(track.number, file.tracks.size()).second)
		throw std::runtime_error("duplicate Matroska track number");
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

struct RawElement {
	uint64_t id{}, data{}, size{}, end{};
};

void read_exact(MatroskaFile &file, uint64_t pos, void *buffer, size_t size) {
	if (size > static_cast<size_t>(INT_MAX) || pos > static_cast<uint64_t>(INT64_MAX))
		throw std::runtime_error("Matroska read is out of range");
	int got = file.input->read(file.input, pos, buffer, static_cast<int>(size));
	if (got != static_cast<int>(size)) throw std::runtime_error("truncated Matroska input");
}

uint8_t read_byte(MatroskaFile &file, uint64_t pos) {
	uint8_t byte;
	read_exact(file, pos, &byte, 1);
	return byte;
}

RawElement raw_element(MatroskaFile &file, uint64_t pos, uint64_t parent_end) {
	uint8_t first = read_byte(file, pos);
	unsigned id_length = 1;
	for (uint8_t mask = 0x80; id_length <= 4 && !(first & mask); mask >>= 1) ++id_length;
	if (id_length > 4) throw std::runtime_error("invalid Matroska element ID");
	uint64_t id = first;
	for (unsigned i = 1; i < id_length; ++i) id = (id << 8) | read_byte(file, pos + i);

	uint64_t size_pos = pos + id_length;
	first = read_byte(file, size_pos);
	unsigned size_length = 1;
	uint8_t marker = 0x80;
	while (size_length <= 8 && !(first & marker)) { marker >>= 1; ++size_length; }
	if (size_length > 8) throw std::runtime_error("invalid Matroska element size");
	uint64_t size = first & (marker - 1);
	for (unsigned i = 1; i < size_length; ++i) size = (size << 8) | read_byte(file, size_pos + i);
	uint64_t unknown = (uint64_t{1} << (7 * size_length)) - 1;
	uint64_t data = size_pos + size_length;
	if (size == unknown) size = parent_end - data;
	if (data > parent_end || size > parent_end - data) throw std::runtime_error("Matroska element exceeds its parent");
	return {id, data, size, data + size};
}

uint64_t raw_uint(MatroskaFile &file, RawElement const& element) {
	if (!element.size || element.size > 8) throw std::runtime_error("invalid Matroska integer");
	uint64_t value = 0;
	for (uint64_t i = 0; i < element.size; ++i) value = (value << 8) | read_byte(file, element.data + i);
	return value;
}

std::pair<uint64_t, unsigned> raw_vint(MatroskaFile &file, uint64_t pos, uint64_t end) {
	if (pos >= end) throw std::runtime_error("truncated Matroska variable integer");
	uint8_t first = read_byte(file, pos);
	unsigned length = 1;
	uint8_t marker = 0x80;
	while (length <= 8 && !(first & marker)) { marker >>= 1; ++length; }
	if (length > 8) throw std::runtime_error("invalid Matroska variable integer");
	if (length > end - pos) throw std::runtime_error("truncated Matroska variable integer");
	uint64_t value = first & (marker - 1);
	for (unsigned i = 1; i < length; ++i) value = (value << 8) | read_byte(file, pos + i);
	return {value, length};
}

uint64_t checked_add(uint64_t left, uint64_t right, char const *message) {
	if (right > UINT64_MAX - left) throw std::runtime_error(message);
	return left + right;
}

uint64_t checked_multiply(uint64_t left, uint64_t right, char const *message) {
	if (left && right > UINT64_MAX / left) throw std::runtime_error(message);
	return left * right;
}

void parse_raw_block(MatroskaFile &file, RawElement const& block, uint64_t cluster_time,
	uint64_t duration, bool duration_known, bool keyframe) {
	auto [track_number, track_bytes] = raw_vint(file, block.data, block.end);
	auto found = file.track_by_number.find(track_number);
	if (found == file.track_by_number.end()) return;
	unsigned track_index = found->second;
	if (track_index >= 32 || ((file.mask >> track_index) & 1)) return;
	if (block.size < track_bytes + 3) throw std::runtime_error("truncated Matroska block header");
	uint64_t cursor = checked_add(block.data, track_bytes, "Matroska block header is out of range");
	int16_t relative = static_cast<int16_t>((read_byte(file, cursor) << 8) | read_byte(file, cursor + 1));
	uint8_t flags = read_byte(file, cursor + 2);
	cursor += 3;
	unsigned lace = (flags >> 1) & 3;
	if (lace && cursor >= block.end) throw std::runtime_error("truncated Matroska lacing header");
	unsigned count = lace ? read_byte(file, cursor++) + 1 : 1;
	std::vector<uint64_t> sizes(count);
	uint64_t payload_end = block.end;
	if (lace == 1) {
		uint64_t sum = 0;
		for (unsigned i = 0; i + 1 < count; ++i) {
			uint64_t value = 0; uint8_t byte;
			do {
				if (cursor >= payload_end) throw std::runtime_error("truncated Xiph lacing");
				byte = read_byte(file, cursor++); value += byte;
			} while (byte == 255);
			sizes[i] = value; sum += value;
		}
		if (cursor > payload_end || sum > payload_end - cursor) throw std::runtime_error("invalid Xiph lacing");
		sizes.back() = payload_end - cursor - sum;
	} else if (lace == 2) {
		if ((payload_end - cursor) % count) throw std::runtime_error("invalid fixed lacing");
		std::fill(sizes.begin(), sizes.end(), (payload_end - cursor) / count);
	} else if (lace == 3) {
		auto [first, used] = raw_vint(file, cursor, payload_end); cursor += used; sizes[0] = first;
		uint64_t sum = first;
		for (unsigned i = 1; i + 1 < count; ++i) {
			auto [encoded, bytes] = raw_vint(file, cursor, payload_end); cursor += bytes;
			int64_t bias = (int64_t{1} << (7 * bytes - 1)) - 1;
			int64_t next = static_cast<int64_t>(sizes[i - 1]) + static_cast<int64_t>(encoded) - bias;
			if (next < 0) throw std::runtime_error("invalid EBML lacing");
			sizes[i] = static_cast<uint64_t>(next); sum += sizes[i];
		}
		if (cursor > payload_end || sum > payload_end - cursor) throw std::runtime_error("invalid EBML lacing");
		sizes.back() = payload_end - cursor - sum;
	} else sizes[0] = payload_end - cursor;

	int64_t ticks = static_cast<int64_t>(cluster_time) + relative;
	uint64_t scale = file.info.TimecodeScale;
	uint64_t frame_duration = duration_known ?
		checked_multiply(duration, scale, "Matroska block duration is out of range") / count :
		file.tracks[track_index].info.DefaultDuration;
	for (unsigned i = 0; i < count; ++i) {
		Frame frame{};
		frame.track = track_index;
		if (ticks < 0 || static_cast<uint64_t>(ticks) > UINT64_MAX / scale) frame.flags |= FRAME_UNKNOWN_START;
		else frame.start = checked_add(checked_multiply(static_cast<uint64_t>(ticks), scale,
			"Matroska timestamp is out of range"), checked_multiply(i, frame_duration,
			"Matroska laced timestamp is out of range"), "Matroska timestamp is out of range");
		if (!duration_known && !frame_duration) frame.flags |= FRAME_UNKNOWN_END;
		else frame.end = checked_add(frame.start, frame_duration, "Matroska timestamp is out of range");
		frame.pos = cursor;
		if (sizes[i] > UINT_MAX) throw std::runtime_error("Matroska frame is too large");
		frame.size = static_cast<unsigned>(sizes[i]);
		if (keyframe || (flags & 0x80)) frame.flags |= FRAME_KF;
		file.frames.push_back(frame);
		cursor = checked_add(cursor, sizes[i], "Matroska frame exceeds its block");
	}
}

void parse_raw_cluster(MatroskaFile &file, ClusterLocation const& cluster) {
	uint64_t cluster_time = 0;
	uint64_t cluster_end = checked_add(cluster.data, cluster.size, "Matroska cluster is out of range");
	for (uint64_t pos = cluster.data; pos < cluster_end;) {
		auto element = raw_element(file, pos, cluster_end);
		if (element.id == 0xE7) cluster_time = raw_uint(file, element);
		else if (element.id == 0xA3) parse_raw_block(file, element, cluster_time, 0, false, false);
		else if (element.id == 0xA0) {
			RawElement block{}; uint64_t duration = 0; bool has_duration = false, keyframe = true;
			for (uint64_t child_pos = element.data; child_pos < element.end;) {
				auto child = raw_element(file, child_pos, element.end);
				if (child.id == 0xA1) block = child;
				else if (child.id == 0x9B) { duration = raw_uint(file, child); has_duration = true; }
				else if (child.id == 0xFB) keyframe = false;
				child_pos = child.end;
			}
			if (block.id) parse_raw_block(file, block, cluster_time, duration, has_duration, keyframe);
		}
		pos = element.end;
	}
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
			if (input->progress && !input->progress(input, current->GetElementPosition(), static_cast<uint64_t>(input_size)))
				throw std::runtime_error("Matroska parsing cancelled");
			if (dynamic_cast<KaxCluster *>(current.get())) {
				f->clusters.push_back({current->GetElementPosition() + current->HeadSize(), current->GetSize()});
				current->SkipData(*f->stream, EBML_CONTEXT(segment));
				if (!next)
					next = f->stream->FindNextElement(EBML_CONTEXT(segment), upper, std::numeric_limits<uint64_t>::max(), true);
				continue;
			}
			bool wanted = dynamic_cast<KaxInfo *>(current.get()) || dynamic_cast<KaxTracks *>(current.get()) ||
						  dynamic_cast<KaxAttachments *>(current.get());
			if (wanted) {
				unsigned cap = input->getcachesize ? input->getcachesize(input) : 16U * 1024 * 1024;
				auto *attachments = dynamic_cast<KaxAttachments *>(current.get());
				if (!attachments && current->GetSize() > cap) {
					current->SkipData(*f->stream, EBML_CONTEXT(segment));
					throw std::runtime_error("Matroska metadata exceeds the configured read limit");
				}
				auto *master = dynamic_cast<EbmlMaster *>(current.get());
				master->Read(*f->stream, EBML_CONTEXT(current.get()), upper, next, true,
					attachments ? SCOPE_PARTIAL_DATA : SCOPE_ALL_DATA);
				if (auto *info = dynamic_cast<KaxInfo *>(master)) {
					f->info.TimecodeScale = uint_value<KaxTimecodeScale>(*info, 1000000);
					if (auto *d = child<KaxDuration>(*info))
						f->info.Duration = static_cast<uint64_t>(static_cast<double>(*d) * f->info.TimecodeScale);
				}
				parse_master(*f, *master);
				f->top.push_back(std::move(current));
			} else
				current->SkipData(*f->stream, EBML_CONTEXT(segment));
			if (!next)
				next = f->stream->FindNextElement(EBML_CONTEXT(segment), upper, std::numeric_limits<uint64_t>::max(),
												  true);
		}
		if (input->progress && !input->progress(input, static_cast<uint64_t>(input_size), static_cast<uint64_t>(input_size)))
			throw std::runtime_error("Matroska parsing cancelled");
		if (!f->info.Duration && std::any_of(f->tracks.begin(), f->tracks.end(), [](auto const& track) {
			return track.info.Type == TT_SUB;
		})) {
			for (auto cluster = f->clusters.rbegin(); cluster != f->clusters.rend() && !f->info.Duration; ++cluster) {
				f->frames.clear();
				parse_raw_cluster(*f, *cluster);
				for (auto const& frame : f->frames)
					if (!(frame.flags & FRAME_UNKNOWN_END)) f->info.Duration = std::max(f->info.Duration, frame.end);
			}
			f->frames.clear();
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
	f->cluster_cursor = 0;
	f->frames.clear();
}
int mkv_ReadFrame(MatroskaFile *f, unsigned mask, unsigned *track, uint64_t *start, uint64_t *end, uint64_t *pos,
				  unsigned *size, unsigned *flags) {
	try {
		if (std::none_of(f->tracks.begin(), f->tracks.end(), [](auto const& item) { return item.info.Type == TT_SUB; })) return -1;
		for (;;) {
			while (f->cursor < f->frames.size()) {
				auto &r = f->frames[f->cursor++];
				if (r.track >= 32 || (((mask | f->mask) >> r.track) & 1)) continue;
				*track = r.track;
				*start = r.start;
				*end = r.end;
				*pos = r.pos;
				*size = r.size;
				*flags = r.flags;
				return 0;
			}
			if (f->cluster_cursor >= f->clusters.size()) return -1;
			auto const& cluster = f->clusters[f->cluster_cursor++];
			if (f->input->progress && !f->input->progress(f->input, cluster.data, static_cast<uint64_t>(f->input->getfilesize(f->input)))) {
				f->error = "Matroska parsing cancelled";
				return -2;
			}
			f->frames.clear();
			f->cursor = 0;
			parse_raw_cluster(*f, cluster);
		}
	} catch (std::exception const& e) {
		f->error = e.what();
		return -2;
	}
}
void mkv_Seek(MatroskaFile *f, uint64_t t, unsigned) {
	try {
		f->frames.clear();
		f->cursor = 0;
		f->cluster_cursor = 0;
		while (f->cluster_cursor < f->clusters.size()) {
			parse_raw_cluster(*f, f->clusters[f->cluster_cursor++]);
			f->cursor = std::lower_bound(f->frames.begin(), f->frames.end(), t,
				[](auto const& frame, uint64_t time) { return frame.start < time; }) - f->frames.begin();
			if (f->cursor < f->frames.size()) return;
			f->frames.clear();
			f->cursor = 0;
		}
	}
	catch (std::exception const& error) {
		f->error = error.what();
		f->frames.clear();
		f->cursor = 0;
		f->cluster_cursor = f->clusters.size();
	}
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
	uint64_t output_limit = UINT64_MAX;
	bool output_limit_exceeded = false;
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

std::vector<unsigned char> inflate_frame(std::vector<unsigned char> const &input, uint64_t output_limit) {
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
		auto produced = sizeof buffer - zstream.avail_out;
		if (produced > output_limit - std::min<uint64_t>(output.size(), output_limit))
			throw std::runtime_error("decompressed Matroska frame exceeds configured limit");
		output.insert(output.end(), buffer, buffer + produced);
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
		if (track.comp_private.size() > stream.output_limit || raw_frame.size() > stream.output_limit - track.comp_private.size())
			throw std::runtime_error("decompressed Matroska frame exceeds configured limit");
		stream.data = track.comp_private;
		stream.data.insert(stream.data.end(), raw_frame.begin(), raw_frame.end());
	} else if (track.info.CompMethod == COMP_ZLIB)
		stream.data = inflate_frame(raw_frame, stream.output_limit);
	else
		throw std::runtime_error("unsupported Matroska compression method");
}
} // namespace

extern "C" {
CompressedStream *cs_Create(MatroskaFile *file, unsigned track, char *message, unsigned message_size) {
	if (!file || track >= file->tracks.size()) {
		if (message && message_size) {
			std::strncpy(message, "invalid track", message_size - 1);
			message[message_size - 1] = 0;
		}
		return nullptr;
	}
	return new CompressedStream{file, track, 0, 0, {}, {}, {}, UINT64_MAX};
}
void cs_Destroy(CompressedStream *c) {
	delete c;
}
void cs_NextFrame(CompressedStream *stream, uint64_t position, unsigned size) {
	stream->frame_position = position;
	stream->frame_size = size;
	stream->data.clear();
	stream->cursor = 0;
	stream->output_limit_exceeded = false;
}
void cs_SetOutputLimit(CompressedStream *stream, uint64_t size) {
	stream->output_limit = size;
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
		stream->output_limit_exceeded = stream->error == "decompressed Matroska frame exceeds configured limit";
		return -1;
	}
}
int cs_OutputLimitExceeded(CompressedStream *stream) {
	return stream->output_limit_exceeded;
}
const char *cs_GetLastError(CompressedStream *c) {
	return c->error.c_str();
}
}
