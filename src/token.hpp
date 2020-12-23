#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

#include "specification.hpp"

namespace kasm
{
	enum class TokenType
	{
		Comma,
		Colon,
		OpenParenthesis,
		CloseParenthesis,
		Plus,
		Minus,
		Identifier,
		Integer,
		IntegerLiteral,
		StringLiteral,
		Register,
		Instruction,
		Directive,
		EndOfLine,
		EndOfFile,
		Address,
		Error,
	};

	enum class LexerError
	{
		UninitializedToken,
		BadIntegerLiteral,
		BadStringLiteral,
		BadRegister,
		BadDirective,
		BadCharacter,
		BadAddress,
		EmptyOperand,
	};

	struct Location
	{
		std::size_t lineNumber = 0;
		std::size_t columnNumber = 0;
		std::size_t size = 0;
		const char* line = nullptr;
	};

	class Token
	{
	public:
		Token() : tokenType(TokenType::Error), lexerError(LexerError::UninitializedToken) {}
		Token(TokenType tokenType_, Location location_) : tokenType(tokenType_), location(location_) {}
		Token(TokenType tokenType_, std::string string_, Location location_) : tokenType(tokenType_), string(string_), location(location_) {}
		Token(TokenType tokenType_, byte_t byte_, Location location_) : tokenType(tokenType_), byte(byte_), location(location_) {}
		Token(TokenType tokenType_, word_t word_, Location location_) : tokenType(tokenType_), word(word_), location(location_) {}
		Token(TokenType tokenType_, double_word_t doubleWord_, Location location_) : tokenType(tokenType_), doubleWord(doubleWord_), location(location_) {}
		Token(TokenType tokenType_, quad_word_t quadWord_, Location location_) : tokenType(tokenType_), quadWord(quadWord_), location(location_) {}
		Token(TokenType tokenType_, OpCode opCode_, Location location_) : tokenType(tokenType_), opCode(opCode_), location(location_) {}
		Token(TokenType tokenType_, Directive directive_, Location location_) : tokenType(tokenType_), directive(directive_), location(location_) {}
		Token(TokenType tokenType_, Register reg_, Location location_) : tokenType(tokenType_), reg(reg_), location(location_) {}
		Token(TokenType tokenType_, LexerError lexerError_, Location location_) : tokenType(tokenType_), lexerError(lexerError_), location(location_) {}
		Token(TokenType tokenType_, Address address_, Location location_) : tokenType(tokenType_), address(address_), location(location_) {}

		TokenType tokenType;
		std::string string;
		union
		{
			byte_t byte;
			word_t word;
			double_word_t doubleWord;
			quad_word_t quadWord;
			OpCode opCode;
			Directive directive;
			Register reg;
			LexerError lexerError;
			Address address;
		};

		Location location;
	};
} // namespace kasm

#endif // !TOKEN_HPP
