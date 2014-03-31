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

#include <libaegisub/fs.h>
#include <libaegisub/option.h>
#include <libaegisub/option_value.h>

#include "main.h"
#include "util.h"

#include <fstream>

static const char default_opt[] = "{\"Valid\" : \"This is valid\"}";

class lagi_option : public libagi {
protected:
	std::string conf_ok;

	void SetUp() override {
		conf_ok = "data/options/string.json";
	}
};

TEST_F(lagi_option, construct_from_file) {
	EXPECT_NO_THROW(agi::Options(conf_ok, default_opt, agi::Options::FLUSH_SKIP));
}

TEST_F(lagi_option, nonexistent_file) {
	EXPECT_NO_THROW(agi::Options("", default_opt, agi::Options::FLUSH_SKIP));
}

TEST_F(lagi_option, get_existing_option) {
	agi::Options opt(conf_ok, default_opt, agi::Options::FLUSH_SKIP);
	ASSERT_NO_THROW(opt.Get("Valid"));
	ASSERT_NO_THROW(opt.Get("Valid")->GetString());
	EXPECT_STREQ("This is valid", opt.Get("Valid")->GetString().c_str());
}

TEST_F(lagi_option, get_nonexistant_option) {
	agi::Options opt(conf_ok, default_opt, agi::Options::FLUSH_SKIP);
	EXPECT_THROW(opt.Get("Nonexistant option"), agi::OptionErrorNotFound);
}

TEST_F(lagi_option, flush_skip) {
	agi::fs::Copy("data/options/string.json", "data/options/tmp");
	{
		agi::Options opt("data/options/tmp", default_opt, agi::Options::FLUSH_SKIP);
		ASSERT_NO_THROW(opt.Get("Valid")->SetString(""));
	}
	EXPECT_TRUE(util::compare("data/options/string.json", "data/options/tmp"));
}

TEST_F(lagi_option, flush_no_skip) {
	agi::fs::Copy("data/options/string.json", "data/options/tmp");
	{
		agi::Options opt("data/options/tmp", default_opt);
		ASSERT_NO_THROW(opt.Get("Valid")->SetString(""));
	}
	EXPECT_FALSE(util::compare("data/options/string.json", "data/options/tmp"));
}

TEST_F(lagi_option, existent_but_invalid_file_uses_default) {
	agi::Options opt("data/options/null.json", default_opt, agi::Options::FLUSH_SKIP);
	EXPECT_NO_THROW(opt.Get("Valid")->GetString());
	EXPECT_THROW(opt.Get("Null"), agi::Exception);
}

TEST_F(lagi_option, load_several_config_files) {
	agi::Options opt("", default_opt, agi::Options::FLUSH_SKIP);
	{ std::ifstream f("data/options/string.json");  EXPECT_NO_THROW(opt.ConfigNext(f)); }
	{ std::ifstream f("data/options/integer.json"); EXPECT_NO_THROW(opt.ConfigNext(f)); }
	{ std::ifstream f("data/options/double.json");  EXPECT_NO_THROW(opt.ConfigNext(f)); }
	{ std::ifstream f("data/options/bool.json");    EXPECT_NO_THROW(opt.ConfigNext(f)); }

	EXPECT_NO_THROW(opt.Get("String")->GetString());
	EXPECT_NO_THROW(opt.Get("Integer")->GetInt());
	EXPECT_NO_THROW(opt.Get("Double")->GetDouble());
	EXPECT_NO_THROW(opt.Get("Bool")->GetBool());
}

TEST_F(lagi_option, arrays) {
	agi::Options opt("", default_opt, agi::Options::FLUSH_SKIP);
	{ std::ifstream f("data/options/array_string.json");  EXPECT_NO_THROW(opt.ConfigNext(f)); }
	{ std::ifstream f("data/options/array_integer.json"); EXPECT_NO_THROW(opt.ConfigNext(f)); }
	{ std::ifstream f("data/options/array_double.json");  EXPECT_NO_THROW(opt.ConfigNext(f)); }
	{ std::ifstream f("data/options/array_bool.json");    EXPECT_NO_THROW(opt.ConfigNext(f)); }

	EXPECT_NO_THROW(opt.Get("String")->GetListString());
	EXPECT_NO_THROW(opt.Get("Integer")->GetListInt());
	EXPECT_NO_THROW(opt.Get("Double")->GetListDouble());
	EXPECT_NO_THROW(opt.Get("Bool")->GetListBool());
}

