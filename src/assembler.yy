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
#include <fstream>
#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#include "compoundInputFileStream.hpp"

struct lexcontext;
}//%code requires

%param { lexcontext& ctx }

%code
{
enum class CTXFlag
{
	None,
	LINE_AS_STRING
};

struct lexcontext
{
	lexcontext(const std::string& aFileName)
		: in(aFileName), flag(CTXFlag::None)
	{
		loc.begin.filename = &in.getIdentifier();
		loc.end.filename = &in.getIdentifier();
	};

	kasm::CompoundInputFileStream in;
	yy::location loc;
	kasm::Assembler* assembler;
	CTXFlag flag;
};

namespace yy { parser::symbol_type yylex(lexcontext& ctx); }

#define INSTRUCTION_RRR(op, r0, r1, r2) {                           \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.register1 = r1;                                 \
	instructionData.register2 = r2;                                 \
	ctx.assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RRL(op, r0, r1, l) {                            \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.register1 = r1;                                 \
	instructionData.immediate = l;                                  \
	ctx.assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RRA(op, r0, r1, a, t) {                           \
	kasm::InstructionData instructionData;                            \
	instructionData.opcode = kasm::Opcode::##op;                      \
	instructionData.register0 = r0;                                   \
	instructionData.register1 = r1;                                   \
	a.type = kasm::AddressType::##t;                                  \
	a.position = GET_LOC();                                           \
	a.instructionData = instructionData;                              \
	ctx.assembler->resolveAddress(a);                                 \
	ctx.assembler->binary.writeWord(a.instructionData.instruction); } \

#define INSTRUCTION_RR(op, r0, r1) {                                \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.register1 = r1;                                 \
	ctx.assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RL(op, r0, l) {                                 \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	instructionData.immediate = l;                                  \
	ctx.assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_RA(op, r0, a, t) {                                \
	kasm::InstructionData instructionData;                            \
	instructionData.opcode = kasm::Opcode::##op;                      \
	instructionData.register0 = r0;                                   \
	a.type = kasm::AddressType::##t;                                  \
	a.position = GET_LOC();                                           \
	a.instructionData = instructionData;                              \
	ctx.assembler->resolveAddress(a);                                 \
	ctx.assembler->binary.writeWord(a.instructionData.instruction); } \

#define INSTRUCTION_R(op, r0) {                                     \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	instructionData.register0 = r0;                                 \
	ctx.assembler->binary.writeWord(instructionData.instruction); } \

#define INSTRUCTION_A(op, a, t) {                                     \
	kasm::InstructionData instructionData;                            \
	instructionData.opcode = kasm::Opcode::##op;                      \
	a.type = kasm::AddressType::##t;                                  \
	a.position = GET_LOC();                                           \
	a.instructionData = instructionData;                              \
	ctx.assembler->resolveAddress(a);                                 \
	ctx.assembler->binary.writeWord(a.instructionData.instruction); } \

#define INSTRUCTION_O(op) {                                         \
	kasm::InstructionData instructionData;                          \
	instructionData.opcode = kasm::Opcode::##op;                    \
	ctx.assembler->binary.writeWord(instructionData.instruction); } \

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

#define ASSEMBLER_ASSERT(condition, message) assert((condition) && message)
#define GET_LOC() ctx.assembler->binary.getLocation()

}//%code

%token END_OF_FILE 0 END_OF_LINE
%token IDENTIFIER LITERAL STRING REGISTER

%token TEXT DATA WORD BYTE ASCII ASCIIZ ALIGN SPACE INCLUDE ERROR MESSAGE MACRO END DBG DEFINE

%token ADD ADDI ADDIU ADDU AND ANDI BEQ BGEZ BGEZAL BGTZ BLEZ BLTZ BLTZAL BNE
%token DIV DIVU J JAL JR LB LUI LW MFHI MFLO MULT MULTU OR ORI SB SLL SLLV NOR
%token SLT SLTI SLTIU SLTU SRA SRL SRLV SUB SUBU SW SYS XOR XORI JALR

