#include <gtest/gtest.h>

#include "matroska.h"
#include "util.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

namespace {
using namespace agi::matroska;

agi::fs::path fixture_dir() {
	return util::test_data_dir() / "matroska" / "fixtures";
}

agi::fs::path fixture(char const *name) {
	return fixture_dir() / name;
}

uint64_t fnv1a(std::vector<uint8_t> const& data) {
	uint64_t hash = 1469598103934665603ULL;
	for (auto byte : data) {
		hash ^= byte;
		hash *= 1099511628211ULL;
	}
	return hash;
}

std::string describe_time(std::optional<Timestamp> const& time) {
	return time ? std::to_string(time->nanoseconds) : "-";
}

// Dump everything the demuxer exposes for a file in a stable text format
std::string describe(agi::fs::path const& path) {
	std::ostringstream out;
	try {
		Demuxer demuxer(OpenFile(path));
		out << "duration\t" << describe_time(demuxer.Duration()) << '\n';
		for (auto const& track : demuxer.SubtitleTracks())
			out << "track\t" << track.id.value << '\t' << track.uid << '\t' << track.codec_id << '\t'
				<< track.language << '\t' << track.name << '\t' << track.codec_private.size() << '\t'
				<< track.enabled << '\t' << track.is_default << '\n';
		for (auto const& attachment : demuxer.Attachments())
			out << "attachment\t" << attachment.name << '\t' << attachment.mime_type << '\t' << attachment.size
				<< '\t' << std::hex << fnv1a(demuxer.ReadAttachment(attachment.id)) << std::dec << '\n';
		for (auto const& track : demuxer.SubtitleTracks()) {
			if (track.codec == SubtitleCodec::unsupported) continue;
			demuxer.SelectTrack(track.id);
			while (auto packet = demuxer.ReadPacket())
				out << "packet\t" << packet->track.value << '\t' << describe_time(packet->start) << '\t'
					<< describe_time(packet->end) << '\t' << packet->data.size() << '\t'
					<< std::hex << fnv1a(packet->data) << std::dec << '\n';
		}
	}
	catch (Error const& e) {
		out << "error\t" << e.GetMessage() << '\n';
	}
	return out.str();
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

TEST(Matroska, ReaderFailureDuringOpenIsAnIoError) {
	auto reader = std::make_unique<ReaderProxy>(fixture("subtitle-attachment.mkv"));
	reader->fail = true;
	EXPECT_THROW(Demuxer(std::move(reader)), IoError);
}

TEST(Matroska, MissingFileIsAnIoError) {
	EXPECT_THROW(OpenFile(fixture("does-not-exist.mkv")), IoError);
}

TEST(Matroska, CancellationCallbackExceptionsCancel) {
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

// Each fixture's .behavior file records the expected describe() output. When a
// fixture is added or changed, the failure message contains the new output.
TEST(Matroska, FixtureBehavior) {
	std::vector<agi::fs::path> fixtures;
	for (auto const& entry : std::filesystem::directory_iterator(fixture_dir())) {
		auto ext = entry.path().extension();
		if (ext == ".mkv" || ext == ".mka")
			fixtures.emplace_back(entry.path());
	}
	std::sort(fixtures.begin(), fixtures.end());
	ASSERT_FALSE(fixtures.empty());

	for (auto const& path : fixtures) {
		auto expected_path = path;
		expected_path.replace_extension(".behavior");
		std::ifstream expected_file(expected_path, std::ios::binary);
		std::stringstream expected;
		expected << expected_file.rdbuf();
		auto actual = describe(path);
		EXPECT_EQ(expected.str(), actual) << path.filename();
	}
}
