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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "../../../aegilib/include/athenasub/athenasub.h"
#include "../suites.h"
using namespace Athenasub;


class AthenasubTimeTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(AthenasubTimeTest);
	CPPUNIT_TEST(testBounds);
	CPPUNIT_TEST(testComparison);
	CPPUNIT_TEST_SUITE_END();

private:
	Time a;
	Time *b;
	Time *c;
	Time *d;
	Time e;

public:
	void setUp()
	{
		a;
		b = new Time(0);
		c = new Time(5000);
		d = new Time(-500);
		e.SetMS(-1000);
	}

	void tearDown()
	{
		delete b;
		delete c;
	}
	
	void testComparison()
	{
		CPPUNIT_ASSERT(a == a);
		CPPUNIT_ASSERT(a <= a);
		CPPUNIT_ASSERT(a >= a);
		CPPUNIT_ASSERT(a == *b);
		CPPUNIT_ASSERT(a == *d);
		CPPUNIT_ASSERT(a != *c);
		CPPUNIT_ASSERT(*b != *c);
		CPPUNIT_ASSERT(a < *c);
		CPPUNIT_ASSERT(a <= *c);
		CPPUNIT_ASSERT(*c > *b);
		CPPUNIT_ASSERT(*c >= *b);
	}

	void testBounds()
	{
		CPPUNIT_ASSERT(d->GetMS() >= 0);
		CPPUNIT_ASSERT(e.GetMS() >= 0);
	}
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AthenasubTimeTest,AegisubSuites::athenasub());
