// Copyright (c) 2026, Aegisub contributors
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

/// @file matroska.cpp
/// @brief Subtitle and attachment demuxer for Matroska files
///
/// The segment's top-level layout, Info and Tracks are read with libebml and
/// libmatroska. Clusters and attachments are walked with a small EBML reader
/// instead so that only the bytes actually needed are read: cluster contents
/// are parsed lazily one cluster at a time, and attachment payloads are only
/// located until something asks for them.

#include "matroska.h"

#include <libaegisub/file_mapping.h>

#include <ebml/EbmlFloat.h>
#include <ebml/EbmlHead.h>
#include <ebml/EbmlStream.h>
#include <ebml/EbmlString.h>
#include <ebml/EbmlUInteger.h>
#include <ebml/EbmlUnicodeString.h>
#include <ebml/IOCallback.h>
#include <matroska/KaxCluster.h>
#include <matroska/KaxSegment.h>
#include <matroska/KaxSemantic.h>
#include <matroska/KaxTracks.h>
#include <zlib.h>

#include <algorithm>
#include <climits>
#include <cstring>
#include <limits>
#include <unordered_map>

namespace agi::matroska {
using namespace libebml;
using namespace libmatroska;

namespace {
// Element IDs handled by the raw EBML reader
constexpr uint64_t id_cluster_timestamp = 0xE7;
constexpr uint64_t id_simple_block = 0xA3;
constexpr uint64_t id_block_group = 0xA0;
constexpr uint64_t id_block = 0xA1;
constexpr uint64_t id_block_duration = 0x9B;
constexpr uint64_t id_attached_file = 0x61A7;
constexpr uint64_t id_file_name = 0x466E;
constexpr uint64_t id_file_description = 0x467E;
constexpr uint64_t id_file_mime_type = 0x4660;
constexpr uint64_t id_file_data = 0x465C;
constexpr uint64_t id_file_uid = 0x46AE;

constexpr uint64_t track_type_subtitle = 0x11;

class FileReader final : public Reader {
	agi::read_file_mapping file;

public:
	explicit FileReader(agi::fs::path const& filename) : file(filename) { }

	uint64_t Size() const override { return file.size(); }

	size_t Read(uint64_t position, void *buffer, size_t size) override {
		if (position >= file.size())
			return 0;
		size = std::min<uint64_t>(size, file.size() - position);
		memcpy(buffer, file.read(position, size), size);
		return size;
	}
};

/// Read from a Reader, reporting any failure as an IoError
size_t read_some(Reader& reader, uint64_t position, void *buffer, size_t size) {
	size_t read;
	try {
		read = reader.Read(position, buffer, size);
	}
	catch (Error const&) {
		throw;
	}
	catch (agi::Exception const& e) {
		throw IoError(e.GetMessage());
	}
	catch (std::exception const& e) {
		throw IoError(e.what());
	}
	catch (...) {
		throw IoError("Matroska input read failed");
	}
	if (read > size)
		throw IoError("Matroska reader returned more bytes than requested");
	return read;
}

/// Read until either the buffer is full or the input ends
size_t read_full(Reader& reader, uint64_t position, void *buffer, size_t size) {
	size_t total = 0;
	while (total < size) {
		size_t read = read_some(reader, position + total, static_cast<char *>(buffer) + total, size - total);
		if (!read) break;
		total += read;
	}
	return total;
}

/// Read-only libebml stream over a Reader. Errors are thrown straight through
/// libebml, which only ever catches exceptions to clean up and rethrow.
class ReaderCallback final : public IOCallback {
	Reader& reader;
	uint64_t position = 0;

public:
	explicit ReaderCallback(Reader& reader) : reader(reader) { }

	uint32 read(void *buffer, size_t size) override {
		size = std::min<size_t>(size, std::numeric_limits<uint32>::max());
		size_t read = read_full(reader, position, buffer, size);
		position += read;
		return static_cast<uint32>(read);
	}

