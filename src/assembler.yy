%no-lines
%require "3.4.1"
%language "c++"

%skeleton "lalr1.cc"

%define api.token.constructor
%define api.value.type variant
%define api.location.file none
%define parse.assert
%define parse.error verbose

%locations

%code requires
{
#include "assembler.hpp"

#include <algorithm> // max
#include <exception>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#include "compoundInputFileStream.hpp"

}//%code requires

%code
{
enum class CTXFlag
{
	None,
	LINE_AS_STRING,
	BLOCK_AS_STRING,
	ARGUMENT_LIST
};

struct MacroCall
{
	unsigned uid;
	std::vector<std::string> paramaters;
	std::vector<std::string> arguments;
};

static kasm::CompoundInputFileStream in;
static yy::location loc;
static kasm::Assembler* assembler;
static CTXFlag flag;
static std::vector<MacroCall> macroCallStack;
static std::stack<bool> labelInMacro;

void eofCallback(unsigned uid)
{
	KASM_ASSERT(!macroCallStack.empty(), "Trying to pop empty macro stack");
	KASM_ASSERT(uid == macroCallStack.back().uid, "Stack order got fucked");
	macroCallStack.pop_back();
}

namespace yy { parser::symbol_type yylex(); }

#define INSTRUCTION_RRR(op, r0, r1, r2) {                           \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.register1 = r1;                                 \
	instructionData.register2 = r2;                                 \
	assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RRL(op, r0, r1, l) {                            \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.register1 = r1;                                 \
	instructionData.immediate = l;                                  \
	assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RRA(op, r0, r1, a, t) {                           \
	kasm::InstructionData instructionData;                            \
	instructionData.opcode = kasm::Opcode::##op;                      \
	instructionData.register0 = r0;                                   \
	instructionData.register1 = r1;                                   \
	a.type = kasm::AddressType::##t;                                  \
	a.position = GET_LOC();                                           \
	a.instructionData = instructionData;                              \
	assembler->resolveAddress(a);                                 \
	assembler->binary.writeWord(a.instructionData.instruction); } \

#define INSTRUCTION_RR(op, r0, r1) {                                \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.register1 = r1;                                 \
	assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RL(op, r0, l) {                                 \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.immediate = l;                                  \
	assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RA(op, r0, a, t) {                                \
	kasm::InstructionData instructionData;                            \
	instructionData.opcode = kasm::Opcode::##op;                      \
	instructionData.register0 = r0;                                   \
	a.type = kasm::AddressType::##t;                                  \
	a.position = GET_LOC();                                           \
	a.instructionData = instructionData;                              \
	assembler->resolveAddress(a);                                 \
	assembler->binary.writeWord(a.instructionData.instruction); } \

#define INSTRUCTION_R(op, r0) {                                     \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_A(op, a, t) {                                     \
	kasm::InstructionData instructionData;                            \
	instructionData.opcode = kasm::Opcode::##op;                      \
	a.type = kasm::AddressType::##t;                                  \
	a.position = GET_LOC();                                           \
	a.instructionData = instructionData;                              \
	assembler->resolveAddress(a);                                 \
	assembler->binary.writeWord(a.instructionData.instruction); } \

#define INSTRUCTION_O(op) {                                         \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	assembler->binary.writeWord(instructionData.instruction); } \

union SplitWord
{
	std::uint32_t value;
#pragma pack(push, 1)
#if 'ABCD' == 0x41424344 // if little endian. trash code TODO: make portable
	struct
	{
		std::uint32_t lo : kasm::IMMEDIATE_BIT;
		std::uint32_t hi : kasm::IMMEDIATE_BIT;
	};
#else
	struct
	{
		std::uint32_t hi : kasm::IMMEDIATE_BIT;
		std::uint32_t lo : kasm::IMMEDIATE_BIT;
	};
#endif
#pragma pack(pop)
};

#define GET_LOC() assembler->binary.getLocation()

kasm::AddressData stackAddress(kasm::Register::SP);
kasm::AddressData frameAddress(kasm::Register::FP);
kasm::AddressData returnAddress(kasm::Register::RA);

}//%code

%token END_OF_FILE 0 END_OF_LINE
%token IDENTIFIER LITERAL STRING REGISTER ARGUMENT_LIST

%token TEXT DATA WORD BYTE ASCII ASCIIZ ALIGN SPACE INCLUDE ERROR MESSAGE MACRO DBG DEFINE DBGBP

%token ADD ADDI ADDIU ADDU AND ANDI BEQ BGEZ BGEZAL BGTZ BLEZ BLTZ BLTZAL BNE
%token DIV DIVU J JAL JR LB LUI LW MFHI MFLO MULT MULTU OR ORI SB SLL SLLV NOR
%token SLT SLTI SLTIU SLTU SNE SEQ SRA SRL SRLV SUB SUBU SW SYS XOR XORI JALR

%token COPY CLR B BAL BGT BLT BGE BLE BGTU BEQZ REM LI LA NOP NOT PUSHW POPW PUSHB POPB RET CALL ENTER

%type<std::string> IDENTIFIER STRING
%type<std::uint32_t> LITERAL REGISTER
%type<std::vector<std::string>> ARGUMENT_LIST identifier_list identifier_list_not_empty
%type<std::vector<std::variant<std::uint32_t, kasm::AddressData>>> literal_list literal_argument
%type<kasm::AddressData> address direct_address
%type<std::uint32_t> statement

%start statement_list

%%

statement_list
	: statement_list statement
	| %empty
	;

statement
    : IDENTIFIER ':' { labelInMacro.push(!macroCallStack.empty()); } statement
	{
		$$ = $4;
		if (labelInMacro.top())
		{
			unsigned x = 0;
			MacroCall& mc = macroCallStack.back();
			while (assembler->isIdentifierDefined($1 + std::to_string(x)) || std::find(mc.paramaters.begin(), mc.paramaters.end(), $1 + std::to_string(x)) != mc.paramaters.end())
			{
				x++;
			}
			mc.paramaters.push_back($1);
			mc.arguments.push_back($1 + std::to_string(x));
			assembler->defineLabel($1 + std::to_string(x), $4);
		}
		else
		{
			assembler->defineLabel($1, $4);
		}
		labelInMacro.pop();
	}
	| END_OF_LINE statement { $$ = $2; }
	| END_OF_FILE { $$ = GET_LOC(); }
	// Directives
	| TEXT end_of_statement { $$ = GET_LOC(); assembler->binary.setSegmentType(kasm::BinaryBuilder::SegmentType::TEXT); }
	| DATA end_of_statement { $$ = GET_LOC(); assembler->binary.setSegmentType(kasm::BinaryBuilder::SegmentType::DATA); }
    | WORD literal_argument end_of_statement
    {
		KASM_ASSERT(assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "word must be in data segment");
        assembler->binary.align(kasm::INSTRUCTION_SIZE);
        $$ = GET_LOC(); 
		for (std::variant<std::uint32_t, kasm::AddressData> word : $2)
		{
			if(std::holds_alternative<kasm::AddressData>(word))
			{
				kasm::AddressData addr = std::get<kasm::AddressData>(word);
				addr.type = kasm::AddressType::DirectAddressAbsoluteWord;
				addr.position = GET_LOC();
				assembler->resolveAddress(addr);
            	assembler->binary.writeWord(addr.instructionData.instruction);
			}
			else
			{
            	assembler->binary.writeWord(std::get<std::uint32_t>(word));
			}
        }
    }
    | BYTE literal_argument end_of_statement
    {
		KASM_ASSERT(assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "byte must be in data segment");
        $$ = GET_LOC(); 
		for (std::variant<std::uint32_t, kasm::AddressData> byte : $2)
        {
			if(std::holds_alternative<kasm::AddressData>(byte))
			{
				kasm::AddressData addr = std::get<kasm::AddressData>(byte);
				addr.type = kasm::AddressType::DirectAddressAbsoluteByte;
				addr.position = GET_LOC();
				assembler->resolveAddress(addr);
            	assembler->binary.writeByte(static_cast<std::uint8_t>(addr.instructionData.instruction));
			}
			else
			{
            	assembler->binary.writeByte(static_cast<std::uint8_t>(std::get<std::uint32_t>(byte)));
			}
        }
    }
    | ASCII STRING end_of_statement
	{
		KASM_ASSERT(assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "ascii must be in data segment");
		$$ = GET_LOC(); 
		assembler->binary.writeString($2.c_str(), $2.size());
	}
    | ASCIIZ STRING end_of_statement
	{
		KASM_ASSERT(assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "asciiz must be in data segment");
		$$ = GET_LOC(); 
		assembler->binary.writeString($2.c_str(), $2.size() + 1);
	}
    | ALIGN LITERAL end_of_statement
    {
		KASM_ASSERT(assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "align must be in data segment");
        $$ = GET_LOC(); 
		unsigned int alignment = 1;
        for (int i = 0; i < $2; i++)
        {
            alignment *= 2;
        }
        assembler->binary.align(alignment);
    }
    | SPACE LITERAL end_of_statement
	{
		KASM_ASSERT(assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "space must be in data segment");
		$$ = GET_LOC(); 
		assembler->binary.pad($2);
	}
	| INCLUDE STRING end_of_statement { in.include($2); } statement { $$ = $5; }
	| ERROR   STRING end_of_statement { std::cout << "ERROR: " << $2 << std::endl; throw std::runtime_error("Assembler user defined error"); } statement { $$ = $5; }
	| MESSAGE STRING end_of_statement { std::cout << "MESSAGE: " << $2 << std::endl; } statement { $$ = $5; }
	| DBG     STRING end_of_statement { in.pushString($2); } statement { $$ = $5; }
	| DBGBP          end_of_statement { KASM_BREAKPOINT(); } statement { $$ = $4; }
	| DEFINE IDENTIFIER { flag = CTXFlag::LINE_AS_STRING; } STRING { flag = CTXFlag::None; assembler->defineMacro($2, $4); } end_of_statement statement { $$ = $7; }
	| MACRO IDENTIFIER '(' identifier_list ')' END_OF_LINE { flag = CTXFlag::BLOCK_AS_STRING; } STRING { flag = CTXFlag::None; assembler->defineMacro($2, $4, $8); } end_of_statement statement { $$ = $11; }
	| IDENTIFIER '(' { flag = CTXFlag::ARGUMENT_LIST; } ARGUMENT_LIST { flag = CTXFlag::None; } end_of_statement { macroCallStack.push_back({in.pushString(assembler->macroFunctions[$1].body, true), assembler->macroFunctions[$1].paramaters, $4}); } statement { $$ = $8; }

	// Instructions
    | ADD    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(ADD, $2, $4, $6); }
	| ADDI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ADDI, $2, $4, $6); }
	| ADDIU  REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ADDIU, $2, $4, $6); }
	| ADDU   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(ADDU, $2, $4, $6); }
	| AND    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(AND, $2, $4, $6); }
	| ANDI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ANDI, $2, $4, $6); }
    | BEQ    REGISTER ',' REGISTER ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRA(BEQ, $2, $4, $6, DirectAddressOffset); }
	| BGEZ   REGISTER ',' direct_address              end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(BGEZ, $2, $4, DirectAddressOffset); }
	| BGEZAL REGISTER ',' direct_address              end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(BGEZAL, $2, $4, DirectAddressOffset); }
	| BGTZ   REGISTER ',' direct_address              end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(BGTZ, $2, $4, DirectAddressOffset); }
	| BLEZ   REGISTER ',' direct_address              end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(BLEZ, $2, $4, DirectAddressOffset); }
	| BLTZ   REGISTER ',' direct_address              end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(BLTZ, $2, $4, DirectAddressOffset); }
	| BLTZAL REGISTER ',' direct_address              end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(BLTZAL, $2, $4, DirectAddressOffset); }
	| BNE    REGISTER ',' REGISTER ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRA(BNE, $2, $4, $6, DirectAddressOffset); }
	| DIV    REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(DIV, $2, $4); }
	| DIVU   REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(DIVU, $2, $4); }
	| J      address                                  end_of_statement { $$ = GET_LOC(); INSTRUCTION_A(J, $2, DirectAddressAbsolute); }
	| JAL    address                                  end_of_statement { $$ = GET_LOC(); INSTRUCTION_A(JAL, $2, DirectAddressAbsolute); }
	| JR     REGISTER                                 end_of_statement { $$ = GET_LOC(); INSTRUCTION_R(JR, $2); }
    | LB     REGISTER ',' address                     end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(LB, $2, $4, IndirectAddressOffset); }
	| LUI    REGISTER ',' LITERAL                     end_of_statement { $$ = GET_LOC(); INSTRUCTION_RL(LUI, $2, $4); }
	| LW     REGISTER ',' address                     end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(LW, $2, $4, IndirectAddressOffset); }
	| MFHI   REGISTER                                 end_of_statement { $$ = GET_LOC(); INSTRUCTION_R(MFHI, $2); }
	| MFLO   REGISTER                                 end_of_statement { $$ = GET_LOC(); INSTRUCTION_R(MFLO, $2); }
	| MULT   REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(MULT, $2, $4); }
	| MULTU  REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(MULTU, $2, $4); }
	| OR     REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(OR, $2, $4, $6); }
	| ORI    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ORI, $2, $4, $6); }
	| SB     REGISTER ',' address                     end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(SB, $2, $4, IndirectAddressOffset); }
	| SLL    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(SLL, $2, $4, $6); }
	| SLLV   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLLV, $2, $4, $6); }
	| SLT    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLT, $2, $4, $6); }
	| SLTI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(SLTI, $2, $4, $6); }
	| SLTIU  REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(SLTIU, $2, $4, $6); }
	| SLTU   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLTU, $2, $4, $6); }
	| SNE    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SNE, $2, $4, $6); }
	| SEQ    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SEQ, $2, $4, $6); }
	| SRA    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(SRA, $2, $4, $6); }
	| SRL    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(SRL, $2, $4, $6); }
	| SRLV   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SRLV, $2, $4, $6); }
	| SUB    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SUB, $2, $4, $6); }
	| SUBU   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SUBU, $2, $4, $6); }
	| SW     REGISTER ',' address                     end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(SW, $2, $4, IndirectAddressOffset); }
	| SYS                                             end_of_statement { $$ = GET_LOC(); INSTRUCTION_O(SYS); }
	| XOR    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(XOR, $2, $4, $6); }
	| XORI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(XORI, $2, $4, $6); }
	| JALR   REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(JALR, $2, $4); }
	| NOR    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(NOR, $2, $4, $6); }

	// Pseudoinstructions
	| COPY   REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(OR, $2, $4, kasm::ZERO); }
	| CLR    REGISTER                                 end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(OR, $2, kasm::ZERO, kasm::ZERO); }
	| ADD    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ADDI, $2, $4, $6); }
	| JALR   REGISTER                                 end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(JALR, $2, kasm::RA); }
	| NOP                                             end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(SLL, kasm::ZERO, kasm::ZERO, 0); }
	| B      direct_address                           end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRA(BEQ, kasm::ZERO, kasm::ZERO, $2, DirectAddressOffset); }
	| BAL    direct_address                           end_of_statement { $$ = GET_LOC(); INSTRUCTION_RA(BGEZAL, kasm::ZERO, $2, DirectAddressOffset); }
	| BGT    REGISTER ',' REGISTER ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLT, kasm::AT, $4, $2); INSTRUCTION_RRA(BNE, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BLT    REGISTER ',' REGISTER ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLT, kasm::AT, $2, $4); INSTRUCTION_RRA(BNE, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BGE    REGISTER ',' REGISTER ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLT, kasm::AT, $2, $4); INSTRUCTION_RRA(BEQ, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BLE    REGISTER ',' REGISTER ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLT, kasm::AT, $4, $2); INSTRUCTION_RRA(BEQ, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BGTU   REGISTER ',' REGISTER ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(SLTU, kasm::AT, $2, $4); INSTRUCTION_RRA(BEQ, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BEQZ   REGISTER ',' direct_address              end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRA(BEQ, $2, kasm::ZERO, $4, DirectAddressOffset); }
	| BEQ    REGISTER ',' LITERAL  ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ORI, kasm::AT, kasm::ZERO, $4); INSTRUCTION_RRA(BEQ, $2, kasm::AT, $6, DirectAddressOffset); }
	| BNE    REGISTER ',' LITERAL  ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ORI, kasm::AT, kasm::ZERO, $4); INSTRUCTION_RRA(BNE, $2, kasm::AT, $6, DirectAddressOffset); }
	| MULT   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(MULT, $4, $6); INSTRUCTION_R(MFLO, $2); }
	| DIV    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(DIV, $4, $6); INSTRUCTION_R(MFLO, $2); }
	| REM    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(DIV, $4, $6); INSTRUCTION_R(MFHI, $2); }
	| NOT    REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(NOR, $2, $4, kasm::ZERO); }
	| PUSHW  REGISTER                                 end_of_statement
	{
		$$ = GET_LOC();
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, -kasm::INSTRUCTION_SIZE);
		INSTRUCTION_RA(SW, $2, stackAddress, IndirectAddressOffset);
	}
	| POPW   REGISTER                                 end_of_statement
	{
		$$ = GET_LOC();
		INSTRUCTION_RA(LW, $2, stackAddress, IndirectAddressOffset);
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, kasm::INSTRUCTION_SIZE);
	}
	| PUSHB  REGISTER                                 end_of_statement
	{
		$$ = GET_LOC();
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, -1);
		INSTRUCTION_RA(SB, $2, stackAddress, IndirectAddressOffset);
	}
	| POPB   REGISTER                                 end_of_statement
	{
		$$ = GET_LOC();
		INSTRUCTION_RA(LB, $2, stackAddress, IndirectAddressOffset);
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, 1);
	}
	| ENTER                                             end_of_statement
	{
		$$ = GET_LOC();
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, -kasm::INSTRUCTION_SIZE);
		INSTRUCTION_RA(SW, kasm::Register::RA, stackAddress, IndirectAddressOffset);
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, -kasm::INSTRUCTION_SIZE);
		INSTRUCTION_RA(SW, kasm::Register::FP, stackAddress, IndirectAddressOffset);
		INSTRUCTION_RRL(OR, kasm::Register::FP, kasm::Register::SP, kasm::ZERO);
	}
	| RET                                             end_of_statement
	{
		$$ = GET_LOC();
		INSTRUCTION_RRL(OR, kasm::Register::SP, kasm::Register::FP, kasm::ZERO);
		INSTRUCTION_RA(LW, kasm::Register::FP, stackAddress, IndirectAddressOffset);
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, kasm::INSTRUCTION_SIZE);
		INSTRUCTION_RA(LW, kasm::Register::RA, stackAddress, IndirectAddressOffset);
		INSTRUCTION_RRL(ADDI, kasm::Register::SP, kasm::Register::SP, kasm::INSTRUCTION_SIZE);
		INSTRUCTION_R(JR, kasm::Register::RA);
	}
	| CALL   direct_address                           end_of_statement
	{
		$$ = GET_LOC();
		INSTRUCTION_A(JAL, $2, DirectAddressAbsolute);
	}
	| LI     REGISTER ',' LITERAL                     end_of_statement
	{
		$$ = GET_LOC(); 
		SplitWord l = {$4};
		if (l.hi)
		{
			INSTRUCTION_RL(LUI, $2, l.hi);
			INSTRUCTION_RRL(ORI, $2, $2, l.lo);
		}
		else
		{
			INSTRUCTION_RRL(ORI, $2, kasm::ZERO, l.lo);
		}
	}
	| LA     REGISTER ',' direct_address              end_of_statement
	{
		$$ = GET_LOC(); 
		$4.type = kasm::AddressType::DirectAddressAbsoluteLoad;
		$4.reg = $2;
		$4.position = GET_LOC();
		assembler->resolveAddress($4);
		SplitWord l = { $4.instructionData.instruction };
		INSTRUCTION_RL(LUI, $2, l.hi);
		INSTRUCTION_RRL(ORI, $2, $2, l.lo);
	}
    ;

