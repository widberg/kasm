#include "specification.hpp"

namespace kasm
{
	const std::unordered_map<OpCode, OperandType> InstructionFormat = {
		{ OpCode::Add, OperandType::None },
	};

	const std::unordered_map<Directive, OperandType> DirectiveFormat = {

	};
} // namespace kasm