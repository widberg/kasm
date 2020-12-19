#ifndef COMMON_HPP
#define COMMON_HPP

#include "specification.hpp"

#define IS_BIG_ENDIAN 0

namespace kasm
{
	union InstructionData
    {
        instruction_t instruction;
#pragma pack(push, 1)
#if IS_BIG_ENDIAN
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
            std::uint32_t directAddressAbsolute : ADDRESS_ABSOLUTE_BIT;
        };
        struct
        {
            std::uint32_t : OPCODE_BIT;
            std::uint32_t : REGISTER_BIT * 2;
            std::int32_t directAddressOffset : ADDRESS_OFFSET_BIT;
        };
#else
        struct
        {
            std::uint32_t : 11;
            std::uint32_t register2 : REGISTER_BIT;
            std::uint32_t register1 : REGISTER_BIT;
            std::uint32_t register0 : REGISTER_BIT;
            std::uint32_t opcode : OPCODE_BIT;
        };
        std::uint32_t immediate : IMMEDIATE_BIT;
        std::uint32_t directAddressAbsolute : ADDRESS_ABSOLUTE_BIT;
        std::int32_t directAddressOffset : ADDRESS_OFFSET_BIT;
#endif
#pragma pack(pop)
    };
}

#endif // !COMMON_HPP