literal_argument
	: literal_list           { $$ = $1; }
	| LITERAL ':' LITERAL    { $$ = std::vector<std::variant<std::uint32_t, kasm::AddressData>>($3, $1); }
	| IDENTIFIER ':' LITERAL { $$ = std::vector<std::variant<std::uint32_t, kasm::AddressData>>($3, kasm::AddressData($1)); }
	;

literal_list
    : LITERAL                     { $$ = {$1}; }
	| IDENTIFIER                  { $$ = {$1}; }
    | literal_list ',' LITERAL    { $1.push_back($3); $$ = $1; }
	| literal_list ',' IDENTIFIER { $1.push_back(kasm::AddressData($3)); $$ = $1; }
    ;

identifier_list
	: identifier_list_not_empty { $$ = $1; }
	| %empty { $$ = std::vector<std::string>(); }
	;

identifier_list_not_empty
	: IDENTIFIER { $$ = { $1 }; }
	| identifier_list ',' IDENTIFIER { $1.push_back($3); $$ = $1; }
	;

direct_address
	: IDENTIFIER
	{
		kasm::AddressData addr;
		addr.label = $1;
		$$ = addr;
	}
	;

address
	: direct_address { $$ = $1; } 
    | IDENTIFIER '+' LITERAL
	{
		kasm::AddressData addr;
		addr.label = $1;
		addr.offset = $3;
		$$ = addr;
	}
    | '(' REGISTER ')'
	{
		kasm::AddressData addr;
		addr.reg = $2;
		$$ = addr;
	}
    | LITERAL '(' REGISTER ')'
	{
		kasm::AddressData addr;
		addr.offset = $1;
		addr.reg = $3;
		$$ = addr;
	}
    | IDENTIFIER '(' REGISTER ')'
	{
		kasm::AddressData addr;
		addr.label = $1;
		addr.reg = $3;
		$$ = addr;
	}
    | IDENTIFIER '+' LITERAL '(' REGISTER ')'
	{
		kasm::AddressData addr;
		addr.label = $1;
		addr.offset = $3;
		addr.reg = $5;
		$$ = addr;
	}
    ;

