#include "binaryBuilder.hpp"

BinaryBuilder::BinaryBuilder(const std::string& programPath)
{
	programFile.open(programPath, std::ios::binary);
}

BinaryBuilder::~BinaryBuilder()
{
}

void BinaryBuilder::align(unsigned int alignment)
{
	unsigned int r = programFile.tellp() % alignment;
	if (r)
	{
		pad(alignment - r);
	}
}

void BinaryBuilder::writeWord(std::uint32_t word)
{
	programFile.write(reinterpret_cast<char*>(&word), sizeof(word));
}

void BinaryBuilder::writeByte(std::uint8_t byte)
{
	programFile.write(reinterpret_cast<char*>(&byte), sizeof(byte));
}

void BinaryBuilder::writeData(const std::uint8_t* pData, unsigned int size)
{
	programFile.write(reinterpret_cast<const char*>(pData), size);
}

void BinaryBuilder::pad(unsigned int size)
{
	if (size > 0)
	{
		programFile.seekp(size - 1, std::ios::cur);
		programFile.write("", 1);
	}
}

long BinaryBuilder::getLocation()
{
	return programFile.tellp();
}

void BinaryBuilder::setLocation(long location)
{
	if (location == END)
	{
		programFile.seekp(0, std::ios::end);
	}
	else
	{
		programFile.seekp(location, std::ios::beg);
	}
}
