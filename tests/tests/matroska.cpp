#include <gtest/gtest.h>

#include "matroska.h"
#include "util.h"

#include <cstring>
#include <vector>

namespace {
using namespace agi::matroska;

agi::fs::path fixture(char const *name) {
	return util::test_data_dir() / "matroska" / "fixtures" / name;
}

class ReaderProxy final : public Reader {
	std::unique_ptr<Reader> source;
public:
	bool fail = false;
	bool short_reads = false;
	explicit ReaderProxy(agi::fs::path const& path) : source(OpenFile(path)) {}
	uint64_t Size() const override { return source->Size(); }
	size_t Read(uint64_t position, void *buffer, size_t size) override {
		if (fail) throw IoError("injected reader failure");
		if (short_reads && size > 1) size = 1;
		return source->Read(position, buffer, size);
	}
};
}

TEST(Matroska, MetadataAndAttachments) {
	Demuxer demuxer(OpenFile(fixture("subtitle-attachment.mkv")));
	ASSERT_FALSE(demuxer.SubtitleTracks().empty());
	ASSERT_EQ(1u, demuxer.Attachments().size());
	auto bytes = demuxer.ReadAttachment(demuxer.Attachments()[0].id);
	EXPECT_EQ(demuxer.Attachments()[0].size, bytes.size());
	EXPECT_THROW(demuxer.ReadAttachment(AttachmentId{UINT64_MAX}), InvalidDataError);
}

TEST(Matroska, ShortReadsAreSupported) {
	auto reader = std::make_unique<ReaderProxy>(fixture("subtitle-attachment.mkv"));
	reader->short_reads = true;
	Demuxer demuxer(std::move(reader));
	EXPECT_FALSE(demuxer.SubtitleTracks().empty());
}

TEST(Matroska, ReaderFailureIsContainedAtCBoundary) {
	auto reader = std::make_unique<ReaderProxy>(fixture("subtitle-attachment.mkv"));
	reader->fail = true;
	EXPECT_THROW(Demuxer(std::move(reader)), InvalidDataError);
}

TEST(Matroska, CancellationCallbackExceptionsCannotCrossC) {
	EXPECT_THROW(Demuxer(OpenFile(fixture("subtitle-attachment.mkv")), []() -> bool { throw 42; }), agi::UserCancelException);
}

TEST(Matroska, PacketScanCanBeCancelled) {
	bool cancelled = false;
	Demuxer demuxer(OpenFile(fixture("subtitle-attachment.mkv")), [&] { return cancelled; });
	demuxer.SelectTrack(demuxer.SubtitleTracks()[0].id);
	cancelled = true;
	EXPECT_THROW((void)demuxer.ReadPacket(), agi::UserCancelException);
}

TEST(Matroska, RejectsUnsupportedTrackAndInvalidIds) {
	Demuxer demuxer(OpenFile(fixture("video-only.mkv")));
	EXPECT_TRUE(demuxer.SubtitleTracks().empty());
	EXPECT_THROW(demuxer.SelectTrack(TrackId{UINT32_MAX}), InvalidDataError);
}

TEST(Matroska, PacketAndAttachmentLimits) {
	Limits limits; limits.packet_size = 1; limits.attachment_size = 1; limits.decompressed_size = 1;
	Demuxer demuxer(OpenFile(fixture("subtitle-attachment.mkv")), {}, limits);
	ASSERT_FALSE(demuxer.SubtitleTracks().empty());
	demuxer.SelectTrack(demuxer.SubtitleTracks()[0].id);
	EXPECT_THROW(demuxer.ReadPacket(), LimitError);
	ASSERT_FALSE(demuxer.Attachments().empty());
	EXPECT_THROW(demuxer.ReadAttachment(demuxer.Attachments()[0].id), LimitError);
}

TEST(Matroska, DecompressionLimitAndFailedPacketConsumption) {
	Limits limits; limits.decompressed_size = 1;
	Demuxer compressed(OpenFile(fixture("compressed-zlib.mkv")), {}, limits);
	compressed.SelectTrack(compressed.SubtitleTracks()[0].id);
	EXPECT_THROW(compressed.ReadPacket(), LimitError);

	auto reader = std::make_unique<ReaderProxy>(fixture("subtitle-attachment.mkv"));
	auto *control = reader.get();
	Demuxer demuxer(std::move(reader));
	demuxer.SelectTrack(demuxer.SubtitleTracks()[0].id);
	control->fail = true;
	EXPECT_THROW(demuxer.ReadPacket(), IoError);
	control->fail = false;
	// The failed packet was already consumed before its payload read began.
	EXPECT_NO_THROW((void)demuxer.ReadPacket());
}

TEST(Matroska, PacketsHaveCheckedOptionalTimestamps) {
	Demuxer demuxer(OpenFile(fixture("compressed-zlib.mkv")));
	ASSERT_FALSE(demuxer.SubtitleTracks().empty());
	demuxer.SelectTrack(demuxer.SubtitleTracks()[0].id);
	auto packet = demuxer.ReadPacket();
	ASSERT_TRUE(packet);
	ASSERT_TRUE(packet->start);
	EXPECT_GE(packet->start->nanoseconds, 0);
}