end_of_statement
	: END_OF_LINE
	| END_OF_FILE
	;

%%

const static std::unordered_map<std::string, kasm::Register> REGISTER_NAMES = {
	{ "zero", kasm::ZERO },
	{ "at", kasm::AT },
	{ "v0", kasm::V0 }, { "v1", kasm::V1 },
	{ "a0", kasm::A0 }, { "a1", kasm::A1 }, { "a2", kasm::A2 }, { "a3", kasm::A3 },
	{ "t0", kasm::T0 }, { "t1", kasm::T1 }, { "t2", kasm::T2 }, { "t3", kasm::T3 }, { "t4", kasm::T4 }, { "t5", kasm::T5 }, { "t6", kasm::T6 }, { "t7", kasm::T7 },
	{ "s0", kasm::S0 }, { "s1", kasm::S1 }, { "s2", kasm::S2 }, { "s3", kasm::S3 }, { "s4", kasm::S4 }, { "s5", kasm::S5 }, { "s6", kasm::S6 }, { "s7", kasm::S7 },
	{ "t8", kasm::T8 }, { "t9", kasm::T9 },
	{ "k0", kasm::K0 }, { "k1", kasm::K1 },
	{ "gp", kasm::GP },
	{ "sp", kasm::SP },
	{ "fp", kasm::FP },
	{ "ra", kasm::RA },
};

