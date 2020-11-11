#include "debugger.hpp"

namespace kasm
{
	void Debugger::loadProgram(const std::string& programPath, const std::string& symbolTablePath)
	{
		VirtualMachine::loadProgram(programPath);

		std::ifstream symbolTableFile(symbolTablePath, std::ios::binary);
		while (!symbolTableFile.eof())
		{
			std::uint8_t labelSize;
			symbolTableFile.read(reinterpret_cast<char*>(&labelSize), sizeof(labelSize));
			std::string label;
			label.resize(labelSize);
			symbolTableFile.read(reinterpret_cast<char*>(label.data()), labelSize);
			std::uint32_t location;
			symbolTableFile.read(reinterpret_cast<char*>(&location), sizeof(location));
			symbolTable.insert({ label, location });
		}
	}

	void Debugger::setBreakpoint(const std::string& label)
	{

	}

	void Debugger::setBreakpoint(std::uint32_t address)
	{

	}

	void Debugger::removeBreakpoint(const std::string& label)
	{

	}

	void Debugger::removeBreakpoint(std::uint32_t address)
	{

	}

	void Debugger::step()
	{

	}

	std::uint32_t Debugger::peakRegister(Register reg)
	{
		return registers[reg];
	}

	std::uint32_t Debugger::peakMemory(std::uint32_t address)
	{
		return program[address];
	}
}
