/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#pragma once

#include "elements.h"
#include "visitor.h"

namespace json
{

class Writer : private ConstVisitor
{
public:
   template <typename ElementTypeT>
   static void Write(const ElementTypeT& element, std::ostream& ostr);

private:
   Writer(std::ostream& ostr);
   void Write(const Object& object);
   void Write(const Array& array);
   void Write(const String& string);
   void Write(const Number& number);
   void Write(const Boolean& boolean);
   void Write(const Null& null);
   void Write(const UnknownElement& unknown);

   virtual void Visit(const Array& array);
   virtual void Visit(const Object& object);
   virtual void Visit(const Number& number);
   virtual void Visit(const String& string);
   virtual void Visit(const Boolean& boolean);
   virtual void Visit(const Null& null);

   std::ostream& m_ostr;
   int m_nTabDepth;
};


} // End namespace

#include "writer.inl"
