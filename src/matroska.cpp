// Copyright (c) 2026 Aegisub contributors
//
#include "matroska.h"

#include "matroska_demux.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <libaegisub/file_mapping.h>
#include <limits>
#include <utility>

namespace agi::matroska {
namespace {
class FileReader final : public Reader {
  agi::read_file_mapping file;

public:
  explicit FileReader(agi::fs::path const &filename)
      : file(filename) {}
  uint64_t Size() const override { return file.size(); }
  size_t Read(uint64_t position, void *buffer, size_t size) override {
    if (position >= file.size())
      return 0;
    size = std::min<uint64_t>(size, file.size() - position);
    memcpy(buffer, file.read(position, size), size);
    return size;
  }
};

std::string copy_string(char const *value) { return value ? value : ""; }

SubtitleCodec codec_from_id(std::string const &id) {
  if (id == "S_TEXT/ASS")
    return SubtitleCodec::ass;
  if (id == "S_TEXT/SSA")
    return SubtitleCodec::ssa;
  if (id == "S_TEXT/UTF8")
    return SubtitleCodec::srt;
  return SubtitleCodec::unsupported;
}
} // namespace

std::unique_ptr<Reader> OpenFile(agi::fs::path const &filename) {
  try {
    return std::make_unique<FileReader>(filename);
  }
  catch (agi::Exception const& error) {
    throw IoError(error.GetMessage());
  }
}

class Demuxer::Impl final : public InputStream {
  std::unique_ptr<Reader> reader;
  CancelCheck cancelled;
  Limits limits;
  std::string input_error;
  std::unique_ptr<MatroskaFile, decltype(&mkv_Close)> file{nullptr, mkv_Close};
  std::unique_ptr<CompressedStream, decltype(&cs_Destroy)> compressed{
      nullptr, cs_Destroy};
  std::vector<SubtitleTrack> tracks;
  std::vector<Attachment> attachments;
  std::vector<::Attachment> attachment_locations;
  std::optional<Timestamp> duration;
  std::optional<TrackId> selected;

  static Impl *Self(InputStream *input) { return static_cast<Impl *>(input); }

  static int Read(InputStream *input, uint64_t position, void *buffer,
                  int count) noexcept {
    if (count <= 0)
      return 0;
    try {
      size_t total = 0;
      while (total < static_cast<size_t>(count)) {
        auto read = Self(input)->reader->Read(position + total,
          static_cast<unsigned char *>(buffer) + total, count - total);
        if (!read) break;
        if (read > static_cast<size_t>(count) - total) return -1;
        total += read;
      }
      return static_cast<int>(total);
    } catch (...) {
      try { Self(input)->input_error = "Matroska input read failed"; }
      catch (...) {}
      return -1;
    }
  }

  static int64_t Scan(InputStream *input, uint64_t start, unsigned signature) noexcept {
    try {
      unsigned window = 0;
      for (uint64_t position = start; position < Self(input)->reader->Size(); ++position) {
        unsigned char byte;
        if (Read(input, position, &byte, 1) != 1) return -1;
        window = (window << 8 | byte) & 0xffffffffU;
        if (window == signature) return position - 3;
      }
    } catch (...) {}
    return -1;
  }

  static int64_t Size(InputStream *input) noexcept {
    try {
      auto size = Self(input)->reader->Size();
      return size > static_cast<uint64_t>(INT64_MAX) ? INT64_MAX : size;
    } catch (...) { return 0; }
  }

  bool IsCancelled() const noexcept {
    if (!cancelled) return false;
    try { return cancelled(); }
    catch (...) { return true; }
  }

  void CheckCancelled() const {
    if (IsCancelled())
      throw agi::UserCancelException("Matroska read cancelled");
  }

