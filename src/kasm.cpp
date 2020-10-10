#include <iostream>

#include "assembler.hpp"
#include "disassembler.hpp"
//#include "compiler.hpp"
#include "virtualMachine.hpp"

#include "binaryBuilder.hpp"

int main()
{
    kasm::Assembler assembler;
	kasm::Disassembler disassembler;
	//Compiler compiler;
    kasm::VirtualMachine virtualMachine;

	int exitCode = 0;

	try
	{
		//compiler.compile("source.k", "source_asm.kasm");
		//assembler.assemble("source_asm.kasm", "program.kexe");
		assembler.assemble("source.kasm", "program.kexe");
		assembler.saveSymbolTable("program.ksym");
		disassembler.disassemble("program.kexe", "d_source.kasm", "program.ksym");
		assembler.assemble("d_source.kasm", "program.kexe");
		virtualMachine.loadProgram("program.kexe");
		exitCode = virtualMachine.execute();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
    
	return exitCode;
}
