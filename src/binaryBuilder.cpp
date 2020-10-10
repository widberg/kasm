#include "binaryBuilder.hpp"

#include <exception>
#include <fstream>

#include "common.hpp"

namespace kasm
{
	BinaryBuilder::BinaryBuilder(const std::string& programPath)
	{
		open(programPath);
	}

	void BinaryBuilder::open(const std::string& aProgramPath)
	{
		programPath = aProgramPath;
		cursor = 0;
		textSegment.clear();
		textSegment.str("");
		dataSegment.clear();
		dataSegment.str("");
	}

	void BinaryBuilder::close()
	{
		std::ofstream programFile(programPath, std::ios::binary);

		std::string textSegmentString = textSegment.str();
		std::string dataSegmentString = dataSegment.str();

		ProgramHeader programHeader;
		programHeader.textSegmentBegin = sizeof(programHeader);
		programHeader.textSegmentLength = textSegmentString.size();
		programHeader.dataSegmentBegin = programHeader.textSegmentBegin + programHeader.textSegmentLength;
		programHeader.dataSegmentLength = dataSegmentString.size();
		programFile.write(reinterpret_cast<char*>(&programHeader), sizeof(programHeader));
		programFile.write(textSegmentString.c_str(), programHeader.textSegmentLength);
		programFile.write(dataSegmentString.c_str(), programHeader.dataSegmentLength);

		programFile.close();
	}

	void BinaryBuilder::align(unsigned int alignment)
	{
		unsigned int r = getLocation() % alignment;
		if (r) pad(alignment - r);
	}

	void BinaryBuilder::writeWord(std::uint32_t word)
	{
		if (cursor < DATA_SEGMENT_OFFSET)
		{
			textSegment.write(reinterpret_cast<char*>(&word), sizeof(word));
		}
		else
		{
			dataSegment.write(reinterpret_cast<char*>(&word), sizeof(word));
		}
		cursor += 4;
	}

	void BinaryBuilder::writeByte(std::uint8_t byte)
	{
		dataSegment.put(byte);
		cursor++;
	}

	void BinaryBuilder::writeData(const std::uint8_t* pData, unsigned int size)
	{
		dataSegment.write(reinterpret_cast<const char*>(pData), size);
		cursor += size;
	}

	void BinaryBuilder::writeString(const char* string, unsigned int size)
	{
		dataSegment.write(string, size);
		cursor += size;
	}

	void BinaryBuilder::pad(unsigned int size)
	{
		setLocation(getLocation() + size);
	}

	std::uint32_t BinaryBuilder::getLocation()
	{
		return cursor;
	}

	void BinaryBuilder::setLocation(std::uint32_t location)
	{
		if (location == END)
		{
			dataSegment.clear();
			dataSegment.seekp(0, std::ios::end);
			dataSegment.clear();
			cursor = DATA_SEGMENT_OFFSET + dataSegment.tellp();
			dataSegment.clear();
		}
		else
		{
			if (location < DATA_SEGMENT_OFFSET)
			{
				textSegment.clear();
				textSegment.seekp(0, std::ios::end);
				textSegment.clear();
				if (location > textSegment.tellp())
				{
					textSegment.clear();
					cursor = textSegment.tellp();
					textSegment.clear();
					while (cursor < location)
					{
						writeByte(0);
					}
				}
				else
				{
					textSegment.clear();
					textSegment.seekp(location, std::ios::beg);
					textSegment.clear();
				}
			}
			else
			{
				dataSegment.clear();
				dataSegment.seekp(0, std::ios::end);
				dataSegment.clear();
				if (location - DATA_SEGMENT_OFFSET > dataSegment.tellp())
				{
					dataSegment.clear();
					cursor = DATA_SEGMENT_OFFSET + dataSegment.tellp();
					dataSegment.clear();
					while (cursor < location)
					{
						writeByte(0);
					}
				}
				else
				{
					dataSegment.clear();
					dataSegment.seekp(location - DATA_SEGMENT_OFFSET, std::ios::beg);
					dataSegment.clear();
				}
			}
			cursor = location;
		}
	}

	BinaryBuilder::SegmentType BinaryBuilder::getSegmentType() const
	{
		if (cursor < DATA_SEGMENT_OFFSET) return SegmentType::TEXT;
		return SegmentType::DATA;
	}

	void BinaryBuilder::setSegmentType(SegmentType segmentType)
	{
		if (segmentType == SegmentType::TEXT)
		{
			textSegment.seekp(0, std::ios::end);
			textSegment.clear();
			cursor = textSegment.tellp();
		}
		else
		{
			dataSegment.seekp(0, std::ios::end);
			dataSegment.clear();
			cursor = DATA_SEGMENT_OFFSET + dataSegment.tellp();
		}
	}
}
