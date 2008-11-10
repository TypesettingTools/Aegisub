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


class AthenasubTimeTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(AthenasubTimeTest);
	CPPUNIT_TEST(testBounds);
	CPPUNIT_TEST(testComparison);
	CPPUNIT_TEST(testOperators);
	CPPUNIT_TEST(testSetGet);
	CPPUNIT_TEST(testParse);
	CPPUNIT_TEST(testToString);
	CPPUNIT_TEST(testComponents);
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
		Time a;
		Time b(0);
		Time c(5000);
		Time d(-500);

		CPPUNIT_ASSERT(a == a);
		CPPUNIT_ASSERT(a <= a);
		CPPUNIT_ASSERT(a >= a);
		CPPUNIT_ASSERT(a == b);
		CPPUNIT_ASSERT(a == d);
		CPPUNIT_ASSERT(a != c);
		CPPUNIT_ASSERT(b != c);
		CPPUNIT_ASSERT(a < c);
		CPPUNIT_ASSERT((c < a) == false);
		CPPUNIT_ASSERT(a <= c);
		CPPUNIT_ASSERT(c > b);
		CPPUNIT_ASSERT((b > c) == false);
		CPPUNIT_ASSERT(c >= b);
	}

	void testBounds()
	{
		Time a(-500);
		CPPUNIT_ASSERT(a.GetMS() == 0);
	}

	void testSetGet()
	{
		Time a;
		CPPUNIT_ASSERT(a.GetMS() == 0);
		a.SetMS(5000);
		CPPUNIT_ASSERT(a.GetMS() == 5000);
		a.SetMS(-5000);
		CPPUNIT_ASSERT(a.GetMS() == 0);
	}

	void testOperators()
	{
		Time a(500);
		CPPUNIT_ASSERT(a + 300 == Time(800));
		CPPUNIT_ASSERT(a - 300 == Time(200));
		CPPUNIT_ASSERT(a - 600 == Time(0));
		CPPUNIT_ASSERT(a + 500 - 500 == a);
	}

	void testParse()
	{
		CPPUNIT_ASSERT(Time("0").GetMS() == 0);
		CPPUNIT_ASSERT(Time("5").GetMS() == 5000);
		CPPUNIT_ASSERT(Time("5.0").GetMS() == 5000);
		CPPUNIT_ASSERT(Time("5,0").GetMS() == 5000);
		CPPUNIT_ASSERT(Time("5.00").GetMS() == 5000);
		CPPUNIT_ASSERT(Time("5.000").GetMS() == 5000);
		CPPUNIT_ASSERT(Time("5.1").GetMS() == 5100);
		CPPUNIT_ASSERT(Time("5.12").GetMS() == 5120);
		CPPUNIT_ASSERT(Time("5.123").GetMS() == 5123);
		CPPUNIT_ASSERT(Time("5,123").GetMS() == 5123);
		CPPUNIT_ASSERT(Time("5,1234").GetMS() == 5123);
		CPPUNIT_ASSERT(Time("5,").GetMS() == 5000);
		CPPUNIT_ASSERT(Time("5.").GetMS() == 5000);
		CPPUNIT_ASSERT(Time("05.12").GetMS() == 5120);
		CPPUNIT_ASSERT(Time("0:05.12").GetMS() == 5120);
		CPPUNIT_ASSERT(Time("0:15.12").GetMS() == 15120);
		CPPUNIT_ASSERT(Time("1:15.12").GetMS() == 75120);
		CPPUNIT_ASSERT(Time("11:15.12").GetMS() == 675120);
		CPPUNIT_ASSERT(Time("2:11:15.12").GetMS() == 675120+7200000);
		CPPUNIT_ASSERT(Time("10:11:15.12").GetMS() == 675120+36000000);
		CPPUNIT_ASSERT(Time(" 10 : 11 : 15 . 12 ").GetMS() == 675120+36000000);
		CPPUNIT_ASSERT_THROW(Time("10:1-1:15.12"),Athenasub::Exception);
	}

	void testToString()
	{
		Time a(1,23,45,678);
		Time b(11,23,45,678);
		Time c(111,23,45,678);
		CPPUNIT_ASSERT(a.GetString() == "1:23:45.678");
		CPPUNIT_ASSERT(a.GetString(2,1) == "1:23:45.67");
		CPPUNIT_ASSERT(a.GetString(3,2) == "01:23:45.678");
		CPPUNIT_ASSERT(a.GetString(0,1) == "1:23:45");
		CPPUNIT_ASSERT(a.GetString(0,0) == "59:59");
		CPPUNIT_ASSERT(b.GetString(3,2) == "11:23:45.678");
		CPPUNIT_ASSERT(b.GetString() == "9:59:59.999");
		CPPUNIT_ASSERT(b.GetString(2,1) == "9:59:59.99");
		CPPUNIT_ASSERT(c.GetString(3,2) == "99:59:59.999");
		CPPUNIT_ASSERT(c.GetString(3,3) == "111:23:45.678");
		CPPUNIT_ASSERT_THROW(a.GetString(-1),Athenasub::Exception);
		CPPUNIT_ASSERT_THROW(a.GetString(4),Athenasub::Exception);
		CPPUNIT_ASSERT_THROW(a.GetString(3,-1),Athenasub::Exception);
	}

	void testComponents()
	{
		Time a(1,23,45,678);
		CPPUNIT_ASSERT(a.GetHoursComponent() == 1);
		CPPUNIT_ASSERT(a.GetMinutesComponent() == 23);
		CPPUNIT_ASSERT(a.GetSecondsComponent() == 45);
		CPPUNIT_ASSERT(a.GetMillisecondsComponent() == 678);
	}
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AthenasubTimeTest,AegisubSuites::athenasub());

#endif
