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
#include <string>
#include <variant>
#include <vector>

struct lexcontext;
}//%code requires

%param { lexcontext& ctx }

%code
{
struct lexcontext
{
	const char* cursor;
	yy::location loc;
    kasm::Assembler* assembler;
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
	a.position = ctx.assembler->binary.getLocation();                 \
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
	a.position = ctx.assembler->binary.getLocation();                 \
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
	a.position = ctx.assembler->binary.getLocation();                 \
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

}//%code

%token END_OF_FILE 0 END_OF_LINE
%token IDENTIFIER LITERAL STRING REGISTER

%token TEXT DATA WORD BYTE ASCII ASCIIZ ALIGN SPACE

%token ADD ADDI ADDIU ADDU AND ANDI BEQ BGEZ BGEZAL BGTZ BLEZ BLTZ BLTZAL BNE
%token DIV DIVU J JAL JR LB LUI LW MFHI MFLO MULT MULTU OR ORI SB SLL SLLV NOR
%token SLT SLTI SLTIU SLTU SRA SRL SRLV SUB SUBU SW SYS XOR XORI JALR

%token MOV CLR B BAL BGT BLT BGE BLE BGTU BEQZ REM LI LA NOP NOT

%type<std::string> IDENTIFIER STRING
%type<std::uint32_t> LITERAL REGISTER
%type<std::vector<std::variant<std::uint32_t, kasm::AddressData>>> literal_list literal_argument
%type<kasm::AddressData> address direct_address

%start statement_list

%%

statement_list
	: statement_list statement
	| %empty
	;

statement
    : IDENTIFIER ':' { ctx.assembler->defineLabel($1, ctx.assembler->binary.getLocation()); } statement
    | IDENTIFIER ':' { ctx.assembler->defineLabel($1, ctx.assembler->binary.getLocation()); } END_OF_FILE
	| END_OF_LINE
	// Directives
	| TEXT end_of_statement { INSTRUCTION_O(TEXT); }
	| DATA end_of_statement { INSTRUCTION_O(DATA); }
    | WORD literal_argument end_of_statement
    {
        ctx.assembler->binary.align(kasm::INSTRUCTION_SIZE);
        for (std::variant<std::uint32_t, kasm::AddressData> word : $2)
		{
			if(std::holds_alternative<kasm::AddressData>(word))
			{
				kasm::AddressData addr = std::get<kasm::AddressData>(word);
				addr.type = kasm::AddressType::DirectAddressAbsoluteWord;
				addr.position = ctx.assembler->binary.getLocation();
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
        for (std::variant<std::uint32_t, kasm::AddressData> byte : $2)
        {
			if(std::holds_alternative<kasm::AddressData>(byte))
			{
				kasm::AddressData addr = std::get<kasm::AddressData>(byte);
				addr.type = kasm::AddressType::DirectAddressAbsoluteByte;
				addr.position = ctx.assembler->binary.getLocation();
				ctx.assembler->resolveAddress(addr);
            	ctx.assembler->binary.writeByte(static_cast<std::uint8_t>(addr.instructionData.instruction));
			}
			else
			{
            	ctx.assembler->binary.writeByte(static_cast<std::uint8_t>(std::get<std::uint32_t>(byte)));
			}
        }
    }
    | ASCII STRING end_of_statement { ctx.assembler->binary.writeString($2.c_str(), $2.size()); }
    | ASCIIZ STRING end_of_statement { ctx.assembler->binary.writeString($2.c_str(), $2.size() + 1); }
    | ALIGN LITERAL end_of_statement
    {
        unsigned int alignment = 1;
        for (int i = 0; i < $2; i++)
        {
            alignment *= 2;
        }
        ctx.assembler->binary.align(alignment);
    }
    | SPACE LITERAL end_of_statement { ctx.assembler->binary.pad($2); }

	// Instructions
    | ADD    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(ADD, $2, $4, $6); }
	| ADDI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(ADDI, $2, $4, $6); }
	| ADDIU  REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(ADDIU, $2, $4, $6); }
	| ADDU   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(ADDU, $2, $4, $6); }
	| AND    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(AND, $2, $4, $6); }
	| ANDI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(ANDI, $2, $4, $6); }
    | BEQ    REGISTER ',' REGISTER ',' direct_address end_of_statement { INSTRUCTION_RRA(BEQ, $2, $4, $6, DirectAddressOffset); }
	| BGEZ   REGISTER ',' direct_address              end_of_statement { INSTRUCTION_RA(BGEZ, $2, $4, DirectAddressOffset); }
	| BGEZAL REGISTER ',' direct_address              end_of_statement { INSTRUCTION_RA(BGEZAL, $2, $4, DirectAddressOffset); }
	| BGTZ   REGISTER ',' direct_address              end_of_statement { INSTRUCTION_RA(BGTZ, $2, $4, DirectAddressOffset); }
	| BLEZ   REGISTER ',' direct_address              end_of_statement { INSTRUCTION_RA(BLEZ, $2, $4, DirectAddressOffset); }
	| BLTZ   REGISTER ',' direct_address              end_of_statement { INSTRUCTION_RA(BLTZ, $2, $4, DirectAddressOffset); }
	| BLTZAL REGISTER ',' direct_address              end_of_statement { INSTRUCTION_RA(BLTZAL, $2, $4, DirectAddressOffset); }
	| BNE    REGISTER ',' REGISTER ',' direct_address end_of_statement { INSTRUCTION_RRA(BNE, $2, $4, $6, DirectAddressOffset); }
	| DIV    REGISTER ',' REGISTER                    end_of_statement { INSTRUCTION_RR(DIV, $2, $4); }
	| DIVU   REGISTER ',' REGISTER                    end_of_statement { INSTRUCTION_RR(DIVU, $2, $4); }
	| J      address                                  end_of_statement { INSTRUCTION_A(J, $2, DirectAddressAbsolute); }
	| JAL    address                                  end_of_statement { INSTRUCTION_A(JAL, $2, DirectAddressAbsolute); }
	| JR     REGISTER                                 end_of_statement { INSTRUCTION_R(JR, $2); }
    | LB     REGISTER ',' address                     end_of_statement { INSTRUCTION_RA(LB, $2, $4, IndirectAddressOffset); }
	| LUI    REGISTER ',' LITERAL                     end_of_statement { INSTRUCTION_RL(LUI, $2, $4); }
	| LW     REGISTER ',' address                     end_of_statement { INSTRUCTION_RA(LW, $2, $4, IndirectAddressOffset); }
	| MFHI   REGISTER                                 end_of_statement { INSTRUCTION_R(MFHI, $2); }
	| MFLO   REGISTER                                 end_of_statement { INSTRUCTION_R(MFLO, $2); }
	| MULT   REGISTER ',' REGISTER                    end_of_statement { INSTRUCTION_RR(MULT, $2, $4); }
	| MULTU  REGISTER ',' REGISTER                    end_of_statement { INSTRUCTION_RR(MULTU, $2, $4); }
	| OR     REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(OR, $2, $4, $6); }
	| ORI    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(OR, $2, $4, $6); }
	| SB     REGISTER ',' address                     end_of_statement { INSTRUCTION_RA(SB, $2, $4, IndirectAddressOffset); }
	| SLL    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(SLL, $2, $4, $6); }
	| SLLV   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(SLLV, $2, $4, $6); }
	| SLT    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(SLLV, $2, $4, $6); }
	| SLTI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(SLTI, $2, $4, $6); }
	| SLTIU  REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(SLTIU, $2, $4, $6); }
	| SLTU   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(SLTU, $2, $4, $6); }
	| SRA    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(SRA, $2, $4, $6); }
	| SRL    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(SRL, $2, $4, $6); }
	| SRLV   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(SRLV, $2, $4, $6); }
	| SUB    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(SUB, $2, $4, $6); }
	| SUBU   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(SUBU, $2, $4, $6); }
	| SW     REGISTER ',' address                     end_of_statement { INSTRUCTION_RA(SB, $2, $4, IndirectAddressOffset); }
	| SYS                                             end_of_statement { INSTRUCTION_O(SYS); }
	| XOR    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(XOR, $2, $4, $6); }
	| XORI   REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(XORI, $2, $4, $6); }
	| JALR   REGISTER ',' REGISTER                    end_of_statement { INSTRUCTION_RR(JALR, $2, $4); }
	| NOR    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RRR(NOR, $2, $4, $6); }

	// Pseudoinstructions
	| MOV    REGISTER ',' REGISTER                    end_of_statement { INSTRUCTION_RRL(OR, $2, $4, kasm::ZERO); }
	| CLR    REGISTER                                 end_of_statement { INSTRUCTION_RRL(OR, $2, kasm::ZERO, kasm::ZERO); }
	| ADD    REGISTER ',' REGISTER ',' LITERAL        end_of_statement { INSTRUCTION_RRL(ADDI, $2, $4, $6); }
	| JALR   REGISTER                                 end_of_statement { INSTRUCTION_RR(JALR, $2, kasm::RA); }
	| NOP                                             end_of_statement { INSTRUCTION_RRL(SLL, kasm::ZERO, kasm::ZERO, 0); }
	| B      direct_address                           end_of_statement { INSTRUCTION_RRA(BEQ, kasm::ZERO, kasm::ZERO, $2, DirectAddressOffset); }
	| BAL    direct_address                           end_of_statement { INSTRUCTION_RA(BGEZAL, kasm::ZERO, $2, DirectAddressOffset); }
	| BGT    REGISTER ',' REGISTER ',' direct_address end_of_statement { INSTRUCTION_RRR(SLT, kasm::AT, $4, $2); INSTRUCTION_RRA(BNE, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BLT    REGISTER ',' REGISTER ',' direct_address end_of_statement { INSTRUCTION_RRR(SLT, kasm::AT, $2, $4); INSTRUCTION_RRA(BNE, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BGE    REGISTER ',' REGISTER ',' direct_address end_of_statement { INSTRUCTION_RRR(SLT, kasm::AT, $2, $4); INSTRUCTION_RRA(BEQ, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BLE    REGISTER ',' REGISTER ',' direct_address end_of_statement { INSTRUCTION_RRR(SLT, kasm::AT, $4, $2); INSTRUCTION_RRA(BEQ, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BGTU   REGISTER ',' REGISTER ',' direct_address end_of_statement { INSTRUCTION_RRR(SLTU, kasm::AT, $2, $4); INSTRUCTION_RRA(BEQ, kasm::AT, kasm::ZERO, $6, DirectAddressOffset); }
	| BEQZ      REGISTER ',' direct_address           end_of_statement { INSTRUCTION_RRA(BEQ, $2, kasm::ZERO, $4, DirectAddressOffset); }
	| BEQ    REGISTER ',' LITERAL  ',' direct_address end_of_statement { INSTRUCTION_RRL(ORI, kasm::AT, kasm::ZERO, $4); INSTRUCTION_RRA(BEQ, $2, kasm::AT, $6, DirectAddressOffset); }
	| BNE    REGISTER ',' LITERAL  ',' direct_address end_of_statement { INSTRUCTION_RRL(ORI, kasm::AT, kasm::ZERO, $4); INSTRUCTION_RRA(BNE, $2, kasm::AT, $6, DirectAddressOffset); }
	| MULT   REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RR(MULT, $4, $6); INSTRUCTION_R(MFLO, $2); }
	| DIV    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RR(DIV, $4, $6); INSTRUCTION_R(MFLO, $2); }
	| REM    REGISTER ',' REGISTER ',' REGISTER       end_of_statement { INSTRUCTION_RR(DIV, $4, $6); INSTRUCTION_R(MFHI, $2); }
	| NOT    REGISTER ',' REGISTER                    end_of_statement { INSTRUCTION_RRR(NOR, $2, $4, kasm::ZERO); }
	| LI     REGISTER ',' LITERAL                     end_of_statement
	{
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
		$4.type = kasm::AddressType::DirectAddressAbsoluteLoad;
		$4.reg = $2;
		$4.position = ctx.assembler->binary.getLocation();
		ctx.assembler->resolveAddress($4);
		SplitWord l = { $4.instructionData.instruction };
		INSTRUCTION_RL(LUI, $2, l.hi);
		INSTRUCTION_RRL(ORI, $2, $2, l.lo);
	}
    ;

literal_argument
	: literal_list                       { $$ = $1; }
	| LITERAL ':' LITERAL                { $$ = std::vector<std::variant<std::uint32_t, kasm::AddressData>>($3, $1); }
	| IDENTIFIER ':' LITERAL             { $$ = std::vector<std::variant<std::uint32_t, kasm::AddressData>>($3, kasm::AddressData($1)); }
	;

literal_list
    : LITERAL                                 { $$ = {$1}; }
	| IDENTIFIER                              { $$ = {$1}; }
    | literal_list ',' LITERAL                { $1.push_back($3); $$ = $1; }
	| literal_list ',' IDENTIFIER             { $1.push_back(kasm::AddressData($3)); $$ = $1; }
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

std::uint32_t resolveRegisterName(const std::string& registerName)
{
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
		{ "ra", kasm::RA }
	};

	return REGISTER_NAMES.at(registerName);
}

yy::parser::symbol_type yy::yylex(lexcontext& ctx)
{
    const char* YYMARKER;
    const char* anchor = ctx.cursor;
#define walk() { ctx.loc.columns(ctx.cursor - anchor); }
#define advance() { anchor = ctx.cursor; ctx.loc.step(); goto init; }
#define token(name) { walk(); return parser::make_##name(ctx.loc); }
#define tokenv(name, ...) { walk(); return parser::make_##name(__VA_ARGS__, ctx.loc); }
    %{ /* Begin re2c lexer */
    re2c:yyfill:enable = 0;
    re2c:define:YYCTYPE = "char";
    re2c:define:YYCURSOR = "ctx.cursor";

	end = "\x00";
	%}
init:
	%{
	// Directives
	".text"|".TEXT"     { token(TEXT); }
	".data"|".DATA"     { token(DATA); }
	".word"|".WORD"     { token(WORD); }
	".byte"|".BYTE"     { token(BYTE); }
	".ascii"|".ASCII"   { token(ASCII); }
	".asciiz"|".ASCIIZ" { token(ASCIIZ); }
	".align"|".ALIGN"   { token(ALIGN); }
	".space"|".SPACE"   { token(SPACE); }

	// Instructions
	"add"|"ADD"         { token(ADD); }
	"addi"|"ADDI"       { token(ADDI); }
	"addiu"|"ADDIU"     { token(ADDIU); }
	"addu"|"ADDU"       { token(ADDU); }
	"and"|"AND"         { token(AND); }
	"andi"|"ANDI"       { token(ANDI); }
	"beq"|"BEQ"         { token(BEQ); }
	"bgez"|"BGEZ"       { token(BGEZ); }
	"bgezal"|"BGEZAL"   { token(BGEZAL); }
	"bgtz"|"BGTZ"       { token(BGTZ); }
	"blez"|"BLEZ"       { token(BLEZ); }
	"bltz"|"BLTZ"       { token(BLTZ); }
	"bltzal"|"BLTZAL"   { token(BLTZAL); }
	"bne"|"BNE"         { token(BNE); }
	"div"|"DIV"         { token(DIV); }
	"divu"|"DIVU"       { token(DIVU); }
	"j"|"J"             { token(J); }
	"jal"|"JAL"         { token(JAL); }
	"jr"|"JR"           { token(JR); }
	"lb"|"LB"           { token(LB); }
	"lui"|"LUI"         { token(LUI); }
	"lw"|"LW"           { token(LW); }
	"mfhi"|"MFHI"       { token(MFHI); }
	"mflo"|"MFLO"       { token(MFLO); }
	"mult"|"MULT"       { token(MULT); }
	"multu"|"MULTU"     { token(MULTU); }
	"or"|"OR"           { token(OR); }
	"ori"|"ORI"         { token(ORI); }
	"sb"|"SB"           { token(SB); }
	"sll"|"SLL"         { token(SLL); }
	"sllv"|"SLLV"       { token(SLLV); }
	"slt"|"SLT"         { token(SLT); }
	"slti"|"SLTI"       { token(SLTI); }
	"sltiu"|"SLTIU"     { token(SLTIU); }
	"sltu"|"SLTU"       { token(SLTU); }
	"sra"|"SRA"         { token(SRA); }
	"srl"|"SRL"         { token(SRL); }
	"srlv"|"SRLV"       { token(SRLV); }
	"sub"|"SUB"         { token(SUB); }
	"subu"|"SUBU"       { token(SUBU); }
	"sw"|"SW"           { token(SW); }
	"sys"|"SYS"         { token(SYS); }
	"xor"|"XOR"         { token(XOR); }
	"xori"|"XORI"       { token(XORI); }
	"jalr"|"JALR"       { token(JALR); }
	"nor"|"NOR"         { token(NOR); }

	// Pseudoinstructions
	"mov"|"MOV"         { token(MOV); }
	"clr"|"CLR"         { token(CLR); }
	"b"|"B"             { token(B); }
	"bal"|"BAL"         { token(BAL); }
	"bgt"|"BGT"         { token(BGT); }
	"blt"|"BLT"         { token(BLT); }
	"bge"|"BGE"         { token(BGE); }
	"ble"|"BLE"         { token(BLE); }
	"bgtu"|"BGTU"       { token(BGTU); }
	"BEQZ"|"BEQZ"       { token(BEQZ); }
	"rem"|"REM"         { token(REM); }
	"li"|"LI"           { token(LI); }
	"la"|"LA"           { token(LA); }
	"nop"|"NOP"         { token(NOP); }
	"not"|"NOT"         { token(NOT); }

	// Identifier
	[a-zA-Z_][a-zA-Z_0-9]* { tokenv(IDENTIFIER, std::string(anchor, ctx.cursor)); }

	// Register
	"$"("zero"|"at"|"gp"|"sp"|"fp"|"ra"|"a"[0-3]|"v"[0-1]|"t"[0-9]|"s"[0-7]|"k"[0-1]) { tokenv(REGISTER, resolveRegisterName(std::string(anchor + 1, ctx.cursor))); }
	"$"([0-9]|[1-2][0-9]|"3"[0-1]) { tokenv(REGISTER, std::stoi(std::string(anchor + 1, ctx.cursor))); }

	// Literals
	[-+]?[0-9]+ { tokenv(LITERAL, std::stoi(std::string(anchor, ctx.cursor))); }
	"0b"[01]+ { tokenv(LITERAL, std::stoi(std::string(anchor, ctx.cursor))); }
	"0x"[0-9a-fA-F]+ { tokenv(LITERAL, std::stoi(std::string(anchor, ctx.cursor))); }
	"'"(.|[\\].)"'"                  { tokenv(LITERAL, anchor[1]); }

	// String
	"\""[^"]*"\""               { tokenv(STRING, std::string(anchor + 1, ctx.cursor - 1)); }

	// Whitespace
	"\r\n"|[\r\n]{ ctx.loc.lines(); ctx.loc.step(); token(END_OF_LINE); }
	[\t\v\b\f ] { ctx.loc.columns(); advance(); }
	end { token(END_OF_FILE); }

	// Comment
	"#"[^\r\n]* { walk(); advance(); }

	// Single character operators
	* { walk(); return parser::symbol_type(parser::token_type(ctx.cursor[-1] & 0xFF), ctx.loc); }
	%}
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

        std::ifstream asmFile(asmPath);
		binary.open(programPath);

        std::string sourceCode;
        asmFile.seekg(0, std::ios::end);
        sourceCode.reserve(asmFile.tellg());
        asmFile.seekg(0, std::ios::beg);

        sourceCode.assign((std::istreambuf_iterator<char>(asmFile)),
            std::istreambuf_iterator<char>());
        
        lexcontext ctx;
		std::string fileName = asmPath;
		ctx.loc.begin.filename = &fileName;
    	ctx.loc.end.filename   = &fileName;
        ctx.cursor = sourceCode.c_str();
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

    bool Assembler::resolveAddress(AddressData& address, bool mustResolve)
    {
		switch (address.type)
		{
		case AddressType::DirectAddressAbsolute:
			if (labelLocations.count(address.label))
			{
				address.instructionData.directAddressAbsolute = labelLocations.at(address.label);
				return true;
			}
			break;
		case AddressType::DirectAddressOffset:
			if (labelLocations.count(address.label))
			{
				address.instructionData.directAddressOffset = static_cast<std::int32_t>(labelLocations.at(address.label)) - address.position;
				return true;
			}
			break;
		case AddressType::IndirectAddressOffset:
			if (address.label.empty())
			{
				address.instructionData.register1 = address.reg;
				address.instructionData.directAddressOffset = static_cast<std::int32_t>(address.offset) - address.position;
				return true;
			}
			else if (labelLocations.count(address.label))
			{
				address.instructionData.register1 = address.reg;
				address.instructionData.directAddressOffset = static_cast<std::int32_t>(labelLocations.at(address.label)) + address.offset - address.position;
				return true;
			}
			break;
		case AddressType::DirectAddressAbsoluteWord:
		case AddressType::DirectAddressAbsoluteByte:
		case AddressType::DirectAddressAbsoluteLoad:
			if (labelLocations.count(address.label))
			{
				address.instructionData.instruction = labelLocations.at(address.label);
				return true;
			}
			break;
		default:
			break;
		}

		if (mustResolve)
		{
            throw std::exception(std::string("Unresolved Label: " + address.label).c_str());
		}

        unresolvedAddressLocations.push_back(address);

		return false;
    }

	void Assembler::defineLabel(const std::string& name, std::uint32_t location)
	{
		if (labelLocations.count(name))
		{
            throw std::exception(std::string("Redefined Label: " + name).c_str());
		}

		labelLocations[name] = location;
	}
}
