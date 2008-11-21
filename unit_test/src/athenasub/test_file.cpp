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
using namespace Athenasub;


class AthenasubFileTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(AthenasubFileTest);
	CPPUNIT_TEST(testLoad);
	CPPUNIT_TEST(testSave);
	CPPUNIT_TEST(testStableRewrite);
	CPPUNIT_TEST_SUITE_END();

private:
	LibAthenaSub lib;
	String fileFolder;
	Model subs;
	Controller controller;

public:
	AthenasubFileTest()
	{
		fileFolder = "test_files/";
		lib = Athenasub::Create("AthenasubTest");
	}

	void setUp()
	{
		subs = lib->CreateModel();
		controller = subs->CreateController();
	}

	void tearDown()
	{
		subs = Model();
		controller = Controller();
	}

	void testLoad()
	{
		CPPUNIT_ASSERT_NO_THROW(controller->LoadFile(fileFolder+"in_test1.ass","UTF-8"));
		CPPUNIT_ASSERT(subs->GetSectionCount() == 3);
		ConstSection section;
		CPPUNIT_ASSERT_NO_THROW(section = subs->GetSection("Script Info"));
		CPPUNIT_ASSERT(section->HasProperty("ScriptType"));
		CPPUNIT_ASSERT(section->GetProperty("ScriptType") == "v4.00+");
		CPPUNIT_ASSERT_NO_THROW(section = subs->GetSection("V4+ Styles"));
		CPPUNIT_ASSERT(section->GetEntryCount() == 7);
		CPPUNIT_ASSERT_NO_THROW(section = subs->GetSection("Events"));
		CPPUNIT_ASSERT(section->GetEntryCount() == 362);
	}

	void testSave()
	{
		CPPUNIT_ASSERT_NO_THROW(controller->LoadFile(fileFolder+"in_test1.ass","UTF-8"));
		CPPUNIT_ASSERT_NO_THROW(controller->SaveFile(fileFolder+"out_test1.ass","UTF-8"));

		CPPUNIT_ASSERT_NO_THROW(controller->LoadFile(fileFolder+"out_test1.ass","UTF-8"));
		CPPUNIT_ASSERT(subs->GetSectionCount() == 3);
		ConstSection section;
		CPPUNIT_ASSERT_NO_THROW(section = subs->GetSection("V4+ Styles"));
		CPPUNIT_ASSERT(section->GetEntryCount() == 7);
		CPPUNIT_ASSERT_NO_THROW(section = subs->GetSection("Events"));
		CPPUNIT_ASSERT(section->GetEntryCount() == 362);
	}

	void testStableRewrite()
	{
		CPPUNIT_ASSERT_NO_THROW(controller->LoadFile(fileFolder+"out_test1.ass","UTF-8"));
		CPPUNIT_ASSERT_NO_THROW(controller->SaveFile(fileFolder+"out_test2.ass","UTF-8"));
		CPPUNIT_ASSERT(AreFilesIdentical(fileFolder+"out_test1.ass",fileFolder+"out_test2.ass"));
		CPPUNIT_ASSERT(AreFilesIdentical(fileFolder+"in_test1.ass",fileFolder+"out_test1.ass") == false);
	}
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AthenasubFileTest,AegisubSuites::athenasub());

#endif
