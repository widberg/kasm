#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class Assembler
{
public:
	Assembler();
	~Assembler();

	void assemble(const std::string& sourcePath, const std::string& programPath);
private:
    std::uint32_t resolveRegisterName(const std::string& registerName, std::uint32_t registerSlot);
	void setLabelLocation(const std::string& labelName, std::uint32_t labelLocation);
	std::uint32_t resolveAddress(std::uint32_t instructionLocation, const std::string& address, std::uint32_t type, std::uint32_t instruction);
	unsigned int align(unsigned int value, unsigned int alignment);

	struct UnresolvedAddressLocation
	{
		std::uint32_t location;
		std::string label;
		std::uint32_t type;
		std::uint32_t instruction;
	};

	std::unordered_map<std::string, std::uint32_t> labelLocations;
	std::vector<UnresolvedAddressLocation> unresolvedAddressLocations;
};
