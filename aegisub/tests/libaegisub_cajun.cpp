// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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


TEST_F(lagi_cajun, ObjectCreateNumber) {
	json::Object obj;
	obj["Integer"] = json::Number(1);
}


TEST_F(lagi_cajun, ObjectCreateString) {
	json::Object obj;
	obj["String"] = json::String("test");
}


TEST_F(lagi_cajun, ObjectCreateBoolean) {
	json::Object obj;
	obj["Boolean"] = json::Boolean(true);
}


TEST_F(lagi_cajun, ObjectCreateNull) {
	json::Object obj;
	obj["Null"] = json::Null();
}


TEST_F(lagi_cajun, ObjectCreateArray) {
	json::Object obj;
	obj["Inside"] = json::String();
	json::Array array;
	array.Insert(obj);
}


TEST_F(lagi_cajun, ObjectEquality) {
	json::Object obj;
	obj["Inside"] = json::String();
	json::Array array;
	array.Insert(obj);
	obj["Array"] = array;
	json::Object obj_dupe = obj;

	EXPECT_TRUE(obj_dupe == obj);

	obj_dupe["NotEqual"] = array;
	EXPECT_FALSE(obj_dupe == obj);

}

// Cajun doesn't have chained exceptions, so there's no real way to test the
// difference in the following exceptions.  I'll try emailing the author to see
// If they'll add them, if not we'll do it ourselves.
TEST_F(lagi_cajun, ExExceptionArrayOutOfBounds) {
	json::Object obj;
	obj["Inside"] = json::String();
	json::Array array;
	array.Insert(obj);
	obj["Array"] = array;
	const json::Array& const_array = obj["Array"];

	EXPECT_THROW({
		const json::String& str = const_array[1];
		str.Value(); // avoid unused variable warning
	}, json::Exception);
}


TEST_F(lagi_cajun, ExExceptionArrayObjNotFound) {
	json::Object obj;
	obj["Inside"] = json::String();
	json::Array array;
	array.Insert(obj);
	obj["Array"] = array;
	const json::Array& const_array = obj["Array"];

	EXPECT_THROW({
		const json::String& str = const_array[0]["Nothere"];
		str.Value(); // avoid unused variable warning
	}, json::Exception);
}


TEST_F(lagi_cajun, ExExceptionArrayBadCast) {
	json::Object obj;
	obj["Inside"] = json::String();
	json::Array array;
	array.Insert(obj);
	obj["Array"] = array;
	const json::Array& const_array = obj["Array"];
	const json::Object& arr_obj = const_array[0];

	EXPECT_THROW({
		const json::UnknownElement& unkn = arr_obj["Array"]["BadCast"];
		unkn["Array"]; // avoid unused variable warning
	}, json::Exception);
}


TEST_F(lagi_cajun, Read) {
	json::Object obj;
	std::istringstream doc("{\"String\" : \"This is a test\"}");
	EXPECT_NO_THROW(json::Reader::Read(obj, doc));
}


TEST_F(lagi_cajun, Write) {
	json::Object obj;
	std::istringstream doc("{\"String\" : \"This is a test\"}");

	std::stringstream stream;
	EXPECT_NO_THROW(json::Writer::Write(obj, stream));
}

TEST_F(lagi_cajun, ReaderExParseException) {
	json::Object obj;
	std::istringstream doc("[1 2]");

	EXPECT_THROW(json::Reader::Read(obj, doc), json::Reader::ParseException);
}


TEST_F(lagi_cajun, ReaderExScanException) {
	json::Object obj;
	std::istringstream doc("[true, false, thiswontwork]");

	EXPECT_THROW(json::Reader::Read(obj, doc), json::Reader::ScanException);
}
