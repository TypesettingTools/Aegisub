// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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
// $Id$

/// @file libaegisub_cajun.cpp
/// @brief Cajun/Json tests.
/// @ingroup cajun

#include <main.h>

#include <libaegisub/cajun/reader.h>
#include <libaegisub/cajun/writer.h>
#include <libaegisub/cajun/elements.h>
#include <libaegisub/cajun/visitor.h>

class lagi_cajun : public libagi { };

TEST(lagi_cajun, CastNonConst) {
	auto Integer = json::UnknownElement{0};
	auto Double  = json::UnknownElement{0.0};
	auto String  = json::UnknownElement{"1"};
	auto Boolean = json::UnknownElement{false};
	auto Array   = json::UnknownElement{json::Array{}};
	auto Object  = json::UnknownElement{json::Object{}};

	EXPECT_NO_THROW(static_cast<json::Integer&>(Integer));
	EXPECT_NO_THROW(static_cast<json::Double&>(Double));
	EXPECT_NO_THROW(static_cast<json::String&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object&>(Object));

	EXPECT_NO_THROW(static_cast<json::Integer const&>(Integer));
	EXPECT_NO_THROW(static_cast<json::Double const&>(Double));
	EXPECT_NO_THROW(static_cast<json::String const&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean const&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array const&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object const&>(Object));
}

TEST(lagi_cajun, CastConst) {
	const auto Integer = json::UnknownElement{10};
	const auto Double  = json::UnknownElement{10.0};
	const auto String  = json::UnknownElement{"1"};
	const auto Boolean = json::UnknownElement{false};
	const auto Array   = json::UnknownElement{json::Array{}};
	const auto Object  = json::UnknownElement{json::Object{}};

	/* these shouldn't compile
	EXPECT_NO_THROW(static_cast<json::Integer&>(Integer));
	EXPECT_NO_THROW(static_cast<json::Double&>(Double));
	EXPECT_NO_THROW(static_cast<json::String&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object&>(Object));
	*/

	EXPECT_NO_THROW(static_cast<json::Integer const&>(Integer));
	EXPECT_NO_THROW(static_cast<json::Double const&>(Double));
	EXPECT_NO_THROW(static_cast<json::String const&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean const&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array const&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object const&>(Object));

	EXPECT_EQ(10, static_cast<json::Integer const&>(Integer));
	EXPECT_EQ(10, static_cast<json::Double const&>(Double));
	EXPECT_STREQ("1", static_cast<json::String const&>(String).c_str());
	EXPECT_FALSE(static_cast<json::Boolean const&>(Boolean));
	EXPECT_TRUE(static_cast<json::Array const&>(Array).empty());
	EXPECT_TRUE(static_cast<json::Object const&>(Object).empty());
}

TEST(lagi_cajun, ObjectStoreInteger) {
	json::Object obj;
	obj["Integer"] = 1;
	EXPECT_EQ(1, static_cast<json::Integer>(obj["Integer"]));

	EXPECT_THROW(static_cast<json::String const&>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Boolean>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Null>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Array const&>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Object const&>(obj["Integer"]), json::Exception);
}

TEST(lagi_cajun, ObjectStoreDouble) {
	json::Object obj;
	obj["Double"] = 1.0;
	EXPECT_EQ(1.0, static_cast<json::Double>(obj["Double"]));

	EXPECT_THROW(static_cast<json::String const&>(obj["Double"]), json::Exception);
	EXPECT_THROW(static_cast<json::Boolean>(obj["Double"]), json::Exception);
	EXPECT_THROW(static_cast<json::Null>(obj["Double"]), json::Exception);
	EXPECT_THROW(static_cast<json::Array const&>(obj["Double"]), json::Exception);
	EXPECT_THROW(static_cast<json::Object const&>(obj["Double"]), json::Exception);
}

TEST(lagi_cajun, ObjectStoreString) {
	json::Object obj;
	obj["String"] = "test";
	EXPECT_STREQ("test", static_cast<std::string>(obj["String"]).c_str());

	EXPECT_THROW(static_cast<json::Integer>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Boolean>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Null>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Array const&>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Object const&>(obj["String"]), json::Exception);
}

TEST(lagi_cajun, ObjectStoreBoolean) {
	json::Object obj;
	obj["Boolean"] = true;
	EXPECT_TRUE(static_cast<json::Boolean>(obj["Boolean"]));

	EXPECT_THROW(static_cast<json::String const&>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Integer>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Null>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Array const&>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Object const&>(obj["Boolean"]), json::Exception);
}

TEST(lagi_cajun, ObjectStoreNull) {
	json::Object obj;
	obj["Null"] = json::Null();

	EXPECT_NO_THROW(static_cast<json::Null>(obj["Null"]));

	// null is implicitly convertible to everything
	EXPECT_NO_THROW(static_cast<json::String const&>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Integer>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Double>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Boolean>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Array const&>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Object const&>(obj["Null"]));

	// obj["Null"] should no longer be of type Null
	EXPECT_THROW(static_cast<json::Null>(obj["Null"]), json::Exception);
}

