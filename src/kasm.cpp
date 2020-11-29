#include <iostream>

#include "assembler.hpp"
#include "debugger.hpp"
#include "disassembler.hpp"
#include "compiler.hpp"
#include "virtualMachine.hpp"

#include "binaryBuilder.hpp"

int main()
{
    kasm::Assembler assembler;
	kasm::Disassembler disassembler;
	kasm::Compiler compiler;
    kasm::VirtualMachine virtualMachine;
	kasm::Debugger debugger;

	int exitCode = 0;

	try
	{
		compiler.compile("source.k", "source_asm.kasm");
		assembler.assemble("source_asm.kasm", "program.kexe", "program.ksym");
		//assembler.assemble("source.kasm", "program.kexe", "program.ksym");
		disassembler.disassemble("program.kexe", "d_source.kasm", "program.ksym");
		//assembler.assemble("d_source.kasm", "program.kexe");
		debugger.loadProgram("program.kexe", "program.ksym");
		debugger.cli();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
    
	return exitCode;
}
