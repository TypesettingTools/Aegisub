// Copyright (c) 2008, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#include "../suites.h"
#if ATHENASUB_TEST == 1

#include <iostream>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "../../../athenasub/include/athenasub/athenasub.h"
using namespace Athenasub;


class AthenasubStringTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(AthenasubStringTest);
	CPPUNIT_TEST(testComparison);
	CPPUNIT_TEST(testStartEnd);
	CPPUNIT_TEST(testConcatenation);
	CPPUNIT_TEST(testConversion);
	CPPUNIT_TEST_SUITE_END();

private:

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testComparison()
	{
		String a;
		String b = "";
		String c = "Hello world!";
		String d = "ABC";
		String e = "abc";

		CPPUNIT_ASSERT(a == "");
		CPPUNIT_ASSERT((a == "lol") == false);
		CPPUNIT_ASSERT(a == a);
		CPPUNIT_ASSERT(a >= a);
		CPPUNIT_ASSERT(a <= a);
		CPPUNIT_ASSERT(a == b);
		CPPUNIT_ASSERT(a != c);
		CPPUNIT_ASSERT(d != e);
		CPPUNIT_ASSERT(d < e);
		CPPUNIT_ASSERT((e < d) == false);
		CPPUNIT_ASSERT(d < c);
		CPPUNIT_ASSERT((c < d) == false);

		CPPUNIT_ASSERT(c.AsciiCompareNoCase("hello world!"));
		CPPUNIT_ASSERT(c.AsciiCompareNoCase("Hello world!"));
		CPPUNIT_ASSERT(c.AsciiCompareNoCase("hello world") == false);
	}

	void testStartEnd()
	{
		String a = "Hello world!";

		CPPUNIT_ASSERT(a.StartsWith("Hello"));
		CPPUNIT_ASSERT(a.StartsWith("hello") == false);
		CPPUNIT_ASSERT(a.StartsWith("hello",false));
		CPPUNIT_ASSERT(a.EndsWith("world!"));
		CPPUNIT_ASSERT(a.EndsWith("World!") == false);
		CPPUNIT_ASSERT(a.EndsWith("world!",false));
	}

	void testConcatenation()
	{
		String a;
		CPPUNIT_ASSERT(a == "");
		a += "Hello";
		CPPUNIT_ASSERT(a == "Hello");
		a += " world!";
		CPPUNIT_ASSERT(a == "Hello world!");
		a += 5;
		CPPUNIT_ASSERT(a == "Hello world!5");
		a = "";
		a += 10.32f;
		CPPUNIT_ASSERT(a == "10.32");
		a = "";
		a += 10.32;
		CPPUNIT_ASSERT(a == "10.32");
		a = "Hello ";
		String b("world!");
		CPPUNIT_ASSERT(a+b == "Hello world!");
		a += b;
		CPPUNIT_ASSERT(a == "Hello world!");
	}

	void testConversion()
	{
		CPPUNIT_ASSERT(String("312").ToInteger() == 312);
		CPPUNIT_ASSERT(String("+312").ToInteger() == 312);
		CPPUNIT_ASSERT(String("-312").ToInteger() == -312);
		CPPUNIT_ASSERT(String("  31 2 ").ToInteger() == 312);
		CPPUNIT_ASSERT_THROW(String("312a").ToInteger(),Athenasub::Exception);
		CPPUNIT_ASSERT_THROW(String("3-12").ToInteger(),Athenasub::Exception);
		CPPUNIT_ASSERT_THROW(String("3+12").ToInteger(),Athenasub::Exception);

		CPPUNIT_ASSERT(String(312) == "312");
		CPPUNIT_ASSERT(String(-312) == "-312");
		CPPUNIT_ASSERT(String(312.0) == "312");
		CPPUNIT_ASSERT(String(312.25) == "312.25");
		CPPUNIT_ASSERT(String(-312.25) == "-312.25");
		CPPUNIT_ASSERT(String(312.0f) == "312");
		CPPUNIT_ASSERT(String(312.25f) == "312.25");
		CPPUNIT_ASSERT(String(-312.25f) == "-312.25");
	}
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AthenasubStringTest,AegisubSuites::athenasub());

#endif
