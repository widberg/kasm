#include "compiler.hpp"

#include <fstream>

Compiler::Compiler()
{

}

Compiler::~Compiler()
{

}

void Compiler::compile(const std::string& sourcePath, const std::string& programPath)
{
	std::ifstream sourceFile(sourcePath);
	std::ofstream programFile(programPath, std::ios::binary);

	//programFile.write();
}
