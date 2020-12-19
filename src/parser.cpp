#include "parser.hpp"

#include "debug.hpp"

namespace kasm
{
	bool Parser::match(TokenType tokenType)
	{
		return lexer.peek().tokenType == tokenType;
	}

	Token Parser::parseToken()
	{
		return lexer.next();
	}

	void Parser::consume()
	{
		lexer.next();
	}

	void Parser::consumeUntil(TokenType tokenType)
	{
		while (!match(tokenType))
		{
			consume();
		}
	}

	void Parser::consumeUntilAfter(TokenType tokenType)
	{
		while (parseToken().tokenType != tokenType);
	}

	bool Parser::matchAddress()
	{
		TokenType tokenType = lexer.peek().tokenType;
		return tokenType == TokenType::OpenParenthesis ||
			tokenType == TokenType::Identifier ||
			tokenType == TokenType::IntegerLiteral;
	}

	AddressData Parser::parseAddress()
	{
		KASM_ASSERT(matchAddress(), "Not an address");

		AddressData addressData;

		if (match(TokenType::Integer))
		{
			addressData.offset = parseToken().quadWord;
		}

		if (match(TokenType::OpenParenthesis))
		{
			consume();

			if (match(TokenType::Register))
			{
				addressData.reg = parseToken().reg;

				if (match(TokenType::CloseParenthesis))
				{
					consume();
				}
				else
				{
					consumeUntil(TokenType::Comma);
				}
			}
			else
			{
				consumeUntil(TokenType::Comma);
			}
		}

		return addressData;
	}

	bool Parser::matchInstruction()
	{
		return lexer.peek().tokenType == TokenType::Instruction;
	}

	OpCode Parser::parseInstruction()
	{
		KASM_ASSERT(matchInstruction(), "Not an instruction");

		return lexer.next().opCode;
	}

	bool Parser::matchDirective()
	{
		return lexer.peek().tokenType == TokenType::Directive;
	}

	Directive Parser::parseDirective()
	{
		KASM_ASSERT(matchDirective(), "Not a directive");

		return lexer.next().directive;
	}

	bool Parser::matchRegister()
	{
		return lexer.peek().tokenType == TokenType::Register;
	}

	Register Parser::parseRegister()
	{
		KASM_ASSERT(matchRegister(), "Not a register");

		return lexer.next().reg;
	}

	bool Parser::matchIntegerLiteral()
	{
		return lexer.peek().tokenType == TokenType::IntegerLiteral;
	}

	quad_word_t Parser::parseIntegerLiteral()
	{
		KASM_ASSERT(matchIntegerLiteral(), "Not an integer literal");

		return lexer.next().quadWord;
	}

	bool Parser::matchStringLiteral()
	{
		return lexer.peek().tokenType == TokenType::StringLiteral;
	}

	std::string Parser::parseStringLiteral()
	{
		KASM_ASSERT(matchStringLiteral(), "Not a string literal");

		return lexer.next().string;
	}

	bool Parser::matchIdentifier()
	{
		return lexer.peek().tokenType == TokenType::Identifier;
	}

	std::string Parser::parseIdentifier()
	{
		KASM_ASSERT(matchIdentifier(), "Not an identifier");

		return lexer.next().string;
	}
} // namespace kasm