	void setFilePointer(int64 offset, seek_mode mode) override {
		int64_t base = 0;
		if (mode == seek_current)
			base = static_cast<int64_t>(position);
		else if (mode == seek_end)
			base = static_cast<int64_t>(std::min<uint64_t>(reader.Size(), INT64_MAX));
		if ((offset > 0 && base > INT64_MAX - offset) || base + offset < 0)
			throw InvalidDataError("Invalid seek in Matroska input");
		position = static_cast<uint64_t>(base + offset);
	}

	size_t write(void const *, size_t) override {
		throw IoError("Matroska input is read-only");
	}

	uint64 getFilePointer() override { return position; }
	void close() override { }
};

template<class T>
T *child(EbmlMaster& master) {
	return static_cast<T *>(master.FindFirstElt(EBML_INFO(T), false));
}

template<class T>
uint64_t uint_value(EbmlMaster& master, uint64_t fallback = 0) {
	if (auto value = child<T>(master))
		return static_cast<uint64_t>(*value);
	return fallback;
}

template<class T>
std::string string_value(EbmlMaster& master, std::string fallback = "") {
	if (auto value = child<T>(master))
		return value->GetValue();
	return fallback;
}

template<class T>
std::string unicode_value(EbmlMaster& master) {
	if (auto value = child<T>(master))
		return value->GetValue().GetUTF8();
	return "";
}

uint64_t checked_add(uint64_t lhs, uint64_t rhs, char const *message) {
	if (rhs > UINT64_MAX - lhs)
		throw InvalidDataError(message);
	return lhs + rhs;
}

uint64_t checked_multiply(uint64_t lhs, uint64_t rhs, char const *message) {
	if (lhs && rhs > UINT64_MAX / lhs)
		throw InvalidDataError(message);
	return lhs * rhs;
}

SubtitleCodec codec_from_id(std::string const& id) {
	if (id == "S_TEXT/ASS")
		return SubtitleCodec::ass;
	if (id == "S_TEXT/SSA")
		return SubtitleCodec::ssa;
	if (id == "S_TEXT/UTF8")
		return SubtitleCodec::srt;
	return SubtitleCodec::unsupported;
}

enum class Compression { none, zlib, header_strip, unsupported };

/// Get the compression applied to a track's frames. zlib_codec_private is set
/// if the track's CodecPrivate is zlib-compressed.
Compression parse_encodings(KaxContentEncodings& encodings, std::vector<uint8_t>& stripped_header, bool& zlib_codec_private) {
	KaxContentEncoding *encoding = nullptr;
	for (auto element : encodings.GetElementList()) {
		if (auto current = dynamic_cast<KaxContentEncoding *>(element)) {
			// Chained encodings aren't used by anything that writes subtitles
			if (encoding) return Compression::unsupported;
			encoding = current;
		}
	}
	if (!encoding)
		return Compression::none;
	// Type 1 is encryption
	if (uint_value<KaxContentEncodingType>(*encoding) != 0)
		return Compression::unsupported;

	auto compression = child<KaxContentCompression>(*encoding);
	if (!compression)
		return Compression::none;

	// Bit 1 is the frame contents and bit 2 is CodecPrivate
	auto scope = uint_value<KaxContentEncodingScope>(*encoding, 1);
	auto algorithm = uint_value<KaxContentCompAlgo>(*compression);
	if (scope & 2) {
		if (algorithm != 0)
			return Compression::unsupported;
		zlib_codec_private = true;
	}
	if (!(scope & 1))
		return Compression::none;

	switch (algorithm) {
		case 0:
			return Compression::zlib;
		case 3:
			if (auto settings = child<KaxContentCompSettings>(*compression))
				stripped_header.assign(settings->GetBuffer(), settings->GetBuffer() + settings->GetSize());
			return Compression::header_strip;
		default:
			return Compression::unsupported;
	}
}

std::vector<uint8_t> inflate_packet(std::vector<uint8_t> const& input, size_t limit) {
	if (input.size() > std::numeric_limits<uInt>::max())
		throw LimitError("Compressed Matroska subtitle packet is too large");

	z_stream stream{};
	if (inflateInit(&stream) != Z_OK)
		throw InvalidDataError("Failed to initialize zlib");
	struct InflateEnd {
		z_stream *stream;
		~InflateEnd() { inflateEnd(stream); }
	} inflate_end{&stream};

	stream.next_in = const_cast<Bytef *>(input.data());
	stream.avail_in = static_cast<uInt>(input.size());

	std::vector<uint8_t> output;
	uint8_t buffer[4096];
	int result;
	do {
		stream.next_out = buffer;
		stream.avail_out = sizeof buffer;
		result = inflate(&stream, Z_NO_FLUSH);
		if (result != Z_OK && result != Z_STREAM_END)
			throw InvalidDataError("Invalid zlib-compressed Matroska subtitle packet");
		size_t produced = sizeof buffer - stream.avail_out;
		if (produced > limit - output.size())
			throw LimitError("Decompressed Matroska subtitle packet exceeds configured limit");
		output.insert(output.end(), buffer, buffer + produced);
	} while (result != Z_STREAM_END);
	return output;
}

/// Demuxing state for every track in the file, not just subtitle tracks
struct TrackState {
	uint64_t uid = 0;
	uint64_t default_duration = 0;
	Compression compression = Compression::none;
	std::vector<uint8_t> stripped_header;
};

struct Frame {
	size_t track = 0;
	std::optional<Timestamp> start;
	std::optional<Timestamp> end;
	uint64_t position = 0;
	uint64_t size = 0;
};

/// The data portion of an element in the input
struct Location {
	uint64_t position = 0;
	uint64_t size = 0;
};

struct Element {
	uint64_t id = 0;
	uint64_t data = 0;
	uint64_t size = 0;
	uint64_t end = 0;
};
} // namespace

std::unique_ptr<Reader> OpenFile(agi::fs::path const& filename) {
	try {
		return std::make_unique<FileReader>(filename);
	}
	catch (agi::Exception const& e) {
		throw IoError(e.GetMessage());
	}
}

class Demuxer::Impl {
	std::unique_ptr<Reader> reader;
	CancelCheck cancelled;
	Limits limits;

