/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#include "libaegisub/cajun/reader.h"

#ifndef LAGI_PRE
#include <cassert>
#include <set>
#include <sstream>
#endif

/*

TODO:
* better documentation
* unicode character decoding

*/

namespace json
{

std::istream& operator >> (std::istream& istr, UnknownElement& elementRoot) {
   Reader::Read(elementRoot, istr);
   return istr;
}

Reader::Location::Location() :
   m_nLine(0),
   m_nLineOffset(0),
   m_nDocOffset(0)
{}


//////////////////////
// Reader::InputStream

// wrapper around istream to keep track of document/line offsets
class Reader::InputStream
{
   std::istream& m_iStr;
   Location m_Location;
public:
   InputStream(std::istream& iStr) : m_iStr(iStr) { }

   int Get() {
      assert(!m_iStr.eof());
      int c = m_iStr.get();

      ++m_Location.m_nDocOffset;
      if (c == '\n') {
         ++m_Location.m_nLine;
         m_Location.m_nLineOffset = 0;
      }
      else {
         ++m_Location.m_nLineOffset;
      }

      return c;
   }
   int Peek() {
      assert(!m_iStr.eof());
      return m_iStr.peek();
   }

   bool EOS() {
      m_iStr.peek(); // apparently eof flag isn't set until a character read is attempted. whatever.
      return m_iStr.eof();
   }

   const Location& GetLocation() const { return m_Location; }
};

//////////////////////
// Reader::TokenStream

class Reader::TokenStream
{
   const Tokens& m_Tokens;
   Tokens::const_iterator m_itCurrent;

public:
   TokenStream(const Tokens& tokens)
   : m_Tokens(tokens),  m_itCurrent(tokens.begin())
   { }

   const Token& Peek() {
      assert(!EOS());
      return *m_itCurrent;
   }
   const Token& Get() {
      assert(!EOS());
      return *m_itCurrent++;
   }