  void ReadMetadata() {
    for (unsigned index = 0; index < mkv_GetNumTracks(file.get()); ++index) {
      auto const *info = mkv_GetTrackInfo(file.get(), index);
      if (info->Type != TT_SUB)
        continue;

      SubtitleTrack track;
      track.id.value = index;
      track.uid = info->UID;
      track.codec_id = copy_string(info->CodecID);
      track.codec = codec_from_id(track.codec_id);
      track.name = copy_string(info->Name);
      track.language = copy_string(info->Language);
      track.enabled = info->Enabled;
      track.is_default = info->Default;
      auto private_data = static_cast<uint8_t const *>(info->CodecPrivate);
      if (private_data)
        track.codec_private.assign(private_data,
                                   private_data + info->CodecPrivateSize);
      tracks.push_back(std::move(track));
    }

    ::Attachment *legacy_attachments = nullptr;
    unsigned count = 0;
    mkv_GetAttachments(file.get(), &legacy_attachments, &count);
    if (count)
      attachment_locations.assign(legacy_attachments,
                                  legacy_attachments + count);
    for (auto const &legacy : attachment_locations) {
      if (std::find_if(attachments.begin(), attachments.end(), [&](auto const &item) {
            return item.id.value == legacy.UID;
          }) != attachments.end())
        continue;
      attachments.push_back({{legacy.UID}, copy_string(legacy.Name),
                             copy_string(legacy.Description),
                             copy_string(legacy.MimeType), legacy.Length});
    }

    auto const *segment = mkv_GetFileInfo(file.get());
    if (segment->Duration <= static_cast<uint64_t>(INT64_MAX))
      duration = Timestamp{static_cast<int64_t>(segment->Duration)};
  }

  std::vector<uint8_t> ReadBytes(uint64_t position, uint64_t size) {
    if (size > std::numeric_limits<size_t>::max())
      throw LimitError("Matroska item exceeds addressable memory");
    std::vector<uint8_t> result(static_cast<size_t>(size));
    size_t offset = 0;
    while (offset < result.size()) {
      CheckCancelled();
      auto read = reader->Read(position + offset, result.data() + offset,
                               result.size() - offset);
      if (!read)
        throw TruncatedError("Unexpected end of Matroska input");
      if (read > result.size() - offset)
        throw IoError("Matroska reader returned more bytes than requested");
      offset += read;
    }
    return result;
  }

public:
  Impl(std::unique_ptr<Reader> input, CancelCheck cancel, Limits configured_limits)
      : reader(std::move(input)), cancelled(std::move(cancel)), limits(configured_limits) {
    if (!reader)
      throw InvalidDataError("Cannot open Matroska from a null reader");
    read = &Read;
    scan = &Scan;
    getfilesize = &Size;
    getcachesize = [](InputStream *) { return 16U * 1024 * 1024; };
    geterror = [](InputStream *input) {
      return Self(input)->input_error.c_str();
    };
    memalloc = [](InputStream *, size_t size) { return std::malloc(size); };
    memrealloc = [](InputStream *, void *memory, size_t size) {
      return std::realloc(memory, size);
    };
    memfree = [](InputStream *, void *memory) { std::free(memory); };
    progress = [](InputStream *input, uint64_t, uint64_t) noexcept {
      return Self(input)->IsCancelled() ? 0 : 1;
    };

    CheckCancelled();
    char error[2048]{};
    file.reset(mkv_Open(this, error, sizeof error));
    if (!file) {
      CheckCancelled();
      throw InvalidDataError(error[0] ? error : "Failed to parse Matroska input");
    }
    ReadMetadata();
  }

  std::vector<SubtitleTrack> const &Tracks() const { return tracks; }
  std::vector<Attachment> const &AttachmentList() const { return attachments; }
  std::optional<Timestamp> Duration() const { return duration; }

  void Select(TrackId track) {
    if (track.value >= mkv_GetNumTracks(file.get()) ||
        track.value >= sizeof(unsigned) * CHAR_BIT)
      throw InvalidDataError("Invalid Matroska subtitle track");
    auto found =
        std::find_if(tracks.begin(), tracks.end(), [&](auto const &item) {
          return item.id.value == track.value;
        });
    if (found == tracks.end())
      throw InvalidDataError("Selected Matroska track is not a subtitle track");
    if (found->codec == SubtitleCodec::unsupported)
      throw UnsupportedError("Selected Matroska subtitle codec is unsupported");

    CheckCancelled();
    mkv_SetTrackMask(file.get(), ~(1U << track.value));
    selected = track;
    compressed.reset();
    if (mkv_GetTrackInfo(file.get(), track.value)->CompEnabled) {
      char error[2048]{};
      compressed.reset(cs_Create(file.get(), track.value, error, sizeof error));
      if (!compressed)
        throw UnsupportedError(error[0] ? error : "Unsupported subtitle compression");
      cs_SetOutputLimit(compressed.get(), limits.decompressed_size);
    }
  }