	uint64_t timestamp_scale = 1000000;
	std::optional<Timestamp> duration;
	std::vector<TrackState> all_tracks;
	std::unordered_map<uint64_t, size_t> track_by_number;
	std::vector<SubtitleTrack> tracks;
	std::vector<Attachment> attachments;
	/// Location of each attachment's data, parallel to attachments
	std::vector<Location> attachment_data;
	std::vector<Location> clusters;

	/// Index into all_tracks of the selected track
	std::optional<size_t> selected;
	size_t next_cluster = 0;
	/// Frames of the selected track in the most recently parsed cluster
	std::vector<Frame> frames;
	size_t next_frame = 0;

	void CheckCancelled() const {
		bool cancel;
		try {
			cancel = cancelled && cancelled();
		}
		catch (...) {
			cancel = true;
		}
		if (cancel)
			throw agi::UserCancelException("Matroska read cancelled");
	}

	void ReadExact(uint64_t position, void *buffer, size_t size) {
		if (read_full(*reader, position, buffer, size) != size)
			throw TruncatedError("Unexpected end of Matroska input");
	}

	uint8_t ReadByte(uint64_t position) {
		uint8_t byte;
		ReadExact(position, &byte, 1);
		return byte;
	}

	std::vector<uint8_t> ReadBytes(Location const& location) {
		if (location.size > std::numeric_limits<size_t>::max())
			throw LimitError("Matroska element exceeds addressable memory");
		std::vector<uint8_t> result(static_cast<size_t>(location.size));
		ReadExact(location.position, result.data(), result.size());
		return result;
	}

