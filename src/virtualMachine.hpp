#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "common.hpp"

namespace kasm
{
	class VirtualMachine
	{
	public:
		VirtualMachine() {};
		~VirtualMachine() {};

		int execute();
		void loadProgram(const std::string& programPath);

		enum class Signal
		{
			SEGMENTATION_FAULT,
			ILLEGAL_OPCODE
		};

		void setSignalHandler(Signal signal, void(*handler)(void));
		void executeSignal(Signal signal);

	protected:
		void advancePc();
		void systemCall();
		std::uint32_t resolveAddress(const InstructionData& instructionData, AddressType type);

		std::uint32_t pc, hi, lo;
		bool shouldExit;
		int exitCode;

		class Program : public std::vector<std::uint8_t>
		{
		public:
			Program() { stack = new std::uint8_t[STACK_SIZE]; global = new std::uint8_t[GLOBAL_SIZE];}
			Program(const std::string& programPath) { stack = new std::uint8_t[STACK_SIZE]; global = new std::uint8_t[GLOBAL_SIZE]; open(programPath); }
			~Program() { delete[] stack; delete[] global; };

			void open(const std::string& programPath)
			{
				std::ifstream programFile(programPath, std::ios::binary);
				programFile.read(reinterpret_cast<char*>(&programHeader), sizeof(programHeader));
				resize(programHeader.textSegmentLength + programHeader.dataSegmentLength);
				programFile.read(reinterpret_cast<char*>(data()), size());
			}

			std::uint32_t& getWord(std::uint32_t i)
			{
				if (i < DATA_SEGMENT_OFFSET)
				{
					return *reinterpret_cast<std::uint32_t*>(data() + i);
				}
				else if (i < STACK_OFFSET)
				{
					return *reinterpret_cast<std::uint32_t*>(data() + i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength);
				}
				else if (i < GLOBAL_OFFSET)
				{
					std::uint32_t tmp = i - STACK_OFFSET;
					return *reinterpret_cast<std::uint32_t*>(stack + tmp);
				}
				else
				{
					std::uint32_t tmp = i - GLOBAL_OFFSET;
					return *reinterpret_cast<std::uint32_t*>(global + tmp);
				}
			}

			char* getCharPtr(std::uint32_t i)
			{
				if (i < DATA_SEGMENT_OFFSET)
				{
					return reinterpret_cast<char*>(data() + i);
				}
				else if (i < STACK_OFFSET)
				{
					return reinterpret_cast<char*>(data() + i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength);
				}
				else if (i < GLOBAL_OFFSET)
				{
					return reinterpret_cast<char*>(stack + i - STACK_OFFSET);
				}
				else
				{
					return reinterpret_cast<char*>(global + i - GLOBAL_OFFSET);
				}
			}

			std::uint32_t getTextSegmentLength() const { return programHeader.textSegmentLength; }

			std::uint8_t operator[](std::uint32_t i) const
			{
				if (i < DATA_SEGMENT_OFFSET)
				{
					return data()[i];
				}
				else if (i < STACK_OFFSET)
				{
					return data()[i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength];
				}
				else if (i < GLOBAL_OFFSET)
				{
					return stack[i - STACK_OFFSET];
				}
				else
				{
					return global[i - GLOBAL_OFFSET];
				}
			}

			std::uint8_t& operator[](std::uint32_t i)
			{
				if (i < DATA_SEGMENT_OFFSET)
				{
					return data()[i];
				}
				else if (i < STACK_OFFSET)
				{
					return data()[i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength];
				}
				else if (i < GLOBAL_OFFSET)
				{
					return stack[i - STACK_OFFSET];
				}
				else
				{
					return global[i - GLOBAL_OFFSET];
				}
			}
		private:
			ProgramHeader programHeader;
			std::uint8_t* stack;
			std::uint8_t* global;
		} program;

		class Registers
		{
		public:
			void clear() { std::fill(std::begin(registers), std::end(registers), 0); }
			std::uint32_t operator [](std::size_t i) const { return i ? registers[i] : 0; }
			std::uint32_t& operator [](std::size_t i) { return registers[i]; }
		private:
			std::uint32_t registers[32];
		} registers;

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

		std::unordered_map<Signal, void(*)(void)> signalHandlers;
	};
}
