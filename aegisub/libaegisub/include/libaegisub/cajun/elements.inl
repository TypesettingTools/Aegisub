/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#include "visitor.h"
#include <cassert>
#include <algorithm>
#include <map>

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
   Imp_T(const ElementTypeT& element) : m_Element(element) {}
   virtual Imp* Clone() const { return new Imp_T<ElementTypeT>(*this); }

   virtual void Accept(ConstVisitor& visitor) const { visitor.Visit(m_Element); }
   virtual void Accept(Visitor& visitor) { visitor.Visit(m_Element); }

   virtual bool Compare(const Imp& imp) const
   {
      ConstCastVisitor_T<ElementTypeT> castVisitor;
      imp.Accept(castVisitor);
      return castVisitor.m_pElement &&
             m_Element == *castVisitor.m_pElement;
   }

private:
   ElementTypeT m_Element;
};


class UnknownElement::ConstCastVisitor : public ConstVisitor
{
   virtual void Visit(const Array& array) {}
   virtual void Visit(const Object& object) {}
   virtual void Visit(const Number& number) {}
   virtual void Visit(const String& string) {}
   virtual void Visit(const Boolean& boolean) {}
   virtual void Visit(const Null& null) {}
};

template <typename ElementTypeT>
class UnknownElement::ConstCastVisitor_T : public ConstCastVisitor
{
public:
   ConstCastVisitor_T() : m_pElement(0) {}
   virtual void Visit(const ElementTypeT& element) { m_pElement = &element; } // we don't know what this is, but it overrides one of the base's no-op functions
   const ElementTypeT* m_pElement;
};


class UnknownElement::CastVisitor : public Visitor
{
   virtual void Visit(Array& array) {}
   virtual void Visit(Object& object) {}
   virtual void Visit(Number& number) {}
   virtual void Visit(String& string) {}
   virtual void Visit(Boolean& boolean) {}
   virtual void Visit(Null& null) {}
};

template <typename ElementTypeT>
class UnknownElement::CastVisitor_T : public CastVisitor
{
public:
   CastVisitor_T() : m_pElement(0) {}
   virtual void Visit(ElementTypeT& element) { m_pElement = &element; } // we don't know what this is, but it overrides one of the base's no-op functions
   ElementTypeT* m_pElement;
};




inline UnknownElement::UnknownElement() :                               m_pImp( new Imp_T<Null>( Null() ) ) {}
inline UnknownElement::UnknownElement(const UnknownElement& unknown) :  m_pImp( unknown.m_pImp->Clone()) {}
inline UnknownElement::UnknownElement(const Object& object) :           m_pImp( new Imp_T<Object>(object) ) {}
inline UnknownElement::UnknownElement(const Array& array) :             m_pImp( new Imp_T<Array>(array) ) {}
inline UnknownElement::UnknownElement(double number) :                  m_pImp( new Imp_T<Number>(number) ) {}
inline UnknownElement::UnknownElement(int number) :                     m_pImp( new Imp_T<Number>(number) ) {}
inline UnknownElement::UnknownElement(bool boolean) :                   m_pImp( new Imp_T<Boolean>(boolean) ) {}
inline UnknownElement::UnknownElement(const char *string) :             m_pImp( new Imp_T<String>(string) ) {}
inline UnknownElement::UnknownElement(const String& string) :           m_pImp( new Imp_T<String>(string) ) {}
inline UnknownElement::UnknownElement(const Null& null) :               m_pImp( new Imp_T<Null>(null) ) {}

inline UnknownElement::~UnknownElement()   { delete m_pImp; }

inline UnknownElement::operator const Object& () const  { return CastTo<Object>(); }
inline UnknownElement::operator const Array& () const   { return CastTo<Array>(); }
inline UnknownElement::operator const Number& () const  { return CastTo<Number>(); }
inline UnknownElement::operator const Boolean& () const { return CastTo<Boolean>(); }
inline UnknownElement::operator const String& () const  { return CastTo<String>(); }
inline UnknownElement::operator const Null& () const    { return CastTo<Null>(); }

inline UnknownElement::operator Object& ()  { return ConvertTo<Object>(); }
inline UnknownElement::operator Array& ()   { return ConvertTo<Array>(); }
inline UnknownElement::operator Number& ()  { return ConvertTo<Number>(); }
inline UnknownElement::operator Boolean& () { return ConvertTo<Boolean>(); }
inline UnknownElement::operator String& ()  { return ConvertTo<String>(); }
inline UnknownElement::operator Null& ()    { return ConvertTo<Null>(); }

inline UnknownElement& UnknownElement::operator = (const UnknownElement& unknown)
{
   delete m_pImp;
   m_pImp = unknown.m_pImp->Clone();
   return *this;
}

inline UnknownElement& UnknownElement::operator[] (const std::string& key)
{
   // the people want an object. make us one if we aren't already
   return ConvertTo<Object>()[key];
}

inline const UnknownElement& UnknownElement::operator[] (const std::string& key) const
{
   // throws if we aren't an object
   Object const& obj = CastTo<Object>();
   Object::const_iterator it = obj.find(key);
   if (it == obj.end())
      throw Exception("Object member not found: " + key);
   return it->second;
}

inline UnknownElement& UnknownElement::operator[] (size_t index)
{
   // the people want an array. make us one if we aren't already
   return ConvertTo<Array>()[index];
}

inline const UnknownElement& UnknownElement::operator[] (size_t index) const
{
   // throws if we aren't an array
   return CastTo<Array>()[index];
}


template <typename ElementTypeT>
const ElementTypeT& UnknownElement::CastTo() const
{
   ConstCastVisitor_T<ElementTypeT> castVisitor;
   m_pImp->Accept(castVisitor);
   if (castVisitor.m_pElement == 0)
      throw Exception("Bad cast");
   return *castVisitor.m_pElement;
}



template <typename ElementTypeT>
ElementTypeT& UnknownElement::ConvertTo()
{
   CastVisitor_T<ElementTypeT> castVisitor;
   m_pImp->Accept(castVisitor);
   if (castVisitor.m_pElement == 0)
   {
      // we're not the right type. fix it & try again
      *this = ElementTypeT();
      m_pImp->Accept(castVisitor);
   }

   return *castVisitor.m_pElement;
}


inline void UnknownElement::Accept(ConstVisitor& visitor) const { m_pImp->Accept(visitor); }
inline void UnknownElement::Accept(Visitor& visitor)            { m_pImp->Accept(visitor); }


inline bool UnknownElement::operator == (const UnknownElement& element) const
{
   return m_pImp->Compare(*element.m_pImp);
}

} // end namespace