	// Raw EBML reading

	/// Read a variable-length integer, returning its value and encoded length
	std::pair<uint64_t, unsigned> ReadVint(uint64_t position, uint64_t end) {
		if (position >= end)
			throw InvalidDataError("Truncated Matroska variable-length integer");
		uint8_t first = ReadByte(position);
		unsigned length = 1;
		uint8_t marker = 0x80;
		while (length <= 8 && !(first & marker)) {
			marker >>= 1;
			++length;
		}
		if (length > 8)
			throw InvalidDataError("Invalid Matroska variable-length integer");
		if (length > end - position)
			throw InvalidDataError("Truncated Matroska variable-length integer");
		uint64_t value = first & (marker - 1);
		for (unsigned i = 1; i < length; ++i)
			value = (value << 8) | ReadByte(position + i);
		return {value, length};
	}

	Element ReadElement(uint64_t position, uint64_t parent_end) {
		uint8_t first = ReadByte(position);
		unsigned id_length = 1;
		for (uint8_t mask = 0x80; id_length <= 4 && !(first & mask); mask >>= 1)
			++id_length;
		if (id_length > 4)
			throw InvalidDataError("Invalid Matroska element ID");
		uint64_t id = first;
		for (unsigned i = 1; i < id_length; ++i)
			id = (id << 8) | ReadByte(position + i);

		auto [size, size_length] = ReadVint(position + id_length, parent_end);
		uint64_t data = position + id_length + size_length;
		// All ones is an unknown size, which extends to the end of the parent
		if (size == (uint64_t{1} << (7 * size_length)) - 1)
			size = parent_end - data;
		if (data > parent_end || size > parent_end - data)
			throw InvalidDataError("Matroska element exceeds its parent");
		return {id, data, size, data + size};
	}

	uint64_t ReadUInt(Element const& element) {
		if (element.size > 8)
			throw InvalidDataError("Invalid Matroska integer");
		uint64_t value = 0;
		for (uint64_t i = 0; i < element.size; ++i)
			value = (value << 8) | ReadByte(element.data + i);
		return value;
	}

	std::string ReadString(Element const& element) {
		if (element.size > limits.metadata_size)
			throw LimitError("Matroska string exceeds configured limit");
		std::string value(static_cast<size_t>(element.size), '\0');
		ReadExact(element.data, value.data(), value.size());
		// Strings may be padded with trailing nulls
		value.resize(strnlen(value.data(), value.size()));
		return value;
	}

	// Metadata

	void ParseInfo(KaxInfo& info) {
		timestamp_scale = uint_value<KaxTimecodeScale>(info, 1000000);
		if (!timestamp_scale)
			throw InvalidDataError("Invalid Matroska timestamp scale");
		if (auto value = child<KaxDuration>(info)) {
			double nanoseconds = static_cast<double>(*value) * timestamp_scale;
			if (nanoseconds > 0 && nanoseconds < static_cast<double>(INT64_MAX))
				duration = Timestamp{static_cast<int64_t>(nanoseconds)};
		}
	}