TEST_F(lagi_option, bad_default_throws_and_null_is_rejected) {
	EXPECT_THROW(agi::Options("", "{\"Null\" : null}", agi::Options::FLUSH_SKIP), agi::Exception);
}

TEST_F(lagi_option, nested_options) {
	const char conf[] = "{ \"a\" : { \"b\" : { \"c\" : { \"c\" : \"value\" } } } }";
	ASSERT_NO_THROW(agi::Options("", conf, agi::Options::FLUSH_SKIP));
	agi::Options opt("", conf, agi::Options::FLUSH_SKIP);
	ASSERT_NO_THROW(opt.Get("a/b/c/c"));
	ASSERT_NO_THROW(opt.Get("a/b/c/c")->GetString());
	EXPECT_STREQ("value", opt.Get("a/b/c/c")->GetString().c_str());
}

TEST_F(lagi_option, heterogeneous_arrays_rejected) {
	EXPECT_NO_THROW(agi::Options("", "{ \"key\" : [ { \"bool\" : true }] }", agi::Options::FLUSH_SKIP));
	EXPECT_THROW(agi::Options("", "{ \"key\" : [ { \"bool\" : true }, { \"double\" : 1.0 } ] }", agi::Options::FLUSH_SKIP), agi::Exception);
}

TEST_F(lagi_option, flush_roundtrip) {
	agi::fs::Remove("data/options/tmp");

	{
		agi::Options opt("data/options/tmp", "{}");
		{ std::ifstream f("data/options/all_types.json"); EXPECT_NO_THROW(opt.ConfigNext(f)); }
		EXPECT_NO_THROW(opt.Get("Integer")->SetInt(1));
		EXPECT_NO_THROW(opt.Get("Double")->SetDouble(1.1));
		EXPECT_NO_THROW(opt.Get("String")->SetString("hello"));
		EXPECT_NO_THROW(opt.Get("Color")->SetColor(agi::Color("rgb(255,255,255)")));
		EXPECT_NO_THROW(opt.Get("Boolean")->SetBool(true));

		std::vector<int64_t> int_arr; int_arr.push_back(1);
		EXPECT_NO_THROW(opt.Get("Array/Integer")->SetListInt(int_arr));
		std::vector<double> double_arr; double_arr.push_back(1.1);
		EXPECT_NO_THROW(opt.Get("Array/Double")->SetListDouble(double_arr));
		std::vector<std::string> str_arr; str_arr.push_back("hello");
		EXPECT_NO_THROW(opt.Get("Array/String")->SetListString(str_arr));
		std::vector<agi::Color> clr_arr; clr_arr.push_back(agi::Color("rgb(255,255,255)"));
		EXPECT_NO_THROW(opt.Get("Array/Color")->SetListColor(clr_arr));
		std::vector<bool> bool_arr; bool_arr.push_back(true);
		EXPECT_NO_THROW(opt.Get("Array/Boolean")->SetListBool(bool_arr));
	}

	{
		agi::Options opt("data/options/tmp", "{}");
		ASSERT_NO_THROW(opt.ConfigUser());

		EXPECT_EQ(1, opt.Get("Integer")->GetInt());
		EXPECT_EQ(1.1, opt.Get("Double")->GetDouble());
		EXPECT_STREQ("hello", opt.Get("String")->GetString().c_str());
		EXPECT_STREQ("rgb(255, 255, 255)", opt.Get("Color")->GetColor().GetRgbFormatted().c_str());
		EXPECT_EQ(true, opt.Get("Boolean")->GetBool());

		EXPECT_EQ(1, opt.Get("Array/Integer")->GetListInt().size());
		EXPECT_EQ(1, opt.Get("Array/Double")->GetListDouble().size());
		EXPECT_EQ(1, opt.Get("Array/String")->GetListString().size());
		EXPECT_EQ(1, opt.Get("Array/Color")->GetListColor().size());
		EXPECT_EQ(1, opt.Get("Array/Boolean")->GetListBool().size());

		EXPECT_EQ(1, opt.Get("Array/Integer")->GetListInt().front());
		EXPECT_EQ(1.1, opt.Get("Array/Double")->GetListDouble().front());
		EXPECT_STREQ("hello", opt.Get("Array/String")->GetListString().front().c_str());
		EXPECT_STREQ("rgb(255, 255, 255)", opt.Get("Array/Color")->GetListColor().front().GetRgbFormatted().c_str());
		EXPECT_EQ(true, opt.Get("Array/Boolean")->GetListBool().front());
	}
}

