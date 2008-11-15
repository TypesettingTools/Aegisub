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
#include "../utils.h"
#if ATHENASUB_TEST == 1

#include <iostream>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "athenasub/athenasub.h"
#include "formats/format_ass.h"
using namespace Athenasub;


class AthenasubFormatASSTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(AthenasubFormatASSTest);
	CPPUNIT_TEST(testDialogueParse);
	CPPUNIT_TEST_SUITE_END();

private:

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testDialogueParse()
	{
		DialogueASS refDiag;
		String refText = "Dialogue: 3,1:23:45.67,2:34:56.78,style name,actor name,0001,0020,3300,effect field,Text, why halo thar?";
		CPPUNIT_ASSERT_NO_THROW(refDiag = DialogueASS(refText,1));
		CPPUNIT_ASSERT(refDiag.GetLayer() == 3);
		CPPUNIT_ASSERT(refDiag.GetStartTime() == Time(1,23,45,670));
		CPPUNIT_ASSERT(refDiag.GetEndTime() == Time(2,34,56,780));
		CPPUNIT_ASSERT(refDiag.GetStyle() == "style name");
		CPPUNIT_ASSERT(refDiag.GetActor() == "actor name");
		CPPUNIT_ASSERT(refDiag.GetMargin(0) == 1);
		CPPUNIT_ASSERT(refDiag.GetMargin(1) == 20);
		CPPUNIT_ASSERT(refDiag.GetMargin(2) == 3300);
		CPPUNIT_ASSERT(refDiag.GetUserField() == "effect field");
		CPPUNIT_ASSERT(refDiag.GetText() == "Text, why halo thar?");

		DialogueASS diag;
	}
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AthenasubFormatASSTest,AegisubSuites::athenasub());

#endif
