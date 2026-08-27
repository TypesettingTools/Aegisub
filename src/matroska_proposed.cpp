// Copyright (c) 2026 Aegisub contributors
//
// This is an unused comparison implementation. It deliberately delegates EBML
// parsing to the current adapter while presenting an Aegisub-native C++
// surface. It is not part of the production build and has no call sites.

#include "matroska_proposed.h"

#include "MatroskaParser.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <utility>

namespace agi::matroska::proposed {
namespace {
class FileReader final : public Reader {
  std::ifstream file;
  uint64_t size = 0;

public:
  explicit FileReader(agi::fs::path const &filename)
      : file(filename, std::ios::binary) {
    if (!file)
      throw ReadError("Failed to open Matroska input");
    file.seekg(0, std::ios::end);
    auto end = file.tellg();
    if (end < 0)
      throw ReadError("Failed to determine Matroska input size");
    size = static_cast<uint64_t>(end);
  }
  uint64_t Size() const override { return size; }
  size_t Read(uint64_t position, void *buffer, size_t size) override {
    if (position >= this->size)
      return 0;
    size = std::min<uint64_t>(size, this->size - position);
    file.clear();
    file.seekg(position);
    file.read(static_cast<char *>(buffer), size);
    return static_cast<size_t>(file.gcount());
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
  return std::make_unique<FileReader>(filename);
}

class Demuxer::Impl final : public InputStream {
  std::unique_ptr<Reader> reader;
  CancelCheck cancelled;
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
                  int count) {
    if (count <= 0)
      return 0;
    try {
      return static_cast<int>(
          Self(input)->reader->Read(position, buffer, count));
    } catch (agi::Exception const &error) {
      Self(input)->input_error = error.GetMessage();
      return -1;
    } catch (std::exception const &error) {
      Self(input)->input_error = error.what();
      return -1;
    }
  }

  static int64_t Scan(InputStream *input, uint64_t start, unsigned signature) {
    unsigned window = 0;
    for (uint64_t position = start; position < Self(input)->reader->Size();
         ++position) {
      unsigned char byte;
      if (Read(input, position, &byte, 1) != 1)
        return -1;
      window = (window << 8 | byte) & 0xffffffffU;
      if (window == signature)
        return position - 3;
    }
    return -1;
  }

  static int64_t Size(InputStream *input) {
    auto size = Self(input)->reader->Size();
    return size > static_cast<uint64_t>(INT64_MAX) ? INT64_MAX : size;
  }

  void CheckCancelled() const {
    if (cancelled && cancelled())
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
    for (auto const &legacy : attachment_locations)
      attachments.push_back({legacy.UID, copy_string(legacy.Name),
                             copy_string(legacy.Description),
                             copy_string(legacy.MimeType), legacy.Length});

    auto const *segment = mkv_GetFileInfo(file.get());
    if (segment->Duration)
      duration = Timestamp{static_cast<int64_t>(segment->Duration)};
  }

  std::vector<uint8_t> ReadBytes(uint64_t position, uint64_t size) {
    if (size > std::numeric_limits<size_t>::max())
      throw ReadError("Matroska item is too large");
    std::vector<uint8_t> result(static_cast<size_t>(size));
    size_t offset = 0;
    while (offset < result.size()) {
      CheckCancelled();
      auto read = reader->Read(position + offset, result.data() + offset,
                               result.size() - offset);
      if (!read)
        throw ReadError("Unexpected end of Matroska input");
      offset += read;
    }
    return result;
  }

public:
  Impl(std::unique_ptr<Reader> input, CancelCheck cancel)
      : reader(std::move(input)), cancelled(std::move(cancel)) {
    if (!reader)
      throw ReadError("Cannot open Matroska from a null reader");
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
    progress = [](InputStream *input, uint64_t, uint64_t) {
      auto self = Self(input);
      return (!self->cancelled || !self->cancelled()) ? 1 : 0;
    };

    CheckCancelled();
    char error[2048]{};
    file.reset(mkv_Open(this, error, sizeof error));
    if (!file) {
      CheckCancelled();
      throw ReadError(error[0] ? error : "Failed to parse Matroska input");
    }
    ReadMetadata();
  }

  std::vector<SubtitleTrack> const &Tracks() const { return tracks; }
  std::vector<Attachment> const &AttachmentList() const { return attachments; }
  std::optional<Timestamp> Duration() const { return duration; }

  void Select(TrackId track) {
    if (track.value >= mkv_GetNumTracks(file.get()) ||
        track.value >= sizeof(unsigned) * CHAR_BIT)
      throw ReadError("Invalid Matroska subtitle track");
    auto found =
        std::find_if(tracks.begin(), tracks.end(), [&](auto const &item) {
          return item.id.value == track.value;
        });
    if (found == tracks.end())
      throw ReadError("Selected Matroska track is not a subtitle track");

    CheckCancelled();
    mkv_SetTrackMask(file.get(), ~(1U << track.value));
    selected = track;
    compressed.reset();
    if (mkv_GetTrackInfo(file.get(), track.value)->CompEnabled) {
      char error[2048]{};
      compressed.reset(cs_Create(file.get(), track.value, error, sizeof error));
      if (!compressed)
        throw ReadError(error[0] ? error : "Unsupported subtitle compression");
    }
  }

  std::optional<SubtitlePacket> NextPacket() {
    if (!selected)
      throw ReadError("No Matroska subtitle track has been selected");
    CheckCancelled();
    unsigned track, frame_size, flags;
    uint64_t start, end, position;
    if (mkv_ReadFrame(file.get(), ~(1U << selected->value), &track, &start,
                      &end, &position, &frame_size, &flags) < 0)
      return std::nullopt;

    std::vector<uint8_t> data;
    if (!compressed)
      data = ReadBytes(position, frame_size);
    else {
      cs_NextFrame(compressed.get(), position, frame_size);
      unsigned char buffer[4096];
      for (;;) {
        CheckCancelled();
        int count = cs_ReadData(
            compressed.get(), reinterpret_cast<char *>(buffer), sizeof buffer);
        if (count < 0)
          throw ReadError(copy_string(cs_GetLastError(compressed.get())));
        if (!count)
          break;
        data.insert(data.end(), buffer, buffer + count);
      }
    }
    return SubtitlePacket{{track},
                          {static_cast<int64_t>(start)},
                          {static_cast<int64_t>(end)},
                          std::move(data)};
  }

  std::vector<uint8_t> AttachmentBytes(uint64_t uid) {
    auto found =
        std::find_if(attachment_locations.begin(), attachment_locations.end(),
                     [&](auto const &item) { return item.UID == uid; });
    if (found == attachment_locations.end())
      throw ReadError("Unknown Matroska attachment");
    return ReadBytes(found->Position, found->Length);
  }
};

Demuxer::Demuxer(std::unique_ptr<Reader> reader, CancelCheck cancelled)
    : impl(std::make_unique<Impl>(std::move(reader), std::move(cancelled))) {}
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
std::vector<uint8_t> Demuxer::ReadAttachment(uint64_t uid) {
  return impl->AttachmentBytes(uid);
}

} // namespace agi::matroska::proposed