TEST_F(lagi_option, mixed_valid_and_invalid_in_user_conf_loads_all_valid) {
	const char def[] = "{\"1\" : false, \"2\" : 1, \"3\" : false }";
	agi::Options opt("data/options/all_bool.json", def, agi::Options::FLUSH_SKIP);
	ASSERT_NO_THROW(opt.ConfigUser());
	EXPECT_EQ(true, opt.Get("1")->GetBool());
	EXPECT_EQ(1, opt.Get("2")->GetInt());
	EXPECT_EQ(true, opt.Get("3")->GetBool());
}

TEST_F(lagi_option, empty_array_works) {
	EXPECT_NO_THROW(agi::Options("", "{ \"arr\" : [] }", agi::Options::FLUSH_SKIP));
}

TEST_F(lagi_option, empty_object_works) {
	EXPECT_NO_THROW(agi::Options("", "{ \"obj\" : {} }", agi::Options::FLUSH_SKIP));
}

TEST_F(lagi_option, unknown_array_type) {
	EXPECT_THROW(agi::Options("", "{ \"arr\" : [ { \"float\" : 5.0 } ] }"), agi::Exception);
}

TEST_F(lagi_option, malformed_arrays) {
	EXPECT_THROW(agi::Options("", "{ \"arr\" : [ {} ] }"), agi::Exception);
	EXPECT_THROW(agi::Options("", "{ \"arr\" : [ { \"double\" : 5.0 }, {} ] }"), agi::Exception);
	EXPECT_THROW(agi::Options("", "{ \"arr\" : [ { \"double\" : 5.0, \"int\" : 5 } ] }"), agi::Exception);
}

TEST_F(lagi_option, int_vs_double) {
	agi::Options opt("", "{ \"int\" : 5, \"double\" : 5.0 }", agi::Options::FLUSH_SKIP);
	EXPECT_NO_THROW(opt.Get("int")->GetInt());
	EXPECT_NO_THROW(opt.Get("double")->GetDouble());
}

struct empty_arr_options : public agi::Options {
	empty_arr_options() : agi::Options("", "{ \"arr\" : [] }", agi::Options::FLUSH_SKIP) { }
};

TEST_F(lagi_option, empty_array_decays_to_first_used_type) {
	ASSERT_NO_THROW(empty_arr_options());

	{
		empty_arr_options opt;
		EXPECT_NO_THROW(opt.Get("arr")->GetListBool());

		EXPECT_THROW(opt.Get("arr")->GetListColor(),  agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListDouble(), agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListInt(),    agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListString(), agi::OptionValueErrorInvalidListType);
	}

	{
		empty_arr_options opt;
		EXPECT_NO_THROW(opt.Get("arr")->GetListColor());

		EXPECT_THROW(opt.Get("arr")->GetListBool(),   agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListDouble(), agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListInt(),    agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListString(), agi::OptionValueErrorInvalidListType);
	}

	{
		empty_arr_options opt;
		EXPECT_NO_THROW(opt.Get("arr")->GetListDouble());

		EXPECT_THROW(opt.Get("arr")->GetListBool(),   agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListColor(),  agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListInt(),    agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListString(), agi::OptionValueErrorInvalidListType);
	}

	{
		empty_arr_options opt;
		EXPECT_NO_THROW(opt.Get("arr")->GetListInt());

		EXPECT_THROW(opt.Get("arr")->GetListBool(),   agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListColor(),  agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListDouble(), agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListString(), agi::OptionValueErrorInvalidListType);
	}

	{
		empty_arr_options opt;
		EXPECT_NO_THROW(opt.Get("arr")->GetListString());

		EXPECT_THROW(opt.Get("arr")->GetListBool(),   agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListColor(),  agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListDouble(), agi::OptionValueErrorInvalidListType);
		EXPECT_THROW(opt.Get("arr")->GetListInt(),    agi::OptionValueErrorInvalidListType);
	}
}

#define CHECK_TYPE(str, type) \
	do { \
		agi::Options opt("", "{ \"" str "\" : \"" str "\" }", agi::Options::FLUSH_SKIP); \
		EXPECT_NO_THROW(opt.Get(str)->Get##type()); \
	} while (false)

TEST_F(lagi_option, color_vs_string) {
	CHECK_TYPE("#", String);
	CHECK_TYPE("#a", String);
	CHECK_TYPE("#abc", Color);
	CHECK_TYPE("#aabbcc", Color);
	CHECK_TYPE("#aabb", String);

	CHECK_TYPE("&", String);
	CHECK_TYPE("&H000000&", Color);
	CHECK_TYPE("&H00000000", Color);
}
