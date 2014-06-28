/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

namespace json {

/////////////////////////////////////////////////
// forward declarations (more info further below)
struct Visitor;
struct ConstVisitor;
class UnknownElement;

typedef int64_t Integer;
typedef double Double;
typedef bool Boolean;
typedef std::string String;
typedef std::vector<UnknownElement> Array;
typedef std::map<std::string, UnknownElement> Object;

struct Null;


/////////////////////////////////////////////////////////////////////////
// Exception - base class for all JSON-related runtime errors
class Exception : public std::runtime_error {
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
class UnknownElement {
public:
   UnknownElement();
   UnknownElement(UnknownElement&& unknown);
   UnknownElement(Object object);
   UnknownElement(Array array);
   UnknownElement(double number);
   UnknownElement(int number);
   UnknownElement(int64_t number);
   UnknownElement(bool boolean);
   UnknownElement(const char *string);
   UnknownElement(String string);
   UnknownElement(Null null);

   ~UnknownElement();

   UnknownElement& operator=(UnknownElement&& unknown);

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

   // implements visitor pattern
   void Accept(ConstVisitor& visitor) const;
   void Accept(Visitor& visitor);

   class Imp;

private:
   UnknownElement(UnknownElement const& unknown) = delete;

   template <typename ElementTypeT>
   ElementTypeT const& CastTo() const;

   template <typename ElementTypeT>
   ElementTypeT& CastTo();

   std::unique_ptr<Imp> m_pImp;
};

/////////////////////////////////////////////////////////////////////////////////
// Null - doesn't do much of anything but satisfy the JSON spec. It is the default
//  element type of UnknownElement
struct Null { };

} // end namespace
