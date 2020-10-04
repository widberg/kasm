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
#include <string>
#include <vector>

#include "binaryBuilder.hpp"
#include "common.hpp"

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
}//%code

%token END_OF_FILE 0
%token IDENTIFIER LITERAL STRING REGISTER
%token ADD ADDI ADDIU ADDU AND ANDI BEQ BGEZ BGEZAL BGTZ BLEZ BLTZ BLTZAL BNE
%token DIV DIVU J JAL JR LB LUI LW MFHI MFLO MULT MULTU NOP OR ORI SB SLL SLLV
%token SLT SLTI SLTIU SLTU SRA SRL SRLV SUB SUBU SW SYS XOR XORI
%token WORD BYTE ASCII ASCIIZ ALIGN SPACE

%type<std::string> IDENTIFIER STRING
%type<std::uint32_t> LITERAL REGISTER
%type<std::vector<std::uint32_t>> literal_list
%type<kasm::Address> direct_address indirect_address

%start statement_list

%%

statement_list
	: statement
	| label_declaration
    | statement_list statement
    | statement_list label_declaration
	;

label_declaration
    : IDENTIFIER ':'
    {
        ctx.assembler->labelLocations[$1] = ctx.assembler->binary.getLocation();
    }
    ;

statement
    : WORD literal_list
    {
        ctx.assembler->binary.align(kasm::INSTRUCTION_SIZE);
        for (std::uint32_t word : $2)
        {
            ctx.assembler->binary.writeWord(word);
        }
    }
    | BYTE literal_list
    {
        for (std::uint8_t byte : $2)
        {
            ctx.assembler->binary.writeData(&byte, sizeof(byte));
        }
    }
    | ASCII STRING
    {
        ctx.assembler->binary.writeData(reinterpret_cast<const std::uint8_t*>($2.c_str()), $2.size());
    }
    | ASCIIZ STRING
    {
        ctx.assembler->binary.writeData(reinterpret_cast<const std::uint8_t*>($2.c_str()), $2.size() + 1);
    }
    | ALIGN LITERAL
    {
        unsigned int alignment = 1;
        for (int i = 0; i < $2; i++)
        {
            alignment *= 2;
        }
        ctx.assembler->binary.align(alignment);
    }
    | SPACE LITERAL
    {
        ctx.assembler->binary.pad($2);
    }
    | ADD REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::ADD;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| ADDI REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::ADDI;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| ADDIU REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::ADDIU;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| AND REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::AND;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| ANDI REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::ANDI;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
    | BEQ REGISTER separator REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BEQ;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $6);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| BGEZ REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BGEZ;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| BGEZAL REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BGEZAL;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| BGTZ REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BGTZ;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| BLEZ REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BLEZ;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| BLTZ REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BLTZ;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| BLTZAL REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BLTZAL;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| BNE REGISTER separator REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::BNE;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $6);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| DIV REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::DIV;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| DIVU REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::DIVU;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| J direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::J;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $2);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| JAL direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::JAL;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $2);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| JR REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::JR;
		instructionData.register0 = $2;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
    | LB REGISTER separator indirect_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::LB;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| LUI REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::LUI;
		instructionData.register0 = $2;
		instructionData.immediate = $4;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| LW REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::LW;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| MFHI REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::MFHI;
		instructionData.register0 = $2;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| MFLO REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::MFLO;
		instructionData.register0 = $2;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| MULT REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::MULT;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| MULTU REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::MULTU;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| NOP
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::NOP;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| OR REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::OR;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| ORI REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::ORI;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SB REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SB;
		instructionData.register0 = $2;
		instructionData.directAddressOffset = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SLL REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SLL;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SLLV REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SLLV;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SLT REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SLT;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SLTI REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SLTI;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SLTIU REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SLTIU;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SRA REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SRA;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SRL REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SRL;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SRLV REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SRLV;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SUB REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SUB;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SUBU REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SUBU;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SW REGISTER separator direct_address
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SW;
		instructionData.register0 = $2;
		instructionData.instruction = ctx.assembler->resolveAddress(ctx.assembler->binary.getLocation(), $4);
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| SYS
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::SYS;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| XOR REGISTER separator REGISTER separator REGISTER
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::XOR;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.register2 = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
	| XORI REGISTER separator REGISTER separator LITERAL
	{
		kasm::InstructionData instructionData;
		instructionData.opcode = kasm::Opcode::XORI;
		instructionData.register0 = $2;
		instructionData.register1 = $4;
		instructionData.immediate = $6;
        ctx.assembler->binary.writeWord(instructionData.instruction);
	}
    ;

literal_list
    : LITERAL
    {
        $$ = std::vector<std::uint32_t>($1);
    }
    | literal_list separator LITERAL
    {
        $1.push_back($3);
		$$ = $1;
    }
    ;

indirect_address
    : direct_address
	{
		$1.type = kasm::AddressType::IndirectAddressAbsolute;
		$$ = $1;
	}
    | '(' REGISTER ')'
	{
		kasm::Address addr = kasm::Address{kasm::AddressType::IndirectAddressAbsolute};
		addr.instructionData.register1 = $2;
		$$ = addr;
	}
    | LITERAL '(' REGISTER ')'
	{
		kasm::Address addr = kasm::Address{kasm::AddressType::IndirectAddressAbsolute};
		addr.offset = $1;
		addr.instructionData.register1 = $3;
		$$ = addr;
	}
    | IDENTIFIER '(' REGISTER ')'
	{
		kasm::Address addr = kasm::Address{kasm::AddressType::IndirectAddressAbsolute};
		addr.label = $1;
		addr.instructionData.register1 = $3;
		$$ = addr;
	}
    | LITERAL '+' LITERAL '(' REGISTER ')'
	{
		kasm::Address addr = kasm::Address{kasm::AddressType::IndirectAddressAbsolute};
		addr.offset = $1 + $3;
		addr.instructionData.register1 = $5;
		$$ = addr;
	}
    | IDENTIFIER '+' LITERAL '(' REGISTER ')'
	{
		kasm::Address addr = kasm::Address{kasm::AddressType::IndirectAddressAbsolute};
		addr.label = $1;
		addr.offset = $3;
		addr.instructionData.register1 = $5;
		$$ = addr;
	}
    ;