const static std::unordered_map<char, char> ESCAPE_SEQUENCES = {
	{ 'a', '\a' },
	{ 'b', '\b' },
	{ 'f', '\f' },
	{ 'n', '\n' },
	{ 'r', '\r' },
	{ 't', '\t' },
	{ 'v', '\v' },
	{ '\\', '\\' },
	{ '\'', '\'' },
	{ '\"', '\"' },
};

static std::string lexStringLiteral(bool resolve = true)
{
	std::string str;

	std::streampos mar;
	for (;;)
	{
		%{ /* Begin re2c lexer */
		re2c:yyfill:enable = 0;
		re2c:flags:input = custom;
		re2c:api:style = free-form;
		re2c:define:YYCTYPE   = char;
		re2c:define:YYPEEK    = "in.peek()";
		re2c:define:YYSKIP    = "do { in.ignore(); if (in.eof()) throw std::runtime_error(\"Unclosed string\"); } while(0);";
		re2c:define:YYBACKUP  = "mar = in.tellg();";
		re2c:define:YYRESTORE = "in.seekg(mar);";
		
		"\n" { throw std::runtime_error("Unclosed string"); }

		"\\"([abfnrtv]|"\\"|"\'"|"\"") { if (resolve) { str.push_back(ESCAPE_SEQUENCES.at(yych)); } else { str.push_back('\\'); str.push_back(yych); } continue; }
		[^\\"\""] { str.push_back(yych); continue; }

		"\"" { break; }
		* { throw std::runtime_error("Illegal escape character in string"); }
		%}
	}

	return str;
}