%token COPY CLR B BAL BGT BLT BGE BLE BGTU BEQZ REM LI LA NOP NOT

%type<std::string> IDENTIFIER STRING
%type<std::uint32_t> LITERAL REGISTER
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
    : IDENTIFIER ':' statement { $$ = $3; ctx.assembler->defineLabel($1, $3); }
	| END_OF_LINE statement { $$ = $2; }
	| END_OF_FILE { $$ = GET_LOC(); }
	// Directives
	| TEXT end_of_statement { $$ = GET_LOC(); ctx.assembler->binary.setSegmentType(kasm::BinaryBuilder::SegmentType::TEXT); }
	| DATA end_of_statement { $$ = GET_LOC(); ctx.assembler->binary.setSegmentType(kasm::BinaryBuilder::SegmentType::DATA); }
    | WORD literal_argument end_of_statement
    {
		ASSEMBLER_ASSERT(ctx.assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "word must be in data segment");
        ctx.assembler->binary.align(kasm::INSTRUCTION_SIZE);
        $$ = GET_LOC(); 
		for (std::variant<std::uint32_t, kasm::AddressData> word : $2)
		{
			if(std::holds_alternative<kasm::AddressData>(word))
			{
				kasm::AddressData addr = std::get<kasm::AddressData>(word);
				addr.type = kasm::AddressType::DirectAddressAbsoluteWord;
				addr.position = GET_LOC();
				ctx.assembler->resolveAddress(addr);
            	ctx.assembler->binary.writeWord(addr.instructionData.instruction);
			}
			else
			{
            	ctx.assembler->binary.writeWord(std::get<std::uint32_t>(word));
			}
        }
    }
    | BYTE literal_argument end_of_statement
    {
		ASSEMBLER_ASSERT(ctx.assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "byte must be in data segment");
        $$ = GET_LOC(); 
		for (std::variant<std::uint32_t, kasm::AddressData> byte : $2)
        {
			if(std::holds_alternative<kasm::AddressData>(byte))
			{
				kasm::AddressData addr = std::get<kasm::AddressData>(byte);
				addr.type = kasm::AddressType::DirectAddressAbsoluteByte;
				addr.position = GET_LOC();
				ctx.assembler->resolveAddress(addr);
            	ctx.assembler->binary.writeByte(static_cast<std::uint8_t>(addr.instructionData.instruction));
			}
			else
			{
            	ctx.assembler->binary.writeByte(static_cast<std::uint8_t>(std::get<std::uint32_t>(byte)));
			}
        }
    }
    | ASCII STRING end_of_statement
	{
		ASSEMBLER_ASSERT(ctx.assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "ascii must be in data segment");
		$$ = GET_LOC(); 
		ctx.assembler->binary.writeString($2.c_str(), $2.size());
	}
    | ASCIIZ STRING end_of_statement
	{
		ASSEMBLER_ASSERT(ctx.assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "asciiz must be in data segment");
		$$ = GET_LOC(); 
		ctx.assembler->binary.writeString($2.c_str(), $2.size() + 1);
	}
    | ALIGN LITERAL end_of_statement
    {
		ASSEMBLER_ASSERT(ctx.assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "align must be in data segment");
        $$ = GET_LOC(); 
		unsigned int alignment = 1;
        for (int i = 0; i < $2; i++)
        {
            alignment *= 2;
        }
        ctx.assembler->binary.align(alignment);
    }
    | SPACE LITERAL end_of_statement
	{
		ASSEMBLER_ASSERT(ctx.assembler->binary.getSegmentType() == kasm::BinaryBuilder::SegmentType::DATA, "space must be in data segment");
		$$ = GET_LOC(); 
		ctx.assembler->binary.pad($2);
	}
	| INCLUDE STRING end_of_statement { ctx.in.include($2); } statement { $$ = $5; }
	| ERROR   STRING end_of_statement statement { $$ = $4; std::cout << "ERROR: " << $2 << std::endl; throw std::exception("Assembler user defined error"); }
	| MESSAGE STRING end_of_statement statement { $$ = $4; std::cout << "MESSAGE: " << $2 << std::endl; }
	| DBG     STRING end_of_statement statement { $$ = $4; ctx.in.pushString($2); }
	| DEFINE IDENTIFIER { ctx.flag = CTXFlag::LINE_AS_STRING; } STRING { ctx.flag = CTXFlag::None; ctx.assembler->defineMacro($2, $4); } end_of_statement statement { $$ = $7; }
	| MACRO IDENTIFIER '(' identifier_list ')' statement_list END end_of_statement statement
	{
		$$ = $9;
	}
	| IDENTIFIER '(' argument_list ')' end_of_statement
	{
		$$ = GET_LOC();
	}

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
	| COPY    REGISTER ',' REGISTER                   end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(OR, $2, $4, kasm::ZERO); }
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
	| BEQZ      REGISTER ',' direct_address           end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRA(BEQ, $2, kasm::ZERO, $4, DirectAddressOffset); }
	| BEQ    REGISTER ',' LITERAL  ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ORI, kasm::AT, kasm::ZERO, $4); INSTRUCTION_RRA(BEQ, $2, kasm::AT, $6, DirectAddressOffset); }
	| BNE    REGISTER ',' LITERAL  ',' direct_address end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRL(ORI, kasm::AT, kasm::ZERO, $4); INSTRUCTION_RRA(BNE, $2, kasm::AT, $6, DirectAddressOffset); }
	| MULT   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(MULT, $4, $6); INSTRUCTION_R(MFLO, $2); }
	| DIV    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(DIV, $4, $6); INSTRUCTION_R(MFLO, $2); }
	| REM    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { $$ = GET_LOC(); INSTRUCTION_RR(DIV, $4, $6); INSTRUCTION_R(MFHI, $2); }
	| NOT    REGISTER ',' REGISTER                    end_of_statement { $$ = GET_LOC(); INSTRUCTION_RRR(NOR, $2, $4, kasm::ZERO); }
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
		ctx.assembler->resolveAddress($4);
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
	: identifier_list IDENTIFIER
	| %empty
	;