direct_address
    : IDENTIFIER
	{
		$$ = kasm::Address{kasm::AddressType::DirectAddressAbsolute, $1};
	}
    | IDENTIFIER '+' LITERAL
	{
		$$ = kasm::Address{kasm::AddressType::DirectAddressAbsolute, "", $3};
	}
    ;

separator
	: ','
	| %empty
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
	// Instructions
	"add"|"ADD" { token(ADD); }
	"addi"|"ADDI" { token(ADDI); }
	"addiu"|"ADDIU" { token(ADDIU); }
	"addu"|"ADDU" { token(ADDU); }
	"and"|"AND" { token(AND); }
	"andi"|"ANDI" { token(ANDI); }
	"beq"|"BEQ" { token(BEQ); }
	"bgez"|"BGEZ" { token(BGEZ); }
	"bgezal"|"BGEZAL" { token(BGEZAL); }
	"bgtz"|"BGTZ" { token(BGTZ); }
	"blez"|"BLEZ" { token(BLEZ); }
	"bltz"|"BLTZ" { token(BLTZ); }
	"bltzal"|"BLTZAL" { token(BLTZAL); }
	"bne"|"BNE" { token(BNE); }
	"div"|"DIV" { token(DIV); }
	"divu"|"DIVU" { token(DIVU); }
	"j"|"J" { token(J); }
	"jal"|"JAL" { token(JAL); }
	"jr"|"JR" { token(JR); }
	"lb"|"LB" { token(LB); }
	"lui"|"LUI" { token(LUI); }
	"lw"|"LW" { token(LW); }
	"mfhi"|"MFHI" { token(MFHI); }
	"mflo"|"MFLO" { token(MFLO); }
	"mult"|"MULT" { token(MULT); }
	"multu"|"MULTU" { token(MULTU); }
	"nop"|"NOP" { token(NOP); }
	"or"|"OR" { token(OR); }
	"ori"|"ORI" { token(ORI); }
	"sb"|"SB" { token(SB); }
	"sll"|"SLL" { token(SLL); }
	"sllv"|"SLLV" { token(SLLV); }
	"slt"|"SLT" { token(SLT); }
	"slti"|"SLTI" { token(SLTI); }
	"sltiu"|"SLTIU" { token(SLTIU); }
	"sltu"|"SLTU" { token(SLTU); }
	"sra"|"SRA" { token(SRA); }
	"srl"|"SRL" { token(SRL); }
	"srlv"|"SRLV" { token(SRLV); }
	"sub"|"SUB" { token(SUB); }
	"subu"|"SUBU" { token(SUBU); }
	"sw"|"SW" { token(SW); }
	"sys"|"SYS" { token(SYS); }
	"xor"|"XOR" { token(XOR); }
	"xori"|"XORI" { token(XORI); }

	// Directives
	".word"|".WORD" { token(WORD); }
	".byte"|".BYTE" { token(BYTE); }
	".ascii"|".ASCII" { token(ASCII); }
	".asciiz"|".ASCIIZ" { token(ASCIIZ); }
	".align"|".ALIGN" { token(ALIGN); }
	".space"|".SPACE" { token(SPACE); }

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
	"\r\n"|[\r\n]{ ctx.loc.lines(); advance(); }
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
    //print source line
    //std::cerr << std::string(l.begin.column - 1, ' ') << '^' << std::string(l.end.column - l.begin.column - 1, '~') << std::endl;
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

        for (UnresolvedAddressLocation unresolvedAddressLocation : unresolvedAddressLocations)
        {
			binary.setLocation(unresolvedAddressLocation.location);
			std::uint32_t instruction = unresolvedAddressLocation.address.instructionData.instruction;

			instruction = resolveAddress(MUST_RESOLVE, unresolvedAddressLocation.address);

			binary.writeWord(instruction);
        }

        binary.setLocation(BinaryBuilder::END);
        binary.align(INSTRUCTION_SIZE);
    }

    std::uint32_t Assembler::resolveAddress(std::uint32_t instructionLocation, const Address& address)
    {
		if (address.label == "")
		{
			switch (address.type)
			{
			case AddressType::DirectAddressAbsolute:
			case AddressType::IndirectAddressAbsolute:
				return address.instructionData.instruction | address.offset;
			case AddressType::DirectAddressOffset:
				return address.instructionData.instruction | address.offset - instructionLocation;
			default:
				break;
			}
		}
		else if (labelLocations.count(address.label))
        {
			switch (address.type)
			{
			case AddressType::DirectAddressAbsolute:
			case AddressType::IndirectAddressAbsolute:
				return address.instructionData.instruction | labelLocations.at(address.label) + address.offset;
			case AddressType::DirectAddressOffset:
				return address.instructionData.instruction | labelLocations.at(address.label) + address.offset - instructionLocation;
			default:
				break;
			}
		}
		else if (instructionLocation == MUST_RESOLVE)
		{
                throw std::exception(std::string("Unresolved Label: " + address.label).c_str());
		}

        unresolvedAddressLocations.push_back({ instructionLocation, address });
        return address.instructionData.instruction;
    }
}
