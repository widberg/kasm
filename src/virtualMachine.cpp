#include "virtualMachine.hpp"

#include <cstdlib>
#include <exception>
#include <stdexcept>
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
        reset();

        run();

        return exitCode;
    }

    InstructionData VirtualMachine::fetchInstruction()
    {
        std::uint32_t instruction = program.getWord(pc);
        InstructionData instructionData = { instruction };
        return instructionData;
    }

    void VirtualMachine::step()
    {
        InstructionData instructionData = fetchInstruction();
        executeInstruction(instructionData);
    }

    void VirtualMachine::run()
    {
        while (pc < program.getTextSegmentLength() && !shouldExit)
        {
            InstructionData instructionData = fetchInstruction();
            executeInstruction(instructionData);
        }
    }

    void VirtualMachine::reset()
    {
        pc = 0;
        shouldExit = false;
        exitCode = 0;

        registers.clear();
        registers[SP] = STACK_OFFSET + STACK_SIZE;
        registers[GP] = GLOBAL_OFFSET;
    }

    void VirtualMachine::executeInstruction(const InstructionData& d)
    {
        switch (d.opcode)
        {
        case ADD:
            registers[d.register0] = registers[d.register1] + registers[d.register2];
            advancePc();
            break;
        case ADDI:
        {
            std::uint32_t unsignedImmediate = 0x0000FFFF & d.immediate;
            std::uint32_t mask = 0x00008000;
            if (mask & unsignedImmediate) unsignedImmediate |= 0xFFFF0000;
            std::int32_t signedImmediate = unsignedImmediate;
            std::int32_t signedSum = reinterpret_cast<std::int32_t&>(registers[d.register1]) + signedImmediate;
            registers[d.register0] = reinterpret_cast<std::uint32_t&>(signedSum);
            advancePc();
        }
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
            registers[d.register0] = program.getWord(resolveAddress(d, AddressType::IndirectAddressOffset));
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
        case SNE:
            registers[d.register0] = registers[d.register1] != registers[d.register2];
            advancePc();
            break;
        case SEQ:
            registers[d.register0] = registers[d.register1] == registers[d.register2];
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
            program.getWord(resolveAddress(d, AddressType::IndirectAddressOffset)) = registers[d.register0];
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
        default:
            executeSignal(Signal::ILLEGAL_OPCODE);
            break;
        }
    }

    void VirtualMachine::loadProgram(const std::string& programPath)
    {
        program.open(programPath);
        /*
        std::cout << "Loaded program: " << programPath << std::endl;
        std::cout << "--- BEGIN PROGRAM MEMORY ---" << std::endl;
        for (int i = 0; i < program.size() / INSTRUCTION_SIZE; i++)
        {
            std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << program.getWord(i * INSTRUCTION_SIZE);
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
        */
    }

    void VirtualMachine::setSignalHandler(Signal signal, void(*handler)(void))
    {
        signalHandlers[signal] = handler;
    }

    void VirtualMachine::executeSignal(Signal signal)
    {
        if (signalHandlers.count(signal))
        {
            signalHandlers[signal]();
        }
        else
        {
            throw signal;
        }
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
                    *program.getCharPtr(registers[A0] + i) = c;
                    i++;
                }
                *program.getCharPtr(registers[A0] + i) = '\0';
            }
            break;
        case WRITE_STRING:
            std::cout << program.getCharPtr(registers[A0]);
            break;
        case ALLOCATE:
            registers[V0] = (std::uint32_t)reinterpret_cast<std::uintptr_t>(new std::uint8_t[registers[A0]]);
            break;
        case DEALLOCATE:
            delete[] reinterpret_cast<std::uint8_t*>(registers[A0]);
            break;
        case OPEN_FILE:
        {
            std::ios_base::openmode mode = std::ios_base::in;

            switch (registers[A2])
            {
            case 1:
                mode = std::ios_base::in;
                break;
            case 2:
                mode = std::ios_base::out;
                break;
            case 3:
                mode = std::ios_base::in | std::ios_base::out;
                break;
            default:
                throw std::runtime_error("Invalid open mode");
                break;
            }

            files.insert({ fileID, new std::fstream(program.getCharPtr(registers[A0]), mode) });
            registers[V0] = fileID++;
        }
            break;
        case CLOSE_FILE:
            delete files.at(registers[A0]);
            files.erase(registers[A0]);
            break;
        case SEEK:
            {
                std::fstream* file = files.at(registers[A0]);

                std::ios_base::seekdir seekdir = std::ios_base::beg;

                switch (registers[A2])
                {
                case 0:
                    seekdir = std::ios_base::beg;
                    break;
                case 1:
                    seekdir = std::ios_base::end;
                    break;
                case 2:
                    seekdir = std::ios_base::cur;
                    break;
                default:
                    throw std::runtime_error("Invalid seek mode");
                    break;
                }

                file->seekg(registers[A1], seekdir);
                file->seekp(registers[A1], seekdir);
            }
            break;
        default:
            throw std::runtime_error(std::string("Illegal system call: " + registers[V0]).c_str());
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
