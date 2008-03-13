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
// AEGISUB/AEGILIB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//


#include "version.h"
#include "tr1.h"
using namespace Aegilib;


////////////////
// Library data
String Aegilib::GetLibraryName()
{
	return _T("Aegilib");
}
String Aegilib::GetLibraryVersionString()
{
	return _T("Aegilib v0.x - EXPERIMENTAL");
}
String Aegilib::GetLibraryURL()
{
	return _T("http://www.aegisub.net");
}


/////////////////////////
// Host application data
static shared_ptr<String> hostName;
static shared_ptr<String> hostURL;

void Aegilib::SetHostApplicationName(const String name)
{
	if (!hostName) hostName = shared_ptr<String> (new String());
	*hostName = name;
}
void Aegilib::SetHostApplicationURL(const String url)
{
	if (!hostURL) hostName = shared_ptr<String> (new String());
	*hostURL = url;
}
String Aegilib::GetHostApplicationName()
{
	if (hostName) return *hostName;
	return L"unknown application";
}
String Aegilib::GetHostApplicationURL()
{
	if (hostURL) return *hostURL;
	return L"";
}
