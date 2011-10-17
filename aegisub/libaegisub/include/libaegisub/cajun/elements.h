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

namespace json
{

/////////////////////////////////////////////////
// forward declarations (more info further below)
class Visitor;
class ConstVisitor;

template <typename ValueTypeT>
class TrivialType_T;

typedef double Number;
typedef bool Boolean;
typedef std::string String;

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
   UnknownElement(double number);
   UnknownElement(int number);
   UnknownElement(bool boolean);
   UnknownElement(const char *string);
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
   UnknownElement& operator[] (const char *key) { return operator[](std::string(key)); }
   const UnknownElement& operator[] (const char *key) const { return operator[](std::string(key)); }
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

class Array : private std::deque<UnknownElement>
{
public:
   using std::deque<UnknownElement>::back;
   using std::deque<UnknownElement>::begin;
   using std::deque<UnknownElement>::clear;
   using std::deque<UnknownElement>::const_iterator;
   using std::deque<UnknownElement>::const_reference;
   using std::deque<UnknownElement>::const_reverse_iterator;
   using std::deque<UnknownElement>::difference_type;
   using std::deque<UnknownElement>::empty;
   using std::deque<UnknownElement>::end;
   using std::deque<UnknownElement>::front;
   using std::deque<UnknownElement>::iterator;
   using std::deque<UnknownElement>::max_size;
   using std::deque<UnknownElement>::pointer;
   using std::deque<UnknownElement>::pop_back;
   using std::deque<UnknownElement>::pop_front;
   using std::deque<UnknownElement>::push_back;
   using std::deque<UnknownElement>::push_front;
   using std::deque<UnknownElement>::rbegin;
   using std::deque<UnknownElement>::reference;
   using std::deque<UnknownElement>::rend;
   using std::deque<UnknownElement>::reverse_iterator;
   using std::deque<UnknownElement>::size;
   using std::deque<UnknownElement>::size_type;
   using std::deque<UnknownElement>::swap;
   using std::deque<UnknownElement>::value_type;

   UnknownElement& operator[](size_t idx);
   const UnknownElement& operator[](size_t idx) const;
   bool operator==(Array const& rgt) const;
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
// Null - doesn't do much of anything but satisfy the JSON spec. It is the default
//  element type of UnknownElement
class Null
{
public:
   bool operator == (const Null& trivial) const { return true; }
};

} // end namespace


#include "elements.inl"
