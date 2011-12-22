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

#include "main.h"

#include <libaegisub/cajun/reader.h>
#include <libaegisub/cajun/writer.h>
#include <libaegisub/cajun/elements.h>
#include <libaegisub/cajun/visitor.h>

class lagi_cajun : public libagi {

protected:
    // place holder for future code placement
};

TEST_F(lagi_cajun, Compare) {
	json::UnknownElement Integer1 = 0;
	json::UnknownElement Integer2 = 1;
	json::UnknownElement String1 = "1";
	json::UnknownElement String2 = "2";
	json::UnknownElement Boolean1 = false;
	json::UnknownElement Boolean2 = true;
	json::UnknownElement Array1 = json::Array();
	json::UnknownElement Array2 = json::Array();
	json::UnknownElement Object1 = json::Object();
	json::UnknownElement Object2 = json::Object();
	json::UnknownElement Null = json::Null();

	static_cast<json::Array&>(Array2).push_back(1);
	Object2["a"] = "b";

	// Test that things are equal to themselves and mixed comparisons are not errors
	EXPECT_EQ(Integer1, Integer1);
	EXPECT_NE(Integer1, Integer2);
	EXPECT_NE(Integer1, String1);
	EXPECT_NE(Integer1, Boolean1);
	EXPECT_NE(Integer1, Array1);
	EXPECT_NE(Integer1, Object1);
	EXPECT_NE(Integer1, Null);

	EXPECT_EQ(String1, String1);
	EXPECT_NE(String1, Integer2);
	EXPECT_NE(String1, String2);
	EXPECT_NE(String1, Boolean1);
	EXPECT_NE(String1, Array1);
	EXPECT_NE(String1, Object1);
	EXPECT_NE(String1, Null);

	EXPECT_EQ(Boolean1, Boolean1);
	EXPECT_NE(Boolean1, Integer2);
	EXPECT_NE(Boolean1, String1);
	EXPECT_NE(Boolean1, Boolean2);
	EXPECT_NE(Boolean1, Array1);
	EXPECT_NE(Boolean1, Object1);
	EXPECT_NE(Boolean1, Null);

	EXPECT_EQ(Array1, Array1);
	EXPECT_NE(Array1, Integer2);
	EXPECT_NE(Array1, String1);
	EXPECT_NE(Array1, Boolean1);
	EXPECT_NE(Array1, Array2);
	EXPECT_NE(Array1, Object1);
	EXPECT_NE(Array1, Null);

	EXPECT_EQ(Object1, Object1);
	EXPECT_NE(Object1, Integer2);
	EXPECT_NE(Object1, String1);
	EXPECT_NE(Object1, Boolean1);
	EXPECT_NE(Object1, Array2);
	EXPECT_NE(Object1, Object2);
	EXPECT_NE(Object1, Null);

	EXPECT_EQ(Null, Null);
	EXPECT_NE(Null, Integer2);
	EXPECT_NE(Null, String1);
	EXPECT_NE(Null, Boolean1);
	EXPECT_NE(Null, Array2);
	EXPECT_NE(Null, Object2);
}

TEST_F(lagi_cajun, CastNonConst) {
	json::UnknownElement Integer = 0;
	json::UnknownElement String = "1";
	json::UnknownElement Boolean = false;
	json::UnknownElement Array = json::Array();
	json::UnknownElement Object = json::Object();

	EXPECT_NO_THROW(static_cast<json::Number&>(Integer));
	EXPECT_NO_THROW(static_cast<json::String&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object&>(Object));

	EXPECT_NO_THROW(static_cast<json::Number const&>(Integer));
	EXPECT_NO_THROW(static_cast<json::String const&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean const&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array const&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object const&>(Object));
}

TEST_F(lagi_cajun, CastConst) {
	const json::UnknownElement Integer = 10;
	const json::UnknownElement String = "1";
	const json::UnknownElement Boolean = false;
	const json::UnknownElement Array = json::Array();
	const json::UnknownElement Object = json::Object();

	/* these shouldn't compile
	EXPECT_NO_THROW(static_cast<json::Number&>(Integer));
	EXPECT_NO_THROW(static_cast<json::String&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object&>(Object));
	*/

	EXPECT_NO_THROW(static_cast<json::Number const&>(Integer));
	EXPECT_NO_THROW(static_cast<json::String const&>(String));
	EXPECT_NO_THROW(static_cast<json::Boolean const&>(Boolean));
	EXPECT_NO_THROW(static_cast<json::Array const&>(Array));
	EXPECT_NO_THROW(static_cast<json::Object const&>(Object));

	EXPECT_EQ(10, static_cast<json::Number const&>(Integer));
	EXPECT_STREQ("1", static_cast<json::String const&>(String).c_str());
	EXPECT_EQ(false, static_cast<json::Boolean const&>(Boolean));
	EXPECT_EQ(true, static_cast<json::Array const&>(Array).empty());
	EXPECT_EQ(true, static_cast<json::Object const&>(Object).empty());
}

