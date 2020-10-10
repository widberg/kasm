#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

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

	private:
		void advancePc();
		void systemCall();
		std::uint32_t resolveAddress(const InstructionData& instructionData, AddressType type);

		std::uint32_t pc, hi, lo;
		bool shouldExit;
		int exitCode;

		class Program : public std::vector<std::uint8_t>
		{
		public:
			Program() {}
			Program(const std::string& programPath) { open(programPath); }

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
				else
				{
					return *reinterpret_cast<std::uint32_t*>(data() + i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength);
				}
			}

			char* getCharPtr(std::uint32_t i)
			{
				if (i < DATA_SEGMENT_OFFSET)
				{
					return reinterpret_cast<char*>(data() + i);
				}
				else
				{
					return reinterpret_cast<char*>(data() + i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength);
				}
			}

			std::uint32_t getTextSegmentLength() const { return programHeader.textSegmentLength; }

			std::uint8_t operator[](std::uint32_t i) const
			{
				if (i < DATA_SEGMENT_OFFSET)
				{
					return data()[i];
				}
				else
				{
					return data()[i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength];
				}
			}

			std::uint8_t& operator[](std::uint32_t i)
			{
				if (i < DATA_SEGMENT_OFFSET)
				{
					return data()[i];
				}
				else
				{
					return data()[i - DATA_SEGMENT_OFFSET + programHeader.textSegmentLength];
				}
			}
		private:
			ProgramHeader programHeader;
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
	};
}
