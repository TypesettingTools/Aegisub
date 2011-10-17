/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#pragma once

#include <deque>
#include <list>
#include <string>
#include <stdexcept>

/*

TODO:
* better documentation (doxygen?)
* Unicode support
* parent element accessors

*/

namespace json
{


/////////////////////////////////////////////////
// forward declarations (more info further below)
class Visitor;
class ConstVisitor;

template <typename ValueTypeT>
class TrivialType_T;

typedef TrivialType_T<double> Number;
typedef TrivialType_T<bool> Boolean;
typedef TrivialType_T<std::string> String;

class Object;
class Array;
class Null;



/////////////////////////////////////////////////////////////////////////
// Exception - base class for all JSON-related runtime errors
class Exception : public std::runtime_error
{
public:
	Exception(const std::string& sMessage) : std::runtime_error(sMessage) { }
};

/////////////////////////////////////////////////////////////////////////
// UnknownElement - provides a typesafe surrogate for any of the JSON-
//  sanctioned element types. This class allows the Array and Object
//  class to effectively contain a heterogeneous set of child elements.
// The cast operators provide convenient implicit downcasting, while
//  preserving dynamic type safety by throwing an exception during a
//  a bad cast.
// The object & array element index operators (operators [std::string]
//  and [size_t]) provide convenient, quick access to child elements.
//  They are a logical extension of the cast operators. These child
//  element accesses can be chained together, allowing the following
//  (when document structure is well-known):
//  String str = objInvoices[1]["Customer"]["Company"];


class UnknownElement
{
public:
   UnknownElement();
   UnknownElement(const UnknownElement& unknown);
   UnknownElement(const Object& object);
   UnknownElement(const Array& array);
   UnknownElement(const Number& number);
   UnknownElement(const Boolean& boolean);
   UnknownElement(const String& string);
   UnknownElement(const Null& null);

   ~UnknownElement();

   UnknownElement& operator = (const UnknownElement& unknown);

   // implicit cast to actual element type. throws on failure
   operator const Object& () const;
   operator const Array& () const;
   operator const Number& () const;
   operator const Boolean& () const;
   operator const String& () const;
   operator const Null& () const;

   // implicit cast to actual element type. *converts* on failure, and always returns success
   operator Object& ();
   operator Array& ();
   operator Number& ();
   operator Boolean& ();
   operator String& ();
   operator Null& ();

   // provides quick access to children when real element type is object
   UnknownElement& operator[] (const std::string& key);
   const UnknownElement& operator[] (const std::string& key) const;

   // provides quick access to children when real element type is array
   UnknownElement& operator[] (size_t index);
   const UnknownElement& operator[] (size_t index) const;

   // implements visitor pattern
   void Accept(ConstVisitor& visitor) const;
   void Accept(Visitor& visitor);

   // tests equality. first checks type, then value if possible
   bool operator == (const UnknownElement& element) const;

private:
   class Imp;

   template <typename ElementTypeT>
   class Imp_T;

   class CastVisitor;
   class ConstCastVisitor;

   template <typename ElementTypeT>
   class CastVisitor_T;

   template <typename ElementTypeT>
   class ConstCastVisitor_T;

   template <typename ElementTypeT>
   const ElementTypeT& CastTo() const;

   template <typename ElementTypeT>
   ElementTypeT& ConvertTo();

   Imp* m_pImp;
};


/////////////////////////////////////////////////////////////////////////////////
// Array - mimics std::deque<UnknownElement>. The array contents are effectively
//  heterogeneous thanks to the ElementUnknown class.
class Array
{
public:
   typedef std::deque<UnknownElement> elements;
   typedef elements::iterator iterator;
   typedef elements::const_iterator const_iterator;

   iterator begin() { return m_Elements.begin(); }
   iterator end() { return m_Elements.end(); }
   const_iterator begin() const { return m_Elements.begin(); }
   const_iterator end() const { return m_Elements.end(); }

   iterator insert(iterator it, const UnknownElement& element) { m_Elements.insert(it, element); }
   void push_back(const UnknownElement& element) { m_Elements.push_back(element); }
   iterator erase(iterator it) { return m_Elements.erase(it); }
   void resize(size_t newsize) { m_Elements.resize(newsize); }
   void clear() { m_Elements.clear(); }

   size_t size() const { return m_Elements.size(); }
   bool empty() const { return m_Elements.empty(); }

   UnknownElement& front() { return m_Elements.front(); }
   const UnknownElement& front() const { return m_Elements.front(); }
   UnknownElement& back() { return m_Elements.back(); }
   const UnknownElement& back() const { return m_Elements.back(); }

   UnknownElement& operator[] (size_t index);
   const UnknownElement& operator[] (size_t index) const;

   bool operator == (const Array& array) const { return m_Elements == array.m_Elements; }

private:
   elements m_Elements;
};


/////////////////////////////////////////////////////////////////////////////////
// Object - mimics std::map<std::string, UnknownElement>. The member value
//  contents are effectively heterogeneous thanks to the UnknownElement class

class Object
{
public:
   typedef std::map<std::string, UnknownElement> Members;
   typedef Members::iterator iterator;
   typedef Members::const_iterator const_iterator;

   bool operator == (const Object& object) const { return m_Members == object.m_Members; }

   iterator begin() { return m_Members.begin(); }
   iterator end() { return m_Members.end(); }
   const_iterator begin() const { return m_Members.begin(); }
   const_iterator end() const { return m_Members.end(); }

   size_t size() const { return m_Members.size(); }
   bool empty() const { return m_Members.empty(); }

   iterator find(const std::string& name) { return m_Members.find(name); }
   const_iterator find(const std::string& name) const { return m_Members.find(name); }

   iterator insert(std::pair<std::string, UnknownElement> const& ele);
   iterator erase(iterator it) { return m_Members.erase(it); }
   void clear() { m_Members.clear(); }

   UnknownElement& operator [](const std::string& name);
   const UnknownElement& operator [](const std::string& name) const;

private:
   Members m_Members;
};


/////////////////////////////////////////////////////////////////////////////////
// TrivialType_T - class template for encapsulates a simple data type, such as
//  a string, number, or boolean. Provides implicit const & noncost cast operators
//  for that type, allowing "DataTypeT type = trivialType;"


template <typename DataTypeT>
class TrivialType_T
{
public:
   TrivialType_T(const DataTypeT& t = DataTypeT());

   operator DataTypeT&();
   operator const DataTypeT&() const;

   DataTypeT& Value();
   const DataTypeT& Value() const;

   bool operator == (const TrivialType_T<DataTypeT>& trivial) const;

private:
   DataTypeT m_tValue;
};



/////////////////////////////////////////////////////////////////////////////////
// Null - doesn't do much of anything but satisfy the JSON spec. It is the default
//  element type of UnknownElement

class Null
{
public:
   bool operator == (const Null& trivial) const;
};


} // end namespace


#include "elements.inl"
