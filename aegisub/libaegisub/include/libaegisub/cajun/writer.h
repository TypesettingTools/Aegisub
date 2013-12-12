/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#pragma once

#include "elements.h"
#include "visitor.h"

#include <ostream>

namespace json {

class Writer : private ConstVisitor {
	Writer(std::ostream& ostr);
	void Write(const Object& object);
	void Write(const Array& array);
	void Write(const String& string);
	void Write(const Integer& number);
	void Write(const Double& number);
	void Write(const Boolean& boolean);
	void Write(const Null& null);
	void Write(const UnknownElement& unknown);

	void Visit(const Array& array) override;
	void Visit(const Object& object) override;
	void Visit(const Integer& number) override;
	void Visit(const Double& number) override;
	void Visit(const String& string) override;
	void Visit(const Boolean& boolean) override;
	void Visit(const Null& null) override;

	std::ostream& m_ostr;
	int tab_depth = 0;

public:
	template <typename ElementTypeT>
	static void Write(const ElementTypeT& element, std::ostream& ostr) {
		Writer writer(ostr);
		writer.Write(element);
		ostr.flush(); // all done
	}
};

inline std::ostream& operator <<(std::ostream& ostr, UnknownElement const& elementRoot) {
    Writer::Write(elementRoot, ostr);
    return ostr;
}

} // End namespace
