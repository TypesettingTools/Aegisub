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
// AEGISUB/ATHENASUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//


#include "version.h"
#include "athenastring.h"
using namespace Athenasub;


////////////////
// Library data
String Athenasub::GetLibraryName()
{
	return "Athenasub";
}
String Athenasub::GetLibraryVersionString()
{
	return "Athenasub v0.x - EXPERIMENTAL VERSION";
}
String Athenasub::GetLibraryURL()
{
	return "http://www.aegisub.net";
}


/////////////////////////
// Host application data
static String* hostName = NULL;
static String* hostURL = NULL;

void Athenasub::SetHostApplicationName(const String name)
{
	if (!hostName) hostName = new String();
	*hostName = name;
}
void Athenasub::SetHostApplicationURL(const String url)
{
	if (!hostURL) hostURL = new String();
	*hostURL = url;
}
String Athenasub::GetHostApplicationName()
{
	if (hostName) return *hostName;
	return "unknown application";
}
String Athenasub::GetHostApplicationURL()
{
	if (hostURL) return *hostURL;
	return "";
}
