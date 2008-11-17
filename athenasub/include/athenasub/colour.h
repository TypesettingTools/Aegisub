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

#pragma once

namespace Athenasub {

	class String;

	// Colour class
	class Colour {
	private:
		unsigned char r, g, b, a;

	public:
		Colour ();
		Colour (unsigned char red,unsigned char green,unsigned char blue,unsigned char alpha=0);
		Colour (int red,int green,int blue,int alpha=0);

		void SetRed(unsigned char red) { r = red; }
		void SetGreen(unsigned char green) { g = green; }
		void SetBlue(unsigned char blue) { b = blue; }
		void SetAlpha(unsigned char alpha) { a = alpha; }
		void SetRed(int red);
		void SetGreen(int green);
		void SetBlue(int blue);
		void SetAlpha(int alpha);

		int GetRed() const { return r; }
		int GetGreen() const { return g; }
		int GetBlue() const { return b; }
		int GetAlpha() const { return a; }

		void Parse(String str,bool reverse);
		String GetVBHex(bool withAlpha=false,bool withHeader=true,bool withFooter=true) const;
	};

}
