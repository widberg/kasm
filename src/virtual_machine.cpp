#include "virtual_machine.hpp"

#include "debug.hpp"

namespace kasm
{
	void VirtualMachine::execute(Instruction instruction)
	{
		static const std::unordered_map<OpCode, void*> Instructions = {
			{ OpCode::Add, &[&](quad_word_t& a, quad_word_t& b)
				{
					quad_word_t result = b + a;

					calculateStatusFlags(a, b, result);

					b = result;
				}
			},
		};

		OpCode opCode = instruction.opCode();

		if (Instructions.count(opCode))
		{
			OperandType format = InstructionFormat.at(opCode);

			switch (format)
			{
			case OperandType::Register0 | OperandType::Register1:
				auto instructionFunction = static_cast<void(*)(quad_word_t&, quad_word_t&)>(Instructions.at(opCode));
				instructionFunction(registers[instruction.register0()], registers[instruction.register1()]);
				break;
			case OperandType::Register0 | OperandType::AddressIndirect:
				auto instructionFunction = static_cast<void(*)(quad_word_t&, quad_word_t&)>(Instructions.at(opCode));
				instructionFunction(registers[instructionData.register0], registers[instructionData.register1]);
				break;
			case OperandType::AddressDirect:
				auto instructionFunction = static_cast<void(*)(quad_word_t&, quad_word_t&)>(Instructions.at(opCode));
				instructionFunction(registers[instructionData.register0], registers[instructionData.register1]);
				break;
			default:
				KASM_ASSERT(false, "OpCode does not have defined format");
				break;
			}
		}
		else
		{
			// Not Valid
		}
	}

	Instruction VirtualMachine::fetch()
	{
		return program.readQuadWord();
	}

	void VirtualMachine::setStatusFlags(byte_t flags, byte_t group)
	{
		statusRegister = statusRegister & ~group;
		statusRegister = statusRegister | flags;
	}

	void VirtualMachine::appendStatusFlags(byte_t flags)
	{
		statusRegister = statusRegister | flags;
	}

	void VirtualMachine::unsetStatusFlags(byte_t flags)
	{
		statusRegister = statusRegister & ~flags;
	}

	bool VirtualMachine::testStatusFlags(byte_t flags)
	{
		return (statusRegister & flags) == flags;
	}

	void VirtualMachine::calculateStatusFlags(quad_word_t a, quad_word_t b, quad_word_t result)
	{
		byte_t flags = 0;

		if (a > 0 && b > std::numeric_limits<quad_word_t>::max() - a)
		{
			flags |= Carry;
		}
			
		if (a < 0 && b < std::numeric_limits<quad_word_t>::min() - a)
		{
			flags |= Overflow;
		}

		if (result < 0)
		{
			flags |= Sign;
		}
		else if (result == 0)
		{
			flags |= Zero;
		}

		setStatusFlags(flags, Status);
	}
} // namespace kasm
