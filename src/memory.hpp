#ifndef MEMORY_HPP

#include <list>
#include <vector>

#include <debug.hpp>
#include "program.hpp"
#include "specification.hpp"

namespace kasm
{
	enum class SectionGuard
	{
		None,
		Any,
		TextOnly,
		DataOnly,
	};

	class Memory
	{
	public:
		Memory(const Program& program);

		template <typename T>
		T& at(address_t address, SectionGuard sectionGuard = SectionGuard::Any);

		template <typename T>
		const T& at(address_t address, SectionGuard sectionGuard = SectionGuard::Any) const;
	private:
		struct Bucket
		{
			address_t beginAddress;
			address_t endAddress;
			std::vector<byte_t> data;
		};

		byte_t* create(address_t beginAddress, address_t endAddress);
		byte_t* search(address_t beginAddress, address_t endAddress);
		void merge(std::list<Bucket>::iterator begin, std::list<Bucket>::iterator end);

		std::list<Bucket> buckets;
	};
} // namespace kasm

#include "memory.tpp"

#endif // !MEMORY_HPP
