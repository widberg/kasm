#include "assembler.hpp"

#include <iostream>

#include "parser.hpp"

namespace kasm
{
	Program Assembler::assemble(const std::string& source)
	{
		Parser parser(source);
		Program program;

		while (!parser.match(TokenType::EndOfFile))
		{
			if (parser.matchIdentifier())
			{
				std::string identifier = parser.parseIdentifier();

				if (parser.match(TokenType::Colon))
				{

				}
				else
				{

				}
			}
			
			if (parser.matchInstruction())
			{
				OpCode opCode = parser.parseInstruction();

				OperandType instructionFormat = InstructionFormat.at(opCode);

				InstructionData instructionData;

				if (instructionFormat & OperandType::Register0)
				{
					if (parser.matchRegister())
					{
						instructionData.register0 = parser.parseRegister();
					}
					else
					{

					}
				}

				if (instructionFormat & OperandType::Register1)
				{
					if (parser.matchRegister())
					{
						instructionData.register1 = parser.parseRegister();
					}
					else
					{

					}
				}
			}
			else if (parser.matchDirective())
			{
				Directive directive = parser.parseDirective();
			}
			else
			{

			}
		}

		return program;
	}
} // namespace kasm
