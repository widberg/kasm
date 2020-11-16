%no-lines
%require "3.4.1"
%language "c++"

%skeleton "lalr1.cc"

%define api.namespace { cyy }
%define api.token.constructor
%define api.value.type variant
%define api.location.file none
%define parse.assert
%define parse.error verbose

%param { kasm::Compiler& compiler }

%locations

%code requires
{
namespace kasm { class Compiler; };

#include <algorithm> // max
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "compoundInputFileStream.hpp"
}//%code requires

%code provides
{
namespace cyy { parser::symbol_type yylex(kasm::Compiler& compiler); }
}

%code
{
#include "compiler.hpp"
} // %code

%token END_OF_FILE 0

%token RETURN INCLUDE

%token<std::string> IDENTIFIER STRING
%token<std::uint32_t> LITERAL
%token<kasm::ast::Type> TYPE

%type<kasm::ast::Node*> statement compound_statement expression expression_or_nothing function_definition identifier literal type

%right ','
%left '+' '-' 
%left '*' '/' '@'
%left '~'

%start program

%%

program
	: %empty { compiler.astRoot = kasm::ast::makeEmpty(); }
	| program statement { compiler.astRoot = kasm::ast::makeCompound(compiler.astRoot, $2); }
	| program function_definition { compiler.astRoot = kasm::ast::makeCompound(compiler.astRoot, $2); }
	;

statement
	: ';' { $$ = kasm::ast::makeEmpty(); }
	| expression ';' { $$ = $1; }
	| RETURN expression ';' { $$ = kasm::ast::makeReturn($2); }
	;

compound_statement
	: %empty { $$ = kasm::ast::makeEmpty(); }
	| compound_statement statement { $$ = kasm::ast::makeCompound($1, $2); }
	;

expression
	: expression '+' expression { $$ = kasm::ast::makeBinaryOperator(kasm::ast::BinaryOperator::ADD, $1, $3); }
	| expression '=' expression { $$ = kasm::ast::makeBinaryOperator(kasm::ast::BinaryOperator::ASSIGNMENT, $1, $3); }
	| identifier '(' expression_or_nothing ')' { $$ = kasm::ast::makeFunctionCall($1, $3); }
	| expression ',' expression { $$ = kasm::ast::makeCompound($1, $3); }
	| identifier ':' type { $$ = kasm::ast::makeVariableDeclaration($1, $3); }
	| literal { $$ = $1; }
	| identifier { $$ = $1; }
	;

expression_or_nothing
	: %empty { $$ = kasm::ast::makeEmpty(); }
	| expression { $$ = $1; }
	;

function_definition
	: identifier '(' expression_or_nothing ')' ':' type '{' compound_statement '}' { $$ = kasm::ast::makeFunctionDefinition($1, $6, $3, $8); }
	;

type
	: TYPE { $$ = kasm::ast::makeType($1); }
	;

identifier
	: IDENTIFIER { $$ = kasm::ast::makeIdentifier($1); }
	;

literal
	: LITERAL { $$ = kasm::ast::makeLiteral($1); }
	;

%%

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

static std::string lexStringLiteral(kasm::CompoundInputFileStream& in)
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
		re2c:define:YYSKIP    = "do { in.ignore(); if (in.eof()) throw std::exception(\"Unclosed string\"); } while(0);";
		re2c:define:YYBACKUP  = "mar = in.tellg();";
		re2c:define:YYRESTORE = "in.seekg(mar);";
		
		"\n" { throw std::exception("Unclosed string"); }

		"\\"([abfnrtv]|"\\"|"\'"|"\"") { str.push_back(ESCAPE_SEQUENCES.at(yych)); continue; }
		[^\\"\""] { str.push_back(yych); continue; }

		"\"" { break; }
		* { throw std::exception("Illegal escape character in string"); }
		%}
	}

	return str;
}