static std::string lineAsString()
{
	std::string str;
	char c;
	std::streampos mar;
	for (;;)
	{
		%{ /* Begin re2c lexer */
		re2c:yyfill:enable = 0;
		re2c:flags:input = custom;
		re2c:api:style = free-form;
		re2c:define:YYCTYPE   = char;
		re2c:define:YYPEEK    = "in.peek()";
		re2c:define:YYSKIP    = "do { in.ignore(); if (in.eof()) throw std::runtime_error(\"Unclosed line as string\"); } while(0);";
		re2c:define:YYBACKUP  = "mar = in.tellg();";
		re2c:define:YYRESTORE = "in.seekg(mar);";
		
		[\x00]|"#"|[\n\r] { c = yych; break; }
		* { str.push_back(yych); continue; }
		%}
	}

	in.put(c);
	return str;
}

static std::string blockAsString()
{
	std::string str;
	
	for (;;)
	{
		char c;
		in.get(c);
		str.push_back(c);
		if (str.length() >= 4)
		{
			std::string eos = str.substr(str.length() - 4, 4);
			for (char& c : eos)
			{
				c = std::tolower(c);
			}
			if (eos == ".end") break;
		}
	}

	return str.substr(0, str.length() - 4);
}

static std::string getString(std::streampos start, std::streampos end)
{
	std::string buffer;
	buffer.resize(end - start);
	in.seekg(start);
	in.read(buffer.data(), end - start);
	return buffer;
}

