#ifndef SPECIFICATION_HPP
#define SPECIFICATION_HPP

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>

#include "common.hpp"

namespace kasm
{
	typedef std::uint8_t  byte_t;
	typedef std::uint16_t word_t;
	typedef std::uint32_t double_word_t;
	typedef std::uint64_t quad_word_t;

	constexpr double_word_t MAGIC_NUMBER = 0x4b41534d; // "KASM"
	constexpr double_word_t VERSION = 0x00000000; // 00.00.00-00

	constexpr std::uint8_t BYTE_SIZE = sizeof(byte_t);
	constexpr std::uint8_t BYTE_BIT = 8;
	constexpr std::uint8_t WORD_SIZE = sizeof(word_t);
	constexpr std::uint8_t WORD_BIT = WORD_SIZE * BYTE_BIT;
	constexpr std::uint8_t DOUBLE_WORD_SIZE = sizeof(double_word_t);
	constexpr std::uint8_t DOUBLE_WORD_BIT= DOUBLE_WORD_SIZE * BYTE_BIT;
	constexpr std::uint8_t QUAD_WORD_SIZE = sizeof(quad_word_t);
	constexpr std::uint8_t QUAD_WORD_BIT = QUAD_WORD_SIZE * BYTE_BIT;

	typedef quad_word_t instruction_t;
	typedef quad_word_t address_t;

	constexpr std::uint8_t INSTRUCTION_SIZE = QUAD_WORD_SIZE;
	constexpr std::uint8_t INSTRUCTION_BIT = INSTRUCTION_SIZE * BYTE_BIT;

	constexpr std::uint8_t OPCODE_BIT = 6;
	constexpr std::uint8_t OPCODE_MASK = 0b1111;
	constexpr std::uint8_t OPCODE_OFFSET = INSTRUCTION_BIT - OPCODE_BIT;

	constexpr std::uint8_t OPERAND_SIZE_BIT = 2;
	constexpr std::uint8_t OPERAND_SIZE_MASK = 0b11;
	constexpr std::uint8_t OPERAND_SIZE_OFFSET = INSTRUCTION_BIT - OPCODE_BIT - OPERAND_SIZE_BIT;

	constexpr std::uint8_t REGISTER_BIT = 5;
	constexpr std::uint8_t REGISTER_MASK = 0b11111;
	constexpr std::uint8_t REGISTER_0_OFFSET = INSTRUCTION_BIT - OPCODE_BIT - OPERAND_SIZE_BIT - REGISTER_BIT;
	constexpr std::uint8_t REGISTER_1_OFFSET = INSTRUCTION_BIT - OPCODE_BIT - OPERAND_SIZE_BIT - REGISTER_BIT * 2;
	constexpr std::uint8_t REGISTER_2_OFFSET = INSTRUCTION_BIT - OPCODE_BIT - OPERAND_SIZE_BIT - REGISTER_BIT * 3;

	constexpr std::uint8_t IMMEDIATE_8_BIT = BYTE_BIT;
	constexpr std::uint8_t IMMEDIATE_8_MASK = 0xFF;
	constexpr std::uint8_t IMMEDIATE_8_OFFSET = INSTRUCTION_BIT - QUAD_WORD_BIT;

	constexpr std::uint8_t IMMEDIATE_32_BIT = DOUBLE_WORD_BIT;
	constexpr std::uint8_t IMMEDIATE_32_MASK = 0xFFFFFFFF;
	constexpr std::uint8_t IMMEDIATE_32_OFFSET = 0;

	constexpr std::uint8_t IMMEDIATE_56_BIT = QUAD_WORD_BIT - BYTE_BIT;
	constexpr std::uint8_t IMMEDIATE_56_MASK = 0xFFFFFFFFFFFFFF;
	constexpr std::uint8_t IMMEDIATE_56_OFFSET = 0;

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

		Immediate32 = 1 << 4,
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

	extern const std::unordered_map<Directive, OperandType> DirectiveFormat;

	enum Register
	{
		RAX,

		Count = 32,
	};

	struct ProgramHeader
	{
		double_word_t magicNumber;
		double_word_t version;
		quad_word_t textSegmentBegin;
		quad_word_t textSegmentSize;
		quad_word_t dataSegmentBegin;
		quad_word_t dataSegmentSize;
	};

	constexpr Register REGISTER_COUNT = Register::Count;

	constexpr quad_word_t GLOBAL_SIZE = 256;
	constexpr quad_word_t STACK_SIZE = 256;

