#include "lexer.hpp"

#include <cctype>

#include "debug.hpp"

namespace kasm
{
	const std::unordered_map<std::string, OpCode> Lexer::instructionNames = {
		{ "add", OpCode::Add },
	};

	const std::unordered_map<std::string, Directive> Lexer::directiveNames = {
		{ "segment", Directive::Segment },
	};

	const std::unordered_map<std::string, Register> Lexer::registerNames = {
		{ "rax", Register::RAX },
	};

	const std::unordered_map<char, TokenType> Lexer::singleCharacterTokens = {
		{ ',', TokenType::Comma },
		{ ':', TokenType::Colon },
		{ '+', TokenType::Plus },
		{ '-', TokenType::Minus },
		{ '(', TokenType::OpenParenthesis },
		{ ')', TokenType::CloseParenthesis },
	};

	const std::unordered_map<char, char> Lexer::escapeCharacters
	{
		{ 'a', '\a' },
		{ 'b', '\b' },
		{ 'f', '\f' },
		{ 'n', '\n' },
		{ 'r', '\r' },
		{ 't', '\t' },
		{ 'v', '\v' },
		{ '\\', '\\' },
		{ '\'', '\'' },
		{ '\"', '\"' },
		{ '\?', '\?' },
	};

	Token Lexer::next()
	{
		if (hasBufferedToken)
		{
			hasBufferedToken = false;
			return bufferedToken;
		}

		while (isSpace())
		{
			consume();
		}

		if (isCommentStart())
		{
			do
			{
				consume();
			}
			while (!isEndOfLine() && !isEndOfFile());
		}
		
		if (isEndOfFile())
		{
			consume();
			return Token(TokenType::EndOfFile, location);
		}
		else if (isEndOfLine())
		{
			Location oldLocation = location;

			if (match('r', 'n') || match('n', 'r'))
			{
				consume();
			}
			consume();

			location.lineNumber++;
			location.line = cursor;

			return Token(TokenType::EndOfLine, oldLocation);
		}
		else if (isIdentifierStart())
		{
			setMarker();

			do
			{
				consume();
			} while (isIdentifierMiddle());

			std::string identifierName = getMarkedString();

			if (instructionNames.count(identifierName))
			{
				return Token(TokenType::Instruction, instructionNames.at(identifierName), location);
			}
			else
			{
				return Token(TokenType::Identifier, identifierName, location);
			}
		}
		else if (isDirectiveStart())
		{
			consume();

			setMarker();

			while (isIdentifierMiddle())
			{
				consume();
			}

			std::string directiveName = getMarkedString();

			if (directiveNames.count(directiveName))
			{
				return Token(TokenType::Directive, directiveNames.at(directiveName), location);
			}
			else
			{
				return Token(TokenType::Error, LexerError::BadDirective, location);
			}
		}
		else if (isRegisterStart())
		{
			consume();

			setMarker();

			while (std::isalpha(*cursor))
			{
				consume();
			}

			std::string registerName = getMarkedString();

			if (registerNames.count(registerName))
			{
				return Token(TokenType::Register, registerNames.at(registerName), location);
			}
			else
			{
				return Token(TokenType::Error, LexerError::BadRegister, location);
			}
		}
		else if (isIntegerLiteralStart())
		{
			consume();
			Token token = Token(TokenType::IntegerLiteral, location);

			if (*cursor == '\'')
			{
				consume();
				char value = *cursor;
				consume();
				if (*cursor == '\'')
				{
					consume();
				}
				return Token(TokenType::IntegerLiteral, static_cast<quad_word_t>(value), location);
			}
			else if (match('0', 'x'))
			{
				consume(2);
				if (lexIntegerNumericLiteral(16, &token.quadWord)) return token;
			}
			else if (match('0', 'b'))
			{
				consume(2);
				if (lexIntegerNumericLiteral(2, &token.quadWord)) return token;
			}
			else
			{
				if (lexIntegerNumericLiteral(10, &token.quadWord)) return token;
			}

			return Token(TokenType::Error, LexerError::BadIntegerLiteral, location);
		}
		else if (isStringLiteralStart())
		{
			consume();

			setMarker();

			while (*cursor != '\"')
			{
				consume();
			}

			std::string string = getMarkedString();

			consume();

			return Token(TokenType::StringLiteral, string, location);
		}
		else if (isSingleCharacterToken())
		{
			return Token(singleCharacterTokens.at(*cursor), location);
		}

		return Token();
	}

	Token Lexer::peek()
	{
		if (!hasBufferedToken)
		{
			bufferedToken = next();
			hasBufferedToken = true;
		}

		return bufferedToken;
	}

	bool Lexer::match(char a, char b)
	{
		if (*cursor == '\0' || *(cursor + 1) == '\0')
			return false;

		return *cursor == a
			&& *(cursor + 1) == b;
	}
	
	bool Lexer::match(char a, char b, char c)
	{
		if (*cursor == '\0' || *(cursor + 1) == '\0' || *(cursor + 2) == '\0')
			return false;

		return *cursor == a
			&& *(cursor + 1) == b
			&& *(cursor + 2) == c;
	}

	bool Lexer::isCommentStart()
	{
		return *cursor == ';';
	}

	bool Lexer::isRegisterStart()
	{
		return *cursor == '%';
	}

	bool Lexer::isDirectiveStart()
	{
		return *cursor == '.';
	}

	bool Lexer::isIntegerLiteralStart()
	{
		return *cursor == '$';
	}

	bool Lexer::isStringLiteralStart()
	{
		return *cursor == '\"';
	}

	bool Lexer::isIdentifierStart()
	{
		return std::isalpha(*cursor) ||
			*cursor == '_';
	}

	bool Lexer::isIdentifierMiddle()
	{
		return isIdentifierStart() ||
			std::isdigit(*cursor);
	}

	bool Lexer::isEndOfFile()
	{
		return *cursor == '\0';
	}

	bool Lexer::isEndOfLine()
	{
		return *cursor == '\n' ||
			*cursor == '\r';
	}

	bool Lexer::isSpace()
	{
		return *cursor == ' ' ||
			*cursor == '\t' ||
			*cursor == '\v';
	}

	bool Lexer::isSingleCharacterToken()
	{
		return singleCharacterTokens.count(*cursor);
	}

	bool Lexer::lexIntegerNumericLiteral(unsigned base, quad_word_t *quadWord)
	{
		KASM_ASSERT(base >= 2 && base <= 36, "Invalid base");

		bool isValid = false;
		*quadWord = 0;

		std::uint8_t digit;
		do
		{
			digit = *cursor <= '9' ? *cursor - '0' : std::toupper(*cursor) - 'A';
			consume();
			if (digit >= 0 && digit < base)
			{
				*quadWord *= base;
				*quadWord += digit;
				isValid = true;
			}
		}
		while (digit >= 0 && digit < base);

		return isValid;
	}

	void Lexer::consume()
	{
		cursor++;
		location.columnNumber++;
	}

	void Lexer::consume(unsigned size)
	{
		cursor += size;
		location.columnNumber += size;
	}

	void Lexer::setMarker()
	{
		marker = cursor;
	}

	std::string Lexer::getMarkedString()
	{
		location.size = cursor - marker;
		return std::string(marker, cursor - marker);
	}
} // namespace kasm
