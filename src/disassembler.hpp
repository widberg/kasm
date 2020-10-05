#pragma once

#include <string>

namespace kasm
{
	class Disassembler
	{
	public:
		Disassembler() {};
		~Disassembler() {};

		void disassemble(const std::string& programPath, const std::string& asmPath);
	private:

	};
}