   bool EOS() const {
      return m_itCurrent == m_Tokens.end();
   }
};

///////////////////
// Reader (finally)
void Reader::Read(Object& object, std::istream& istr)                { Read_i(object, istr); }
void Reader::Read(Array& array, std::istream& istr)                  { Read_i(array, istr); }
void Reader::Read(String& string, std::istream& istr)                { Read_i(string, istr); }
void Reader::Read(Number& number, std::istream& istr)                { Read_i(number, istr); }
void Reader::Read(Boolean& boolean, std::istream& istr)              { Read_i(boolean, istr); }
void Reader::Read(Null& null, std::istream& istr)                    { Read_i(null, istr); }
void Reader::Read(UnknownElement& unknown, std::istream& istr)       { Read_i(unknown, istr); }

template <typename ElementTypeT>
void Reader::Read_i(ElementTypeT& element, std::istream& istr)
{
   Reader reader;

   Tokens tokens;
   InputStream inputStream(istr);
   reader.Scan(tokens, inputStream);

   TokenStream tokenStream(tokens);
   reader.Parse(element, tokenStream);

   if (!tokenStream.EOS())
   {
      const Token& token = tokenStream.Peek();
      throw ParseException("Expected End of token stream; found " + token.sValue, token.locBegin, token.locEnd);
   }
}

void Reader::Scan(Tokens& tokens, InputStream& inputStream)
{
   while (EatWhiteSpace(inputStream), !inputStream.EOS())
   {
      // if all goes well, we'll create a token each pass
      Token token;
      token.locBegin = inputStream.GetLocation();

      // gives us null-terminated string
      std::string sChar;
      sChar.push_back(inputStream.Peek());

      switch (sChar[0])
      {
         case '{':
            token.sValue = sChar[0];
            MatchExpectedString(sChar, inputStream);
            token.nType = Token::TOKEN_OBJECT_BEGIN;
            break;

         case '}':
            token.sValue = sChar[0];
            MatchExpectedString(sChar, inputStream);
            token.nType = Token::TOKEN_OBJECT_END;
            break;

         case '[':
            token.sValue = sChar[0];
            MatchExpectedString(sChar, inputStream);
            token.nType = Token::TOKEN_ARRAY_BEGIN;
            break;

         case ']':
            token.sValue = sChar[0];
            MatchExpectedString(sChar, inputStream);
            token.nType = Token::TOKEN_ARRAY_END;
            break;

         case ',':
            token.sValue = sChar[0];
            MatchExpectedString(sChar, inputStream);
            token.nType = Token::TOKEN_NEXT_ELEMENT;
            break;

         case ':':
            token.sValue = sChar[0];
            MatchExpectedString(sChar, inputStream);
            token.nType = Token::TOKEN_MEMBER_ASSIGN;
            break;

         case '"':
            MatchString(token.sValue, inputStream);
            token.nType = Token::TOKEN_STRING;
            break;

         case '-':
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            MatchNumber(token.sValue, inputStream);
            token.nType = Token::TOKEN_NUMBER;
            break;

         case 't':
            token.sValue = "true";
            MatchExpectedString(token.sValue, inputStream);
            token.nType = Token::TOKEN_BOOLEAN;
            break;

         case 'f':
            token.sValue = "false";
            MatchExpectedString(token.sValue, inputStream);
            token.nType = Token::TOKEN_BOOLEAN;
            break;

         case 'n':
            token.sValue = "null";
            MatchExpectedString(token.sValue, inputStream);
            token.nType = Token::TOKEN_NULL;
            break;

         default:
            throw ScanException("Unexpected character in stream: " + sChar, inputStream.GetLocation());
      }

      token.locEnd = inputStream.GetLocation();
      tokens.push_back(token);
   }
}


void Reader::EatWhiteSpace(InputStream& inputStream)
{
   while (!inputStream.EOS() && ::isspace(inputStream.Peek()))
      inputStream.Get();
}

void Reader::MatchExpectedString(const std::string& sExpected, InputStream& inputStream)
{
   std::string::const_iterator it(sExpected.begin()),
                               itEnd(sExpected.end());
   for ( ; it != itEnd; ++it) {
      if (inputStream.EOS() ||      // did we reach the end before finding what we're looking for...
          inputStream.Get() != *it) // ...or did we find something different?
      {
         throw ScanException("Expected string: " + sExpected, inputStream.GetLocation());
      }
   }

   // all's well if we made it here, return quietly
}


void Reader::MatchString(std::string& string, InputStream& inputStream)
{
   MatchExpectedString("\"", inputStream);

   while (inputStream.EOS() == false &&
          inputStream.Peek() != '"')
   {
      char c = inputStream.Get();

      // escape?
      if (c == '\\' &&
          inputStream.EOS() == false) // shouldn't have reached the end yet
      {
         c = inputStream.Get();
         switch (c) {
            case '/':      string.push_back('/');     break;
            case '"':      string.push_back('"');     break;
            case '\\':     string.push_back('\\');    break;
            case 'b':      string.push_back('\b');    break;
            case 'f':      string.push_back('\f');    break;
            case 'n':      string.push_back('\n');    break;
            case 'r':      string.push_back('\r');    break;
            case 't':      string.push_back('\t');    break;
            case 'u':      // TODO: what do we do with this?
            default:
               throw ScanException("Unrecognized escape sequence found in string: \\" + c, inputStream.GetLocation());
         }
      }
      else {
         string.push_back(c);
      }
   }

   // eat the last '"' that we just peeked
   MatchExpectedString("\"", inputStream);
}


void Reader::MatchNumber(std::string& sNumber, InputStream& inputStream)
{
   const char sNumericChars[] = "0123456789.eE-+";
   std::set<char> numericChars;
   numericChars.insert(sNumericChars, sNumericChars + sizeof(sNumericChars));

   while (inputStream.EOS() == false &&
          numericChars.find(inputStream.Peek()) != numericChars.end())
   {
      sNumber.push_back(inputStream.Get());
   }
}


void Reader::Parse(UnknownElement& element, Reader::TokenStream& tokenStream)
{
   if (tokenStream.EOS()) {
      throw ParseException("Unexpected end of token stream", Location(), Location()); // nowhere to point to
   }

   const Token& token = tokenStream.Peek();
   switch (token.nType) {
      case Token::TOKEN_OBJECT_BEGIN:
      {
         // implicit non-const cast will perform conversion for us (if necessary)
         Object& object = element;
         Parse(object, tokenStream);
         break;
      }

      case Token::TOKEN_ARRAY_BEGIN:
      {
         Array& array = element;
         Parse(array, tokenStream);
         break;
      }

      case Token::TOKEN_STRING:
      {
         String& string = element;
         Parse(string, tokenStream);
         break;
      }

      case Token::TOKEN_NUMBER:
      {
         Number& number = element;
         Parse(number, tokenStream);
         break;
      }

      case Token::TOKEN_BOOLEAN:
      {
         Boolean& boolean = element;
         Parse(boolean, tokenStream);
         break;
      }

      case Token::TOKEN_NULL:
      {
         Null& null = element;
         Parse(null, tokenStream);
         break;
      }

      default:
      {
         throw ParseException("Unexpected token: " + token.sValue, token.locBegin, token.locEnd);
      }
   }
}


void Reader::Parse(Object& object, Reader::TokenStream& tokenStream)
{
   MatchExpectedToken(Token::TOKEN_OBJECT_BEGIN, tokenStream);

   bool bContinue = (tokenStream.EOS() == false &&
                     tokenStream.Peek().nType != Token::TOKEN_OBJECT_END);
   while (bContinue)
   {
      // first the member name. save the token in case we have to throw an exception
      const Token& tokenName = tokenStream.Peek();
      std::string const& name = MatchExpectedToken(Token::TOKEN_STRING, tokenStream);

      if (object.count(name))
      {
         throw ParseException("Duplicate object member token: " + name, tokenName.locBegin, tokenName.locEnd);
      }

      // ...then the key/value separator...
      MatchExpectedToken(Token::TOKEN_MEMBER_ASSIGN, tokenStream);

      // ...then the value itself (can be anything).
      UnknownElement value;
      Parse(value, tokenStream);

     object[name] = value;

      bContinue = (tokenStream.EOS() == false &&
                   tokenStream.Peek().nType == Token::TOKEN_NEXT_ELEMENT);
      if (bContinue)
         MatchExpectedToken(Token::TOKEN_NEXT_ELEMENT, tokenStream);
   }

   MatchExpectedToken(Token::TOKEN_OBJECT_END, tokenStream);
}


void Reader::Parse(Array& array, Reader::TokenStream& tokenStream)
{
   MatchExpectedToken(Token::TOKEN_ARRAY_BEGIN, tokenStream);

   bool bContinue = (tokenStream.EOS() == false &&
                     tokenStream.Peek().nType != Token::TOKEN_ARRAY_END);
   while (bContinue)
   {
      // ...what's next? could be anything
      array.push_back(UnknownElement());
      UnknownElement& element = array.back();
      Parse(element, tokenStream);

      bContinue = (tokenStream.EOS() == false &&
                   tokenStream.Peek().nType == Token::TOKEN_NEXT_ELEMENT);
      if (bContinue)
         MatchExpectedToken(Token::TOKEN_NEXT_ELEMENT, tokenStream);
   }

   MatchExpectedToken(Token::TOKEN_ARRAY_END, tokenStream);
}


void Reader::Parse(String& string, Reader::TokenStream& tokenStream)
{
   string = MatchExpectedToken(Token::TOKEN_STRING, tokenStream);
}


void Reader::Parse(Number& number, Reader::TokenStream& tokenStream)
{
   const Token& currentToken = tokenStream.Peek(); // might need this later for throwing exception
   const std::string& sValue = MatchExpectedToken(Token::TOKEN_NUMBER, tokenStream);

   std::istringstream iStr(sValue);
   double dValue;
   iStr >> dValue;

   // did we consume all characters in the token?
   if (iStr.eof() == false)
   {
      throw ParseException("Unexpected character in NUMBER token: " + iStr.peek(), currentToken.locBegin, currentToken.locEnd);
   }

   number = dValue;
}


void Reader::Parse(Boolean& boolean, Reader::TokenStream& tokenStream)
{
   const std::string& sValue = MatchExpectedToken(Token::TOKEN_BOOLEAN, tokenStream);
   boolean = (sValue == "true");
}


void Reader::Parse(Null&, Reader::TokenStream& tokenStream)
{
   MatchExpectedToken(Token::TOKEN_NULL, tokenStream);
}


const std::string& Reader::MatchExpectedToken(Token::Type nExpected, Reader::TokenStream& tokenStream)
{
   if (tokenStream.EOS())
   {
      throw ParseException("Unexpected End of token stream", Location(), Location()); // nowhere to point to
   }

   const Token& token = tokenStream.Get();
   if (token.nType != nExpected)
   {
      throw ParseException("Unexpected token: " + token.sValue, token.locBegin, token.locEnd);
   }

   return token.sValue;
}

} // End namespace