static char getChar(std::streampos start, std::streampos end)
{
	in.seekg(start);
	char c;
	in.get(c);
	in.seekg(end);
	return c;
}

#define GET_STRING() getString(s, e)
#define GET_CHAR() getChar(s, e)

static std::vector<std::string> argumentList()
{
	std::string argument;
	std::vector<std::string> arguments;
	std::streampos mar, s, e;
	/*!stags:re2c format = 'std::streampos @@;'; */
	bool empty = true;
	for (;;)
	{
		%{ /* Begin re2c lexer */
		re2c:yyfill:enable = 0;
		re2c:flags:input = custom;
		re2c:api:style = free-form;
		re2c:define:YYCTYPE   = char;
		re2c:define:YYPEEK    = "in.peek()";
		re2c:define:YYSKIP    = "do { in.ignore(); if (in.eof()) throw std::runtime_error(\"Unclosed argument list\"); } while(0);";
		re2c:define:YYBACKUP  = "mar = in.tellg();";
		re2c:define:YYRESTORE = "in.seekg(mar);";
		re2c:define:YYSTAGP      = "@@{tag} = in.eof() ? 0 : in.tellg();";
		re2c:define:YYSTAGN      = "@@{tag} = 0;";
		re2c:define:YYSHIFTSTAG  = "@@{tag} += @@{shift};";
        re2c:flags:tags = 1;
		
		@s [a-zA-Z_][a-zA-Z_0-9]* @e
		{
			std::string identifier = GET_STRING();
			if (!macroCallStack.empty())
			{
				MacroCall& mc = macroCallStack.back();
				auto it = std::find(mc.paramaters.begin(), mc.paramaters.end(), identifier);
				if (it != mc.paramaters.end())
				{
					auto index = std::distance(mc.paramaters.begin(), it);
					argument += mc.arguments[index];
					empty = false;
					continue;
				}
			}
			
			if (assembler->macros.count(identifier))
			{
				argument += assembler->macros[identifier];
				continue;
			}

			argument += identifier;
			empty = false;
		}

		")" { if (!empty) { arguments.push_back(argument); } break; }
		"\"" { argument += std::string(1, '\"') + lexStringLiteral(false) + std::string(1, '\"'); empty = false; }
		"," { arguments.push_back(argument); argument = ""; continue; }
		* { argument.push_back(yych); empty = false; continue; }
		%}
	}

	return arguments;
}

