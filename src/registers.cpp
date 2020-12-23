#include "registers.hpp"

#include "debug.hpp"

namespace kasm
{
	quad_word_t Registers::operator[](Register i) const
	{
		KASM_ASSERT(i >= 0 && i < REGISTER_COUNT, "Invalid register");
		return i ? registers[i] : 0;
	}

	quad_word_t& Registers::operator[](Register i)
	{
		KASM_ASSERT(i >= 0 && i < REGISTER_COUNT, "Invalid register");
		return registers[i];
	}
} // namespace kasm
