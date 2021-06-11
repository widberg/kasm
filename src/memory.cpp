#include "memory.hpp"

#include <algorithm>

namespace kasm
{
	Memory::Memory(const Program& program)
	{

	}

	byte_t* Memory::create(address_t beginAddress, address_t endAddress)
	{

	}

	byte_t* Memory::search(address_t beginAddress, address_t endAddress)
	{
		std::list<Bucket>::iterator begin;
		std::list<Bucket>::iterator end;

		std::list<Bucket>::iterator it;
		for (it = buckets.begin(); it != buckets.end(); ++it)
		{

		}

		end = it;

		end->endAddress = endAddress;

		merge(begin, end);

		return begin->data.data() + (beginAddress - begin->beginAddress);
	}

	void Memory::merge(std::list<Bucket>::iterator begin, std::list<Bucket>::iterator end)
	{
		Bucket& bucket = *begin;

		++begin;

		for (auto it = begin; it != end;)
		{
			bucket.data.resize(it->endAddress - bucket.beginAddress);
			std::copy(it->data.begin(), it->data.end(), bucket.data.data() + (it->beginAddress - bucket.beginAddress));
			buckets.erase(it++);
		}
	}
} // namespace kasm
