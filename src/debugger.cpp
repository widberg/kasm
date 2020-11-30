#include "debugger.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

namespace kasm
{
	void Debugger::cli()
	{
		reset();

		std::string lastInput = "e";

		while (true)
		{
			std::cout << "> ";

			std::string input;
			std::getline(std::cin, input);

			if (input.empty())
			{
				input = lastInput;
			}

			try
			{
				switch (input[0])
				{
				case 'r':
					std::cout << peakRegister(static_cast<Register>(std::stoi(input.c_str() + 1))) << std::endl;
					break;
				case 'm':
					std::cout << peakMemory(static_cast<std::uint32_t>(std::stoi(input.c_str() + 1, nullptr, 16))) << std::endl;
					break;
				case 'b':
					setBreakpoint(static_cast<std::uint32_t>(std::stoi(input.c_str() + 1, nullptr, 16)));
					break;
				case 'e':
					execute();
					break;
				case 'c':
					run();
					break;
				case 'i':
					step();
					break;
				case 'd':
					std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << pc << std::endl;
					break;
				case 'u':
					{
						std::uint32_t address = static_cast<std::uint32_t>(std::stoi(input.c_str() + 1, nullptr, 16));
						until.insert(address);
						setBreakpoint(address);
						run();
					}
					break;
				case 'q':
					return;
				default:
					break;
				}
			}
			catch (const Signal& signal)
			{
				if (signal == Signal::ILLEGAL_OPCODE)
				{
					if (until.count(pc))
					{
						until.erase(pc);
						removeBreakpoint(pc);
					}
					else
					{
						std::cout << "breakpoint hit at " << "0x" << std::hex << std::setw(8) << std::setfill('0') << pc << std::endl;
					}
				}
			}

			lastInput = input;

			if (shouldExit)
			{
				std::cout << "--- program exited with code " << exitCode << " ---" << std::endl;
			}
		}
	}

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
		if (symbolTable.count(label))
		{
			setBreakpoint(symbolTable[label]);
		}
	}

	void Debugger::setBreakpoint(std::uint32_t address)
	{
		if (!breakpoints.count(address))
		{
			breakpoints.insert({ address, program.getWord(address) });
			program.getWord(address) = 0xFFFFFFFF;
		}
	}

	void Debugger::removeBreakpoint(const std::string& label)
	{
		if (symbolTable.count(label))
		{
			removeBreakpoint(symbolTable[label]);
		}
	}

	void Debugger::removeBreakpoint(std::uint32_t address)
	{
		if (breakpoints.count(address))
		{
			program.getWord(address) = breakpoints.at(address);
			breakpoints.erase(address);
		}
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
