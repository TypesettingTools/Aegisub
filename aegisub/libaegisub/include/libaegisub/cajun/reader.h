/**********************************************

License: BSD
Project Webpage: http://cajun-jsonapi.sourceforge.net/
Author: Terry Caton

***********************************************/

#pragma once

#include "elements.h"

#ifndef LAGI_PRE
#include <iostream>
#include <vector>
#endif

namespace json
{

class Reader {
public:
	// this structure will be reported in one of the exceptions defined below
	struct Location {
		Location() : m_nLine(0), m_nLineOffset(0), m_nDocOffset(0) { }

		unsigned int m_nLine;       // document line, zero-indexed
		unsigned int m_nLineOffset; // character offset from beginning of line, zero indexed
		unsigned int m_nDocOffset;  // character offset from entire document, zero indexed
	};

	// thrown during the first phase of reading. generally catches low-level
	// problems such as errant characters or corrupt/incomplete documents
	class ScanException : public Exception {
	public:
		ScanException(std::string const& sMessage, Reader::Location const& locError)
		: Exception(sMessage)
		, m_locError(locError)
		{ }

		Reader::Location m_locError;
	};

	// thrown during the second phase of reading. generally catches
	// higher-level problems such as missing commas or brackets
	class ParseException : public Exception {
	public:
		ParseException(std::string const& sMessage, Reader::Location const& locTokenBegin, Reader::Location const& locTokenEnd)
		: Exception(sMessage)
		, m_locTokenBegin(locTokenBegin)
		, m_locTokenEnd(locTokenEnd)
		{ }

		Reader::Location m_locTokenBegin;
		Reader::Location m_locTokenEnd;
	};

	static void Read(UnknownElement& elementRoot, std::istream& istr);

private:
	struct Token {
		enum Type {
			TOKEN_OBJECT_BEGIN,  // {
			TOKEN_OBJECT_END,    // }
			TOKEN_ARRAY_BEGIN,   // [
			TOKEN_ARRAY_END,     // ]
			TOKEN_NEXT_ELEMENT,  // ,
			TOKEN_MEMBER_ASSIGN, // :
			TOKEN_STRING,        // "xxx"
			TOKEN_NUMBER,        // [+/-]000.000[e[+/-]000]
			TOKEN_BOOLEAN,       // true -or- false
			TOKEN_NULL           // null
		};

		Type nType;
		std::string sValue;

		// for malformed file debugging
		Reader::Location locBegin;
		Reader::Location locEnd;
	};

	class InputStream;
	class TokenStream;
	typedef std::vector<Token> Tokens;

	// scanning istream into token sequence
	void Scan(Tokens& tokens, InputStream& inputStream);

	void EatWhiteSpace(InputStream& inputStream);
	void MatchString(std::string& sValue, InputStream& inputStream);
	void MatchNumber(std::string& sNumber, InputStream& inputStream);
	void MatchExpectedString(std::string const& sExpected, InputStream& inputStream);

	// parsing token sequence into element structure
	UnknownElement Parse(TokenStream& tokenStream);
	UnknownElement ParseObject(TokenStream& tokenStream);
	UnknownElement ParseArray(TokenStream& tokenStream);
	UnknownElement ParseString(TokenStream& tokenStream);
	UnknownElement ParseNumber(TokenStream& tokenStream);
	UnknownElement ParseBoolean(TokenStream& tokenStream);
	UnknownElement ParseNull(TokenStream& tokenStream);

	std::string const& MatchExpectedToken(Token::Type nExpected, TokenStream& tokenStream);
};

}