	void ParseTrack(KaxTrackEntry& entry) {
		TrackState state;
		state.uid = uint_value<KaxTrackUID>(entry);
		// Tracks may be repeated so that a reader joining a live stream can
		// see them, so skip entries which were already seen
		if (state.uid && std::any_of(all_tracks.begin(), all_tracks.end(), [&](TrackState const& track) {
			return track.uid == state.uid;
		}))
			return;

		state.default_duration = uint_value<KaxTrackDefaultDuration>(entry);
		bool zlib_codec_private = false;
		if (auto encodings = child<KaxContentEncodings>(entry))
			state.compression = parse_encodings(*encodings, state.stripped_header, zlib_codec_private);

		if (!track_by_number.emplace(uint_value<KaxTrackNumber>(entry), all_tracks.size()).second)
			throw InvalidDataError("Duplicate Matroska track number");

		if (uint_value<KaxTrackType>(entry) == track_type_subtitle) {
			SubtitleTrack track;
			track.id.value = static_cast<uint32_t>(all_tracks.size());
			track.uid = state.uid;
			track.codec_id = string_value<KaxCodecID>(entry);
			track.codec = state.compression == Compression::unsupported ? SubtitleCodec::unsupported : codec_from_id(track.codec_id);
			track.name = unicode_value<KaxTrackName>(entry);
			track.language = string_value<KaxTrackLanguage>(entry, "eng");
			track.enabled = uint_value<KaxTrackFlagEnabled>(entry, 1);
			track.is_default = uint_value<KaxTrackFlagDefault>(entry, 1);
			if (auto codec_private = child<KaxCodecPrivate>(entry))
				track.codec_private.assign(codec_private->GetBuffer(), codec_private->GetBuffer() + codec_private->GetSize());
			if (zlib_codec_private && !track.codec_private.empty()) {
				try {
					track.codec_private = inflate_packet(track.codec_private, limits.metadata_size);
				}
				catch (Error const&) {
					track.codec = SubtitleCodec::unsupported;
				}
			}
			tracks.push_back(std::move(track));
		}
		all_tracks.push_back(std::move(state));
	}

	void ParseAttachments(Location const& list) {
		uint64_t end = checked_add(list.position, list.size, "Matroska attachments are out of range");
		for (uint64_t position = list.position; position < end;) {
			auto file = ReadElement(position, end);
			position = file.end;
			if (file.id != id_attached_file)
				continue;

			Attachment attachment;
			std::optional<Location> data;
			for (uint64_t child_position = file.data; child_position < file.end;) {
				auto element = ReadElement(child_position, file.end);
				child_position = element.end;
				switch (element.id) {
					case id_file_name:        attachment.name = ReadString(element); break;
					case id_file_description: attachment.description = ReadString(element); break;
					case id_file_mime_type:   attachment.mime_type = ReadString(element); break;
					case id_file_uid:         attachment.uid = ReadUInt(element); break;
					case id_file_data:        data = Location{element.data, element.size}; break;
				}
			}
			if (!data)
				continue;

			attachment.id.value = attachments.size();
			attachment.size = data->size;
			attachments.push_back(std::move(attachment));
			attachment_data.push_back(*data);
		}
	}

	void ParseSegment() {
		ReaderCallback io(*reader);
		EbmlStream stream(io);

		std::unique_ptr<EbmlElement> head(stream.FindNextID(EBML_INFO(EbmlHead), UINT64_MAX));
		if (!head)
			throw InvalidDataError("EBML header not found");
		head->SkipData(stream, EBML_CONTEXT(head.get()));

		std::unique_ptr<EbmlElement> segment(stream.FindNextID(EBML_INFO(KaxSegment), UINT64_MAX));
		if (!segment)
			throw InvalidDataError("Matroska segment not found");
		auto const& context = EBML_CONTEXT(segment.get());

		int upper = 0;
		std::unique_ptr<EbmlElement> current(stream.FindNextElement(context, upper, UINT64_MAX, true));
		while (current && upper <= 0) {
			CheckCancelled();
			std::unique_ptr<EbmlElement> next;
			Location location{current->GetElementPosition() + current->HeadSize(), current->GetSize()};

			auto info = dynamic_cast<KaxInfo *>(current.get());
			auto track_list = dynamic_cast<KaxTracks *>(current.get());
			if (info || track_list) {
				if (current->GetSize() > limits.metadata_size)
					throw LimitError("Matroska metadata exceeds configured limit");
				EbmlElement *found = nullptr;
				static_cast<EbmlMaster&>(*current).Read(stream, EBML_CONTEXT(current.get()), upper, found, true);
				next.reset(found);
				// A positive level means the element found belongs to an ancestor
				if (upper > 0)
					--upper;

				if (info)
					ParseInfo(*info);
				else {
					for (auto element : track_list->GetElementList()) {
						if (auto entry = dynamic_cast<KaxTrackEntry *>(element))
							ParseTrack(*entry);
					}
				}
			}
			else {
				if (dynamic_cast<KaxCluster *>(current.get()))
					clusters.push_back(location);
				else if (dynamic_cast<KaxAttachments *>(current.get()))
					ParseAttachments(location);
				current->SkipData(stream, context);
			}

			if (!next)
				next.reset(stream.FindNextElement(context, upper, UINT64_MAX, true));
			current = std::move(next);
		}
	}

