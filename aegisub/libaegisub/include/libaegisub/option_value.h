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

/// @file option_value.h
/// @brief Container for holding an actual option value.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <stdint.h>

#include <fstream>
#include <map>
#include <vector>
#endif

#include <libaegisub/exception.h>
#include <libaegisub/colour.h>

namespace agi {


DEFINE_BASE_EXCEPTION_NOINNER(OptionValueError, Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionValueErrorNotFound, OptionValueError, "options/not_found")
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionValueErrorInvalidType, OptionValueError, "options/invalid_type")
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionValueErrorInvalidListType, OptionValueError, "options/array/invalid_type")


class OptionValue;
class ConfigVisitor;


/// @class OptionValueListener
/// Inherit from this class to get the proper type for the notification callback
/// signature.
class OptionValueListener {
public:
	/// @brief Type of a notification callback function for option value changes
	typedef void (OptionValueListener::*ChangeEvent)(const OptionValue &option);
};


/// @class OptionValue
/// Holds an actual option.
class OptionValue {
	typedef std::map<OptionValueListener*, OptionValueListener::ChangeEvent> ChangeListenerSet;
	ChangeListenerSet listeners;

protected:
	void NotifyChanged();

public:
	OptionValue() {};
	virtual ~OptionValue() {};


	/// Option type
	/// No bitsets here.
	enum OptionType {
		Type_String = 0,		///< String
		Type_Int = 1,			///< Integer
		Type_Double = 2,		///< Double
		Type_Colour = 3,		///< Colour
		Type_Bool = 4,			///< Bool
		Type_List_String = 100,	///< List of Strings
		Type_List_Int = 101,	///< List of Integers
		Type_List_Double = 102,	///< List of Doubles
		Type_List_Colour = 103,	///< List of Colours
		Type_List_Bool = 104	///< List of Bools
	};

	virtual OptionType GetType() const = 0;
	virtual std::string GetName() const = 0;
	virtual bool IsDefault() const = 0;
	virtual void Reset() = 0;

	virtual std::string GetString() const { throw OptionValueErrorInvalidType("Attempt to retrieve string from non-string value"); }
	virtual int64_t GetInt() const { throw OptionValueErrorInvalidType("Attempt to retrieve int from non-int value"); }
	virtual double GetDouble() const { throw OptionValueErrorInvalidType("Attempt to retrieve double from non-double value"); }
	virtual Colour GetColour() const { throw OptionValueErrorInvalidType("Attempt to retrieve colour from non-colour value"); }
	virtual bool GetBool() const { throw OptionValueErrorInvalidType("Attempt to retrieve bool from non-bool value"); }

	virtual void SetString(const std::string val) { throw OptionValueErrorInvalidType("Attempt to set string in a non-string value"); }
	virtual void SetInt(const int64_t val) { throw OptionValueErrorInvalidType("Attempt to set int in a non-int value"); }
	virtual void SetDouble(const double val) { throw OptionValueErrorInvalidType("Attempt to set double in a non-double value"); }
	virtual void SetColour(const Colour val) { throw OptionValueErrorInvalidType("Attempt to set colour in a non-colour value"); }
	virtual void SetBool(const bool val) { throw OptionValueErrorInvalidType("Attempt to set bool in a non-bool value"); }

	virtual std::string GetDefaultString() const { throw OptionValueErrorInvalidType("Attempt to retrieve string from non-string value"); }
	virtual int64_t GetDefaultInt() const { throw OptionValueErrorInvalidType("Attempt to retrieve int from non-int value"); }
	virtual double GetDefaultDouble() const { throw OptionValueErrorInvalidType("Attempt to retrieve double from non-double value"); }
	virtual Colour GetDefaultColour() const { throw OptionValueErrorInvalidType("Attempt to retrieve colour from non-colour value"); }
	virtual bool GetDefaultBool() const { throw OptionValueErrorInvalidType("Attempt to retrieve bool from non-bool value"); }


