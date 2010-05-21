// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file validator.h
/// @brief Input validation.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#endif

#include <libaegisub/colour.h>

namespace agi {


class Validator {
public:
	/// Types supported.
	enum ValidType {
		Type_Any = 0,		///< Any (should be used instead of "String"
                            ///  to accept any value for code clarity.)
		Type_String = 1,	///< String
		Type_Int = 2,		///< Integer
		Type_Bool = 3,		///< Bool
		Type_Colour = 4		///< Colour
	};

	/// @brief Check value type.
	/// @param value Value
	/// @return true/false
	///
	/// If the value type is "int" or "string" it will return true/false based on this alone.
	/// This should validate against full values or single characters to make it suitable for input boxes.
	virtual bool CheckType(std::string &value)=0;

	/// @brief Check value including constraints.
	/// @param value Value
	/// @return true/false
	///
	/// Check value including bounds checking.
	/// CheckType() should always be the first function called.
	virtual bool Check(std::string &value)=0;

	/// @brief Return validation type.
	/// @return Type
	virtual ValidType GetType()=0;
};


#define VALID_BASE(type_name)                                      \
	class Valid##type_name : public Validator {                    \
	public:                                                        \
		ValidType GetType() { return Type_##type_name; }           \
		bool CheckType(std::string &value);                        \
		virtual bool Check(std::string &value);                    \
	};

VALID_BASE(Any)
VALID_BASE(String)
VALID_BASE(Int)
VALID_BASE(Bool)


class ValidColour: public ValidString {
public:
	ValidType GetType() { return Type_Colour; }
	virtual bool Check(std::string &value);
};

} // namespace agi
