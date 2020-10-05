#pragma once

#include <algorithm>
#include <cstdint>
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

		std::vector<std::uint8_t> program;

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
