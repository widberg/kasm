#pragma once

#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "binaryBuilder.hpp"
#include "common.hpp"

// forawrd declare parser class so we can friend it
namespace yy { class parser; };

namespace kasm
{
	class Assembler
	{
	public:
		Assembler() {};
		~Assembler() {};

		void assemble(const std::string& asmPath, const std::string& programPath);
		void saveSymbolTable(const std::string& symbolTablePath);
	private:
		bool resolveAddress(AddressData& address, bool mustResolve = false);
		void defineLabel(const std::string& name, std::uint32_t location);

		static const bool MUST_RESOLVE = true;

		BinaryBuilder binary;
		std::unordered_map<std::string, std::uint32_t> labelLocations;
		std::vector<AddressData> unresolvedAddressLocations;

		friend class yy::parser;
	};
}
