#include "virtual_machine.hpp"

namespace kasm
{
	quad_word_t VirtualMachine::execute()
	{
		static const std::unordered_map<OpCode, void*> Instruction = {
			{ OpCode::Add, &[&](quad_word_t& a, quad_word_t& b)
				{
					b = b + a;
					setStatusFlags(Sign | Overflow, Status);
				}
			},
		};

		OpCode opCode = Invalid;

		if (Instruction.count(opCode))
		{
			OperandType format = InstructionFormat.at(opCode);

			if (format == OperandType::Register0 | OperandType::Register1)
			{
				static_cast<void(*)(quad_word_t&, quad_word_t&)>(Instruction.at(opCode))(reg0, reg1);
			}
		}
		else
		{
			// Not Valid
		}

		return 0;
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
} // namespace kasm
