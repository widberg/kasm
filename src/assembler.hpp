#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>

#include "program.hpp"

namespace kasm
{
	class Assembler
	{
	public:
		static Program assemble(const std::string& source);
	};
} // namespace kasm

#endif // !ASSEMBLER_HPP