	/// Use the end of the last frame as the duration when the header lacks one
	void ComputeDuration() {
		if (duration || tracks.empty())
			return;
		for (auto cluster = clusters.rbegin(); cluster != clusters.rend(); ++cluster) {
			CheckCancelled();
			std::vector<Frame> cluster_frames;
			try {
				cluster_frames = ParseCluster(*cluster, std::nullopt);
			}
			catch (InvalidDataError const&) {
				continue;
			}
			catch (TruncatedError const&) {
				continue;
			}

			int64_t last_end = 0;
			for (auto const& frame : cluster_frames) {
				if (frame.end)
					last_end = std::max(last_end, frame.end->nanoseconds);
			}
			if (last_end > 0) {
				duration = Timestamp{last_end};
				return;
			}
		}
	}

	// Clusters

	std::optional<Timestamp> MakeTimestamp(uint64_t nanoseconds) {
		if (nanoseconds > static_cast<uint64_t>(INT64_MAX))
			throw InvalidDataError("Matroska timestamp is out of range");
		return Timestamp{static_cast<int64_t>(nanoseconds)};
	}

	/// Split a block into its frames, appending those for only_track (or all tracks) to frames
	void ParseBlock(Element const& block, uint64_t cluster_time, std::optional<uint64_t> block_duration,
	                std::optional<size_t> only_track, std::vector<Frame>& out) {
		auto [track_number, track_bytes] = ReadVint(block.data, block.end);
		auto found = track_by_number.find(track_number);
		if (found == track_by_number.end())
			return;
		size_t track = found->second;
		if (only_track && *only_track != track)
			return;

		if (block.size < track_bytes + 3)
			throw InvalidDataError("Truncated Matroska block header");
		uint64_t cursor = block.data + track_bytes;
		auto relative = static_cast<int16_t>((ReadByte(cursor) << 8) | ReadByte(cursor + 1));
		uint8_t flags = ReadByte(cursor + 2);
		cursor += 3;

		unsigned lacing = (flags >> 1) & 3;
		if (lacing && cursor >= block.end)
			throw InvalidDataError("Truncated Matroska lacing header");
		unsigned count = lacing ? ReadByte(cursor++) + 1 : 1;
		std::vector<uint64_t> sizes(count);
		uint64_t end = block.end;
		if (lacing == 1) { // Xiph
			uint64_t sum = 0;
			for (unsigned i = 0; i + 1 < count; ++i) {
				uint64_t value = 0;
				uint8_t byte;
				do {
					if (cursor >= end)
						throw InvalidDataError("Truncated Matroska Xiph lacing");
					byte = ReadByte(cursor++);
					value += byte;
				} while (byte == 255);
				sizes[i] = value;
				sum += value;
			}
			if (cursor > end || sum > end - cursor)
				throw InvalidDataError("Invalid Matroska Xiph lacing");
			sizes.back() = end - cursor - sum;
		}
		else if (lacing == 2) { // Fixed-size
			if ((end - cursor) % count)
				throw InvalidDataError("Invalid Matroska fixed-size lacing");
			std::fill(sizes.begin(), sizes.end(), (end - cursor) / count);
		}
		else if (lacing == 3) { // EBML
			auto [first, first_length] = ReadVint(cursor, end);
			cursor += first_length;
			sizes[0] = first;
			uint64_t sum = first;
			for (unsigned i = 1; i + 1 < count; ++i) {
				auto [encoded, length] = ReadVint(cursor, end);
				cursor += length;
				int64_t bias = (int64_t{1} << (7 * length - 1)) - 1;
				int64_t value = static_cast<int64_t>(sizes[i - 1]) + static_cast<int64_t>(encoded) - bias;
				if (value < 0 || static_cast<uint64_t>(value) > end - cursor)
					throw InvalidDataError("Invalid Matroska EBML lacing");
				sizes[i] = static_cast<uint64_t>(value);
				sum += sizes[i];
			}
			if (cursor > end || sum > end - cursor)
				throw InvalidDataError("Invalid Matroska EBML lacing");
			sizes.back() = end - cursor - sum;
		}
		else
			sizes[0] = end - cursor;

		if (cluster_time > static_cast<uint64_t>(INT64_MAX) || (relative > 0 && cluster_time > static_cast<uint64_t>(INT64_MAX - relative)))
			throw InvalidDataError("Matroska timestamp is out of range");
		int64_t ticks = static_cast<int64_t>(cluster_time) + relative;
		uint64_t frame_duration = block_duration
			? checked_multiply(*block_duration, timestamp_scale, "Matroska block duration is out of range") / count
			: all_tracks[track].default_duration;

		for (unsigned i = 0; i < count; ++i) {
			Frame frame;
			frame.track = track;
			frame.position = cursor;
			frame.size = sizes[i];
			if (ticks >= 0) {
				uint64_t start = checked_add(
					checked_multiply(static_cast<uint64_t>(ticks), timestamp_scale, "Matroska timestamp is out of range"),
					checked_multiply(i, frame_duration, "Matroska timestamp is out of range"),
					"Matroska timestamp is out of range");
				frame.start = MakeTimestamp(start);
				if (block_duration || frame_duration)
					frame.end = MakeTimestamp(checked_add(start, frame_duration, "Matroska timestamp is out of range"));
			}
			out.push_back(frame);
			cursor = checked_add(cursor, sizes[i], "Matroska frame exceeds its block");
		}
	}

