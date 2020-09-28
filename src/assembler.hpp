#pragma once

#include <utility>
#include <set>
#include <string>
#include <unordered_map>

class Assembler
{
public:
	Assembler();
	~Assembler();

	void assemble(const std::string& sourcePath, const std::string& programPath);
private:
    std::uint32_t resolveRegisterName(const std::string& registerName, std::uint32_t registerSlot);
	void setLabelLocation(const std::string& labelName, std::uint16_t labelLocation);
	std::uint16_t getLabelLocation(std::uint32_t instructionLocation, const std::string& labelName);

	std::unordered_map<std::string, std::uint16_t> labelLocations;
	std::set<std::pair<std::uint32_t, std::string>> unresolvedLabelLocations;

	enum Token
	{
		None,
		Register0 = 1 << 1,
		Register1 = 1 << 2,
		Register2 = 1 << 3,
		DirectAddressAbsolute = 1 << 4,
		DirectAddressOffset = 1 << 5,
		IndirectAddressOffset = 1 << 6,
		Immediate = 1 << 7
	};
};
