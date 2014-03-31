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
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionValueErrorInvalidType, OptionValueError, "options/invalid_type")

/// Option type
/// No bitsets here.
enum class OptionType {
	String     = 0,	///< String
	Int        = 1,	///< Integer
	Double     = 2,	///< Double
	Color      = 3,	///< Color
	Bool       = 4,	///< Bool
	ListString = 100,	///< List of Strings
	ListInt    = 101,	///< List of Integers
	ListDouble = 102,	///< List of Doubles
	ListColor  = 103,	///< List of Colors
	ListBool   = 104	///< List of Bools
};

/// @class OptionValue
/// Holds an actual option.
class OptionValue {
	agi::signal::Signal<OptionValue const&> ValueChanged;
	std::string name;

	std::string TypeToString(OptionType type) const {
		switch (type) {
			case OptionType::String:     return "String";
			case OptionType::Int:        return "Integer";
			case OptionType::Double:     return "Double";
			case OptionType::Color:      return "Color";
			case OptionType::Bool:       return "Bool";
			case OptionType::ListString: return "List of Strings";
			case OptionType::ListInt:    return "List of Integers";
			case OptionType::ListDouble: return "List of Doubles";
			case OptionType::ListColor:  return "List of Colors";
			case OptionType::ListBool:   return "List of Bools";
		}
		throw agi::InternalError("Invalid option type", nullptr);
	}

	OptionValueErrorInvalidType TypeError(OptionType type) const {
		return OptionValueErrorInvalidType("Invalid type for option " + name + ": expected " + TypeToString(type) + ", got " + TypeToString(GetType()));
	}

	template<typename T>
	T *As(OptionType type) {
		if (GetType() == type)
			return static_cast<T *>(this);
		throw TypeError(type);
	}

	template<typename T>
	const T *As(OptionType type) const {
		if (GetType() == type)
			return static_cast<const T *>(this);
		throw TypeError(type);
	}

protected:
	void NotifyChanged() { ValueChanged(*this); }

	OptionValue(std::string name) : name(std::move(name)) { }

public:
	virtual ~OptionValue() {}

	std::string GetName() const { return name; }
	virtual OptionType GetType() const = 0;
	virtual bool IsDefault() const = 0;
	virtual void Reset() = 0;

	std::string const& GetString() const;
	int64_t const& GetInt() const;
	double const& GetDouble() const;
	Color const& GetColor() const;
	bool const& GetBool() const;

	void SetString(const std::string);
	void SetInt(const int64_t);
	void SetDouble(const double);
	void SetColor(const Color);
	void SetBool(const bool);

	std::vector<std::string> const& GetListString() const;
	std::vector<int64_t> const& GetListInt() const;
	std::vector<double> const& GetListDouble() const;
	std::vector<Color> const& GetListColor() const;
	std::vector<bool> const& GetListBool() const;

	void SetListString(std::vector<std::string>);
	void SetListInt(std::vector<int64_t>);
	void SetListDouble(std::vector<double>);
	void SetListColor(std::vector<Color>);
	void SetListBool(std::vector<bool>);

	virtual void Set(const OptionValue *new_value)=0;

	DEFINE_SIGNAL_ADDERS(ValueChanged, Subscribe)
};

#define CONFIG_OPTIONVALUE(type_name, type)                                           \
	class OptionValue##type_name final : public OptionValue {                         \
		type value;                                                                   \
		type value_default;                                                           \
	public:                                                                           \
		typedef type value_type;                                                      \
		OptionValue##type_name(std::string member_name, type member_value)            \
		: OptionValue(std::move(member_name))                                         \
		, value(member_value), value_default(member_value) { }                        \
		type const& GetValue() const { return value; }                                \
		void SetValue(type new_val) { value = std::move(new_val); NotifyChanged(); }  \
		OptionType GetType() const { return OptionType::type_name; }                  \
		void Reset() { value = value_default; NotifyChanged(); }                      \
		bool IsDefault() const { return value == value_default; }                     \
		void Set(const OptionValue *new_val) { SetValue(new_val->Get##type_name()); } \
	};

CONFIG_OPTIONVALUE(String, std::string)
CONFIG_OPTIONVALUE(Int, int64_t)
CONFIG_OPTIONVALUE(Double, double)
CONFIG_OPTIONVALUE(Color, Color)
CONFIG_OPTIONVALUE(Bool, bool)

#define CONFIG_OPTIONVALUE_LIST(type_name, type)                                          \
	class OptionValueList##type_name final : public OptionValue {                         \
		std::vector<type> array;                                                          \
		std::vector<type> array_default;                                                  \
		std::string name;                                                                 \
	public:                                                                               \
		typedef std::vector<type> value_type;                                             \
		OptionValueList##type_name(std::string name, std::vector<type> const& value = std::vector<type>()) \
		: OptionValue(std::move(name))                                                    \
		, array(value), array_default(value) { }                                          \
		std::vector<type> const& GetValue() const { return array; }                       \
		void SetValue(std::vector<type> val) { array = std::move(val); NotifyChanged(); } \
		OptionType GetType() const { return OptionType::List##type_name; }                \
		void Reset() { array = array_default; NotifyChanged(); }                          \
		bool IsDefault() const { return array == array_default; }                         \
		void Set(const OptionValue *nv) { SetValue(nv->GetList##type_name()); }           \
	};

CONFIG_OPTIONVALUE_LIST(String, std::string)
CONFIG_OPTIONVALUE_LIST(Int, int64_t)
CONFIG_OPTIONVALUE_LIST(Double, double)
CONFIG_OPTIONVALUE_LIST(Color, Color)
CONFIG_OPTIONVALUE_LIST(Bool, bool)

#define CONFIG_OPTIONVALUE_ACCESSORS(ReturnType, Type) \
	inline ReturnType const& OptionValue::Get##Type() const { return As<OptionValue##Type>(OptionType::Type)->GetValue(); } \
	inline void OptionValue::Set##Type(ReturnType v) { As<OptionValue##Type>(OptionType::Type)->SetValue(std::move(v)); }

CONFIG_OPTIONVALUE_ACCESSORS(std::string, String)
CONFIG_OPTIONVALUE_ACCESSORS(int64_t, Int)
CONFIG_OPTIONVALUE_ACCESSORS(double, Double)
CONFIG_OPTIONVALUE_ACCESSORS(Color, Color)
CONFIG_OPTIONVALUE_ACCESSORS(bool, Bool)
CONFIG_OPTIONVALUE_ACCESSORS(std::vector<std::string>, ListString)
CONFIG_OPTIONVALUE_ACCESSORS(std::vector<int64_t>, ListInt)
CONFIG_OPTIONVALUE_ACCESSORS(std::vector<double>, ListDouble)
CONFIG_OPTIONVALUE_ACCESSORS(std::vector<Color>, ListColor)
CONFIG_OPTIONVALUE_ACCESSORS(std::vector<bool>, ListBool)

#undef CONFIG_OPTIONVALUE
#undef CONFIG_OPTIONVALUE_LIST
#undef CONFIG_OPTIONVALUE_ACCESSORS
} // namespace agi
