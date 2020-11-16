#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hpp"
#include "compoundInputFileStream.hpp"
#include "compiler.tab.hpp"

namespace kasm
{
	class Compiler
	{
	public:
		Compiler();
		~Compiler();

		void compile(const std::string& sourcePath, const std::string& asmPath);

	private:
		void writeLine(const std::string& line);

		std::ofstream asmFile;

		static std::uint32_t getSizeOfType(ast::Type type);
		static ast::Node* negate(ast::Node* astNode);

		static void prettyPrint(const ast::Node* astNode, unsigned int level = 0);
		void semanticAnalysis(ast::Node* astNode);
		void optimize(const ast::Node* astNode);
		void codeGeneration(const ast::Node* astNode);

		CompoundInputFileStream in;
		cyy::location loc;
		ast::Node* astRoot;

		friend class cyy::parser;
		friend cyy::parser::symbol_type cyy::yylex(kasm::Compiler& compiler);
	};
}
