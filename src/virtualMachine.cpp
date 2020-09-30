#include "virtualMachine.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>

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

        InstructionData d = { instruction };

        std::uint32_t instructionFormat = INSTRUCTION_FORMATS.at(d.opcode);

        std::uint32_t address;
        if (instructionFormat & AnyAddress)
        {
            address = extractAddress(instruction, instructionFormat & AnyAddress);
        }

        switch (d.opcode)
        {
        case NOP:
            advancePc();
            break;
        case SYSTEM_CALL:
            systemCall();
            advancePc();
            break;
        case JUMP:
            pc = address;
            break;
        case JUMP_REGISTER:
            pc = getRegister(d.register0);
            break;
        case JUMP_AND_LINK:
            registers[RA] = pc + INSTRUCTION_SIZE;
            pc = address;
            break;
        case JUMP_AND_LINK_REGISTER:
            registers[RA] = pc + INSTRUCTION_SIZE;
            pc = getRegister(d.register0);
            break;
        case BRANCH_UNCONDITIONAL:
            pc = address;
            break;
        case BRANCH_EQUALS:
            if (getRegister(d.register0) == getRegister(d.register1))
            {
                pc = address;
            }
            else
            {
                advancePc();
            }
            break;
        case BRANCH_NOT_EQUALS:
            if (getRegister(d.register0) != getRegister(d.register1))
            {
                pc = address;
            }
            else
            {
                advancePc();
            }
            break;
        case BRANCH_LESS_THAN:
            if (getRegister(d.register0) < getRegister(d.register1))
            {
                pc = address;
            }
            else
            {
                advancePc();
            }
            break;
        case BRANCH_GREATER_THAN:
            if (getRegister(d.register0) > getRegister(d.register1))
            {
                pc = address;
            }
            else
            {
                advancePc();
            }
            break;
        case BRANCH_LESS_THAN_OR_EQUALS:
            if (getRegister(d.register0) <= getRegister(d.register1))
            {
                pc = address;
            }
            else
            {
                advancePc();
            }
            break;
        case BRANCH_GREATER_THAN_OR_EQUALS:
            if (getRegister(d.register0) >= getRegister(d.register1))
            {
                pc = address;
            }
            else
            {
                advancePc();
            }
            break;
        case LOAD_IMMEDIATE:
            setRegister(d.register0, d.immediate);
            advancePc();
            break;
        case LOAD_ADDRESS:
            setRegister(d.register0, address);
            advancePc();
            break;
        case LOAD_WORD:
            setRegister(d.register0, *reinterpret_cast<std::uint32_t*>(program.data() + address));
            advancePc();
            break;
        case SAVE_WORD:
            *reinterpret_cast<std::uint32_t*>(program.data() + address) = getRegister(d.register0);
            advancePc();
            break;
        case LOAD_BYTE:
            setRegister(d.register0, program[address]);
            advancePc();
            break;
        case SAVE_BYTE:
            program[address] = getRegister(d.register0);
            advancePc();
            break;
        case ADD:
            setRegister(d.register0, getRegister(d.register1) + getRegister(d.register2));
            advancePc();
            break;
        case SUB:
            setRegister(d.register0, getRegister(d.register1) - getRegister(d.register2));
            advancePc();
            break;
        case MUL:
            setRegister(d.register0, getRegister(d.register1) * getRegister(d.register2));
            advancePc();
            break;
        case DIV:
            setRegister(d.register0, getRegister(d.register1) / getRegister(d.register2));
            advancePc();
            break;
        case MOD:
            setRegister(d.register0, getRegister(d.register1) % getRegister(d.register2));
            advancePc();
            break;
        default:
            throw std::exception(std::string("Illegal Opcode: " + d.opcode).c_str());
            break;
        }
    }

    return 0;
}

void VirtualMachine::loadProgram(const std::string& programPath)
{
    std::ifstream programFile(programPath, std::ios::ate | std::ios::binary);
    program.resize(programFile.tellg());
    programFile.seekg(0, std::ios::beg);
    programFile.read(reinterpret_cast<char*>(program.data()), program.size());
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

std::uint32_t VirtualMachine::extractRegister(std::uint32_t instruction, int registerSlot)
{
    return (instruction >> (REGISTER_0_OFFSET - REGISTER_BIT * registerSlot)) & REGISTER_MASK;
}

std::uint32_t VirtualMachine::extractAddress(std::uint32_t instruction, std::uint32_t type)
{
    switch (type)
    {
    case DirectAddressAbsolute:
        return instruction & DIRECT_ADDRESS_ABSOLUTE_MASK;
        break;
    case DirectAddressOffset:
        return (instruction & DIRECT_ADDRESS_OFFSET_MASK) + pc;
        break;
    case IndirectAddressAbsolute:
        return getRegister(extractRegister(instruction, 1)) + (instruction & DIRECT_ADDRESS_OFFSET_MASK);
        break;
    default:
        break;
    }
    return 0;
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
