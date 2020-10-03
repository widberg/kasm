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
		std::cout << $1 << std::endl;
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
		std::cout << "align " << std::to_string($2) << std::endl;
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
    | ADD REGISTER ',' REGISTER ',' REGISTER
	{
		
	}
    | BEQ REGISTER ',' REGISTER ',' direct_address
	{

	}
    | LB REGISTER ',' indirect_address
	{

	}
    ;

literal_list
    : LITERAL
    {
        $$ = std::vector<std::uint32_t>($1);
    }
    | literal_list ',' LITERAL
    {
        $1.push_back($3);
		$$ = $1;
    }
    ;

indirect_address
    : direct_address
    | '(' REGISTER ')'
    | LITERAL '(' REGISTER ')'
    | IDENTIFIER '(' REGISTER ')'
    | LITERAL '+' LITERAL '(' REGISTER ')'
    | IDENTIFIER '+' LITERAL '(' REGISTER ')'
    ;

direct_address
    : IDENTIFIER
    | IDENTIFIER '+' LITERAL
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
#define advance() { anchor = ctx.cursor; ctx.loc.step(); }
#define token(name) { walk(); return parser::make_##name(ctx.loc); }
#define tokenv(name, ...) { walk(); return parser::make_##name(__VA_ARGS__, ctx.loc); }

    %{ /* Begin re2c lexer */
    re2c:yyfill:enable = 0;
    re2c:define:YYCTYPE = "char";
    re2c:define:YYCURSOR = "ctx.cursor";

	end = "\x00";

	// Instructions
	"add" | "ADD" { token(ADD); }
	"addi" | "ADDI" { token(ADDI); }
	"addiu" | "ADDIU" { token(ADDIU); }
	"addu" | "ADDU" { token(ADDU); }
	"and" | "AND" { token(AND); }
	"andi" | "ANDI" { token(ANDI); }
	"beq" | "BEQ" { token(BEQ); }
	"bgez" | "BGEZ" { token(BGEZ); }
	"bgezal" | "BGEZAL" { token(BGEZAL); }
	"bgtz" | "BGTZ" { token(BGTZ); }
	"blez" | "BLEZ" { token(BLEZ); }
	"bltz" | "BLTZ" { token(BLTZ); }
	"bltzal" | "BLTZAL" { token(BLTZAL); }
	"bne" | "BNE" { token(BNE); }
	"div" | "DIV" { token(DIV); }
	"divu" | "DIVU" { token(DIVU); }
	"j" | "J" { token(J); }
	"jal" | "JAL" { token(JAL); }
	"jr" | "JR" { token(JR); }
	"lb" | "LB" { token(LB); }
	"lui" | "LUI" { token(LUI); }
	"lw" | "LW" { token(LW); }
	"mfhi" | "MFHI" { token(MFHI); }
	"mflo" | "MFLO" { token(MFLO); }
	"mult" | "MULT" { token(MULT); }
	"multu" | "MULTU" { token(MULTU); }
	"nop" | "NOP" { token(NOP); }
	"or" | "OR" { token(OR); }
	"ori" | "ORI" { token(ORI); }
	"sb" | "SB" { token(SB); }
	"sll" | "SLL" { token(SLL); }
	"sllv" | "SLLV" { token(SLLV); }
	"slt" | "SLT" { token(SLT); }
	"slti" | "SLTI" { token(SLTI); }
	"sltiu" | "SLTIU" { token(SLTIU); }
	"sltu" | "SLTU" { token(SLTU); }
	"sra" | "SRA" { token(SRA); }
	"srl" | "SRL" { token(SRL); }
	"srlv" | "SRLV" { token(SRLV); }
	"sub" | "SUB" { token(SUB); }
	"subu" | "SUBU" { token(SUBU); }
	"sw" | "SW" { token(SW); }
	"sys" | "SYS" { token(SYS); }
	"xor" | "XOR" { token(XOR); }
	"xori" | "XORI" { token(XORI); }

	// Directives
	".word" | ".WORD" { token(WORD); }
	".byte" | ".BYTE" { token(BYTE); }
	".ascii" | ".ASCII" { token(ASCII); }
	".asciiz" | ".ASCIIZ" { token(ASCIIZ); }
	".align" | ".ALIGN" { token(ALIGN); }
	".space" | ".SPACE" { token(SPACE); }

	// Identifier
	[a-zA-Z_][a-zA-Z_0-9]* { tokenv(IDENTIFIER, std::string(anchor, ctx.cursor)); }

	// Register
	"$"("zero"|"at"|"gp"|"sp"|"fp"|"ra"|"a"[0-3]|"v"[0-1]|"t"[0-9]|"s"[0-7]|"k"[0-1]) { tokenv(REGISTER, resolveRegisterName(std::string(anchor + 1, ctx.cursor))); }
	"$"([0-9]|[1-2][0-9]|"3"[0-1]) { tokenv(REGISTER, std::stoi(std::string(anchor + 1, ctx.cursor))); }

	// Literals
	"LITERAL" { tokenv(LITERAL, 0); }
	//"0b"[01]+ { tokenv(LITERAL, 0); }
	//"0x"[0-9a-fA-F]+ { tokenv(LITERAL, 0); }
	//"'"(.|[\\].)"'"                  { tokenv(LITERAL, anchor[1]); }

	// String
	"\"".*"\""               { tokenv(STRING, std::string(anchor + 1, ctx.cursor - 1)); }

	// Whitespace
	"\r\n"|[\r\n]{ ctx.loc.lines(); advance(); }
	[\t\v\b\f\s] { ctx.loc.columns(); advance(); }
	end { token(END_OF_FILE); }

	// Comment
	"#"[^ \r\n] * { walk(); advance(); }

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
            if (labelLocations.count(unresolvedAddressLocation.label))
            {
                binary.setLocation(unresolvedAddressLocation.location);
                std::uint32_t instruction = unresolvedAddressLocation.instruction;

                switch (unresolvedAddressLocation.type)
                {
                case AddressType::DirectAddressAbsolute:
                    instruction |= labelLocations.at(unresolvedAddressLocation.label);
                    break;
                case AddressType::DirectAddressOffset:
                    instruction |= labelLocations.at(unresolvedAddressLocation.label) - unresolvedAddressLocation.location;
                    break;
                case AddressType::IndirectAddressAbsolute:
                    instruction |= (instruction & 0xFFFF) + labelLocations.at(unresolvedAddressLocation.label);
                    break;
                default:
                    break;
                }

                binary.writeWord(instruction);
            }
            else
            {
                throw std::exception(std::string("Unresolved Label: " + unresolvedAddressLocation.label).c_str());
            }
        }

        binary.setLocation(BinaryBuilder::END);
        binary.align(INSTRUCTION_SIZE);
    }

    std::uint32_t Assembler::resolveAddress(std::uint32_t instructionLocation, const std::string& address, AddressType type, std::uint32_t instruction)
    {
        switch (type)
        {
        case AddressType::DirectAddressAbsolute:
            if (labelLocations.count(address))
            {
                return instruction | labelLocations.at(address);
            }
            break;
        case AddressType::DirectAddressOffset:
            if (labelLocations.count(address))
            {
                return instruction | (labelLocations.at(address) - instructionLocation);
            }
            break;
        case AddressType::IndirectAddressAbsolute:
            if (labelLocations.count(address))
            {
                return instruction | labelLocations.at(address);
            }
            break;
            //return instruction | resolveRegisterName(address, 1);
            break;
        default:
            break;
        }

        unresolvedAddressLocations.push_back({ instructionLocation, address, type, instruction });
        return instruction;
    }
}