TEST_F(lagi_cajun, UnknownIsIndexable) {
	json::Object obj;
	obj["Integer"] = 1;
	json::UnknownElement unk_obj = obj;

	EXPECT_NO_THROW(unk_obj["Integer"]);
	EXPECT_EQ(1, (json::Number)unk_obj["Integer"]);
	EXPECT_THROW(unk_obj[0], json::Exception);
	EXPECT_NO_THROW(unk_obj["Nonexistent Key"]);

	json::UnknownElement const& const_unk_obj = obj;
	EXPECT_NO_THROW(const_unk_obj["Integer"]);
	EXPECT_THROW(const_unk_obj["Another nonexistent Key"], json::Exception);

	json::Array arr;
	arr.push_back(1);
	json::UnknownElement unk_arr = arr;

	EXPECT_NO_THROW(unk_arr[0]);
	EXPECT_EQ(1, (json::Number)unk_arr[0]);
	EXPECT_THROW(unk_arr["Integer"], json::Exception);

	json::Number number = 1;
	json::UnknownElement const& unk_num = number;

	EXPECT_THROW(unk_num[0], json::Exception);
	EXPECT_THROW(unk_num[""], json::Exception);
}

TEST_F(lagi_cajun, ObjectStoreNumber) {
	json::Object obj;
	obj["Integer"] = 1;
	EXPECT_EQ(1, static_cast<json::Number>(obj["Integer"]));

	EXPECT_THROW(static_cast<json::String const&>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Boolean>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Null>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Array const&>(obj["Integer"]), json::Exception);
	EXPECT_THROW(static_cast<json::Object const&>(obj["Integer"]), json::Exception);
}

TEST_F(lagi_cajun, ObjectStoreString) {
	json::Object obj;
	obj["String"] = "test";
	EXPECT_STREQ("test", static_cast<std::string>(obj["String"]).c_str());

	EXPECT_THROW(static_cast<json::Number>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Boolean>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Null>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Array const&>(obj["String"]), json::Exception);
	EXPECT_THROW(static_cast<json::Object const&>(obj["String"]), json::Exception);
}

TEST_F(lagi_cajun, ObjectStoreBoolean) {
	json::Object obj;
	obj["Boolean"] = true;
	EXPECT_EQ(true, static_cast<json::Boolean>(obj["Boolean"]));

	EXPECT_THROW(static_cast<json::String const&>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Number>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Null>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Array const&>(obj["Boolean"]), json::Exception);
	EXPECT_THROW(static_cast<json::Object const&>(obj["Boolean"]), json::Exception);
}

TEST_F(lagi_cajun, ObjectStoreNull) {
	json::Object obj;
	obj["Null"] = json::Null();

	EXPECT_NO_THROW(static_cast<json::Null>(obj["Null"]));

	// null is implicitly convertible to everything
	EXPECT_NO_THROW(static_cast<json::String const&>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Number>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Boolean>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Array const&>(obj["Null"]));

	obj["Null"] = json::Null();
	EXPECT_NO_THROW(static_cast<json::Object const&>(obj["Null"]));

	// obj["Null"] should no longer be of type Null
	EXPECT_THROW(static_cast<json::Null>(obj["Null"]), json::Exception);
}

TEST_F(lagi_cajun, ObjectCreateArray) {
	json::Object obj;
	obj["Inside"] = "";
	json::Array array;
	array.push_back(obj);

	EXPECT_STREQ("", static_cast<std::string>(array[0]["Inside"]).c_str());
}

TEST_F(lagi_cajun, ObjectEquality) {
	json::Object obj;
	obj["Inside"] = "";
	json::Array array;
	array.push_back(obj);
	obj["Array"] = array;
	json::Object obj_dupe = obj;

	EXPECT_TRUE(obj_dupe == obj);

	obj_dupe["NotEqual"] = array;
	EXPECT_FALSE(obj_dupe == obj);
}

