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
	private:
		std::uint32_t resolveAddress(std::uint32_t instructionLocation, Address address);

		static const std::uint32_t MUST_RESOLVE = std::numeric_limits<std::uint32_t>::max();

		struct UnresolvedAddressLocation
		{
			std::uint32_t location;
			Address address;
		};

		BinaryBuilder binary;
		std::unordered_map<std::string, std::uint32_t> labelLocations;
		std::vector<UnresolvedAddressLocation> unresolvedAddressLocations;

		friend class yy::parser;
	};
}
