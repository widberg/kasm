#include "disassembler.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#include "common.hpp"

namespace kasm
{
	void Disassembler::disassemble(const std::string& programPath, const std::string& asmPath)
	{
		std::ifstream programFile(programPath, std::ios::binary);
		std::ofstream asmFile(asmPath);

		static const char* instructionNames[] = { "add", "addi", "addiu", "addu", "and", "andi", "beq", "bgez", "bgezal", "bgtz", "blez", "bltz", "bltzal", "bne", "div", "divu", "j", "jal", "jr", "lb", "lui", "lw", "mfhi", "mflo", "mult", "multu", "or", "ori", "sb", "sll", "sllv", "slt", "slti", "sltiu", "sltu", "sra", "srl", "srlv", "sub", "subu", "sw", "sys", "xor", "xori", "jalr", "nor", ".text", ".data" };
		static const char* registerNames[] = { "$zero", "$at", "$v0", "v1", "$a0", "a1", "a2", "a3", "$t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "$s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "$t8", "t9", "$k0", "k1", "$gp", "$sp", "$fp", "$ra" };

		enum InstrucionElement
		{
			NONE = 0,
			R0 = 1,
			R1 = 1 << 1,
			R2 = 1 << 2,
			DAA = 1 << 3,
			DAO = 1 << 4,
			I = 1 << 5,
			IDA = 1 << 6,

			RRR = R0 | R1 | R2,
			RRI = R0 | R1 | I,
			RRDAO = R0 | R1 | DAO,
			RR = R0 | R1,
			RI = R0 | I,
			RDAA = R0 | DAA,
			RIDA = R0 | IDA,
		};

		static const std::unordered_map<std::uint32_t, std::uint32_t>  instructionFormats =
		{
			{ ADD, RRR },
			{ ADDI, RRI },
			{ ADDIU, RRI },
			{ ADDU, RRR },
			{ AND, RRR },
			{ ANDI, R0 | R1 | I },
			{ BEQ, RRDAO },
			{ BGEZ, RRDAO },
			{ BGEZAL, RRDAO },
			{ BGTZ, RRDAO },
			{ BLEZ, RRDAO },
			{ BLTZ, RRDAO },
			{ BLTZAL, RRDAO },
			{ BNE, RRDAO },
			{ DIV, RR },
			{ DIVU, RR },
			{ J, DAA },
			{ JAL, DAA },
			{ JR, R0 },
			{ LB, RIDA },
			{ LUI, RI },
			{ LW, RIDA },
			{ MFHI, R0 },
			{ MFLO, R0 },
			{ MULT, RR },
			{ MULTU, RR },
			{ OR, RRR },
			{ ORI, RRI },
			{ SB, RIDA },
			{ SLL, RRI },
			{ SLLV, RRR },
			{ SLT, RRI },
			{ SLTI, RRI },
			{ SLTIU, RRI },
			{ SLTU, RRI },
			{ SRA, RRI },
			{ SRL, RRI },
			{ SRLV, RRR },
			{ SUB, RRR },
			{ SUBU, RRR },
			{ SW, RIDA },
			{ SYS, NONE },
			{ XOR, RRR },
			{ XORI, RRI },
			{ JALR, RR },
			{ NOR, RRR },
		};

		InstructionData textData;
		textData.opcode = TEXT;
		InstructionData dataData;
		dataData.opcode = DATA;
		InstructionData st = textData;
		InstructionData d;
		while (programFile.read(reinterpret_cast<char*>(&d), INSTRUCTION_SIZE) && programFile.good())
		{
			std::uint32_t pc = static_cast<std::uint32_t>(programFile.tellg()) - 4;
			asmFile << "_" << std::dec << std::setw(4) << std::setfill('0') << pc << ": ";

			if (d.instruction == textData.instruction || d.instruction == dataData.instruction)
			{
				st.instruction = d.instruction;
				asmFile << instructionNames[st.opcode] << std::endl;
				continue;
			}

			if (st.instruction == textData.instruction)
			{
				std::uint32_t instructionFormat = instructionFormats.at(d.opcode);
				asmFile << std::dec << std::setw(0) << instructionNames[d.opcode] << " ";
				
				if (instructionFormat & R0)
				{
					asmFile << registerNames[d.register0] << ", ";
				}
				if (instructionFormat & R1)
				{
					asmFile << registerNames[d.register1] << ", ";
				}
				if (instructionFormat & R2)
				{
					asmFile << registerNames[d.register2] << ", ";
				}
				if (instructionFormat & DAA)
				{
					asmFile << "_" << d.directAddressAbsolute - d.directAddressAbsolute % INSTRUCTION_SIZE << "+" << d.directAddressAbsolute % INSTRUCTION_SIZE << ", ";
				}
				if (instructionFormat & DAO)
				{
					asmFile << "_" << d.directAddressOffset + pc - ((d.directAddressOffset + pc) % INSTRUCTION_SIZE )<< "+" << (d.directAddressOffset + pc) % INSTRUCTION_SIZE << ", ";
				}
				if (instructionFormat & I)
				{
					asmFile << d.immediate << ", ";
				}
				if (instructionFormat & IDA)
				{
					asmFile << "_" << d.directAddressOffset - d.directAddressOffset % INSTRUCTION_SIZE << "+" << d.directAddressOffset % INSTRUCTION_SIZE << "(" << registerNames[d.register1] << "), ";
				}

				if (instructionFormat != NONE)
				{
					asmFile.seekp(-2, std::ios::cur);
				}
				asmFile << std::endl;
			}
			else
			{
				asmFile << "0x" << std::hex << std::setw(8) << std::setfill('0') << d.instruction << std::endl;
			}
		}
	}
}
