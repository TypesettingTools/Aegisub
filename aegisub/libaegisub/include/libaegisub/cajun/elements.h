/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#pragma once

#ifndef LAGI_PRE
#include <deque>
#include <list>
#include <map>
#include <string>
#include <stdexcept>

#include "stdint.h"
#endif

namespace json
{

/////////////////////////////////////////////////
// forward declarations (more info further below)
struct Visitor;
struct ConstVisitor;
class UnknownElement;

template <typename ValueTypeT>
class TrivialType_T;

typedef int64_t Integer;
typedef double Double;
typedef bool Boolean;
typedef std::string String;
typedef std::deque<UnknownElement> Array;
typedef std::map<std::string, UnknownElement> Object;

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
   UnknownElement(long number);
   UnknownElement(int64_t number);
   UnknownElement(bool boolean);
   UnknownElement(const char *string);
   UnknownElement(const String& string);
   UnknownElement(const Null& null);

   ~UnknownElement();

   UnknownElement& operator = (const UnknownElement& unknown);

   // implicit cast to actual element type. throws on failure
   operator Object const&() const;
   operator Array const&() const;
   operator Integer const&() const;
   operator Double const&() const;
   operator Boolean const&() const;
   operator String const&() const;
   operator Null const&() const;
   operator Object&();
   operator Array&();
   operator Integer&();
   operator Double&();
   operator Boolean&();
   operator String&();
   operator Null&();

   // provides quick access to children when real element type is object
   template<int N>
   UnknownElement& operator[] (const char (&key)[N]) { return operator[](std::string(key)); }
   template<int N>
   const UnknownElement& operator[] (const char (&key)[N]) const { return operator[](std::string(key)); }
   UnknownElement& operator[] (const std::string& key);
   const UnknownElement& operator[] (const std::string& key) const;

   // provides quick access to children when real element type is array
   UnknownElement& operator[] (size_t index);
   const UnknownElement& operator[] (size_t index) const;

   // implements visitor pattern
   void Accept(ConstVisitor& visitor) const;
   void Accept(Visitor& visitor);

   // tests equality. first checks type, then value if possible
   bool operator ==(const UnknownElement& element) const;
   bool operator !=(const UnknownElement& element) const { return !(*this == element); }

private:
   class Imp;

   template <typename ElementTypeT>
   class Imp_T;

   template <typename ElementTypeT>
   ElementTypeT const& CastTo() const;

   template <typename ElementTypeT>
   ElementTypeT& CastTo();

   Imp* m_pImp;
};

/////////////////////////////////////////////////////////////////////////////////
// Null - doesn't do much of anything but satisfy the JSON spec. It is the default
//  element type of UnknownElement
class Null
{
public:
   bool operator == (const Null&) const { return true; }
};

} // end namespace