TEST_F(lagi_cajun, Read) {
	json::Object obj;
	std::istringstream doc("{\"String\" : \"This is a test\", \"Boolean\" : false, \"Null\" : null }");
	EXPECT_NO_THROW(json::Reader::Read(obj, doc));
	EXPECT_NO_THROW(obj["String"]);
	EXPECT_STREQ("This is a test", static_cast<std::string>(obj["String"]).c_str());
	EXPECT_EQ(false, static_cast<json::Boolean>(obj["Boolean"]));
	EXPECT_NO_THROW(static_cast<json::Null>(obj["Null"]));
}


TEST_F(lagi_cajun, Write) {
	json::Object obj;
	obj["Boolean"] = true;
	obj["String"] = "This \"is\" \\a \t test";

	std::stringstream stream;
	EXPECT_NO_THROW(json::Writer::Write(obj, stream));
	EXPECT_STREQ("{\n\t\"Boolean\" : true,\n\t\"String\" : \"This \\\"is\\\" \\\\a \\t test\"\n}", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(json::Writer::Write(json::Array(), stream));
	EXPECT_STREQ("[]", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(json::Writer::Write(json::Object(), stream));
	EXPECT_STREQ("{}", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(json::Writer::Write(true, stream));
	EXPECT_STREQ("true", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(json::Writer::Write(false, stream));
	EXPECT_STREQ("false", stream.str().c_str());

	stream.str("");
	EXPECT_NO_THROW(json::Writer::Write(json::Null(), stream));
	EXPECT_STREQ("null", stream.str().c_str());
}

TEST_F(lagi_cajun, ReaderParserErrors) {
	json::Array arr;
	std::istringstream missing_comma("[1 2]");
	EXPECT_THROW(json::Reader::Read(arr, missing_comma), json::Exception);

	json::Number num;
	std::istringstream garbage_after_number("123eee");
	EXPECT_THROW(json::Reader::Read(num, garbage_after_number), json::Exception);

	json::String str;
	std::istringstream unexpected_eof("[");
	EXPECT_THROW(json::Reader::Read(str, unexpected_eof), json::Exception);

	std::istringstream bad_initial_token("]");
	EXPECT_THROW(json::Reader::Read(str, bad_initial_token), json::Exception);

	std::istringstream garbage_after_end("[]a");
	EXPECT_THROW(json::Reader::Read(str, garbage_after_end), json::Exception);

	json::Null null;
	std::istringstream empty_str("");
	EXPECT_THROW(json::Reader::Read(null, empty_str), json::Exception);

	json::Object obj;
	std::istringstream dupe_keys("{\"a\": [], \"a\": 0}");
	EXPECT_THROW(json::Reader::Read(obj, dupe_keys), json::Exception);

	std::istringstream unique_keys("{\"a\": [], \"b\": 0}");
	EXPECT_NO_THROW(json::Reader::Read(obj, unique_keys));
}

TEST_F(lagi_cajun, ReaderScanErrors) {
	json::Object obj;
	std::istringstream doc("[true, false, thiswontwork]");

	EXPECT_THROW(json::Reader::Read(obj, doc), json::Exception);

	json::Number num;
	std::istringstream garbage_after_number("123abc");
	EXPECT_THROW(json::Reader::Read(num, garbage_after_number), json::Exception);

	json::String str;
	std::istringstream bad_escape("\"\\j\"");
	EXPECT_THROW(json::Reader::Read(str, bad_escape), json::Exception);

	std::istringstream unexpected_eof("\"abc");
	EXPECT_THROW(json::Reader::Read(str, unexpected_eof), json::Exception);
}

std::string roundtrip_test(const char *in) {
	std::istringstream iss(in);
	json::UnknownElement ele;
	json::Reader::Read(ele, iss);

	std::stringstream oss;
	json::Writer::Write(ele, oss);
	return oss.str();
}

TEST_F(lagi_cajun, round_double_roundtrips) {
	EXPECT_STREQ("1.0", roundtrip_test("1.0").c_str());
}

TEST_F(lagi_cajun, representable_double_roundtrips) {
	EXPECT_STREQ("1.5", roundtrip_test("1.5").c_str());
}

TEST_F(lagi_cajun, int_roundtrips) {
	EXPECT_STREQ("1", roundtrip_test("1").c_str());
}

TEST_F(lagi_cajun, bool_roundtrips) {
	EXPECT_STREQ("true", roundtrip_test("true").c_str());
	EXPECT_STREQ("false", roundtrip_test("false").c_str());
}

TEST_F(lagi_cajun, null_roundtrips) {
	EXPECT_STREQ("null", roundtrip_test("null").c_str());
}
