#pragma once

#include <climits>
#include <cstdint>

const int INSTRUCTION_SIZE = sizeof(std::uint32_t);
const int INSTRUCTION_BIT = CHAR_BIT * INSTRUCTION_SIZE;
const int OPCODE_BIT = 6;
const int REGISTER_BIT = 5;
const int DIRECT_ADDRESS_ABSOLUTE_BIT = INSTRUCTION_BIT - OPCODE_BIT;
const int DIRECT_ADDRESS_OFFSET_BIT = 16;
const int IMMEDIATE_BIT = 16;

const std::uint32_t OPCODE_OFFSET = INSTRUCTION_BIT - OPCODE_BIT;
const std::uint32_t REGISTER_0_OFFSET = OPCODE_OFFSET - REGISTER_BIT;
const std::uint32_t REGISTER_1_OFFSET = REGISTER_0_OFFSET - REGISTER_BIT;
const std::uint32_t REGISTER_2_OFFSET = REGISTER_1_OFFSET - REGISTER_BIT;
const std::uint32_t ADDRESS_OFFSET = 0;
const std::uint32_t IMMEDIATE_OFFSET = 0;

const std::uint32_t REGISTER_MASK = 0b11111;
const std::uint32_t DIRECT_ADDRESS_OFFSET_MASK = 0xFFFF;
const std::uint32_t DIRECT_ADDRESS_ABSOLUTE_MASK = 0x03FFFFFF;

union InstructionData
{
    std::uint32_t instruction;
#if 'ABCD' == 0x41424344 // if little endian. trash code TODO: make portable
#pragma pack(1)
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
#pragma pack(1)
    struct
    {
        std::uint32_t opcode : OPCODE_BIT;
        std::uint32_t register0 : REGISTER_BIT;
        std::uint32_t register1 : REGISTER_BIT;
        std::uint32_t register2 : REGISTER_BIT;
    };
#pragma pack(1)
    struct
    {
        std::uint32_t : OPCODE_BIT;
        std::uint32_t : REGISTER_BIT * 2;
        std::uint32_t immediate : IMMEDIATE_BIT;
    };
#pragma pack(1)
    struct
    {
        std::uint32_t : OPCODE_BIT;
        std::uint32_t directAddressAbsolute : DIRECT_ADDRESS_ABSOLUTE_BIT;
    };
#pragma pack(1)
    struct
    {
        std::uint32_t : OPCODE_BIT;
        std::uint32_t : REGISTER_BIT * 2;
        std::uint32_t directAddressOffset : DIRECT_ADDRESS_OFFSET_BIT;
    };
#endif
};

enum Opcode : std::uint32_t
{
    NOP,
    SYSTEM_CALL,
    JUMP,
    JUMP_REGISTER,
    JUMP_AND_LINK,
    JUMP_AND_LINK_REGISTER,
    BRANCH_UNCONDITIONAL,
    BRANCH_EQUALS,
    BRANCH_NOT_EQUALS,
    BRANCH_LESS_THAN,
    BRANCH_GREATER_THAN,
    BRANCH_LESS_THAN_OR_EQUALS,
    BRANCH_GREATER_THAN_OR_EQUALS,
    LOAD_IMMEDIATE,
    LOAD_ADDRESS,
    LOAD_WORD,
    SAVE_WORD,
    LOAD_BYTE,
    SAVE_BYTE,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
};

enum Register
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

enum Token
{
    None,
    Register0 = 1 << 1,
    Register1 = 1 << 2,
    Register2 = 1 << 3,
    DirectAddressAbsolute = 1 << 4,
    DirectAddressOffset = 1 << 5,
    IndirectAddressAbsolute = 1 << 6,
    Immediate = 1 << 7,

    AnyAddress = DirectAddressAbsolute | DirectAddressOffset | IndirectAddressAbsolute,
};

const static std::unordered_map<std::uint32_t, std::uint32_t> INSTRUCTION_FORMATS = {
        { NOP, None },
        { JUMP, DirectAddressAbsolute },
        { JUMP_REGISTER, Register0 },
        { JUMP_AND_LINK, DirectAddressAbsolute },
        { JUMP_AND_LINK_REGISTER, Register0 | Register1 },
        { BRANCH_UNCONDITIONAL, DirectAddressOffset },
        { BRANCH_EQUALS, Register0 | Register1 | DirectAddressOffset },
        { BRANCH_NOT_EQUALS, Register0 | Register1 | DirectAddressOffset },
        { BRANCH_LESS_THAN, Register0 | Register1 | DirectAddressOffset },
        { BRANCH_GREATER_THAN, Register0 | Register1 | DirectAddressOffset },
        { BRANCH_LESS_THAN_OR_EQUALS, Register0 | Register1 | DirectAddressOffset },
        { BRANCH_GREATER_THAN_OR_EQUALS, Register0 | Register1 | DirectAddressOffset },
        { LOAD_IMMEDIATE, Register0 | Immediate },
        { LOAD_ADDRESS, Register0 | IndirectAddressAbsolute },
        { LOAD_WORD, Register0 | IndirectAddressAbsolute },
        { SAVE_WORD, Register0 | IndirectAddressAbsolute },
        { LOAD_BYTE, Register0 | IndirectAddressAbsolute },
        { SAVE_BYTE, Register0 | IndirectAddressAbsolute },
        { SYSTEM_CALL, None},
        { ADD, Register0 | Register1 | Register2 },
        { SUB, Register0 | Register1 | Register2 },
        { MUL, Register0 | Register1 | Register2 },
        { DIV, Register0 | Register1 | Register2 },
        { MOD, Register0 | Register1 | Register2 },
};