	std::vector<Frame> ParseCluster(Location const& cluster, std::optional<size_t> only_track) {
		std::vector<Frame> out;
		uint64_t cluster_time = 0;
		uint64_t end = checked_add(cluster.position, cluster.size, "Matroska cluster is out of range");
		for (uint64_t position = cluster.position; position < end;) {
			// Clusters have no size limit, so check per element rather than per cluster
			CheckCancelled();
			auto element = ReadElement(position, end);
			position = element.end;
			if (element.id == id_cluster_timestamp)
				cluster_time = ReadUInt(element);
			else if (element.id == id_simple_block)
				ParseBlock(element, cluster_time, std::nullopt, only_track, out);
			else if (element.id == id_block_group) {
				std::optional<Element> block;
				std::optional<uint64_t> block_duration;
				for (uint64_t child_position = element.data; child_position < element.end;) {
					auto child = ReadElement(child_position, element.end);
					child_position = child.end;
					if (child.id == id_block)
						block = child;
					else if (child.id == id_block_duration)
						block_duration = ReadUInt(child);
				}
				if (block)
					ParseBlock(*block, cluster_time, block_duration, only_track, out);
			}
		}
		return out;
	}

	std::vector<uint8_t> Decode(TrackState const& track, std::vector<uint8_t> data) const {
		switch (track.compression) {
			case Compression::none:
				return data;
			case Compression::header_strip:
				if (track.stripped_header.size() > limits.decompressed_size || data.size() > limits.decompressed_size - track.stripped_header.size())
					throw LimitError("Decompressed Matroska subtitle packet exceeds configured limit");
				data.insert(data.begin(), track.stripped_header.begin(), track.stripped_header.end());
				return data;
			case Compression::zlib:
				return inflate_packet(data, limits.decompressed_size);
			case Compression::unsupported:
				break;
		}
		throw UnsupportedError("Unsupported Matroska content encoding");
	}

public:
	Impl(std::unique_ptr<Reader> input, CancelCheck cancel, Limits configured_limits)
	: reader(std::move(input))
	, cancelled(std::move(cancel))
	, limits(configured_limits)
	{
		if (!reader)
			throw InvalidDataError("Cannot open Matroska from a null reader");
		CheckCancelled();
		try {
			ParseSegment();
		}
		catch (std::exception const& e) {
			// Our own errors are agi::Exceptions, so this is only libebml rejecting the file
			throw InvalidDataError(e.what());
		}
		ComputeDuration();
	}

