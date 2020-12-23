namespace kasm
{
		template <typename T>
		T& Program::at(program_pos_t address)
		{
			if (address >= TEXT_SEGMENT_OFFSET && address < TEXT_SEGMENT_OFFSET + TEXT_SEGMENT_SIZE)
			{
				if (textSegment.size() < address + sizeof(T))
				{
					textSegment.resize(address + sizeof(T));
				}

				return *reinterpret_cast<T*>(textSegment.data() + address);
			}
			else if (address >= DATA_SEGMENT_OFFSET && address < DATA_SEGMENT_OFFSET + DATA_SEGMENT_SIZE)
			{
				program_pos_t workingAddress = address - DATA_SEGMENT_OFFSET;

				if (dataSegment.size() < workingAddress + sizeof(T))
				{
					dataSegment.resize(workingAddress + sizeof(T));
				}

				return *reinterpret_cast<T*>(dataSegment.data() + address);
			}
		}

		template <typename T>
		void Program::write(T data)
		{
			at<T>(position) = data;
			position += sizeof(T);
		}
} // namespace kasm
