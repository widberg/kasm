#ifndef VIRTUAL_MACHINE_HPP
#define VIRTUAL_MACHINE_HPP

#include "memory.hpp"
#include "program.hpp"
#include "registers.hpp"
#include "specification.hpp"

namespace kasm
{
	class VirtualMachine
	{
	public:
		VirtualMachine(const Program& program_) : memory(program_) {};

		void execute(Instruction instructionData);
		Instruction fetch();

	private:
		Memory memory;
		Registers registers;

		quad_word_t programCounter;
		byte_t statusRegister;

		enum StatusCodeMask : byte_t
		{
			// Control
			Trap = 1 << 4,
			Control = Trap,

			// Status
			Sign = 1 << 3,
			Zero = 1 << 2,
			Carry = 1 << 1,
			Overflow = 1 << 0,
			Status = Sign | Zero | Carry | Overflow,

			All = 0xFF,
		};

		void setStatusFlags(byte_t flags, byte_t group = StatusCodeMask::All);
		void appendStatusFlags(byte_t flags);
		void unsetStatusFlags(byte_t flags);
		bool testStatusFlags(byte_t flags);
		void calculateStatusFlags(quad_word_t a, quad_word_t b, quad_word_t result);
	};
} // namespace kasm

#endif // !VIRTUAL_MACHINE_HPP
