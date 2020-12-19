#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"

namespace kasm
{
	class Parser
	{
	public:
		Parser(const std::string& source) : lexer(source) {}

		bool match(TokenType tokenType);
		Token parseToken();
		void consume();
		void consumeUntil(TokenType tokenType);
		void consumeUntilAfter(TokenType tokenType);

		bool matchAddress();
		AddressData parseAddress();

		bool matchInstruction();
		OpCode parseInstruction();

		bool matchDirective();
		Directive parseDirective();

		bool matchRegister();
		Register parseRegister();

		bool matchIntegerLiteral();
		quad_word_t parseIntegerLiteral();

		bool matchStringLiteral();
		std::string parseStringLiteral();

		bool matchIdentifier();
		std::string parseIdentifier();
	private:
		Lexer lexer;
	};
} // namespace kasm

#endif // !PARSER_HPP
