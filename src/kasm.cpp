#include <iostream>

#include "assembler.hpp"
#include "virtualMachine.hpp"

int main()
{
    Assembler assembler;
    VirtualMachine virtualMachine;

	int exitCode = 0;

	try
	{
		assembler.assemble("source.kasm", "program.kexe");
		virtualMachine.loadProgram("program.kexe");
		exitCode = virtualMachine.execute();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
    
	return exitCode;
}
