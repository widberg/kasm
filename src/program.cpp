#include "program.hpp"

#include <fstream>

#include "debug.hpp"
#include "exception.hpp"

namespace kasm
{
	void Program::seek(program_pos_t position_)
	{
		position = position_;
	}

	program_pos_t Program::tell()
	{
		return position;
	}

	void Program::align(program_size_t alignment)
	{
		program_size_t r = position % alignment;
		if (r)
		{
			position += alignment - r;

			// if bigger, resize vector
		}
	}

	void Program::pad(program_size_t size)
	{
		position += size;

		// if bigger, resize vector
	}

	void Program::writeData(const byte_t* const pData, program_size_t size)
	{
		if (position >= TEXT_SEGMENT_OFFSET && position < TEXT_SEGMENT_OFFSET + TEXT_SEGMENT_SIZE)
		{
			if (textSegment.size() < position + size)
			{
				textSegment.resize(position + size);
			}

			std::memcpy(textSegment.data() + position, pData, size);
		}
		else if (position >= DATA_SEGMENT_OFFSET && position < DATA_SEGMENT_OFFSET + DATA_SEGMENT_SIZE)
		{
			program_pos_t workingAddress = position - DATA_SEGMENT_OFFSET;

			if (dataSegment.size() < workingAddress + size)
			{
				dataSegment.resize(workingAddress + size);
			}

			std::memcpy(dataSegment.data() + position, pData, size);
		}
	}

	Program Program::readProgramFromFile(const std::filesystem::path& path)
	{
		Program program;

		std::ifstream programFile(path);

		if (programFile.fail())
		{
			throw Exception("Failed to open file");
		}

		ProgramHeader programHeader;

		programFile.read(reinterpret_cast<char*>(&programHeader), sizeof(programHeader));

		if (programHeader.magicNumber != MAGIC_NUMBER)
		{
			throw Exception("Bad magic number");
		}

		if (programHeader.version != VERSION)
		{
			throw Exception("Version mismatch");
		}

		programFile.seekg(programHeader.textSegmentBegin, std::ios::beg);
		program.textSegment.resize(programHeader.textSegmentSize);
		programFile.read(reinterpret_cast<char*>(program.textSegment.data()), programHeader.textSegmentSize);

		programFile.seekg(programHeader.dataSegmentBegin, std::ios::beg);
		program.dataSegment.resize(programHeader.dataSegmentSize);
		programFile.read(reinterpret_cast<char*>(program.dataSegment.data()), programHeader.dataSegmentSize);
	}

	void Program::writeProgramToFile(const std::filesystem::path& path, const Program& program)
	{
		std::ofstream programFile(path);

		if (programFile.fail())
		{
			throw Exception("Failed to open file");
		}

		ProgramHeader programHeader;

		programHeader.magicNumber = MAGIC_NUMBER;
		programHeader.version = VERSION;
		programHeader.textSegmentBegin = sizeof(programHeader);
		programHeader.textSegmentSize = program.textSegment.size();
		programHeader.dataSegmentBegin = programHeader.textSegmentBegin + programHeader.textSegmentSize;
		programHeader.dataSegmentSize = program.dataSegment.size();

		programFile.write(reinterpret_cast<const char*>(&programHeader), sizeof(programHeader));
		programFile.write(reinterpret_cast<const char*>(program.textSegment.data()), programHeader.textSegmentSize);
		programFile.write(reinterpret_cast<const char*>(program.dataSegment.data()), programHeader.dataSegmentSize);
	}
} // namespace kasm
