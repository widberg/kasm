#pragma once

#include <cstdint>
#include <string>
#include <vector>

class VirtualMachine
{
public:
	VirtualMachine();
	~VirtualMachine();

	int execute();
	void loadProgram(const std::string& programPath);

private:
	void advancePc();
	void systemCall();
	std::uint32_t extractOpcode(std::uint32_t instruction);
	std::uint32_t extractRegister(std::uint32_t instruction, int registerSlot);
	std::uint32_t extractAddress(std::uint32_t instruction);
	std::uint32_t extractImmediate(std::uint32_t instruction);
	void setRegister(std::uint32_t reg, std::uint32_t value);
	std::uint32_t getRegister(std::uint32_t reg);

	std::uint32_t registers[32];
	std::uint32_t pc;

	std::vector<std::uint8_t> program;

	enum Syscall : std::uint8_t
	{
		EXIT,
		READ_INT,
		WRITE_INT
	};
};
