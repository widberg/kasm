#include <iostream>
#include <string>

#include "assembler.hpp"
#include "debugger.hpp"
#include "disassembler.hpp"
#include "compiler.hpp"
#include "virtualMachine.hpp"

#include "binaryBuilder.hpp"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Subcommand required [asm|dsm|vm]\n";
		return -1;
	}

	std::string subcommand = argv[1];

    kasm::Assembler assembler;
	kasm::Disassembler disassembler;
	kasm::Compiler compiler;
    kasm::VirtualMachine virtualMachine;
	kasm::Debugger debugger;

	int exitCode = 0;

	try
	{
		//compiler.compile("source.k", "source_asm.kasm");
		//assembler.assemble("source_asm.kasm", "program.kexe", "program.ksym");
		////assembler.assemble("source.kasm", "program.kexe", "program.ksym");
		//disassembler.disassemble("program.kexe", "d_source.kasm", "program.ksym");
		////assembler.assemble("d_source.kasm", "program.kexe");
		//debugger.loadProgram("program.kexe", "program.ksym");
		//debugger.cli();

		if (subcommand == "asm")
		{
			if (argc < 4)
			{
				std::cerr << "Subcommand asm requires source and output paths\n";
				return -1;
			}

			std::string source = argv[2];
			std::string output = argv[3];

			assembler.assemble(source, output);
		}
		else if (subcommand == "dsm")
		{
			if (argc < 4)
			{
				std::cerr << "Subcommand dsm requires executable and output paths\n";
				return -1;
			}

			std::string executable = argv[2];
			std::string output = argv[3];

			disassembler.disassemble(executable, output);
		}
		else if (subcommand == "vm")
		{
			if (argc < 3)
			{
				std::cerr << "Subcommand vm requires executable path\n";
				return -1;
			}

			std::string executable = argv[2];

			virtualMachine.loadProgram(executable);
			exitCode = virtualMachine.execute();
		}
		else
		{
			std::cerr << "Invalid subcommand [asm|dsm|vm]\n";
			return -1;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
    
	return exitCode;
}