argument_list
	: IDENTIFIER
	| LITERAL
	| direct_address
	| address
	| STRING
	| REGISTER
	| %empty
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

std::string lexStringLiteral(lexcontext& ctx)
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
		re2c:define:YYPEEK    = "ctx.in.peek()";
		re2c:define:YYSKIP    = "do { ctx.in.ignore(); if (ctx.in.eof()) throw std::exception(\"Unclosed string\"); } while(0);";
		re2c:define:YYBACKUP  = "mar = ctx.in.tellg();";
		re2c:define:YYRESTORE = "ctx.in.seekg(mar);";
		
		"\n" { throw std::exception("Unclosed string"); }

		"\\"([abfnrtv]|"\\"|"\'"|"\"") { str.push_back(ESCAPE_SEQUENCES.at(yych)); continue; }
		[^\\"\""] { str.push_back(yych); continue; }

		"\"" { break; }
		* { throw std::exception("Illegal escape character in string"); }
		%}
	}

	return str;
}

std::string lineAsString(lexcontext& ctx)
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
		re2c:define:YYPEEK    = "ctx.in.peek()";
		re2c:define:YYSKIP    = "do { ctx.in.ignore(); if (ctx.in.eof()) throw std::exception(\"Unclosed string\"); } while(0);";
		re2c:define:YYBACKUP  = "mar = ctx.in.tellg();";
		re2c:define:YYRESTORE = "ctx.in.seekg(mar);";
		
		[\x00]|"#"|[\n\r] { c = yych; break; }
		* { str.push_back(yych); continue; }
		%}
	}

	ctx.in.put(c);
	return str;
}

