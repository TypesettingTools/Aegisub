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

namespace {
	using namespace json;

	class CastVisitorBase : public Visitor, public ConstVisitor {
		void Visit(Array&) { }
		void Visit(Object&) { }
		void Visit(Integer&) { }
		void Visit(Double&) { }
		void Visit(String&) { }
		void Visit(Boolean&) { }
		void Visit(Null&) { is_null = true; }
		void Visit(Array const&) { }
		void Visit(Object const&) { }
		void Visit(Integer const&) { }
		void Visit(Double const&) { }
		void Visit(String const&) { }
		void Visit(Boolean const&) { }
		void Visit(Null const&) { is_null = true; }
	public:
		bool is_null;
		CastVisitorBase() : is_null(false) { }
	};

	template<class T>
	struct CastVisitor : public CastVisitorBase {
		T *element;
		CastVisitor() : element(0) { }
		void Visit(T& ele) { element = &ele; }
	};
}

namespace json
{

/////////////////////////
// UnknownElement members
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
   Imp_T(const ElementTypeT& element) : m_Element(element) { }
   Imp* Clone() const { return new Imp_T<ElementTypeT>(*this); }

   void Accept(ConstVisitor& visitor) const { visitor.Visit(m_Element); }
   void Accept(Visitor& visitor)            { visitor.Visit(m_Element); }

   bool Compare(const Imp& imp) const
   {
      CastVisitor<const ElementTypeT> castVisitor;
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
UnknownElement::UnknownElement(double number) :                  m_pImp( new Imp_T<Double>(number) ) {}
UnknownElement::UnknownElement(int number) :                     m_pImp( new Imp_T<Integer>(number) ) {}
UnknownElement::UnknownElement(int64_t number) :                 m_pImp( new Imp_T<Integer>(number) ) {}
UnknownElement::UnknownElement(long number) :                    m_pImp( new Imp_T<Integer>(number) ) {}
UnknownElement::UnknownElement(bool boolean) :                   m_pImp( new Imp_T<Boolean>(boolean) ) {}
UnknownElement::UnknownElement(const char *string) :             m_pImp( new Imp_T<String>(string) ) {}
UnknownElement::UnknownElement(const String& string) :           m_pImp( new Imp_T<String>(string) ) {}
UnknownElement::UnknownElement(const Null& null) :               m_pImp( new Imp_T<Null>(null) ) {}

UnknownElement::~UnknownElement()   { delete m_pImp; }

UnknownElement::operator Object const&()  const { return CastTo<Object>(); }
UnknownElement::operator Array const&()   const { return CastTo<Array>(); }
UnknownElement::operator Integer const&() const { return CastTo<Integer>(); }
UnknownElement::operator Double const&()  const { return CastTo<Double>(); }
UnknownElement::operator Boolean const&() const { return CastTo<Boolean>(); }
UnknownElement::operator String const&()  const { return CastTo<String>(); }
UnknownElement::operator Null const&()    const { return CastTo<Null>(); }

UnknownElement::operator Object&()  { return CastTo<Object>(); }
UnknownElement::operator Array&()   { return CastTo<Array>(); }
UnknownElement::operator Integer&() { return CastTo<Integer>(); }
UnknownElement::operator Double&()  { return CastTo<Double>(); }
UnknownElement::operator Boolean&() { return CastTo<Boolean>(); }
UnknownElement::operator String&()  { return CastTo<String>(); }
UnknownElement::operator Null&()    { return CastTo<Null>(); }

UnknownElement& UnknownElement::operator =(const UnknownElement& unknown)
{
   delete m_pImp;
   m_pImp = unknown.m_pImp->Clone();
   return *this;
}

UnknownElement& UnknownElement::operator[](const std::string& key)
{
   return CastTo<Object>()[key];
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

UnknownElement&       UnknownElement::operator[](size_t index)       { return CastTo<Array>()[index]; }
UnknownElement const& UnknownElement::operator[](size_t index) const { return CastTo<Array>()[index]; }


template <typename ElementTypeT>
ElementTypeT const& UnknownElement::CastTo() const
{
   CastVisitor<const ElementTypeT> castVisitor;
   Accept(castVisitor);
   if (!castVisitor.element)
      throw Exception("Bad cast");
   return *castVisitor.element;
}

template <typename ElementTypeT>
ElementTypeT& UnknownElement::CastTo()
{
   CastVisitor<ElementTypeT> castVisitor;
   Accept(castVisitor);

   // If this element is uninitialized, implicitly convert it to the desired type
   if (castVisitor.is_null) {
      *this = ElementTypeT();
      return *this;
   }

   // Otherwise throw an exception
   if (!castVisitor.element)
      throw Exception("Bad cast");
   return *castVisitor.element;
}

void UnknownElement::Accept(ConstVisitor& visitor) const { m_pImp->Accept(visitor); }
void UnknownElement::Accept(Visitor& visitor)            { m_pImp->Accept(visitor); }


bool UnknownElement::operator == (const UnknownElement& element) const
{
   return m_pImp->Compare(*element.m_pImp);
}

} // end namespace
