// Copyright (c) 2007, Rodrigo Braz Monteiro
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file vector2d.h
/// @see vector2d.cpp
/// @ingroup utility visual_ts
///




/// DOCME
/// @class Vector2D
/// @brief DOCME
///
/// DOCME
class Vector2D {
public:

	/// DOCME

	/// DOCME
	float x,y;

	Vector2D ();
	Vector2D (float _x,float _y);
	Vector2D (const Vector2D &vec);

	void operator = (const Vector2D param);
	bool operator == (const Vector2D param) const;
	bool operator != (const Vector2D param) const;

	Vector2D operator - () const;
	Vector2D operator + (const Vector2D param) const;
	Vector2D operator - (const Vector2D param) const;
	Vector2D operator * (float param) const;
	Vector2D operator / (float param) const;

	Vector2D operator += (const Vector2D param);
	Vector2D operator -= (const Vector2D param);
	Vector2D operator *= (float param);
	Vector2D operator /= (float param);

	Vector2D Unit () const;
	float Cross (const Vector2D param) const;
	float Dot (const Vector2D param) const;

	float Len () const;

	/// @brief DOCME
	/// @return 
	///
	float Length () const { return Len(); }
	float SquareLen () const;

	/// @brief DOCME
	///
	float SquareLength () const { return SquareLen(); }
};


////////////////////
// Global operators
Vector2D operator * (float f,const Vector2D &v);
Vector2D operator / (float f,const Vector2D &v);