	virtual void GetListString(std::vector<std::string> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive string list from non-string list"); }
	virtual void GetListInt(std::vector<int64_t> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive int list from non-int list"); }
	virtual void GetListDouble(std::vector<double> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive double list from non-double list"); }
	virtual void GetListColour(std::vector<Colour> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive colour list from non-colour list"); }
	virtual void GetListBool(std::vector<bool> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive string bool from non-bool list"); }

	virtual void SetListString(const std::vector<std::string> val) { throw OptionValueErrorInvalidListType("Attempt to set string list in a non-string list"); }
	virtual void SetListInt(const std::vector<int64_t> val) { throw OptionValueErrorInvalidListType("Attempt to set int list in a non-int list"); }
	virtual void SetListDouble(const std::vector<double> val) { throw OptionValueErrorInvalidListType("Attempt to set double list in a non-double list"); }
	virtual void SetListColour(const std::vector<Colour> val) { throw OptionValueErrorInvalidListType("Attempt to set colour list in a non-colour list"); }
	virtual void SetListBool(const std::vector<bool> val) { throw OptionValueErrorInvalidListType("Attempt to set string in a non-bool list"); }

	virtual void GetDefaultListString(std::vector<std::string> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive string list from non-string list"); }
	virtual void GetDefaultListInt(std::vector<int64_t> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive int list from non-int list"); }
	virtual void GetDefaultListDouble(std::vector<double> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive double list from non-double list"); }
	virtual void GetDefaultListColour(std::vector<Colour> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive colour list from non-colour list"); }
	virtual void GetDefaultListBool(std::vector<bool> &out) const { throw OptionValueErrorInvalidListType("Attempt to retrive string bool from non-bool list"); }


	void Subscribe(OptionValueListener *listener, OptionValueListener::ChangeEvent function);
	void Unsubscribe(OptionValueListener *listener, OptionValueListener::ChangeEvent function);

};

#define CONFIG_OPTIONVALUE(type_name, type)                                                   \
	class OptionValue##type_name : public OptionValue {                                       \
		type value;                                                                           \
		type value_default;                                                                   \
		std::string name;                                                                     \
	public:                                                                                   \
		OptionValue##type_name(std::string member_name, type member_value):                   \
						  value(member_value), name(member_name) {}                           \
		type Get##type_name() const { return value; }                                         \
		void Set##type_name(const type new_val) { value = new_val; NotifyChanged(); }         \
		type GetDefault##type_name() const { return value_default; }                          \
		OptionType GetType() const { return OptionValue::Type_##type_name; }                  \
		std::string GetName() const { return name; }                                          \
		void Reset() { value = value_default; NotifyChanged(); }                              \
		bool IsDefault() const { return (value == value_default) ? 1 : 0; }                   \
	};

CONFIG_OPTIONVALUE(String, std::string)
CONFIG_OPTIONVALUE(Int, int64_t)
CONFIG_OPTIONVALUE(Double, double)
CONFIG_OPTIONVALUE(Colour, Colour)
CONFIG_OPTIONVALUE(Bool, bool)


class OptionValueList: public OptionValue {
	friend class ConfigVisitor;

protected:
	OptionValueList() {};
	virtual ~OptionValueList() {};
	virtual void InsertString(const std::string val) { throw OptionValueErrorInvalidListType("Attempt to insert string in a non-string list"); }
	virtual void InsertInt(const int64_t val) { throw OptionValueErrorInvalidListType("Attempt to insert int in a non-int list"); }
	virtual void InsertDouble(const double val) { throw OptionValueErrorInvalidListType("Attempt to insert double in a non-double list"); }
	virtual void InsertColour(const Colour val) { throw OptionValueErrorInvalidListType("Attempt insert set colour in a from non-colour list"); }
	virtual void InsertBool(const bool val) { throw OptionValueErrorInvalidListType("Attempt to insert bool in a non-bool list"); }
};


#define CONFIG_OPTIONVALUE_LIST(type_name, type)                                              \
	class OptionValueList##type_name : public OptionValueList {                               \
		std::vector<type> array;                                                              \
		std::vector<type> array_default;                                                      \
		std::string name;                                                                     \
		void Insert##type_name(const type val) { array.push_back(val); }                      \
	public:                                                                                   \
	virtual std::string GetString() const { return "";}                                       \
		OptionValueList##type_name(std::string member_name): name(member_name) {}             \
		void GetList##type_name(std::vector<type> &out) const { out = array; }                \
		void SetList##type_name(const std::vector<type> val) { array = val;  NotifyChanged(); } \
		void GetDefaultList##type_name(std::vector<type> &out) const { out = array_default; } \
		OptionType GetType() const { return OptionValue::Type_List_##type_name; }             \
		std::string GetName() const { return name; }                                          \
		void Reset() { array = array_default; NotifyChanged(); }                              \
		bool IsDefault() const { return (array == array_default) ? 1 : 0; }                   \
	};


CONFIG_OPTIONVALUE_LIST(String, std::string)
CONFIG_OPTIONVALUE_LIST(Int, int64_t)
CONFIG_OPTIONVALUE_LIST(Double, double)
CONFIG_OPTIONVALUE_LIST(Colour, Colour)
CONFIG_OPTIONVALUE_LIST(Bool, bool)

} // namespace agi