yy::parser::symbol_type yy::yylex()
{
    std::streampos mar, s, e;
    /*!stags:re2c format = 'std::streampos @@;'; */

#define TOKEN(name) do { return parser::make_##name(loc); } while(0)
#define TOKENV(name, ...) do { return parser::make_##name(__VA_ARGS__, loc); } while(0)

	switch (flag)
	{
	case CTXFlag::LINE_AS_STRING:
		TOKENV(STRING, lineAsString());
		break;
	case CTXFlag::BLOCK_AS_STRING:
		TOKENV(STRING, blockAsString());
		break;
	case CTXFlag::ARGUMENT_LIST:
		TOKENV(ARGUMENT_LIST, argumentList());
		break;
	default:
		break;
	}

	for (;;)
	{
		%{ /* Begin re2c lexer */
		re2c:yyfill:enable = 0;
		re2c:flags:input = custom;
		re2c:api:style = free-form;
		re2c:define:YYCTYPE      = char;
		re2c:define:YYPEEK       = "in.peek()";
		re2c:define:YYSKIP       = "do { in.ignore(); if (in.eof()) TOKEN(END_OF_FILE); } while(0);";
		re2c:define:YYBACKUP     = "mar = in.tellg();";
		re2c:define:YYRESTORE    = "in.seekg(mar);";
		re2c:define:YYSTAGP      = "@@{tag} = in.eof() ? 0 : in.tellg();";
		re2c:define:YYSTAGN      = "@@{tag} = 0;";
		re2c:define:YYSHIFTSTAG  = "@@{tag} += @@{shift};";
        re2c:flags:tags = 1;

		// Directives
		'.text'       { TOKEN(TEXT); }
		'.data'       { TOKEN(DATA); }
		'.word'       { TOKEN(WORD); }
		'.byte'       { TOKEN(BYTE); }
		'.ascii'      { TOKEN(ASCII); }
		'.asciiz'     { TOKEN(ASCIIZ); }
		'.align'      { TOKEN(ALIGN); }
		'.space'      { TOKEN(SPACE); }
		'.include'    { TOKEN(INCLUDE); }
		'.error'      { TOKEN(ERROR); }
		'.message'    { TOKEN(MESSAGE); }
		'.macro'      { TOKEN(MACRO); }
		'.dbg'        { TOKEN(DBG); }
		'.dbgbp'      { TOKEN(DBGBP); }
		'.define'     { TOKEN(DEFINE); }

		// Instructions
		'add'           { TOKEN(ADD); }
		'addi'         { TOKEN(ADDI); }
		'addiu'       { TOKEN(ADDIU); }
		'addu'         { TOKEN(ADDU); }
		'and'           { TOKEN(AND); }
		'andi'         { TOKEN(ANDI); }
		'beq'           { TOKEN(BEQ); }
		'bgez'         { TOKEN(BGEZ); }
		'bgezal'     { TOKEN(BGEZAL); }
		'bgtz'         { TOKEN(BGTZ); }
		'blez'         { TOKEN(BLEZ); }
		'bltz'         { TOKEN(BLTZ); }
		'bltzal'     { TOKEN(BLTZAL); }
		'bne'           { TOKEN(BNE); }
		'div'           { TOKEN(DIV); }
		'divu'         { TOKEN(DIVU); }
		'j'               { TOKEN(J); }
		'jal'           { TOKEN(JAL); }
		'jr'             { TOKEN(JR); }
		'lb'             { TOKEN(LB); }
		'lui'           { TOKEN(LUI); }
		'lw'             { TOKEN(LW); }
		'mfhi'         { TOKEN(MFHI); }
		'mflo'         { TOKEN(MFLO); }
		'mult'         { TOKEN(MULT); }
		'multu'       { TOKEN(MULTU); }
		'or'             { TOKEN(OR); }
		'ori'           { TOKEN(ORI); }
		'sb'             { TOKEN(SB); }
		'sll'           { TOKEN(SLL); }
		'sllv'         { TOKEN(SLLV); }
		'slt'           { TOKEN(SLT); }
		'slti'         { TOKEN(SLTI); }
		'sltiu'       { TOKEN(SLTIU); }
		'sltu'         { TOKEN(SLTU); }
		'sne'           { TOKEN(SNE); }
		'seq'           { TOKEN(SEQ); }
		'sra'           { TOKEN(SRA); }
		'srl'           { TOKEN(SRL); }
		'srlv'         { TOKEN(SRLV); }
		'sub'           { TOKEN(SUB); }
		'subu'         { TOKEN(SUBU); }
		'sw'             { TOKEN(SW); }
		'sys'           { TOKEN(SYS); }
		'xor'           { TOKEN(XOR); }
		'xori'         { TOKEN(XORI); }
		'jalr'         { TOKEN(JALR); }
		'nor'           { TOKEN(NOR); }

		// Pseudoinstructions
		'copy'         { TOKEN(COPY); }
		'clr'           { TOKEN(CLR); }
		'b'               { TOKEN(B); }
		'bal'           { TOKEN(BAL); }
		'bgt'           { TOKEN(BGT); }
		'blt'           { TOKEN(BLT); }
		'bge'           { TOKEN(BGE); }
		'ble'           { TOKEN(BLE); }
		'bgtu'         { TOKEN(BGTU); }
		'beqz'         { TOKEN(BEQZ); }
		'rem'           { TOKEN(REM); }
		'li'             { TOKEN(LI); }
		'la'             { TOKEN(LA); }
		'nop'           { TOKEN(NOP); }
		'not'           { TOKEN(NOT); }
		'pushw'       { TOKEN(PUSHW); }
		'popw'         { TOKEN(POPW); }
		'pushb'       { TOKEN(PUSHB); }
		'popb'         { TOKEN(POPB); }
		'ret'           { TOKEN(RET); }
		'call'         { TOKEN(CALL); }
		'enter'       { TOKEN(ENTER); }

		// Identifier
		@s [a-zA-Z_][a-zA-Z_0-9]* @e
		{
			std::string identifier = GET_STRING();
			if (!macroCallStack.empty())
			{
				MacroCall& mc = macroCallStack.back();
				auto it = std::find(mc.paramaters.begin(), mc.paramaters.end(), identifier);
				if (it != mc.paramaters.end())
				{
					auto index = std::distance(mc.paramaters.begin(), it);
					in.pushString(mc.arguments[index]);
					continue;
				}
			}
			
			if (assembler->macros.count(identifier))
			{
				in.pushString(assembler->macros[identifier]);
				continue;
			}
			
			TOKENV(IDENTIFIER, identifier);
		}

		// Register
		"$" @s ("zero"|"at"|"gp"|"sp"|"fp"|"ra"|"a"[0-3]|"v"[0-1]|"t"[0-9]|"s"[0-7]|"k"[0-1]) @e { TOKENV(REGISTER, REGISTER_NAMES.at(GET_STRING())); }
		"$" @s ([0-9]|[1-2][0-9]|"3"[0-1]) @e { TOKENV(REGISTER, std::stoi(GET_STRING())); }

		// Literals
		@s [-+]?[0-9]+ @e      { TOKENV(LITERAL, std::stoi(GET_STRING(), nullptr, 10)); }
		@s "0b"[01]+ @e        { TOKENV(LITERAL, std::stoi(GET_STRING(), nullptr, 2)); }
		@s "0x"[0-9a-fA-F]+ @e { TOKENV(LITERAL, std::stoi(GET_STRING(), nullptr, 16)); }
		"'\\" @s ."'" @e       { TOKENV(LITERAL, ESCAPE_SEQUENCES.at(GET_CHAR())); }
		"'" @s [^\\"\'"]"'" @e { TOKENV(LITERAL, GET_CHAR()); }

		// String
		"\""                   { TOKENV(STRING, lexStringLiteral()); }

		// Whitespace
		"\r\n"|[\r\n]          { loc.lines(); loc.step(); TOKEN(END_OF_LINE); }
		[ \t\v\b\f]            { loc.columns(); continue; }

		// Comment
		@s "#"[^\r\n]* @e { continue; }

		// Single character operators
		@s [:,+()] @e { return parser::symbol_type(parser::token_type(GET_CHAR()), loc); }

		* { throw std::runtime_error(std::string("Invalid character of value: " + std::to_string(GET_CHAR())).c_str()); }
		%}
	}
}

