#pragma once

#include <fstream>
#include <string>

class BinaryBuilder
{
public:
	BinaryBuilder(const std::string& programPath);
	~BinaryBuilder();

	void align(unsigned int alignment);
	void writeWord(std::uint32_t word);
	void writeByte(std::uint8_t byte);
	void writeData(const std::uint8_t* pData, unsigned int size);
	void pad(unsigned int size);
	long getLocation();
	void setLocation(long location);

	static const int BEG = 0;
	static const int END = -1;

private:
	std::ofstream programFile;
};
