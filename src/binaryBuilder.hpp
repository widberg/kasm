#pragma once

#include <fstream>
#include <limits>
#include <string>

namespace kasm
{
	class BinaryBuilder
	{
	public:
		BinaryBuilder() {};
		BinaryBuilder(const std::string& programPath);
		~BinaryBuilder() { close(); };

		void open(const std::string& programPath);
		void close();
		void align(unsigned int alignment);
		void writeWord(std::uint32_t word);
		void writeByte(std::uint8_t byte);
		void writeData(const std::uint8_t* pData, unsigned int size);
		void writeString(const char* string, unsigned int size);
		void pad(unsigned int size);
		std::uint32_t getLocation();
		void setLocation(std::uint32_t location);

		static const std::uint32_t BEG = 0;
		static const std::uint32_t END = std::numeric_limits<std::uint32_t>::max();

	private:
		std::ofstream programFile;
	};
}
