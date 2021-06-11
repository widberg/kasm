#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "node.hpp"

#include <unordered_map>
#include <vector>

namespace kasm
{
	class Parser
	{
	public:
		Parser(const std::string& source) : lexer(source) {}

		Node* parse();

		bool match(TokenType tokenType);
		Token parseToken();
		void consume();
		void consumeUntil(TokenType tokenType);
		void consumeUntilAfter(TokenType tokenType);

		std::vector<Token> parseStatement();

		bool matchAddress();
		Address parseAddress();

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

		enum InstructionLayout
		{
			NONE = 0,

			ARGUMENT0_REGISTER0 = 1 << 0,
			ARGUMENT0_REGISTER1 = 1 << 1,
			ARGUMENT0_ADDRESS_DIRECT = 1 << 2,
			ARGUMENT0_ADDRESS_INDIRECT = 1 << 3,
			ARGUMENT1_REGISTER0 = 1 << 4,
			ARGUMENT1_REGISTER1 = 1 << 5,
			ARGUMENT1_ADDRESS_DIRECT = 1 << 6,
			ARGUMENT1_ADDRESS_INDIRECT = 1 << 7,
		};

		static const std::unordered_map<OpCode, InstructionLayout> instructionLayouts;
	};
} // namespace kasm

#endif // !PARSER_HPP
