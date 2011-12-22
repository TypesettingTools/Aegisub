/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton
***********************************************/

#include "libaegisub/cajun/elements.h"

#include "libaegisub/cajun/visitor.h"

#ifndef LAGI_PRE
#include <algorithm>
#include <cassert>
#endif

namespace json
{

/////////////////////////
// UnknownElement members
class CastVisitor : public ConstVisitor
{
   virtual void Visit(const Array&) { }
   virtual void Visit(const Object&) { }
   virtual void Visit(const Number&) { }
   virtual void Visit(const String&) { }
   virtual void Visit(const Boolean&) { }
   virtual void Visit(const Null&) { }
};

template <typename ElementTypeT>
class CastVisitor_T : public CastVisitor
{
public:
   const ElementTypeT *element;
   CastVisitor_T() : element(0) { }

   // we don't know what this is, but it overrides one of the base's no-op functions
   void Visit(const ElementTypeT& element) { this->element = &element; }
};

class UnknownElement::Imp
{
public:
   virtual ~Imp() {}
   virtual Imp* Clone() const = 0;

   virtual bool Compare(const Imp& imp) const = 0;

   virtual void Accept(ConstVisitor& visitor) const = 0;
   virtual void Accept(Visitor& visitor) = 0;
};


template <typename ElementTypeT>
class UnknownElement::Imp_T : public UnknownElement::Imp
{
public:
   Imp_T(const ElementTypeT& element) : m_Element(element) {}
   virtual Imp* Clone() const { return new Imp_T<ElementTypeT>(*this); }

   virtual void Accept(ConstVisitor& visitor) const { visitor.Visit(m_Element); }
   virtual void Accept(Visitor& visitor) { visitor.Visit(m_Element); }

   virtual bool Compare(const Imp& imp) const
   {
      CastVisitor_T<ElementTypeT> castVisitor;
      imp.Accept(castVisitor);
      return castVisitor.element && m_Element == *castVisitor.element;
   }

private:
   ElementTypeT m_Element;
};

UnknownElement::UnknownElement() :                               m_pImp( new Imp_T<Null>( Null() ) ) {}
UnknownElement::UnknownElement(const UnknownElement& unknown) :  m_pImp( unknown.m_pImp->Clone()) {}
UnknownElement::UnknownElement(const Object& object) :           m_pImp( new Imp_T<Object>(object) ) {}
UnknownElement::UnknownElement(const Array& array) :             m_pImp( new Imp_T<Array>(array) ) {}
UnknownElement::UnknownElement(double number) :                  m_pImp( new Imp_T<Number>(number) ) {}
UnknownElement::UnknownElement(int number) :                     m_pImp( new Imp_T<Number>(number) ) {}
UnknownElement::UnknownElement(long number) :                    m_pImp( new Imp_T<Number>(number) ) {}
UnknownElement::UnknownElement(bool boolean) :                   m_pImp( new Imp_T<Boolean>(boolean) ) {}
UnknownElement::UnknownElement(const char *string) :             m_pImp( new Imp_T<String>(string) ) {}
UnknownElement::UnknownElement(const String& string) :           m_pImp( new Imp_T<String>(string) ) {}
UnknownElement::UnknownElement(const Null& null) :               m_pImp( new Imp_T<Null>(null) ) {}

UnknownElement::~UnknownElement()   { delete m_pImp; }

UnknownElement::operator const Object& () const  { return CastTo<Object>(); }
UnknownElement::operator const Array& () const   { return CastTo<Array>(); }
UnknownElement::operator const Number& () const  { return CastTo<Number>(); }
UnknownElement::operator const Boolean& () const { return CastTo<Boolean>(); }
UnknownElement::operator const String& () const  { return CastTo<String>(); }
UnknownElement::operator const Null& () const    { return CastTo<Null>(); }

UnknownElement::operator Object& ()  { return ConvertTo<Object>(); }
UnknownElement::operator Array& ()   { return ConvertTo<Array>(); }
UnknownElement::operator Number& ()  { return ConvertTo<Number>(); }
UnknownElement::operator Boolean& () { return ConvertTo<Boolean>(); }
UnknownElement::operator String& ()  { return ConvertTo<String>(); }
UnknownElement::operator Null& ()    { return ConvertTo<Null>(); }

UnknownElement& UnknownElement::operator = (const UnknownElement& unknown)
{
   delete m_pImp;
   m_pImp = unknown.m_pImp->Clone();
   return *this;
}

UnknownElement& UnknownElement::operator[] (const std::string& key)
{
   // the people want an object. make us one if we aren't already
   return ConvertTo<Object>()[key];
}

const UnknownElement& UnknownElement::operator[] (const std::string& key) const
{
   // throws if we aren't an object
   Object const& obj = CastTo<Object>();
   Object::const_iterator it = obj.find(key);
   if (it == obj.end())
      throw Exception("Object member not found: " + key);
   return it->second;
}

UnknownElement& UnknownElement::operator[] (size_t index)
{
   // the people want an array. make us one if we aren't already
   return ConvertTo<Array>()[index];
}

const UnknownElement& UnknownElement::operator[] (size_t index) const
{
   // throws if we aren't an array
   return CastTo<Array>()[index];
}


template <typename ElementTypeT>
const ElementTypeT& UnknownElement::CastTo() const
{
   CastVisitor_T<ElementTypeT> castVisitor;
   m_pImp->Accept(castVisitor);
   if (!castVisitor.element)
      throw Exception("Bad cast");
   return *castVisitor.element;
}

template <typename ElementTypeT>
ElementTypeT& UnknownElement::ConvertTo()
{
   CastVisitor_T<ElementTypeT> castVisitor;
   Accept(castVisitor);
   if (!castVisitor.element)
   {
      // we're not the right type. fix it & try again
      *this = ElementTypeT();
   }

   return *this;
}

void UnknownElement::Accept(ConstVisitor& visitor) const { m_pImp->Accept(visitor); }
void UnknownElement::Accept(Visitor& visitor)            { m_pImp->Accept(visitor); }


bool UnknownElement::operator == (const UnknownElement& element) const
{
   return m_pImp->Compare(*element.m_pImp);
}

} // end namespace
