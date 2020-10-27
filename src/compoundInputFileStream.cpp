#include "compoundInputFileStream.hpp"

#include <iostream>

#include "debug.hpp"

namespace kasm
{
	CompoundInputFileStream::CompoundInputFileStream(const std::string& aFileName)
		: in(nullptr)
	{
		include(aFileName);
	}

	CompoundInputFileStream::~CompoundInputFileStream()
	{
		for (auto it = files.begin(); it != files.end(); it++)
		{
			delete it->second.stream;
		}
	}

	char CompoundInputFileStream::peek()
	{
		if (eof()) return '\0';
		//std::cout << (char)(in->peek());
		return in->peek();
	}

	void CompoundInputFileStream::ignore()
	{
		if (eof()) return;
		in->ignore();
	}

	void CompoundInputFileStream::seekg(const std::streampos& streamPos)
	{
		if (streamPos > fileEntries.back().end) throw std::exception("compound file stream attempting to seek past end of available input");
		for (auto i = fileEntries.begin(); i != fileEntries.end(); i++)
		{
			if (streamPos >= i->start && streamPos <= i->end)
			{
				it = i;
				in = it->file->stream;
				in->seekg(streamPos - it->start + it->offset, std::ios::beg);
				break;
			}
		}
	}

	std::streampos CompoundInputFileStream::tellgInternal() const
	{
		return it->start + in->tellg() - it->offset;
	}

	std::streampos CompoundInputFileStream::tellg()
	{
		if (eof()) return std::numeric_limits<std::streampos>::max();
		return tellgInternal();
	}

	bool CompoundInputFileStream::eof()
	{
		if (in == nullptr) return true;
		if (it == fileEntries.end()) return true;
		ASSEMBLER_ASSERT(it->file->stream == in, "WEEE WOOO WEEE WOO");
		while (in->peek() == std::istream::traits_type::eof() || tellgInternal() > it->end)
		{
			in->clear();
			it++;
			if (it == fileEntries.end())
			{
				in = nullptr;
				return true;
			}
			in = it->file->stream;
			in->seekg(it->offset, std::ios::beg);
		}
		return false;
	}

	void CompoundInputFileStream::get(char& c)
	{
		if (eof()) return;
		in->get(c);
	}

	void CompoundInputFileStream::read(char* buffer, unsigned size)
	{
		if (eof()) return;
		in->read(buffer, size);
	}

	bool CompoundInputFileStream::include(const std::string& aFileName)
	{

		if (!files.count(aFileName))
		{
			std::istream* tmpIn = new std::ifstream(aFileName, std::ios::binary | std::ios::ate);
			tmpIn->exceptions(std::ifstream::failbit | std::ifstream::badbit);
			std::streampos length = tmpIn->tellg();
			tmpIn->seekg(0, std::ios::beg);
			files[aFileName] = { tmpIn, length };
		}

		identifier = aFileName;
		pushEntry(aFileName);

		return true;
	}

	bool CompoundInputFileStream::pushString(const std::string& str)
	{
		if (!files.count(str))
		{
			std::istream* tmpIn = new std::istringstream(str, std::ios::binary);
			tmpIn->exceptions(std::istringstream::failbit | std::istringstream::badbit);
			files[str] = { tmpIn, std::streampos(str.length()) };
		}

		pushEntry(str);

		return true;
	}

	bool CompoundInputFileStream::put(char c)
	{
		return pushString(std::string(1, c));
	}

	std::string& CompoundInputFileStream::getIdentifier()
	{
		return identifier;
	}

	void CompoundInputFileStream::pushEntry(const std::string& entryName)
	{
		FileData& file = files[entryName];

		if (eof())
		{
			fileEntries.push_back({ &file, 0, file.length - std::streampos(1), 0 });
			it = --fileEntries.end();
		}
		else
		{
			it->end = tellgInternal();
			std::streampos start = it->end + std::streampos(1);
			auto i = it;
			i++;
			fileEntries.insert(i, { &file, start, start + file.length - std::streampos(1), 0 });
			fileEntries.insert(i, { it->file, start + file.length, start + file.length + it->file->length - (it->end - it->start + it->offset) - std::streampos(2), it->end - it->start + it->offset});
			
			for (; i != fileEntries.end(); i++)
			{
				i->start += file.length;
				i->end += file.length;
			}
			it++;
		}

		in = it->file->stream;
		in->seekg(it->offset);
	}

	void CompoundInputFileStream::debugPrint() const
	{
		std::cout << "Debug Print Begin:" << std::endl;
		std::streampos cursor = tellgInternal();
		for (auto i : fileEntries)
		{
			std::cout << (i.start == it->start ? "->" : "") << "[" << i.start << ", " << i.end << "]" << std::endl;
			i.file->stream->clear();
			std::streampos sav = i.file->stream->tellg();
			i.file->stream->seekg(i.offset, std::ios::beg);
			
			if (cursor == 1295)
			{
				KASM_BREAKPOINT();
			}
			if (cursor >= i.start && cursor <= i.end)
			{
				std::streampos size = cursor - i.start;
				std::string buffer;
				buffer.resize(size);
				i.file->stream->read(static_cast<char*>(buffer.data()), size);
				std::cout << buffer;

				std::cout << "[->]";

				size = i.end - cursor + 1;
				buffer.resize(size);
				i.file->stream->read(static_cast<char*>(buffer.data()), size);
				std::cout << buffer << std::endl;
			}
			else
			{
				std::streampos size = i.end - i.start + 1;
				std::string buffer;
				buffer.resize(size);
				i.file->stream->read(static_cast<char*>(buffer.data()), size);
				std::cout << buffer << std::endl;
			}

			i.file->stream->clear();
			i.file->stream->seekg(sav, std::ios::beg);
		}
		std::cout << "Debug Print End;" << std::endl;
	}
}
