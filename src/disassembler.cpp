#include "disassembler.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#include "common.hpp"

namespace kasm
{
	void Disassembler::disassemble(const std::string& programPath, const std::string& asmPath, const std::string& symbolTablePath)
	{
		std::ifstream programFile(programPath, std::ios::binary);
		std::ofstream asmFile(asmPath);

		static const char* instructionNames[] = { "add", "addi", "addiu", "addu", "and", "andi", "beq", "bgez", "bgezal", "bgtz", "blez", "bltz", "bltzal", "bne", "div", "divu", "j", "jal", "jr", "lb", "lui", "lw", "mfhi", "mflo", "mult", "multu", "or", "ori", "sb", "sll", "sllv", "slt", "slti", "sltiu", "sltu", "sra", "srl", "srlv", "sub", "subu", "sw", "sys", "xor", "xori", "jalr", "nor", ".text", ".data" };
		static const char* registerNames[] = { "$zero", "$at", "$v0", "v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };

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
			{ ANDI, RRI },
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

		ProgramHeader programHeader;
		programFile.read(reinterpret_cast<char*>(&programHeader), sizeof(programHeader));
		std::uint32_t pc = 0;

		if (!symbolTablePath.empty())
		{
			std::ifstream symbolTableFile(symbolTablePath, std::ios::binary);
			while (!symbolTableFile.eof())
			{
				std::uint8_t labelSize;
				symbolTableFile.read(reinterpret_cast<char*>(&labelSize), sizeof(labelSize));
				std::string label;
				label.resize(labelSize);
				symbolTableFile.read(reinterpret_cast<char*>(label.data()), labelSize);
				std::uint32_t location;
				symbolTableFile.read(reinterpret_cast<char*>(&location), sizeof(location));
				symbolTable.insert({ location, label });
			}
		}

		if (programHeader.textSegmentLength)
		{
			asmFile << "\t.text" << std::endl;
		}

		InstructionData d;
		while (programFile.tellg() < programHeader.textSegmentBegin + programHeader.textSegmentLength)
		{
			if (symbolTable.count(pc))
			{
				asmFile << "_" << std::hex << std::setw(8) << std::setfill('0') << pc << ":" << std::endl;
			}
			asmFile << getLabelFromAddress(pc) << ": ";

			programFile.read(reinterpret_cast<char*>(&d), sizeof(d));

			std::uint32_t instructionFormat = instructionFormats.at(d.opcode);
			asmFile << std::hex << std::setw(0) << instructionNames[d.opcode] << " ";
				
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
				asmFile << getLabelFromAddress(d.directAddressAbsolute) << ", ";
			}
			if (instructionFormat & DAO)
			{
				asmFile << getLabelFromAddress(d.directAddressOffset + pc) << ", ";
			}
			if (instructionFormat & I)
			{
				asmFile << "0x" << std::hex << std::setw(4) << std::setfill('0') << d.immediate << ", ";
			}
			if (instructionFormat & IDA)
			{
				std::uint32_t offset = (d.directAddressOffset + pc) % INSTRUCTION_SIZE;
				asmFile << getLabelFromAddress(d.directAddressOffset + DATA_SEGMENT_OFFSET - offset + pc);
				if (offset)
				{
					asmFile << "+" << offset;
				}
				if (d.register1)
				{
					asmFile << "(" << registerNames[d.register1] << ")";
				}
				asmFile << ", ";
			}

			if (instructionFormat != NONE)
			{
				asmFile.seekp(-2, std::ios::cur);
			}
			asmFile << std::endl;

			pc += INSTRUCTION_SIZE;
		}

		if (programHeader.dataSegmentLength)
		{
			pc = DATA_SEGMENT_OFFSET;
			asmFile << "\t.data" << std::endl;
		}

		while (programFile.read(reinterpret_cast<char*>(&d), sizeof(d)) && !programFile.eof())
		{
			bool splitWordToBytes = false;
			for (std::uint32_t i = 0; i < 4; i++)
			{
				if (symbolTable.count(pc + i))
				{
					splitWordToBytes = true;
					break;
				}
			}

			if (splitWordToBytes)
			{
				for (std::uint32_t i = 0; i < 4; i++)
				{
					if (symbolTable.count(pc + i))
					{
						asmFile << "_" << std::hex << std::setw(8) << std::setfill('0') << pc + i << ":" << std::endl;
					}
					asmFile << getLabelFromAddress(pc + i) << ": " << ".byte 0x" << std::hex << std::setw(2) << std::setfill('0') << ((d.instruction & (0xFF << (i * CHAR_BIT))) >> (i * CHAR_BIT)) << std::endl;
				}
			}
			else
			{
				asmFile << getLabelFromAddress(pc) << ": " << ".word 0x" << std::hex << std::setw(8) << std::setfill('0') << d.instruction << std::endl;
			}
			pc += INSTRUCTION_SIZE;
		}
	}

	std::string Disassembler::getLabelFromAddress(std::uint32_t location)
	{
		std::stringstream ss;
		if (symbolTable.count(location))
		{
			ss << std::setw(9) << std::setfill(' ') << symbolTable.at(location);
		}
		else
		{
			ss << "_" << std::hex << std::setw(8) << std::setfill('0') << location;
		}
		return ss.str();
	}
}