static std::string getString(kasm::CompoundInputFileStream& in, std::streampos start, std::streampos end)
{
	std::string buffer;
	buffer.resize(end - start);
	in.seekg(start);
	in.read(buffer.data(), end - start);
	return buffer;
}

static char getChar(kasm::CompoundInputFileStream& in, std::streampos start, std::streampos end)
{
	in.seekg(start);
	char c;
	in.get(c);
	in.seekg(end);
	return c;
}

#define GET_STRING() getString(compiler.in, s, e)
#define GET_CHAR() getChar(compiler.in, s, e)

cyy::parser::symbol_type cyy::yylex(kasm::Compiler& compiler)
{
    std::streampos mar, s, e;
    /*!stags:re2c format = 'std::streampos @@;'; */

#define TOKEN(name) do { return parser::make_##name(compiler.loc); } while(0)
#define TOKENV(name, ...) do { return parser::make_##name(__VA_ARGS__, compiler.loc); } while(0)

	for (;;)
	{
		%{ /* Begin re2c lexer */
		re2c:yyfill:enable = 0;
		re2c:flags:input = custom;
		re2c:api:style = free-form;
		re2c:define:YYCTYPE      = char;
		re2c:define:YYPEEK       = "compiler.in.peek()";
		re2c:define:YYSKIP       = "do { compiler.in.ignore(); if (compiler.in.eof()) TOKEN(END_OF_FILE); } while(0);";
		re2c:define:YYBACKUP     = "mar = compiler.in.tellg();";
		re2c:define:YYRESTORE    = "compiler.in.seekg(mar);";
		re2c:define:YYSTAGP      = "@@{tag} = compiler.in.eof() ? 0 : compiler.in.tellg();";
		re2c:define:YYSTAGN      = "@@{tag} = 0;";
		re2c:define:YYSHIFTSTAG  = "@@{tag} += @@{shift};";
        re2c:flags:tags = 1;


		// Directives
		"%include"       { TOKEN(INCLUDE); }

		// Keywords
		"return"           { TOKEN(RETURN); }

		// Types
		"u8"           { TOKENV(TYPE, kasm::ast::Type::U8); }
		"u32"           { TOKENV(TYPE, kasm::ast::Type::U32); }
		"28"           { TOKENV(TYPE, kasm::ast::Type::S8); }
		"s32"           { TOKENV(TYPE, kasm::ast::Type::S32); }

		// Identifier
		@s [a-zA-Z_][a-zA-Z_0-9]* @e { TOKENV(IDENTIFIER, GET_STRING()); }

		// Literals
		@s [-+]?[0-9]+ @e      { TOKENV(LITERAL, std::stoi(GET_STRING(), nullptr, 10)); }
		@s "0b"[01]+ @e        { TOKENV(LITERAL, std::stoi(GET_STRING(), nullptr, 2)); }
		@s "0x"[0-9a-fA-F]+ @e { TOKENV(LITERAL, std::stoi(GET_STRING(), nullptr, 16)); }
		"'\\" @s ."'" @e       { TOKENV(LITERAL, ESCAPE_SEQUENCES.at(GET_CHAR())); }
		"'" @s [^\\"\'"]"'" @e { TOKENV(LITERAL, GET_CHAR()); }

		// String
		"\""                   { TOKENV(STRING, lexStringLiteral(compiler.in)); }

		// Whitespace
		"\r\n"|[\r\n]          { compiler.loc.lines(); compiler.loc.step(); continue; }
		[ \t\v\b\f]            { compiler.loc.columns(); continue; }

		// Comment
		@s "//"[^\r\n]* @e { continue; }

		// Single character operators
		@s [:,+(){}*-/.<>@&|~?;=] @e { return parser::symbol_type(parser::token_type(GET_CHAR()), compiler.loc); }

		@s * @e { throw std::exception(std::string("Invalid character of value: " + std::to_string(GET_CHAR())).c_str()); }
		%}
	}
}

void cyy::parser::error(const location_type& l, const std::string& message)
{
    std::cerr << l.begin.filename->c_str() << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << message << '\n';
}