TEST(lagi_cajun, ObjectCreateArray) {
	json::Object obj;
	obj["Inside"] = "";
	json::Array array;
	array.push_back(std::move(obj));

	EXPECT_STREQ("", static_cast<std::string>(static_cast<json::Object&>(array[0])["Inside"]).c_str());
}

TEST(lagi_cajun, Read) {
	json::UnknownElement root;
	std::istringstream doc("{\"String\" : \"This is a test\", \"Boolean\" : false, \"Null\" : null }");
	EXPECT_NO_THROW(json::Reader::Read(root, doc));
	json::Object& obj = root;
	EXPECT_NO_THROW(obj["String"]);
	EXPECT_STREQ("This is a test", static_cast<std::string>(obj["String"]).c_str());
	EXPECT_FALSE(static_cast<json::Boolean>(obj["Boolean"]));
	EXPECT_NO_THROW(static_cast<json::Null>(obj["Null"]));
}

TEST(lagi_cajun, Write) {
	json::Object obj;
	obj["Boolean"] = true;
	obj["String"] = "This \"is\" \\a \t test";

	std::stringstream stream;
	EXPECT_NO_THROW(agi::JsonWriter::Write(obj, stream));
	EXPECT_STREQ("{\n\t\"Boolean\" : true,\n\t\"String\" : \"This \\\"is\\\" \\\\a \\t test\"\n}", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(agi::JsonWriter::Write(json::Array(), stream));
	EXPECT_STREQ("[]", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(agi::JsonWriter::Write(json::Object(), stream));
	EXPECT_STREQ("{}", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(agi::JsonWriter::Write(true, stream));
	EXPECT_STREQ("true", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(agi::JsonWriter::Write(false, stream));
	EXPECT_STREQ("false", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(agi::JsonWriter::Write(json::Null(), stream));
	EXPECT_STREQ("null", stream.str().c_str());
}

TEST(lagi_cajun, ReaderParserErrors) {
	json::UnknownElement ue;

	std::istringstream missing_comma("[1 2]");
	EXPECT_THROW(json::Reader::Read(ue, missing_comma), json::Exception);

	std::istringstream garbage_after_number("123!");
	EXPECT_THROW(json::Reader::Read(ue, garbage_after_number), json::Exception);

	std::istringstream unexpected_eof("[");
	EXPECT_THROW(json::Reader::Read(ue, unexpected_eof), json::Exception);

	std::istringstream bad_initial_token("]");
	EXPECT_THROW(json::Reader::Read(ue, bad_initial_token), json::Exception);

	std::istringstream garbage_after_end("[]a");
	EXPECT_THROW(json::Reader::Read(ue, garbage_after_end), json::Exception);

	std::istringstream empty_str("");
	EXPECT_THROW(json::Reader::Read(ue, empty_str), json::Exception);

	std::istringstream dupe_keys("{\"a\": [], \"a\": 0}");
	EXPECT_THROW(json::Reader::Read(ue, dupe_keys), json::Exception);

	std::istringstream unique_keys("{\"a\": [], \"b\": 0}");
	EXPECT_NO_THROW(json::Reader::Read(ue, unique_keys));
}

TEST(lagi_cajun, ReaderScanErrors) {
	json::UnknownElement ue;

	std::istringstream doc("[true, false, thiswontwork]");
	EXPECT_THROW(json::Reader::Read(ue, doc), json::Exception);

	std::istringstream garbage_after_number("123abc");
	EXPECT_THROW(json::Reader::Read(ue, garbage_after_number), json::Exception);

	std::istringstream bad_escape("\"\\j\"");
	EXPECT_THROW(json::Reader::Read(ue, bad_escape), json::Exception);

	std::istringstream unexpected_eof("\"abc");
	EXPECT_THROW(json::Reader::Read(ue, unexpected_eof), json::Exception);
}

std::string roundtrip_test(const char *in) {
	std::istringstream iss(in);
	json::UnknownElement ele;
	json::Reader::Read(ele, iss);

	std::stringstream oss;
	agi::JsonWriter::Write(ele, oss);
	return oss.str();
}

TEST(lagi_cajun, round_double_roundtrips) {
	EXPECT_STREQ("1.0", roundtrip_test("1.0").c_str());
}

TEST(lagi_cajun, representable_double_roundtrips) {
	EXPECT_STREQ("1.5", roundtrip_test("1.5").c_str());
}

TEST(lagi_cajun, int_roundtrips) {
	EXPECT_STREQ("1", roundtrip_test("1").c_str());
}

TEST(lagi_cajun, bool_roundtrips) {
	EXPECT_STREQ("true", roundtrip_test("true").c_str());
	EXPECT_STREQ("false", roundtrip_test("false").c_str());
}

TEST(lagi_cajun, null_roundtrips) {
	EXPECT_STREQ("null", roundtrip_test("null").c_str());
}
