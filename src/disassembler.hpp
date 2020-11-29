#pragma once

#include <string>
#include <sstream>
#include <unordered_map>

namespace kasm
{
	class Disassembler
	{
	public:
		Disassembler() {};
		~Disassembler() {};

		void disassemble(const std::string& programPath, const std::string& asmPath, const std::string& symbolTablePath = "");
	private:
		std::unordered_map<std::uint32_t, std::string> symbolTable;
		std::string getLabelFromAddress(std::uint32_t location, bool padded = false, bool dataOnly = false);
	};
}
