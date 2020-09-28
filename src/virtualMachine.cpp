#include "virtualMachine.hpp"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "common.hpp"

VirtualMachine::VirtualMachine()
{
}

VirtualMachine::~VirtualMachine()
{
}

void VirtualMachine::advancePc()
{
    pc += INSTRUCTION_SIZE;
}

int VirtualMachine::execute()
{
    std::fill(std::begin(registers), std::end(registers), 0);
    pc = 0;

    while (pc < program.size())
    {
        std::uint32_t instruction = *reinterpret_cast<std::uint32_t*>(program.data() + pc);
        switch (extractOpcode(instruction))
        {
        case NOP:
            advancePc();
            break;
        case SYSTEM_CALL:
            systemCall();
            advancePc();
            break;
        case JUMP:
            pc = extractAddress(instruction);
            break;
        case JUMP_REGISTER:
            pc = getRegister(extractRegister(instruction, 0));
            break;
        case JUMP_AND_LINK:
            registers[RA] = pc + INSTRUCTION_SIZE;
            pc = extractAddress(instruction);
            break;
        case JUMP_AND_LINK_REGISTER:
            registers[RA] = pc + 4;
            pc = getRegister(extractRegister(instruction, 0));
            break;
        case BRANCH_EQUALS:
            if (getRegister(extractRegister(instruction, 0)) == getRegister(extractRegister(instruction, 1)))
            {
                pc = extractAddress(instruction);
            }
            else
            {
                advancePc();
            }
            break;
        case LOAD_IMMEDIATE:
            setRegister(extractRegister(instruction, 0), extractImmediate(instruction));
            advancePc();
            break;
        case LOAD_ADDRESS:
            setRegister(extractRegister(instruction, 0), extractAddress(instruction));
            advancePc();
            break;
        case LOAD_WORD:
            setRegister(extractRegister(instruction, 0), *reinterpret_cast<std::uint32_t*>(program.data() + extractAddress(instruction)));
            advancePc();
            break;
        case SAVE_WORD:
            *reinterpret_cast<std::uint32_t*>(program.data() + extractAddress(instruction)) = getRegister(extractRegister(instruction, 0));
            advancePc();
            break;
        case ADD:
            setRegister(extractRegister(instruction, 0), getRegister(extractRegister(instruction, 1)) + getRegister(extractRegister(instruction, 2)));
            advancePc();
            break;
        default:
            return -1;
        }
    }

    return 0;
}

void VirtualMachine::loadProgram(const std::string& programPath)
{
    std::ifstream programFile(programPath, std::ios::ate | std::ios::binary);
    program.resize(programFile.tellg());
    programFile.seekg(0, std::ios::beg);
    programFile.read(reinterpret_cast<char*>(program.data()), program.capacity());
}

void VirtualMachine::systemCall()
{
    switch (registers[V0])
    {
    case EXIT:
        exit(registers[A0]);
    case READ_INT:
        std::cin >> registers[A0];
        break;
    case WRITE_INT:
        std::cout << registers[A0];
        break;
    default:
        break;
    }
}

std::uint32_t VirtualMachine::extractOpcode(std::uint32_t instruction)
{
    return (instruction >> OPCODE_OFFSET) & OPCODE_MASK;
}

std::uint32_t VirtualMachine::extractRegister(std::uint32_t instruction, int registerSlot)
{
    return (instruction >> (REGISTER_0_OFFSET - REGISTER_BIT * registerSlot)) & REGISTER_MASK;
}

std::uint32_t VirtualMachine::extractAddress(std::uint32_t instruction)
{
    return (instruction >> ADDRESS_OFFSET) & RELATIVE_ADDRESS_MASK;
}

std::uint32_t VirtualMachine::extractImmediate(std::uint32_t instruction)
{
    return (instruction >> IMMEDIATE_OFFSET) & IMMEDIATE_MASK;
}

void VirtualMachine::setRegister(std::uint32_t reg, std::uint32_t value)
{
    if (reg == ZERO) return;
    registers[reg] = value;
}

std::uint32_t VirtualMachine::getRegister(std::uint32_t reg)
{
    if (reg == ZERO) return 0;
    return registers[reg];
}
