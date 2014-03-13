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

/// @file option_value.h
/// @brief Container for holding an actual option value.
/// @ingroup libaegisub

#include <cstdint>
#include <vector>

#include <libaegisub/color.h>
#include <libaegisub/exception.h>
#include <libaegisub/signal.h>

namespace agi {
DEFINE_BASE_EXCEPTION_NOINNER(OptionValueError, Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionValueErrorNotFound, OptionValueError, "options/not_found")
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionValueErrorInvalidType, OptionValueError, "options/invalid_type")
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionValueErrorInvalidListType, OptionValueError, "options/array/invalid_type")

/// @class OptionValue
/// Holds an actual option.
class OptionValue {
	agi::signal::Signal<OptionValue const&> ValueChanged;
	std::string name;
protected:
	void NotifyChanged() { ValueChanged(*this); }

	OptionValueErrorInvalidType TypeError(std::string type, std::string op = " retrieve ") const {
		return OptionValueErrorInvalidType("Attempt to" + op + type + " with non-" + type + " value " + GetName());
	}
	OptionValueErrorInvalidListType ListTypeError(std::string type, std::string op = " retrieve ") const {
		return OptionValueErrorInvalidListType("Attempt to" + op + type + " with non-" + type + " list " + GetName());
	}

	OptionValue(std::string name) : name(std::move(name)) { }

public:
	virtual ~OptionValue() {};

	/// Option type
	/// No bitsets here.
	enum OptionType {
		Type_String = 0,		///< String
		Type_Int = 1,			///< Integer
		Type_Double = 2,		///< Double
		Type_Color = 3,		///< Color
		Type_Bool = 4,			///< Bool
		Type_List_String = 100,	///< List of Strings
		Type_List_Int = 101,	///< List of Integers
		Type_List_Double = 102,	///< List of Doubles
		Type_List_Color = 103,	///< List of Colors
		Type_List_Bool = 104	///< List of Bools
	};

	std::string GetName() const { return name; }
	virtual OptionType GetType() const = 0;
	virtual bool IsDefault() const = 0;
	virtual void Reset() = 0;

	virtual std::string GetString() const { throw TypeError("string"); }
	virtual int64_t GetInt() const { throw TypeError("int"); }
	virtual double GetDouble() const { throw TypeError("double"); }
	virtual Color GetColor() const { throw TypeError("color"); }
	virtual bool GetBool() const { throw TypeError("bool"); }

	virtual void SetString(const std::string) { throw TypeError("string", " set "); }
	virtual void SetInt(const int64_t) { throw TypeError("int", " set "); }
	virtual void SetDouble(const double) { throw TypeError("double", " set "); }
	virtual void SetColor(const Color) { throw TypeError("color", " set "); }
	virtual void SetBool(const bool) { throw TypeError("bool", " set "); }

	virtual std::vector<std::string> const& GetListString() const { throw ListTypeError("string"); }
	virtual std::vector<int64_t> const& GetListInt() const { throw ListTypeError("int"); }
	virtual std::vector<double> const& GetListDouble() const { throw ListTypeError("double"); }
	virtual std::vector<Color> const& GetListColor() const { throw ListTypeError("color"); }
	virtual std::vector<bool> const& GetListBool() const { throw ListTypeError("string"); }

	virtual void SetListString(const std::vector<std::string>&) { throw ListTypeError("string", " set "); }
	virtual void SetListInt(const std::vector<int64_t>&) { throw ListTypeError("int", " set "); }
	virtual void SetListDouble(const std::vector<double>&) { throw ListTypeError("double", " set "); }
	virtual void SetListColor(const std::vector<Color>&) { throw ListTypeError("color", " set "); }
	virtual void SetListBool(const std::vector<bool>&) { throw ListTypeError("string", " set "); }

	virtual void Set(const OptionValue *new_value)=0;

	DEFINE_SIGNAL_ADDERS(ValueChanged, Subscribe)
};

#define CONFIG_OPTIONVALUE(type_name, type)                                                   \
	class OptionValue##type_name final : public OptionValue {                                 \
		type value;                                                                           \
		type value_default;                                                                   \
	public:                                                                                   \
		OptionValue##type_name(std::string member_name, type member_value)                    \
		: OptionValue(std::move(member_name))                                                 \
		, value(member_value), value_default(member_value) { }                                \
		type Get##type_name() const { return value; }                                         \
		void Set##type_name(const type new_val) { value = new_val; NotifyChanged(); }         \
		OptionType GetType() const { return OptionValue::Type_##type_name; }                  \
		void Reset() { value = value_default; NotifyChanged(); }                              \
		bool IsDefault() const { return value == value_default; }                             \
		void Set(const OptionValue *new_val) { Set##type_name(new_val->Get##type_name()); }   \
	};

CONFIG_OPTIONVALUE(String, std::string)
CONFIG_OPTIONVALUE(Int, int64_t)
CONFIG_OPTIONVALUE(Double, double)
CONFIG_OPTIONVALUE(Color, Color)
CONFIG_OPTIONVALUE(Bool, bool)

#define CONFIG_OPTIONVALUE_LIST(type_name, type)                                              \
	class OptionValueList##type_name final : public OptionValue {                             \
		std::vector<type> array;                                                              \
		std::vector<type> array_default;                                                      \
		std::string name;                                                                     \
	public:                                                                                   \
		OptionValueList##type_name(std::string name, std::vector<type> const& value = std::vector<type>()) \
		: OptionValue(std::move(name))                                                        \
		, array(value), array_default(value) { }                                              \
		std::vector<type> const& GetList##type_name() const { return array; }                 \
		void SetList##type_name(const std::vector<type>& val) { array = val; NotifyChanged(); } \
		OptionType GetType() const { return OptionValue::Type_List_##type_name; }             \
		void Reset() { array = array_default; NotifyChanged(); }                              \
		bool IsDefault() const { return array == array_default; }                             \
		void Set(const OptionValue *nv) { SetList##type_name(nv->GetList##type_name()); }     \
	};

CONFIG_OPTIONVALUE_LIST(String, std::string)
CONFIG_OPTIONVALUE_LIST(Int, int64_t)
CONFIG_OPTIONVALUE_LIST(Double, double)
CONFIG_OPTIONVALUE_LIST(Color, Color)
CONFIG_OPTIONVALUE_LIST(Bool, bool)

} // namespace agi
