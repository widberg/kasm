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
			if (parser.matchInstruction())
			{

			}
			else if (parser.matchDirective())
			{

			}
			else if (parser.matchIdentifier())
			{

			}
		}

		return program;
	}
} // namespace kasm
