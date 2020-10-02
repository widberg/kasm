#pragma once

#include <string>

class Compiler
{
public:
	Compiler();
	~Compiler();

	void compile(const std::string& sourcePath, const std::string& programPath);

private:
	
};
