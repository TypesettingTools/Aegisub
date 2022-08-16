/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton
***********************************************/

#include "libaegisub/cajun/elements.h"

#include "libaegisub/cajun/visitor.h"

#include <utility>

namespace {
using namespace json;

class CastVisitorBase : public Visitor {
	void Visit(Array&) override { }
	void Visit(Object&) override { }
	void Visit(Integer&) override { }
	void Visit(Double&) override { }
	void Visit(String&) override { }
	void Visit(Boolean&) override { }
	void Visit(Null&) override { is_null = true; }
public:
	bool is_null = false;
};

template<typename T>
struct CastVisitor final : public CastVisitorBase {
	T *element = nullptr;
	void Visit(T& ele) override { element = &ele; }
};
}

namespace json {
class UnknownElement::Imp {
public:
	virtual ~Imp() = default;
	virtual void Accept(ConstVisitor& visitor) const = 0;
	virtual void Accept(Visitor& visitor) = 0;
};
}

namespace {
template <typename ElementTypeT>
class Imp_T final : public UnknownElement::Imp {
	ElementTypeT m_Element;

public:
	Imp_T(ElementTypeT element) : m_Element(std::move(element)) { }

	void Accept(ConstVisitor& visitor) const override { visitor.Visit(m_Element); }
	void Accept(Visitor& visitor) override            { visitor.Visit(m_Element); }
};
}

namespace json {
UnknownElement::UnknownElement() noexcept = default;
UnknownElement::UnknownElement(int number)               : m_pImp(new Imp_T<Integer>(number)) {}
UnknownElement::UnknownElement(const char *string)       : m_pImp(new Imp_T<String>(string)) {}
UnknownElement::UnknownElement(std::string_view string)  : m_pImp(new Imp_T<String>(String(string))) {}

UnknownElement::UnknownElement(UnknownElement&& unknown) noexcept = default;
UnknownElement& UnknownElement::operator=(UnknownElement&& unknown) noexcept = default;
UnknownElement::~UnknownElement() = default;

#define DEFINE_UE_TYPE(Type) \
	UnknownElement::UnknownElement(Type val) : m_pImp(new Imp_T<Type>(std::move(val))) { } \
	UnknownElement::operator Type const&() const { return CastTo<Type>(); } \
	UnknownElement::operator Type&() { return CastTo<Type>(); }

DEFINE_UE_TYPE(Object)
DEFINE_UE_TYPE(Array)
DEFINE_UE_TYPE(Integer)
DEFINE_UE_TYPE(Double)
DEFINE_UE_TYPE(Boolean)
DEFINE_UE_TYPE(String)
DEFINE_UE_TYPE(Null)

template <typename ElementTypeT>
ElementTypeT const& UnknownElement::CastTo() const {
	CastVisitor<ElementTypeT> castVisitor;
	const_cast<UnknownElement *>(this)->Accept(castVisitor);
	if (!castVisitor.element)
		throw Exception("Bad cast");
	return *castVisitor.element;
}

template <typename ElementTypeT>
ElementTypeT& UnknownElement::CastTo() {
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

void UnknownElement::Accept(ConstVisitor& visitor) const {
	if (m_pImp)
		m_pImp->Accept(visitor);
	else
		visitor.Visit(Null());
}

void UnknownElement::Accept(Visitor& visitor) {
	if (m_pImp)
		m_pImp->Accept(visitor);
	else {
		Null null;
		visitor.Visit(null);
	}
}
}
