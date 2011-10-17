/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#include "writer.h"
#include <iostream>
#include <iomanip>

/*

TODO:
* better documentation
* unicode character encoding

*/

namespace json
{

inline Writer::Writer(std::ostream& ostr) :
   m_ostr(ostr),
   m_nTabDepth(0)
{}

template <typename ElementTypeT>
void Writer::Write(const ElementTypeT& element, std::ostream& ostr)
{
   Writer writer(ostr);
   writer.Write(element);
   ostr.flush(); // all done
}

inline void Writer::Write(const Array& array)
{
   if (array.empty())
      m_ostr << "[]";
   else
   {
      m_ostr << '[' << std::endl;
      ++m_nTabDepth;

      Array::const_iterator it(array.begin()), itend(array.end());
      while (it != itend) {
         m_ostr << std::string(m_nTabDepth, '\t');

         Write(*it);

         if (++it != itend)
            m_ostr << ',';
         m_ostr << std::endl;
      }

      --m_nTabDepth;
      m_ostr << std::string(m_nTabDepth, '\t') << ']';
   }
}

inline void Writer::Write(const Object& object)
{
   if (object.empty())
      m_ostr << "{}";
   else
   {
      m_ostr << '{' << std::endl;
      ++m_nTabDepth;

      Object::const_iterator it(object.begin()), itend(object.end());
      while (it != itend) {
         m_ostr << std::string(m_nTabDepth, '\t') << '"' << it->first << "\" : ";
         Write(it->second);

         if (++it != itend)
            m_ostr << ',';
         m_ostr << std::endl;
      }

      --m_nTabDepth;
      m_ostr << std::string(m_nTabDepth, '\t') << '}';
   }
}

inline void Writer::Write(const Number& numberElement)
{
   m_ostr << std::setprecision(20) << numberElement;
}

inline void Writer::Write(const Boolean& booleanElement)
{
   m_ostr << (booleanElement ? "true" : "false");
}

inline void Writer::Write(const String& stringElement)
{
   m_ostr << '"';

   const std::string& s = stringElement;
   std::string::const_iterator it(s.begin()), itend(s.end());
   for (; it != itend; ++it)
   {
      switch (*it)
      {
         case '"':         m_ostr << "\\\"";   break;
         case '\\':        m_ostr << "\\\\";   break;
         case '\b':        m_ostr << "\\b";    break;
         case '\f':        m_ostr << "\\f";    break;
         case '\n':        m_ostr << "\\n";    break;
         case '\r':        m_ostr << "\\r";    break;
         case '\t':        m_ostr << "\\t";    break;
         //case '\u':        m_ostr << "";    break;  ??
         default:          m_ostr << *it;       break;
      }
   }

   m_ostr << '"';
}

inline void Writer::Write(const Null& )
{
   m_ostr << "null";
}

inline void Writer::Write(const UnknownElement& unknown)
{
   unknown.Accept(*this);
}

inline void Writer::Visit(const Array& array)       { Write(array); }
inline void Writer::Visit(const Object& object)     { Write(object); }
inline void Writer::Visit(const Number& number)     { Write(number); }
inline void Writer::Visit(const String& string)     { Write(string); }
inline void Writer::Visit(const Boolean& boolean)   { Write(boolean); }
inline void Writer::Visit(const Null& null)         { Write(null); }

} // end namespace
