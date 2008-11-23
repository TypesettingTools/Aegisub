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
#include "format_manager.h"
using namespace Athenasub;


class AthenasubFormatsTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(AthenasubFormatsTest);
	CPPUNIT_TEST(testIdentifyFormat);
	CPPUNIT_TEST_SUITE_END();

private:
	String getFormatName(String file)
	{
		String fileFolder = "test_files/";
		std::vector<Format> formats = FormatManager::GetCompatibleFormatList(fileFolder+file,"UTF-8");
		if (formats.size() == 0) return "";
		return formats[0]->GetName();
	}

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testIdentifyFormat()
	{
		std::vector<Format> formats;
		CPPUNIT_ASSERT_NO_THROW(FormatManager::InitializeFormats());

		CPPUNIT_ASSERT(getFormatName("format_ssa.ssa") == "Substation Alpha");
		CPPUNIT_ASSERT(getFormatName("format_ass.ass") == "Advanced Substation Alpha");
		CPPUNIT_ASSERT(getFormatName("format_ass2.ass") == "Advanced Substation Alpha 2");
		//CPPUNIT_ASSERT(getFormatName("format_srt.srt") == "SubRip Text");
	}
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AthenasubFormatsTest,AegisubSuites::athenasub());

#endif
