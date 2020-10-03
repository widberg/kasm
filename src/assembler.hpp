#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "binaryBuilder.hpp"
#include "common.hpp"

namespace yy { class parser; };

namespace kasm
{
	class Assembler
	{
	public:
		Assembler() {};
		~Assembler() {};

		void assemble(const std::string& asmPath, const std::string& programPath);
	private:
		std::uint32_t resolveAddress(std::uint32_t instructionLocation, const std::string& address, AddressType type, std::uint32_t instruction);

		struct UnresolvedAddressLocation
		{
			std::uint32_t location;
			std::string label;
			AddressType type;
			std::uint32_t instruction;
		};

		BinaryBuilder binary;
		std::unordered_map<std::string, std::uint32_t> labelLocations;
		std::vector<UnresolvedAddressLocation> unresolvedAddressLocations;

		friend class yy::parser;
	};
}
