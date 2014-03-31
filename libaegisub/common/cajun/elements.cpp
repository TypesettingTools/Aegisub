/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton
***********************************************/

#include "../../config.h"

#include "libaegisub/cajun/elements.h"

#include "libaegisub/cajun/visitor.h"

#include <algorithm>
#include <cassert>

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
	struct CastVisitor final : public CastVisitorBase {
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
class UnknownElement::Imp_T final : public UnknownElement::Imp
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

UnknownElement::UnknownElement() :                              m_pImp(new Imp_T<Null>(Null())) {}
UnknownElement::UnknownElement(const UnknownElement& unknown) : m_pImp(unknown.m_pImp->Clone()) {}
UnknownElement::UnknownElement(int number) :                    m_pImp(new Imp_T<Integer>(number)) {}
UnknownElement::UnknownElement(const char *string) :            m_pImp(new Imp_T<String>(string)) {}
UnknownElement::~UnknownElement() { }

#define DEFINE_UE_TYPE(Type) \
	UnknownElement::UnknownElement(Type const& val) : m_pImp(new Imp_T<Type>(val)) { } \
	UnknownElement::operator Type const&() const { return CastTo<Type>(); } \
	UnknownElement::operator Type&() { return CastTo<Type>(); }

DEFINE_UE_TYPE(Object)
DEFINE_UE_TYPE(Array)
DEFINE_UE_TYPE(Integer)
DEFINE_UE_TYPE(Double)
DEFINE_UE_TYPE(Boolean)
DEFINE_UE_TYPE(String)
DEFINE_UE_TYPE(Null)

UnknownElement& UnknownElement::operator =(const UnknownElement& unknown)
{
   m_pImp.reset(unknown.m_pImp->Clone());
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
