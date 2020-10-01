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
	void setRegister(std::uint32_t reg, std::uint32_t value);
	std::uint32_t getRegister(std::uint32_t reg);

	std::uint32_t registers[32];
	std::uint32_t pc;

	std::vector<std::uint8_t> program;

	enum Syscall : std::uint8_t
	{
		EXIT,
		READ_INT,
		WRITE_INT,
		READ_CHAR,
		WRITE_CHAR,
		READ_STRING,
		WRITE_STRING,
		ALLOCATE,
		DEALLOCATE,
	};
};
