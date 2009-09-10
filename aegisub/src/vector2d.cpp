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

/// @file vector2d.cpp
/// @brief 2D mathematical vector used in visual typesetting
/// @ingroup utility visual_ts
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <math.h>
#endif

#include "vector2d.h"


/// @brief Null constructor 
///
Vector2D::Vector2D () {
	x = y = 0;
}



/// @brief Standard constructor 
/// @param _x 
/// @param _y 
///
Vector2D::Vector2D (float _x,float _y) {
	x = _x;
	y = _y;
}



/// @brief Construction from another vector 
/// @param vec 
///
Vector2D::Vector2D (const Vector2D &vec) {
	x = vec.x;
	y = vec.y;
}



/// @brief Assignment 
/// @param param 
///
void Vector2D::operator = (const Vector2D param) {
	x = param.x;
	y = param.y;
}



/// @brief Comparison 
/// @param param 
/// @return 
///
bool Vector2D::operator == (const Vector2D param) const {
	return ((x == param.x) && (y == param.y));
}


/// @brief DOCME
/// @param param 
/// @return 
///
bool Vector2D::operator != (const Vector2D param) const {
	return ((x != param.x) || (y == param.y));
}



/// @brief Adition 
/// @param param 
/// @return 
///
Vector2D Vector2D::operator + (const Vector2D param) const {
	return Vector2D(x + param.x,y + param.y);
}


/// @brief DOCME
/// @param param 
/// @return 
///
Vector2D Vector2D::operator += (const Vector2D param) {
	x += param.x;
	y += param.y;
	return *this;
}



/// @brief Subtraction 
/// @param param 
/// @return 
///
Vector2D Vector2D::operator - (const Vector2D param) const {
	return Vector2D(x - param.x,y - param.y);
}


/// @brief DOCME
/// @param param 
/// @return 
///
Vector2D Vector2D::operator -= (const Vector2D param) {
	x -= param.x;
	y -= param.y;
	return *this;
}



/// @brief Negate 
/// @return 
///
Vector2D Vector2D::operator - () const {
	return Vector2D(-x,-y);
}



/// @brief Multiplication by scalar 
/// @param param 
/// @return 
///
Vector2D Vector2D::operator * (float param) const {
	return Vector2D(x * param,y * param);
}


/// @brief DOCME
/// @param param 
/// @return 
///
Vector2D Vector2D::operator *= (float param) {
	x *= param;
	y *= param;
	return *this;
}


/// @brief DOCME
/// @param f 
/// @param v 
/// @return 
///
Vector2D operator * (float f,const Vector2D &v) {
	return Vector2D(v.x * f,v.y * f);
}



/// @brief Division by scalar 
/// @param param 
/// @return 
///
Vector2D Vector2D::operator / (float param) const {
	return Vector2D(x / param,y / param);
}


/// @brief DOCME
/// @param param 
/// @return 
///
Vector2D Vector2D::operator /= (float param) {
	x /= param;
	y /= param;
	return *this;
}


/// @brief DOCME
/// @param f 
/// @param v 
/// @return 
///
Vector2D operator / (float f,const Vector2D &v) {
	return Vector2D(v.x / f,v.y / f);
}



/// @brief Cross product 
/// @param param 
/// @return 
///
float Vector2D::Cross (const Vector2D param) const {
	return x * param.y - y * param.x;
}



/// @brief Dot product 
/// @param param 
/// @return 
///
float Vector2D::Dot (const Vector2D param) const {
	return (x * param.x) + (y * param.y);
}



/// @brief Length 
/// @return 
///
float Vector2D::Len () const {
	return sqrt(x*x + y*y);
}



/// @brief Squared Length 
/// @return 
///
float Vector2D::SquareLen () const {
	return x*x + y*y;
}



/// @brief Unitary 
///
Vector2D Vector2D::Unit () const {
	float l = Len();
	if (l != 0) {
		Vector2D temp;
		temp.x = x;
		temp.y = y;
		return temp / l;
	}
	else return Vector2D(0,0);
}


