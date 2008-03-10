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

#include "tokenizer.h"
#include "exception.h"
#include <wx/tokenzr.h>
using namespace Aegilib;

///////////////
// Constructor
Tokenizer::Tokenizer(String string,String token)
{
	tkn = new wxStringTokenizer(string,token,wxTOKEN_RET_EMPTY_ALL);
}
Tokenizer::~Tokenizer()
{
	delete tkn;
}


////////////
// Has more
bool Tokenizer::HasMore()
{
	return tkn->HasMoreTokens();
}


////////////////
// Get position
int Tokenizer::GetPosition()
{
	return (int)tkn->GetPosition();
}


/////////////
// Get token
String Tokenizer::GetString()
{
	return tkn->GetNextToken();
}
int Tokenizer::GetInt()
{
	long value;
	wxString temp = tkn->GetNextToken();
	temp.ToLong(&value);
	return (int) value;
}
long Tokenizer::GetLong()
{
	long value;
	wxString temp = tkn->GetNextToken();
	temp.ToLong(&value);
	return value;
}
float Tokenizer::GetFloat()
{
	double value;
	wxString temp = tkn->GetNextToken();
	temp.ToDouble(&value);
	return (float) value;
}
double Tokenizer::GetDouble()
{
	double value;
	wxString temp = tkn->GetNextToken();
	temp.ToDouble(&value);
	return value;
}
bool Tokenizer::GetBool()
{
	long value;
	wxString temp = tkn->GetNextToken();
	temp.ToLong(&value);
	return value != 0;
}