void yy::parser::error(const location_type& l, const std::string& message)
{
    std::cerr << l.begin.filename->c_str() << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << message << '\n';
	char buffer[20];
	in.read(buffer, 19);
	buffer[19] = 0;
	std::cerr << buffer << std::endl;
}

namespace kasm
{
    void Assembler::assemble(const std::string& asmPath, const std::string& programPath, const std::string& symbolTablePath)
    {
        labelLocations.clear();
        unresolvedAddressLocations.clear();
		macros.clear();
		macroFunctions.clear();

		in.setCallback(eofCallback);
		in.open(asmPath);
		flag = CTXFlag::None;

		loc.begin.filename = &in.getIdentifier();
		loc.end.filename = &in.getIdentifier();

		binary.open(programPath);

		assembler = this;
        yy::parser parser;
        if (parser.parse()) return;

        for (AddressData unresolvedAddressLocation : unresolvedAddressLocations)
        {
			binary.setLocation(unresolvedAddressLocation.position);
			
			resolveAddress(unresolvedAddressLocation, MUST_RESOLVE);

			if (unresolvedAddressLocation.type == AddressType::DirectAddressAbsoluteByte)
			{
				binary.writeByte(static_cast<std::uint8_t>(unresolvedAddressLocation.instructionData.instruction));
			}
			else if (unresolvedAddressLocation.type == AddressType::DirectAddressAbsoluteLoad)
			{
				SplitWord l = { unresolvedAddressLocation.instructionData.instruction };
				INSTRUCTION_RL(LUI, unresolvedAddressLocation.reg, l.hi);
				INSTRUCTION_RRL(ORI, unresolvedAddressLocation.reg, unresolvedAddressLocation.reg, l.lo);
			}
			else
			{
				binary.writeWord(unresolvedAddressLocation.instructionData.instruction);
			}
        }

        binary.setLocation(BinaryBuilder::END);
        binary.align(INSTRUCTION_SIZE);
		binary.close();

		if (!symbolTablePath.empty())
		{
			saveSymbolTable(symbolTablePath);
		}
    }
}