  std::optional<SubtitlePacket> NextPacket() {
    if (!selected)
      throw InvalidDataError("No Matroska subtitle track has been selected");
    CheckCancelled();
    unsigned track, frame_size, flags;
    uint64_t start, end, position;
    int read_result = mkv_ReadFrame(file.get(), ~(1U << selected->value), &track, &start,
                                   &end, &position, &frame_size, &flags);
    CheckCancelled();
    if (read_result < -1 && !input_error.empty())
      throw IoError(input_error);
    if (read_result < -1)
      throw InvalidDataError(copy_string(mkv_GetLastError(file.get())));
    if (read_result < 0)
      return std::nullopt;

    std::vector<uint8_t> data;
    if (frame_size > limits.packet_size)
      throw LimitError("Matroska subtitle packet exceeds configured limit");
    if (!compressed)
      data = ReadBytes(position, frame_size);
    else {
      cs_NextFrame(compressed.get(), position, frame_size);
      unsigned char buffer[4096];
      for (;;) {
        CheckCancelled();
        int count = cs_ReadData(
            compressed.get(), reinterpret_cast<char *>(buffer), sizeof buffer);
        if (count < 0 && cs_OutputLimitExceeded(compressed.get()))
          throw LimitError("Decompressed Matroska subtitle packet exceeds configured limit");
        if (count < 0 && !input_error.empty())
          throw IoError(input_error);
        if (count < 0)
          throw InvalidDataError(copy_string(cs_GetLastError(compressed.get())));
        if (!count)
          break;
        if (data.size() + static_cast<size_t>(count) > limits.decompressed_size)
          throw LimitError("Decompressed Matroska subtitle packet exceeds configured limit");
        data.insert(data.end(), buffer, buffer + count);
      }
    }
    auto checked_time = [](uint64_t value, unsigned unknown_flag, unsigned flags) -> std::optional<Timestamp> {
      if (flags & unknown_flag) return std::nullopt;
      if (value > static_cast<uint64_t>(INT64_MAX))
        throw InvalidDataError("Matroska timestamp is out of range");
      return Timestamp{static_cast<int64_t>(value)};
    };
    return SubtitlePacket{{track},
                          checked_time(start, FRAME_UNKNOWN_START, flags),
                          checked_time(end, FRAME_UNKNOWN_END, flags),
                          std::move(data)};
  }

  std::vector<uint8_t> AttachmentBytes(AttachmentId id) {
    auto found =
        std::find_if(attachment_locations.begin(), attachment_locations.end(),
                         [&](auto const &item) { return item.UID == id.value; });
    if (found == attachment_locations.end())
      throw InvalidDataError("Unknown Matroska attachment");
    if (found->Length > limits.attachment_size)
      throw LimitError("Matroska attachment exceeds configured limit");
    return ReadBytes(found->Position, found->Length);
  }
};

Demuxer::Demuxer(std::unique_ptr<Reader> reader, CancelCheck cancelled, Limits limits)
    : impl(std::make_unique<Impl>(std::move(reader), std::move(cancelled), limits)) {}
Demuxer::~Demuxer() = default;
Demuxer::Demuxer(Demuxer &&) noexcept = default;
Demuxer &Demuxer::operator=(Demuxer &&) noexcept = default;
std::vector<SubtitleTrack> const &Demuxer::SubtitleTracks() const {
  return impl->Tracks();
}
std::vector<Attachment> const &Demuxer::Attachments() const {
  return impl->AttachmentList();
}
std::optional<Timestamp> Demuxer::Duration() const { return impl->Duration(); }
void Demuxer::SelectTrack(TrackId track) { impl->Select(track); }
std::optional<SubtitlePacket> Demuxer::ReadPacket() {
  return impl->NextPacket();
}
std::vector<uint8_t> Demuxer::ReadAttachment(AttachmentId id) {
  return impl->AttachmentBytes(id);
}

} // namespace agi::matroska
