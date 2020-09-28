#include "assembler.hpp"

#include <exception>
#include <fstream>
#include <utility>

#include "common.hpp"

Assembler::Assembler()
{
}

Assembler::~Assembler()
{
}

void Assembler::assemble(const std::string& sourcePath, const std::string& programPath)
{
    const static std::unordered_map<std::string, std::pair<std::uint32_t, std::uint32_t>> INSTRUCTION_FORMATS = {
        { "nop", { NOP, None } },
        { "j", { JUMP, DirectAddressAbsolute } },
        { "jr", { JUMP_REGISTER, Register0 } },
        { "jal", { JUMP_AND_LINK, DirectAddressAbsolute } },
        { "jalr", { JUMP_AND_LINK_REGISTER, Register0 } },
        { "b", { BRANCH_UNCONDITIONAL, DirectAddressOffset } },
        { "beq", { BRANCH_EQUALS, Register0 | Register1 | DirectAddressOffset } },
        { "li", { LOAD_IMMEDIATE, Register0 | Immediate } },
        { "lw", { LOAD_WORD, Register0 | IndirectAddressOffset } },
        { "sw", { SAVE_WORD, Register0 | IndirectAddressOffset } },
        { "lb", { LOAD_BYTE, Register0 | IndirectAddressOffset } },
        { "sb", { SAVE_BYTE, Register0 | IndirectAddressOffset } },
        { "sys", { SYSTEM_CALL, None} },
        { "add", { ADD, Register0 | Register1 | Register2 } },
        { "sub", { SUB, Register0 | Register1 | Register2 } },
        { "mul", { MUL, Register0 | Register1 | Register2 } },
        { "div", { DIV, Register0 | Register1 | Register2 } },
        { "mod", { MOD, Register0 | Register1 | Register2 } },
    };

    labelLocations.clear();
    unresolvedLabelLocations.clear();

    std::ifstream sourceFile(sourcePath);
    std::ofstream programFile(programPath, std::ios::binary);

    std::string current;
    while (sourceFile >> current)
    {
        if (INSTRUCTION_FORMATS.count(current))
        {
            std::pair<std::uint32_t, std::uint32_t> instructionData = INSTRUCTION_FORMATS.at(current);
            std::uint32_t instruction = instructionData.first << OPCODE_OFFSET;

            for (int i = 0; i < 3; i++)
            {
                if (instructionData.second & (Register0 << i))
                {
                    std::string reg;
                    sourceFile >> reg;
                    instruction |= resolveRegisterName(reg, i);
                }
            }

            if (instructionData.second & DirectAddressAbsolute)
            {
                std::string label;
                sourceFile >> label;
                instruction |= getLabelLocation(programFile.tellp(), label);
            }

            if (instructionData.second & DirectAddressOffset)
            {
                std::string label;
                sourceFile >> label;
                instruction |= getLabelLocation(programFile.tellp(), label);
            }

            if (instructionData.second & IndirectAddressOffset)
            {
                std::string label;
                sourceFile >> label;
                instruction |= getLabelLocation(programFile.tellp(), label);
            }

            if (instructionData.second & Immediate)
            {
                std::uint32_t immediate;
                sourceFile >> immediate;
                instruction |= immediate;
            }

            programFile.write(reinterpret_cast<char*>(&instruction), INSTRUCTION_SIZE);
        }
        else if (current == "word")
        {
            std::uint32_t data;
            sourceFile >> data;
            programFile.write(reinterpret_cast<char*>(&data), INSTRUCTION_SIZE);
        }
        else if (current == "byte")
        {
            std::uint8_t data;
            sourceFile >> data;
            programFile.write(reinterpret_cast<char*>(&data), 1);
        }
        else if (current == "space")
        {
            unsigned int spaces;
            sourceFile >> spaces;
            programFile.seekp(spaces);
        }
        else if (current[0] == '#')
        {
            sourceFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else if (current[current.length() - 1] == ':')
        {
            current.pop_back();
            labelLocations[current] = programFile.tellp();
        }
        else
        {
            throw std::exception(std::string("Illegal symbol: " + current).c_str());
        }
    }

    for (std::pair<std::uint32_t, std::string> entry : unresolvedLabelLocations)
    {
        if (labelLocations.count(entry.second))
        {
            programFile.seekp(entry.first, std::ios::beg);
            programFile.write(reinterpret_cast<char*>(&labelLocations[entry.second]), 2);
        }
        else
        {
            throw std::exception(std::string("Unresolved label: " + entry.second).c_str());
        }
    }
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

void Assembler::setLabelLocation(const std::string& labelName, std::uint16_t labelLocation)
{
    labelLocations[labelName] = labelLocation;
}

std::uint16_t Assembler::getLabelLocation(std::uint32_t instructionLocation, const std::string& labelName)
{
    if (labelLocations.count(labelName))
    {
        return labelLocations.at(labelName);
    }

    unresolvedLabelLocations.insert(std::pair<std::uint32_t, std::string>(instructionLocation, labelName));

    return 0;
}