	std::vector<SubtitleTrack> const& Tracks() const { return tracks; }
	std::vector<Attachment> const& AttachmentList() const { return attachments; }
	std::optional<Timestamp> Duration() const { return duration; }

	void Select(TrackId id) {
		auto track = std::find_if(tracks.begin(), tracks.end(), [&](SubtitleTrack const& track) {
			return track.id == id;
		});
		if (track == tracks.end()) {
			if (id.value >= all_tracks.size())
				throw InvalidDataError("Invalid Matroska subtitle track");
			throw InvalidDataError("Selected Matroska track is not a subtitle track");
		}
		if (track->codec == SubtitleCodec::unsupported)
			throw UnsupportedError("Selected Matroska subtitle codec is unsupported");

		CheckCancelled();
		selected = id.value;
		next_cluster = 0;
		frames.clear();
		next_frame = 0;
	}

	std::optional<SubtitlePacket> NextPacket() {
		if (!selected)
			throw InvalidDataError("No Matroska subtitle track has been selected");
		CheckCancelled();

		while (next_frame == frames.size()) {
			if (next_cluster == clusters.size())
				return std::nullopt;
			// Clear first so that a cluster which fails to parse is skipped on retry
			frames.clear();
			next_frame = 0;
			frames = ParseCluster(clusters[next_cluster++], selected);
			CheckCancelled();
		}

		auto const& frame = frames[next_frame++];
		if (frame.size > limits.packet_size)
			throw LimitError("Matroska subtitle packet exceeds configured limit");
		auto data = Decode(all_tracks[*selected], ReadBytes({frame.position, frame.size}));
		return SubtitlePacket{{static_cast<uint32_t>(*selected)}, frame.start, frame.end, std::move(data)};
	}

	std::vector<uint8_t> AttachmentBytes(AttachmentId id) {
		if (id.value >= attachments.size())
			throw InvalidDataError("Unknown Matroska attachment");
		auto const& data = attachment_data[id.value];
		if (data.size > limits.attachment_size)
			throw LimitError("Matroska attachment exceeds configured limit");
		CheckCancelled();
		return ReadBytes(data);
	}
};

Demuxer::Demuxer(std::unique_ptr<Reader> reader, CancelCheck cancelled, Limits limits)
: impl(std::make_unique<Impl>(std::move(reader), std::move(cancelled), limits))
{
}

Demuxer::~Demuxer() = default;
Demuxer::Demuxer(Demuxer&&) noexcept = default;
Demuxer& Demuxer::operator=(Demuxer&&) noexcept = default;

std::vector<SubtitleTrack> const& Demuxer::SubtitleTracks() const {
	return impl->Tracks();
}

std::vector<Attachment> const& Demuxer::Attachments() const {
	return impl->AttachmentList();
}

std::optional<Timestamp> Demuxer::Duration() const {
	return impl->Duration();
}

void Demuxer::SelectTrack(TrackId track) {
	impl->Select(track);
}

std::optional<SubtitlePacket> Demuxer::ReadPacket() {
	return impl->NextPacket();
}

std::vector<uint8_t> Demuxer::ReadAttachment(AttachmentId id) {
	return impl->AttachmentBytes(id);
}

} // namespace agi::matroska
