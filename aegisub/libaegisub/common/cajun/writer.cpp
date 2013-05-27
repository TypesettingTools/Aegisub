/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#include "libaegisub/cajun/writer.h"

#include <cmath>
#include <iostream>
#include <iomanip>

/*

TODO:
* better documentation
* unicode character encoding

*/

namespace json
{

Writer::Writer(std::ostream& ostr)
: m_ostr(ostr)
, tab_depth(0)
{
}

void Writer::Write(Array const& array) {
	if (array.empty())
		m_ostr << "[]";
	else {
		m_ostr << '[' << std::endl;
		++tab_depth;

		Array::const_iterator it(array.begin()), itend(array.end());
		while (it != itend) {
			m_ostr << std::string(tab_depth, '\t');

			Write(*it);

			if (++it != itend)
				m_ostr << ',';
			m_ostr << std::endl;
		}

		--tab_depth;
		m_ostr << std::string(tab_depth, '\t') << ']';
	}
}

void Writer::Write(Object const& object) {
	if (object.empty())
		m_ostr << "{}";
	else {
		m_ostr << '{' << std::endl;
		++tab_depth;

		Object::const_iterator it(object.begin()), itend(object.end());
		while (it != itend) {
			m_ostr << std::string(tab_depth, '\t');
			Write(it->first);
			m_ostr << " : ";
			Write(it->second);

			if (++it != itend)
				m_ostr << ',';
			m_ostr << std::endl;
		}

		--tab_depth;
		m_ostr << std::string(tab_depth, '\t') << '}';
	}
}

void Writer::Write(Double const& numberElement) {
	m_ostr << std::setprecision(20) << numberElement;

	double unused;
	if (!std::modf(numberElement, &unused))
		m_ostr << ".0";
}

void Writer::Write(Integer const& numberElement) {
	m_ostr << numberElement;
}

void Writer::Write(Boolean const& booleanElement) {
	m_ostr << (booleanElement ? "true" : "false");
}

void Writer::Write(String const& stringElement) {
	m_ostr << '"';

	std::string::const_iterator it(stringElement.begin()), itend(stringElement.end());
	for (; it != itend; ++it) {
		switch (*it) {
			case '"':  m_ostr << "\\\""; break;
			case '\\': m_ostr << "\\\\"; break;
			case '\b': m_ostr << "\\b";  break;
			case '\f': m_ostr << "\\f";  break;
			case '\n': m_ostr << "\\n";  break;
			case '\r': m_ostr << "\\r";  break;
			case '\t': m_ostr << "\\t";  break;
			default:   m_ostr << *it;    break;
		}
	}

	m_ostr << '"';
}

void Writer::Write(Null const&) {
	m_ostr << "null";
}

void Writer::Write(UnknownElement const& unknown) {
	unknown.Accept(*this);
}

void Writer::Visit(Array const& array)     { Write(array);   }
void Writer::Visit(Object const& object)   { Write(object);  }
void Writer::Visit(Integer const& integer) { Write(integer); }
void Writer::Visit(Double const& dbl)      { Write(dbl);     }
void Writer::Visit(String const& string)   { Write(string);  }
void Writer::Visit(Boolean const& boolean) { Write(boolean); }
void Writer::Visit(Null const& null)       { Write(null);    }

} // end namespace
