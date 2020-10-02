#include <iostream>

#include "assembler.hpp"
#include "compiler.hpp"
#include "virtualMachine.hpp"

int main()
{
    Assembler assembler;
	Compiler compiler;
    VirtualMachine virtualMachine;

	int exitCode = 0;

	try
	{
		compiler.compile("source.k", "source_asm.kasm");
		assembler.assemble("source_asm.kasm", "program.kexe");
		virtualMachine.loadProgram("program.kexe");
		exitCode = virtualMachine.execute();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
    
	return exitCode;
}
