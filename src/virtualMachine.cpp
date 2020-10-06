#include "virtualMachine.hpp"

#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace kasm
{
    void VirtualMachine::advancePc()
    {
        pc += INSTRUCTION_SIZE;
    }

    int VirtualMachine::execute()
    {
        registers.clear();
        pc = 0;
        shouldExit = false;
        exitCode = 0;

        std::uint8_t* stack = new std::uint8_t[144];
        registers[SP] = reinterpret_cast<std::uint32_t>(stack + 144 - 1);

        while (pc < program.size() && !shouldExit)
        {
            std::uint32_t instruction = *reinterpret_cast<std::uint32_t*>(program.data() + pc);

            InstructionData d = { instruction };

            switch (d.opcode)
            {
            case ADD:
                registers[d.register0] = registers[d.register1] + registers[d.register2];
                advancePc();
                break;
            case ADDI:
                registers[d.register0] = registers[d.register1] + d.immediate;
                advancePc();
                break;
            case ADDIU:
                registers[d.register0] = registers[d.register1] + d.immediate;
                advancePc();
                break;
            case ADDU:
                registers[d.register0] = registers[d.register1] + registers[d.register2];
                advancePc();
                break;
            case AND:
                registers[d.register0] = registers[d.register1] & registers[d.register2];
                advancePc();
                break;
            case ANDI:
                registers[d.register0] = registers[d.register1] & d.immediate;
                advancePc();
                break;
            case BEQ:
                if (registers[d.register0] == registers[d.register1]) pc = resolveAddress(d, AddressType::DirectAddressOffset); else advancePc();
                break;
            case BGEZ:
                if (registers[d.register0] >= 0) pc = resolveAddress(d, AddressType::DirectAddressOffset); else advancePc();
                break;
            case BGEZAL:
                if (registers[d.register0] >= 0)
                {
                    registers[RA] = pc + INSTRUCTION_SIZE;
                    pc = resolveAddress(d, AddressType::DirectAddressOffset);
                }
                else
                {
                    advancePc();
                }
                break;
            case BGTZ:
                if (registers[d.register0] > 0) pc = resolveAddress(d, AddressType::DirectAddressOffset); else advancePc();
                break;
            case BLEZ:
                if (registers[d.register0] <= 0) pc = resolveAddress(d, AddressType::DirectAddressOffset); else advancePc();
                break;
            case BLTZ:
                if (registers[d.register0] < 0) pc = resolveAddress(d, AddressType::DirectAddressOffset); else advancePc();
                break;
            case BLTZAL:
                advancePc();
                if (registers[d.register0] < 0)
                {
                    registers[RA] = pc + INSTRUCTION_SIZE;
                    pc = resolveAddress(d, AddressType::DirectAddressOffset);
                }
                else
                {
                    advancePc();
                }
                break;
            case BNE:
                if (registers[d.register0] != registers[d.register1]) pc = resolveAddress(d, AddressType::DirectAddressOffset); else advancePc();
                break;
            case DIV:
                lo = registers[d.register0] / registers[d.register1];
                hi = registers[d.register0] % registers[d.register1];
                advancePc();
                break;
            case DIVU:
                lo = registers[d.register0] / registers[d.register1];
                hi = registers[d.register0] % registers[d.register1];
                advancePc();
                break;
            case J:
                pc = resolveAddress(d, AddressType::DirectAddressAbsolute);
                break;
            case JAL:
                registers[RA] = pc + INSTRUCTION_SIZE;
                pc = resolveAddress(d, AddressType::DirectAddressAbsolute);
                break;
            case JR:
                pc = registers[d.register0];
                break;
            case LB:
                registers[d.register0] = program[resolveAddress(d, AddressType::IndirectAddressOffset)];
                advancePc();
                break;
            case LUI:
                registers[d.register0] = d.immediate << (INSTRUCTION_BIT / 2);
                advancePc();
                break;
            case LW:
                registers[d.register0] = *reinterpret_cast<std::uint32_t*>(program.data() + resolveAddress(d, AddressType::IndirectAddressOffset));
                advancePc();
                break;
            case MFHI:
                registers[d.register0] = hi;
                advancePc();
                break;
            case MFLO:
                registers[d.register0] = lo;
                advancePc();
                break;
            case MULT:
            {
                std::uint64_t product = static_cast<std::uint64_t>(registers[d.register0]) * static_cast<std::uint64_t>(registers[d.register1]);
                lo = static_cast<std::uint32_t>(product);
                hi = static_cast<std::uint32_t>(product >> 32);
                advancePc();
                break;
            }
            case MULTU:
            {
                std::uint64_t product = static_cast<std::uint64_t>(registers[d.register0]) * static_cast<std::uint64_t>(registers[d.register1]);
                lo = static_cast<std::uint32_t>(product);
                hi = static_cast<std::uint32_t>(product >> 32);
                advancePc();
                break;
            }
            case OR:
                registers[d.register0] = registers[d.register1] | registers[d.register2];
                advancePc();
                break;
            case ORI:
                registers[d.register0] = registers[d.register1] | d.immediate;
                advancePc();
                break;
            case SB:
                program[resolveAddress(d, AddressType::IndirectAddressOffset)] = registers[d.register0];
                advancePc();
                break;
            case SLL:
                registers[d.register0] = registers[d.register1] << d.immediate;
                advancePc();
                break;
            case SLLV:
                registers[d.register0] = registers[d.register1] << registers[d.register2];
                advancePc();
                break;
            case SLT:
                registers[d.register0] = registers[d.register1] < registers[d.register2];
                advancePc();
                break;
            case SLTI:
                registers[d.register0] = registers[d.register1] < d.immediate;
                advancePc();
                break;
            case SLTIU:
                registers[d.register0] = registers[d.register1] < d.immediate;
                advancePc();
                break;
            case SLTU:
                registers[d.register0] = registers[d.register1] < registers[d.register2];
                advancePc();
                break;
            case SRA:
                registers[d.register0] = (registers[d.register1] >> d.immediate) | ~(~0U >> d.immediate);
                advancePc();
                break;
            case SRL:
                registers[d.register0] = registers[d.register1] >> d.immediate;
                advancePc();
                break;
            case SRLV:
                registers[d.register0] = registers[d.register1] >> registers[d.register2];
                advancePc();
                break;
            case SUB:
                registers[d.register0] = registers[d.register1] - registers[d.register2];
                advancePc();
                break;
            case SUBU:
                registers[d.register0] = registers[d.register1] - registers[d.register2];
                advancePc();
                break;
            case SW:
                *reinterpret_cast<std::uint32_t*>(program.data() + resolveAddress(d, AddressType::IndirectAddressOffset)) = registers[d.register0];
                advancePc();
                break;
            case SYS:
                systemCall();
                advancePc();
                break;
            case XOR:
                registers[d.register0] = registers[d.register1] ^ registers[d.register2];
                advancePc();
                break;
            case XORI:
                registers[d.register0] = registers[d.register1] ^ d.immediate;
                advancePc();
                break;
            case JALR:
                registers[d.register1] = pc + INSTRUCTION_SIZE;
                pc = registers[d.register0];
                break;
            case NOR:
                registers[d.register0] = ~(registers[d.register1] | registers[d.register2]);
                advancePc();
                break;
            case TEXT:
            case DATA:
                advancePc();
                break;
            default:
                throw std::exception(std::string("Illegal opcode: " + d.opcode).c_str());
                break;
            }
        }

        delete[] stack;

        return exitCode;
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
            shouldExit = true;
            exitCode = registers[A0];
            break;
        case READ_INT:
            std::cin >> registers[A0];
            break;
        case WRITE_INT:
            std::cout << registers[A0];
            break;
        case READ_CHAR:
        {
            char c;
            std::cin >> c;
            registers[A0] = static_cast<std::uint32_t>(c);
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
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
        case ALLOCATE:
            registers[V0] = reinterpret_cast<std::uint32_t>(new std::uint8_t[registers[A0]]);
            break;
        case DEALLOCATE:
            delete[] reinterpret_cast<std::uint8_t*>(registers[A0]);
            break;
        default:
            throw std::exception(std::string("Illegal system call: " + registers[V0]).c_str());
            break;
        }
    }

    std::uint32_t VirtualMachine::resolveAddress(const InstructionData& instructionData, AddressType type)
    {
        switch (type)
        {
        case kasm::AddressType::DirectAddressAbsolute:
            return instructionData.directAddressAbsolute;
        case kasm::AddressType::DirectAddressOffset:
            return instructionData.directAddressOffset + pc;
        case kasm::AddressType::IndirectAddressOffset:
            return registers[instructionData.register1] + instructionData.directAddressOffset + pc;
        default:
            break;
        }
    }
}
