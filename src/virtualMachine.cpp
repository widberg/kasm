#include "virtualMachine.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
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
        switch (instructionFormat & AnyAddress)
        {
        case DirectAddressAbsolute:
            address = d.directAddressAbsolute;
            break;
        case DirectAddressOffset:
            address = d.directAddressOffset + pc;
            break;
        case IndirectAddressAbsolute:
            address = getRegister(d.register1) + d.directAddressOffset;
            break;
        default:
            break;
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

    std::cout << "Loaded program: " << programPath << std::endl;
    std::cout << "--- BEGIN PROGRAM MEMORY ---" << std::endl;
    for (int i = 0; i < program.size() / INSTRUCTION_SIZE; i++)
    {
        std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << *reinterpret_cast<std::uint32_t*>(program.data() + i * INSTRUCTION_SIZE);
        if ((i + 1) % 8 && i < program.size() / INSTRUCTION_SIZE - 1)
        {
            std::cout << " ";
        }
        else
        {
            std::cout << std::endl;
        }
    }
    std::cout << "---  END PROGRAM MEMORY  ---" << std::endl;
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
    case READ_CHAR:
        std::cin >> reinterpret_cast<char&>(registers[A0]);
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        break;
    case WRITE_CHAR:
        std::cout << static_cast<char>(registers[A0]);
        break;
    case READ_STRING:
        if (registers[A1] < 1)
        {
            break;
        }
        else
        {
            char c;
            unsigned int i = 0;
            while ((c = std::cin.get()) != '\n' && i < registers[A1] - 1)
            {
                *reinterpret_cast<char*>(program.data() + registers[A0] + i) = c;
                i++;
            }
            *reinterpret_cast<char*>(program.data() + registers[A0] + i) = '\0';
        }
        break;
    case WRITE_STRING:
        std::cout << reinterpret_cast<char*>(program.data() + registers[A0]);
        break;
    default:
        break;
    }
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
