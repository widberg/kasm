#pragma once

#include <climits>
#include <cstdint>

const int INSTRUCTION_SIZE = sizeof(std::uint32_t);
const int INSTRUCTION_BIT = CHAR_BIT * INSTRUCTION_SIZE;
const int OPCODE_BIT = 6;
const int REGISTER_BIT = 5;
const int ADDRESS_BIT = 16;
const int IMMEDIATE_BIT = 16;

const std::uint32_t OPCODE_OFFSET = INSTRUCTION_BIT - OPCODE_BIT;
const std::uint32_t REGISTER_0_OFFSET = OPCODE_OFFSET - REGISTER_BIT;
const std::uint32_t REGISTER_1_OFFSET = REGISTER_0_OFFSET - REGISTER_BIT;
const std::uint32_t REGISTER_2_OFFSET = REGISTER_1_OFFSET - REGISTER_BIT;
const std::uint32_t ADDRESS_OFFSET = 0;
const std::uint32_t IMMEDIATE_OFFSET = 0;

const std::uint32_t OPCODE_MASK = 0b111111;
const std::uint32_t REGISTER_MASK = 0b11111;
const std::uint32_t IMMEDIATE_MASK = 0xFFFF;
const std::uint32_t RELATIVE_ADDRESS_MASK = 0xFFFF;
const std::uint32_t ABSOLUTE_ADDRESS_MASK = 0x03FFFFFF;

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
