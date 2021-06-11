#include "assembler.hpp"

#include <memory>
#include <iostream>

#include "debug.hpp"
#include "parser.hpp"

namespace kasm
{
	Program Assembler::assemble(const std::string& source)
	{
		Parser parser(source);
		Program program;

		std::unique_ptr<Node> root = std::make_unique<Node>(parser.parse());

#if KASM_DEBUG
		root->dump(std::cout);
#endif

		if (parser.good())
		{
			root->write(program);
		}
		else
		{
			throw 0;
		}

		return program;
	}
} // namespace kasm
