#pragma once

#include <climits>
#include <cstdint>

namespace kasm
{
    const int INSTRUCTION_SIZE = sizeof(std::uint32_t);
    const int INSTRUCTION_BIT = CHAR_BIT * INSTRUCTION_SIZE;
    const int OPCODE_BIT = 6;
    const int REGISTER_BIT = 5;
    const int DIRECT_ADDRESS_ABSOLUTE_BIT = INSTRUCTION_BIT - OPCODE_BIT;
    const int DIRECT_ADDRESS_OFFSET_BIT = 16;
    const int IMMEDIATE_BIT = 16;

    union InstructionData
    {
        std::uint32_t instruction;
#pragma pack(push, 1)
#if 'ABCD' == 0x41424344 // if little endian. trash code TODO: make portable
        struct
        {
            std::uint32_t : 11;
            std::uint32_t register2 : REGISTER_BIT;
            std::uint32_t register1 : REGISTER_BIT;
            std::uint32_t register0 : REGISTER_BIT;
            std::uint32_t opcode : OPCODE_BIT;
        };
        std::uint32_t immediate : IMMEDIATE_BIT;
        std::uint32_t directAddressAbsolute : DIRECT_ADDRESS_ABSOLUTE_BIT;
        std::uint32_t directAddressOffset : DIRECT_ADDRESS_OFFSET_BIT;
#else
        struct
        {
            std::uint32_t opcode : OPCODE_BIT;
            std::uint32_t register0 : REGISTER_BIT;
            std::uint32_t register1 : REGISTER_BIT;
            std::uint32_t register2 : REGISTER_BIT;
        };
        struct
        {
            std::uint32_t : OPCODE_BIT;
            std::uint32_t : REGISTER_BIT * 2;
            std::uint32_t immediate : IMMEDIATE_BIT;
        };
        struct
        {
            std::uint32_t : OPCODE_BIT;
            std::uint32_t directAddressAbsolute : DIRECT_ADDRESS_ABSOLUTE_BIT;
        };
        struct
        {
            std::uint32_t : OPCODE_BIT;
            std::uint32_t : REGISTER_BIT * 2;
            std::uint32_t directAddressOffset : DIRECT_ADDRESS_OFFSET_BIT;
        };
#endif
#pragma pack(pop)
    };

    enum Opcode : std::uint32_t
    {
        ADD,
        ADDI,
        ADDIU,
        ADDU,
        AND,
        ANDI,
        BEQ,
        BGEZ,
        BGEZAL,
        BGTZ,
        BLEZ,
        BLTZ,
        BLTZAL,
        BNE,
        DIV,
        DIVU,
        J,
        JAL,
        JR,
        LB,
        LUI,
        LW,
        MFHI,
        MFLO,
        MULT,
        MULTU,
        NOP,
        OR,
        ORI,
        SB,
        SLL,
        SLLV,
        SLT,
        SLTI,
        SLTIU,
        SLTU,
        SRA,
        SRL,
        SRLV,
        SUB,
        SUBU,
        SW,
        SYS,
        XOR,
        XORI,
    };

    enum Register : std::uint8_t
    {
        ZERO,
        AT,
        V0, V1,
        A0, A1, A2, A3,
        T0, T1, T2, T3, T4, T5, T6, T7,
        S0, S1, S2, S3, S4, S5, S6, S7,
        T8, T9,
        K0, K1,
        GP,
        SP,
        FP,
        RA
    };

    enum class AddressType : std::uint8_t
    {
        DirectAddressAbsolute,
        DirectAddressOffset,
        IndirectAddressAbsolute,
    };
}
