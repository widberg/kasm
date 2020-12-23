#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <filesystem>
#include <vector>

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

		void align(program_size_t alignment);
		void pad(program_size_t size);
		
		template <typename T>
		T& at(program_pos_t address);

		template <typename T>
		void write(T data);

		void writeData(const byte_t* const pData, program_size_t size);

		static Program readProgramFromFile(const std::filesystem::path& path);
		static void writeProgramToFile(const std::filesystem::path& path, const Program& program);

		static constexpr program_pos_t BEG = 0;
		static constexpr program_pos_t END = std::numeric_limits<program_pos_t>::max();
	private:
		std::vector<byte_t> textSegment;
		std::vector<byte_t> dataSegment;
		program_pos_t position;
	};
} // namespace kasm

#include "program.tpp"

#endif // !PROGRAM_HPP