	constexpr quad_word_t ADDRESS_MIN = std::numeric_limits<quad_word_t>::min();
	constexpr quad_word_t ADDRESS_MAX = std::numeric_limits<quad_word_t>::max();
	constexpr quad_word_t GLOBAL_OFFSET = 0xFFFF0000;
	constexpr quad_word_t STACK_OFFSET = 0x80000000;
	constexpr quad_word_t DATA_SEGMENT_OFFSET = 0x10010000;
	constexpr quad_word_t DATA_SEGMENT_SIZE = ADDRESS_MAX - DATA_SEGMENT_OFFSET;
	constexpr quad_word_t TEXT_SEGMENT_OFFSET = ADDRESS_MIN;
	constexpr quad_word_t TEXT_SEGMENT_SIZE = DATA_SEGMENT_OFFSET - TEXT_SEGMENT_OFFSET;

	enum class AddressingMode
	{
		Invalid,
		DirectAddress,
		IndirectAddress,
	};

	struct Address
	{
		AddressingMode addressingMode;
		quad_word_t offsetImmediate;
		byte_t indexImmediate;
		Register offsetRegister;
		Register indexRegister;
	};

	class Instruction
	{
	public:
		Instruction() : instruction(0) {}
		Instruction(instruction_t instruction_) : instruction(instruction_) {}

		OpCode opCode() const { return static_cast<OpCode>((instruction >> OPCODE_OFFSET) & OPCODE_MASK); }
		Register register0() const { return static_cast<Register>((instruction >> REGISTER_0_OFFSET) & REGISTER_MASK); }
		Register register1() const { return static_cast<Register>((instruction >> REGISTER_1_OFFSET) & REGISTER_MASK); }
		Register register2() const { return static_cast<Register>((instruction >> REGISTER_2_OFFSET) & REGISTER_MASK); }
		quad_word_t immediate56() const { return static_cast<double_word_t>((instruction >> IMMEDIATE_56_OFFSET) & IMMEDIATE_56_MASK); }
		double_word_t immediate32() const { return static_cast<double_word_t>((instruction >> IMMEDIATE_32_OFFSET) & IMMEDIATE_32_MASK); }
		byte_t immediate8() const { return static_cast<double_word_t>((instruction >> IMMEDIATE_8_OFFSET) & IMMEDIATE_8_MASK); }
		Address address(AddressingMode addressType) const
		{
			Address address;
			address.addressingMode = AddressingMode::DirectAddress;

			switch (addressType)
			{
			case AddressingMode::DirectAddress:
				address.offsetImmediate = immediate56();
				break;
			case AddressingMode::IndirectAddress:
				address.offsetImmediate = immediate32();
				address.indexImmediate = immediate8();
				address.offsetRegister = register1();
				address.indexRegister = register2();
				break;
			default:
				break;
			}

			return address;
		}
		
		void opCode(OpCode opCode) { instruction = (instruction & ~(OPCODE_MASK << OPCODE_OFFSET)) | ((opCode & OPCODE_MASK) << OPCODE_OFFSET);  }
		void register0(Register register0) { instruction = (instruction & ~(REGISTER_MASK << REGISTER_0_OFFSET)) | ((register0 & REGISTER_MASK) << REGISTER_0_OFFSET); }
		void register1(Register register1) { instruction = (instruction & ~(REGISTER_MASK << REGISTER_1_OFFSET)) | ((register1 & REGISTER_MASK) << REGISTER_1_OFFSET); }
		void register2(Register register2) { instruction = (instruction & ~(REGISTER_MASK << REGISTER_2_OFFSET)) | ((register2 & REGISTER_MASK) << REGISTER_2_OFFSET); }
		void immediate56(quad_word_t immediate) { instruction = (instruction & ~(IMMEDIATE_56_MASK << IMMEDIATE_56_OFFSET)) | ((immediate & IMMEDIATE_56_MASK) << IMMEDIATE_56_OFFSET); }
		void immediate32(double_word_t immediate) { instruction = (instruction & ~(IMMEDIATE_32_MASK << IMMEDIATE_32_OFFSET)) | ((immediate & IMMEDIATE_32_MASK) << IMMEDIATE_32_OFFSET); }
		void immediate8(byte_t immediate) { instruction = (instruction & ~(IMMEDIATE_8_MASK << IMMEDIATE_8_OFFSET)) | ((immediate & IMMEDIATE_8_MASK) << IMMEDIATE_8_OFFSET); }
		void address(Address address)
		{
			switch (address.addressingMode)
			{
			case AddressingMode::DirectAddress:
				immediate56(address.offsetImmediate);
				break;
			case AddressingMode::IndirectAddress:
				immediate32(address.offsetImmediate);
				immediate8(address.indexImmediate);
				register1(address.offsetRegister);
				register2(address.indexRegister);
				break;
			default:
				break;
			}
		}
	private:
		instruction_t instruction;
	};
} // namespace kasm

#endif // SPECIFICATION_HPP
