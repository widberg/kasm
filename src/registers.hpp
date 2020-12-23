#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include "specification.hpp"

namespace kasm
{
	class Registers
	{
	public:
		quad_word_t operator[](Register i) const;
		quad_word_t& operator[](Register i);
	private:
		quad_word_t registers[REGISTER_COUNT];
	};
} // namespace kasm

#endif // !REGISTERS_HPP
