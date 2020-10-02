#include "assembler.hpp"

#include <exception>
#include <fstream>

#include "binaryBuilder.hpp"
#include "common.hpp"

Assembler::Assembler()
{
}

Assembler::~Assembler()
{
}

void Assembler::assemble(const std::string& sourcePath, const std::string& programPath)
{
    const static std::unordered_map<std::string, std::uint32_t> INSTRUCTION_NAMES = {
        { "nop", NOP },
        { "j", JUMP },
        { "jr", JUMP_REGISTER },
        { "jal", JUMP_AND_LINK },
        { "jalr", JUMP_AND_LINK_REGISTER },
        { "b", BRANCH_UNCONDITIONAL },
        { "beq", BRANCH_EQUALS },
        { "bne", BRANCH_NOT_EQUALS },
        { "blt", BRANCH_LESS_THAN },
        { "bgt", BRANCH_GREATER_THAN },
        { "ble", BRANCH_LESS_THAN_OR_EQUALS },
        { "bge", BRANCH_GREATER_THAN_OR_EQUALS },
        { "li", LOAD_IMMEDIATE },
        { "la", LOAD_ADDRESS },
        { "lw", LOAD_WORD },
        { "sw", SAVE_WORD },
        { "lb", LOAD_BYTE },
        { "sb", SAVE_BYTE },
        { "sys", SYSTEM_CALL },
        { "add", ADD },
        { "sub", SUB },
        { "mul", MUL },
        { "div", DIV },
        { "mod", MOD },
    };

    labelLocations.clear();
    unresolvedAddressLocations.clear();

    std::ifstream sourceFile(sourcePath);
    BinaryBuilder binary(programPath);

    std::string current;
    while (sourceFile >> current)
    {
        if (INSTRUCTION_NAMES.count(current))
        {
            std::uint32_t opcode = INSTRUCTION_NAMES.at(current);
            std::uint32_t instructionFormat = INSTRUCTION_FORMATS.at(opcode);
            InstructionData instruction = { 0 };
            instruction.opcode = opcode;

            for (int i = 0; i < 3; i++)
            {
                if (instructionFormat & (Register0 << i))
                {
                    std::string reg;
                    sourceFile >> reg;
                    instruction.instruction |= resolveRegisterName(reg, i);
                }
            }

            if (instructionFormat & AnyAddress)
            {
                std::string address;
                sourceFile >> address;
                instruction.instruction = resolveAddress(binary.getLocation(), address, instructionFormat & AnyAddress, instruction.instruction);
            }
            else if (instructionFormat & Immediate)
            {
                std::uint32_t immediate;
                sourceFile >> immediate;
                instruction.immediate |= immediate;
            }

            binary.align(INSTRUCTION_SIZE);
            binary.writeWord(instruction.instruction);
        }
        else if (current == "word")
        {
            std::uint32_t data;
            sourceFile >> data;
            binary.align(INSTRUCTION_SIZE);
            binary.writeWord(data);
        }
        else if (current == "byte")
        {
            int number;
            sourceFile >> number;
            std::uint8_t data = reinterpret_cast<std::int8_t&>(number);
            binary.writeByte(data);
        }
        else if (current == "space")
        {
            unsigned int spaces;
            sourceFile >> spaces;
            binary.pad(spaces);
        }
        else if (current == "align")
        {
            unsigned int alignmentPower;
            sourceFile >> alignmentPower;
            unsigned int alignment = 1;
            for (int i = 0; i < alignmentPower; i++)
            {
                alignment *= 2;
            }
            binary.align(alignment);
        }
        else if (current == "asciiz")
        {
            sourceFile.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
            std::uint32_t start = sourceFile.tellg();
            sourceFile.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
            std::uint32_t end = sourceFile.tellg();
            sourceFile.seekg(start);
            int size = end - start - 1;
            std::uint8_t* data = new std::uint8_t[size + 1];
            sourceFile.read(reinterpret_cast<char*>(data), size);
            data[size] = 0;
            binary.writeData(data, size + 1);
            delete[] data;
            sourceFile.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
        }
        else if (current[0] == '#')
        {
            sourceFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else if (current[current.length() - 1] == ':')
        {
            current.pop_back();
            labelLocations[current] = binary.getLocation();
        }
        else
        {
            throw std::exception(std::string("Illegal symbol: " + current).c_str());
        }
    }

    for (UnresolvedAddressLocation unresolvedAddressLocation : unresolvedAddressLocations)
    {
        if (labelLocations.count(unresolvedAddressLocation.label))
        {
            binary.setLocation(unresolvedAddressLocation.location);
            std::uint32_t instruction = unresolvedAddressLocation.instruction;

            switch (unresolvedAddressLocation.type)
            {
            case DirectAddressAbsolute:
                instruction |= labelLocations.at(unresolvedAddressLocation.label);
                break;
            case DirectAddressOffset:
                instruction |= labelLocations.at(unresolvedAddressLocation.label) - unresolvedAddressLocation.location;
                break;
            case IndirectAddressAbsolute:
                instruction |= (instruction & DIRECT_ADDRESS_OFFSET_MASK) + labelLocations.at(unresolvedAddressLocation.label);
                break;
            default:
                break;
            }

            binary.writeWord(instruction);
        }
        else
        {
            throw std::exception(std::string("Unresolved label: " + unresolvedAddressLocation.label).c_str());
        }
    }

    binary.setLocation(BinaryBuilder::END);
    binary.align(INSTRUCTION_SIZE);
}

std::uint32_t Assembler::resolveRegisterName(const std::string& registerName, std::uint32_t registerSlot)
{
    const static std::unordered_map<std::string, Register> REGISTER_NAMES = {
        { "zero", ZERO },
        { "at", AT },
        { "v0", V0 }, { "v1", V1 },
        { "a0", A0 }, { "a1", A1 }, { "a2", A2 }, { "a3", A3 },
        { "t0", T0 }, { "t1", T1 }, { "t2", T2 }, { "t3", T3 }, { "t4", T4 }, { "t5", T5 }, { "t6", T6 }, { "t7", T7 },
        { "s0", S0 }, { "s1", S1 }, { "s2", S2 }, { "s3", S3 }, { "s4", S4 }, { "s5", S5 }, { "s6", S6 }, { "s7", S7 },
        { "t8", T8 }, { "t9", T9 },
        { "k0", K0 }, { "k1", K1 },
        { "gp", GP },
        { "sp", SP },
        { "fp", FP },
        { "ra", RA }
    };

    if (REGISTER_NAMES.count(registerName))
    {
        return REGISTER_NAMES.at(registerName) << (REGISTER_0_OFFSET - REGISTER_BIT * registerSlot);
    }

    int registerId = -1;
    try
    {
        registerId = std::stoi(registerName);
    }
    catch (const std::exception& e)
    {
    }

    if (registerId < 0 || registerId > 31)
    {
        throw std::exception(std::string("Illegal register: " + registerName).c_str());
    }

    return registerId << (REGISTER_0_OFFSET - REGISTER_BIT * registerSlot);
}

void Assembler::setLabelLocation(const std::string& labelName, std::uint32_t labelLocation)
{
    labelLocations[labelName] = labelLocation;
}

std::uint32_t Assembler::resolveAddress(std::uint32_t instructionLocation, const std::string& address, std::uint32_t type, std::uint32_t instruction)
{
    switch (type)
    {
    case DirectAddressAbsolute:
        if (labelLocations.count(address))
        {
            return instruction | labelLocations.at(address);
        }
        break;
    case DirectAddressOffset:
        if (labelLocations.count(address))
        {
            return instruction | (labelLocations.at(address) - instructionLocation);
        }
        break;
    case IndirectAddressAbsolute:
        if (labelLocations.count(address))
        {
            return instruction | labelLocations.at(address);
        }
        break;
        //return instruction | resolveRegisterName(address, 1);
        break;
    default:
        break;
    }

    unresolvedAddressLocations.push_back({ instructionLocation, address, type, instruction });
    return instruction;
}
