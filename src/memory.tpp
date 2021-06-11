namespace kasm
{
	template <typename T>
	T& Memory::at(address_t address, SectionGuard sectionGuard)
	{
		KASM_ASSERT(sectionGuard is allowed);

		for (auto bucket : buckets)
		{
			if (address >= bucket.beginAddress && address < bucket.endAddress)
			{
				// idk
			}
		}
	}

	template <typename T>
	const T& Memory::at(address_t address, SectionGuard sectionGuard) const
	{
		return at<T>(address);
	}
} // namespace kasm
