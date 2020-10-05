#include "binaryBuilder.hpp"

#include <exception>

namespace kasm
{
	BinaryBuilder::BinaryBuilder(const std::string& programPath)
	{
		programFile.open(programPath, std::ios::binary);
	}

	void BinaryBuilder::open(const std::string& programPath)
	{
		programFile.open(programPath, std::ios::binary);
		if (programFile.fail())
		{
			throw std::exception(std::string("Unable to open binary file: " + programPath).c_str());
		}
	}

	void BinaryBuilder::close()
	{
		unsigned int newEOFLocation = getLocation();
		setLocation(END);
		unsigned int trueEOFLocation = getLocation();
		unsigned int eofPadding = newEOFLocation - trueEOFLocation;
		if (eofPadding > 0)
		{
			pad(eofPadding - 1);
			writeByte(0);
		}
		programFile.close();
	}

	void BinaryBuilder::align(unsigned int alignment)
	{
		unsigned int r = getLocation() % alignment;
		if (r) pad(alignment - r);
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

	void BinaryBuilder::writeString(const char* string, unsigned int size)
	{
		programFile.write(string, size);
	}

	void BinaryBuilder::pad(unsigned int size)
	{
		if (size > 0)
		{
			programFile.seekp(size, std::ios::cur);
		}
	}

	std::uint32_t BinaryBuilder::getLocation()
	{
		return static_cast<std::uint32_t>(programFile.tellp());
	}

	void BinaryBuilder::setLocation(std::uint32_t location)
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
}
