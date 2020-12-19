#ifndef SPECIFICATION_HPP
#define SPECIFICATION_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

namespace kasm
{
	typedef std::uint8_t  byte_t;
	typedef std::uint16_t word_t;
	typedef std::uint32_t double_word_t;
	typedef std::uint64_t quad_word_t;

	constexpr std::uint8_t BYTE_SIZE = sizeof(byte_t);
	constexpr std::uint8_t BYTE_BIT = 8;
	constexpr std::uint8_t WORD_SIZE = sizeof(word_t);
	constexpr std::uint8_t WORD_BIT = WORD_SIZE * BYTE_BIT;
	constexpr std::uint8_t DOUBLE_WORD_SIZE = sizeof(double_word_t);
	constexpr std::uint8_t DOUBLE_WORD_BIT= DOUBLE_WORD_SIZE * BYTE_BIT;
	constexpr std::uint8_t QUAD_WORD_SIZE = sizeof(quad_word_t);
	constexpr std::uint8_t QUAD_WORD_BIT = QUAD_WORD_SIZE * BYTE_BIT;

	typedef quad_word_t instruction_t;

	constexpr std::uint8_t INSTRUCTION_SIZE = QUAD_WORD_SIZE;
	constexpr std::uint8_t INSTRUCTION_BIT = INSTRUCTION_SIZE * BYTE_BIT;

	constexpr std::uint8_t OPCODE_BIT = 6;
	constexpr std::uint8_t OPERAND_SIZE_BIT = 2;
	constexpr std::uint8_t REGISTER_BIT = 5;
	constexpr std::uint8_t ADDRESS_ABSOLUTE_BIT = INSTRUCTION_BIT - OPCODE_BIT - OPERAND_SIZE_BIT;
	constexpr std::uint8_t ADDRESS_OFFSET_BIT = DOUBLE_WORD_BIT;
	constexpr std::uint8_t IMMEDIATE_BIT = DOUBLE_WORD_BIT;

	enum OpCode : byte_t
	{
		Add,

		Invalid = 0xFF,
	};
	
	enum OperandType : byte_t
	{
		None = 0,

		Register0 = 1 << 0,
		Register1 = 1 << 1,

		AddressDirect = 1 << 2,
		AddressIndirect = 1 << 3,
	};

	extern const std::unordered_map<OpCode, OperandType> InstructionFormat;

	enum class OperandSize
	{
		Byte,
		Word,
		DoubleWord,
		QuadWord,
	};

	enum class Directive
	{
		Segment
	};

	enum class Register
	{
		RAX
	};

	struct ProgramHeader
	{
		quad_word_t textSegmentBegin;
		quad_word_t textSegmentSize;
		quad_word_t dataSegmentBegin;
		quad_word_t dataSegmentSize;
	};

	const std::uint32_t GLOBAL_SIZE = 256;
	const std::uint32_t STACK_SIZE = 256;

	const std::uint32_t GLOBAL_OFFSET = 0xFFFF0000;
	const std::uint32_t STACK_OFFSET = 0x80000000;
	const std::uint32_t DATA_SEGMENT_OFFSET = 0x10010000;
	const std::uint32_t TEXT_SEGMENT_OFFSET = 0x00000000;

	enum class AddressType
	{
		Invalid,
		DirectAddressAbsolute,
		DirectAddressOffset,
		IndirectAddressOffset,
		DirectAddressAbsoluteWord,
		DirectAddressAbsoluteByte,
		DirectAddressAbsoluteLoad
	};

	struct AddressData
	{
		AddressType type;
		std::string label;
		quad_word_t position;
		double_word_t offset = 0;
		Register reg = Register::RAX;
		bool valid;
	};
} // namespace kasm

#endif // SPECIFICATION_HPP
