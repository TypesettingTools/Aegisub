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
   static void Write(const ElementTypeT& element, std::ostream& ostr)
   {
      Writer writer(ostr);
      writer.Write(element);
      ostr.flush(); // all done
   }

private:
   Writer(std::ostream& ostr);
   void Write(const Object& object);
   void Write(const Array& array);
   void Write(const String& string);
   void Write(const Number& number);
   void Write(const Boolean& boolean);
   void Write(const Null& null);
   void Write(const UnknownElement& unknown);

   void Visit(const Array& array);
   void Visit(const Object& object);
   void Visit(const Number& number);
   void Visit(const String& string);
   void Visit(const Boolean& boolean);
   void Visit(const Null& null);

   std::ostream& m_ostr;
   int m_nTabDepth;
};


} // End namespace
