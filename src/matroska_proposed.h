// Copyright (c) 2026 Aegisub contributors
//
// Permission to use, copy, modify, and distribute this software for any purpose
// with or without fee is hereby granted, provided that the above copyright
// notice and this permission notice appear in all copies.

#pragma once

#include <libaegisub/exception.h>
#include <libaegisub/fs.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// This is an unused proposal for side-by-side API review. Nothing in Aegisub
// constructs these types, and this file is intentionally absent from the build.
namespace agi::matroska::proposed {

DEFINE_EXCEPTION(ReadError, agi::Exception);

struct Timestamp {
  int64_t nanoseconds = 0;
};

struct TrackId {
  uint32_t value = 0;
};

enum class SubtitleCodec { ass, ssa, srt, unsupported };

struct SubtitleTrack {
  TrackId id;
  uint64_t uid = 0;
  SubtitleCodec codec = SubtitleCodec::unsupported;
  std::string codec_id;
  std::string name;
  std::string language;
  std::vector<uint8_t> codec_private;
  bool enabled = true;
  bool is_default = false;
};

struct Attachment {
  uint64_t uid = 0;
  std::string name;
  std::string description;
  std::string mime_type;
  uint64_t size = 0;
};

struct SubtitlePacket {
  TrackId track;
  Timestamp start;
  Timestamp end;
  std::vector<uint8_t> data;
};

using CancelCheck = std::function<bool()>;

/// Random-access input owned by Demuxer for its entire lifetime.
class Reader {
public:
  virtual ~Reader() = default;
  virtual uint64_t Size() const = 0;
  /// Read up to size bytes at position. Returning zero means end of input.
  virtual size_t Read(uint64_t position, void *buffer, size_t size) = 0;
};

/// Open a file-backed reader using Aegisub's normal mapped-file input.
std::unique_ptr<Reader> OpenFile(agi::fs::path const &filename);

/// Subtitle/attachment-only Matroska view proposed for Aegisub.
///
/// The demuxer owns its Reader and all parser state. Returned metadata and
/// packet bytes are values, so none of them borrow storage from the parser.
class Demuxer final {
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  explicit Demuxer(std::unique_ptr<Reader> reader, CancelCheck cancelled = {});
  ~Demuxer();

  Demuxer(Demuxer &&) noexcept;
  Demuxer &operator=(Demuxer &&) noexcept;
  Demuxer(Demuxer const &) = delete;
  Demuxer &operator=(Demuxer const &) = delete;

  std::vector<SubtitleTrack> const &SubtitleTracks() const;
  std::vector<Attachment> const &Attachments() const;
  std::optional<Timestamp> Duration() const;

  /// Select one subtitle track and rewind packet iteration to its beginning.
  void SelectTrack(TrackId track);
  /// Return the next decoded packet, or nullopt at end of input.
  std::optional<SubtitlePacket> ReadPacket();
  /// Copy an attachment from the input. The returned vector owns its bytes.
  std::vector<uint8_t> ReadAttachment(uint64_t uid);
};

} // namespace agi::matroska::proposed
