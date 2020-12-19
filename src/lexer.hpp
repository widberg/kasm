#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <unordered_map>

#include "token.hpp"

namespace kasm
{
	class Lexer
	{
	public:
		Lexer(const std::string& source) : cursor(source.c_str()) { location.line = cursor; }

		Token next();
		Token peek();
	private:
		bool match(char a, char b);
		bool match(char a, char b, char c);
		bool isCommentStart();
		bool isRegisterStart();
		bool isDirectiveStart();
		bool isIntegerLiteralStart();
		bool isStringLiteralStart();
		bool isIdentifierStart();
		bool isIdentifierMiddle();
		bool isEndOfFile();
		bool isEndOfLine();
		bool isSpace();
		bool isSingleCharacterToken();

		bool lexIntegerNumericLiteral(unsigned base, quad_word_t* quadWord);

		void setMarker();
		std::string getMarkedString();

		void consume();
		void consume(unsigned size);

		static const std::unordered_map<std::string, OpCode> instructionNames;
		static const std::unordered_map<std::string, Directive> directiveNames;
		static const std::unordered_map<std::string, Register> registerNames;
		static const std::unordered_map<char, TokenType> singleCharacterTokens;
		static const std::unordered_map<char, char> escapeCharacters;

		bool hasBufferedToken = false;
		Token bufferedToken;
		const char* cursor;
		const char* marker;

		Location location;
	};
} // namespace kasm

#endif // !LEXER_HPP