yy::parser::symbol_type yy::yylex(lexcontext& ctx)
{
    std::streampos mar, s, e;
    /*!stags:re2c format = 'std::streampos @@;'; */

	auto getString = [](kasm::CompoundInputFileStream& in, std::streampos start, std::streampos end) -> std::string
	{
		std::string buffer;
		buffer.resize(end - start);
		in.seekg(start);
		in.read(buffer.data(), end - start);
		return buffer;
	};

	auto getChar = [](kasm::CompoundInputFileStream& in, std::streampos start, std::streampos end) -> char
	{
		in.seekg(start);
		char c;
		in.get(c);
		in.seekg(end);
		return c;
	};

#define GET_STRING() getString(ctx.in, s, e)
#define GET_CHAR() getChar(ctx.in, s, e)
#define TOKEN(name) do { return parser::make_##name(ctx.loc); } while(0)
#define TOKENV(name, ...) do { return parser::make_##name(__VA_ARGS__, ctx.loc); } while(0)

	switch (ctx.flag)
	{
	case CTXFlag::LINE_AS_STRING:
		TOKENV(STRING, lineAsString(ctx));
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
		re2c:define:YYPEEK       = "ctx.in.peek()";
		re2c:define:YYSKIP       = "do { ctx.in.ignore(); if (ctx.in.eof()) TOKEN(END_OF_FILE); } while(0);";
		re2c:define:YYBACKUP     = "mar = ctx.in.tellg();";
		re2c:define:YYRESTORE    = "ctx.in.seekg(mar);";
		re2c:define:YYSTAGP      = "@@{tag} = ctx.in.eof() ? 0 : ctx.in.tellg();";
		re2c:define:YYSTAGN      = "@@{tag} = 0;";
		re2c:define:YYSHIFTSTAG  = "@@{tag} += @@{shift};";
        re2c:flags:tags = 1;

		// Directives
		".text"|".TEXT"       { TOKEN(TEXT); }
		".data"|".DATA"       { TOKEN(DATA); }
		".word"|".WORD"       { TOKEN(WORD); }
		".byte"|".BYTE"       { TOKEN(BYTE); }
		".ascii"|".ASCII"     { TOKEN(ASCII); }
		".asciiz"|".ASCIIZ"   { TOKEN(ASCIIZ); }
		".align"|".ALIGN"     { TOKEN(ALIGN); }
		".space"|".SPACE"     { TOKEN(SPACE); }
		".include"|".INCLUDE" { TOKEN(INCLUDE); }
		".error"|".ERROR"     { TOKEN(ERROR); }
		".message"|".MESSAGE" { TOKEN(MESSAGE); }
		".macro"|".MACRO"     { TOKEN(MACRO); }
		".end"|".END"         { TOKEN(END); }
		".dbg"|".DBG"         { TOKEN(DBG); }
		".define"|".DEFINE"   { TOKEN(DEFINE); }

		// Instructions
		"add"|"ADD"           { TOKEN(ADD); }
		"addi"|"ADDI"         { TOKEN(ADDI); }
		"addiu"|"ADDIU"       { TOKEN(ADDIU); }
		"addu"|"ADDU"         { TOKEN(ADDU); }
		"and"|"AND"           { TOKEN(AND); }
		"andi"|"ANDI"         { TOKEN(ANDI); }
		"beq"|"BEQ"           { TOKEN(BEQ); }
		"bgez"|"BGEZ"         { TOKEN(BGEZ); }
		"bgezal"|"BGEZAL"     { TOKEN(BGEZAL); }
		"bgtz"|"BGTZ"         { TOKEN(BGTZ); }
		"blez"|"BLEZ"         { TOKEN(BLEZ); }
		"bltz"|"BLTZ"         { TOKEN(BLTZ); }
		"bltzal"|"BLTZAL"     { TOKEN(BLTZAL); }
		"bne"|"BNE"           { TOKEN(BNE); }
		"div"|"DIV"           { TOKEN(DIV); }
		"divu"|"DIVU"         { TOKEN(DIVU); }
		"j"|"J"               { TOKEN(J); }
		"jal"|"JAL"           { TOKEN(JAL); }
		"jr"|"JR"             { TOKEN(JR); }
		"lb"|"LB"             { TOKEN(LB); }
		"lui"|"LUI"           { TOKEN(LUI); }
		"lw"|"LW"             { TOKEN(LW); }
		"mfhi"|"MFHI"         { TOKEN(MFHI); }
		"mflo"|"MFLO"         { TOKEN(MFLO); }
		"mult"|"MULT"         { TOKEN(MULT); }
		"multu"|"MULTU"       { TOKEN(MULTU); }
		"or"|"OR"             { TOKEN(OR); }
		"ori"|"ORI"           { TOKEN(ORI); }
		"sb"|"SB"             { TOKEN(SB); }
		"sll"|"SLL"           { TOKEN(SLL); }
		"sllv"|"SLLV"         { TOKEN(SLLV); }
		"slt"|"SLT"           { TOKEN(SLT); }
		"slti"|"SLTI"         { TOKEN(SLTI); }
		"sltiu"|"SLTIU"       { TOKEN(SLTIU); }
		"sltu"|"SLTU"         { TOKEN(SLTU); }
		"sra"|"SRA"           { TOKEN(SRA); }
		"srl"|"SRL"           { TOKEN(SRL); }
		"srlv"|"SRLV"         { TOKEN(SRLV); }
		"sub"|"SUB"           { TOKEN(SUB); }
		"subu"|"SUBU"         { TOKEN(SUBU); }
		"sw"|"SW"             { TOKEN(SW); }
		"sys"|"SYS"           { TOKEN(SYS); }
		"xor"|"XOR"           { TOKEN(XOR); }
		"xori"|"XORI"         { TOKEN(XORI); }
		"jalr"|"JALR"         { TOKEN(JALR); }
		"nor"|"NOR"           { TOKEN(NOR); }

		// Pseudoinstructions
		"copy"|"COPY"         { TOKEN(COPY); }
		"clr"|"CLR"           { TOKEN(CLR); }
		"b"|"B"               { TOKEN(B); }
		"bal"|"BAL"           { TOKEN(BAL); }
		"bgt"|"BGT"           { TOKEN(BGT); }
		"blt"|"BLT"           { TOKEN(BLT); }
		"bge"|"BGE"           { TOKEN(BGE); }
		"ble"|"BLE"           { TOKEN(BLE); }
		"bgtu"|"BGTU"         { TOKEN(BGTU); }
		"BEQZ"|"BEQZ"         { TOKEN(BEQZ); }
		"rem"|"REM"           { TOKEN(REM); }
		"li"|"LI"             { TOKEN(LI); }
		"la"|"LA"             { TOKEN(LA); }
		"nop"|"NOP"           { TOKEN(NOP); }
		"not"|"NOT"           { TOKEN(NOT); }

		// Identifier
		@s [a-zA-Z_][a-zA-Z_0-9]* @e
		{
			std::string identifier = GET_STRING();
			if (ctx.assembler->macros.count(identifier))
			{
				ctx.in.pushString(ctx.assembler->macros[identifier]);
				continue;
			}
			else
			{
				TOKENV(IDENTIFIER, identifier);
			}
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
		"\""                   { TOKENV(STRING, lexStringLiteral(ctx)); }

		// Whitespace
		"\r\n"|[\r\n]          { ctx.loc.lines(); ctx.loc.step(); TOKEN(END_OF_LINE); }
		[ \t\v\b\f]            { ctx.loc.columns(); continue; }

		// Comment
		@s "#"[^\r\n]* @e { continue; }

		// Single character operators
		@s [:,+()] @e { return parser::symbol_type(parser::token_type(GET_CHAR()), ctx.loc); }

		@s * @e { throw std::exception(std::string("Invalid character of value: " + std::to_string(GET_CHAR())).c_str()); }
		%}
	}
}

void yy::parser::error(const location_type& l, const std::string& message)
{
    std::cerr << l.begin.filename->c_str() << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << message << '\n';
}

namespace kasm
{
    void Assembler::assemble(const std::string& asmPath, const std::string& programPath)
    {
        labelLocations.clear();
        unresolvedAddressLocations.clear();

        lexcontext ctx(asmPath);
		binary.open(programPath);

		ctx.assembler = this;
        yy::parser parser(ctx);
        parser.parse();

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
    }
}
