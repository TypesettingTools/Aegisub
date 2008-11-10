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
		CPPUNIT_ASSERT(a <= c);
		CPPUNIT_ASSERT(c > b);
		CPPUNIT_ASSERT(c >= b);
	}

	void testBounds()
	{
		Time a(-500);
		CPPUNIT_ASSERT(a.GetMS() >= 0);
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
	}
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AthenasubTimeTest,AegisubSuites::athenasub());

#endif
