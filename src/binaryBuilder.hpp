#pragma once

#include <fstream>
#include <string>

namespace kasm
{
	class BinaryBuilder
	{
	public:
		BinaryBuilder() {};
		BinaryBuilder(const std::string& programPath);
		~BinaryBuilder() {};

		void open(const std::string& programPath);
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
}
