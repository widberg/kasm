#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <filesystem>
#include <sstream>

#include "specification.hpp"

namespace kasm
{
	typedef quad_word_t program_pos_t;
	typedef quad_word_t program_size_t;

	class Program
	{
	public:
		void seek(program_pos_t position_);
		program_pos_t tell();

		void align(program_size_t alignemnt);
		void pad(program_size_t size);

		byte_t readByte();
		word_t readWord();
		double_word_t readDoubleWord();
		quad_word_t readQuadWord();
		
		void writeByte(byte_t byte);
		void writeWord(word_t word);
		void writeDoubleWord(double_word_t doubleWord);
		void writeQuadWord(quad_word_t quadWord);
		void writeData(const byte_t* const pData, program_size_t size);

		static Program readProgramFromFile(const std::filesystem::path& path);
		static void writeProgramToFile(const std::filesystem::path& path, const Program& program);

		static const program_pos_t BEG = 0;
		static const program_pos_t END = std::numeric_limits<program_pos_t>::max();
	private:
		std::stringstream textSegment, dataSegment;
		program_pos_t position;
	};
} // namespace kasm

#endif // !PROGRAM_HPP
