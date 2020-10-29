#include "compoundInputFileStream.hpp"

#include <iostream>

#include "debug.hpp"

namespace kasm
{
	CompoundInputFileStream::CompoundInputFileStream()
		: eoFCallback(nullptr)
	{
		reset();
	}

	CompoundInputFileStream::CompoundInputFileStream(const std::string& aFileName, void(*aEOFCallback)(unsigned))
		: eoFCallback(aEOFCallback)
	{
		open(aFileName);
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
			if (streamPos >= i->start && streamPos <= i->end && i->file != nullptr)
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
		if (fileEntries.empty()) return 0;
		return it->start + in->tellg() - it->offset;
	}

	std::streampos CompoundInputFileStream::tellg()
	{
		if (eof()) return std::numeric_limits<std::streampos>::max();
		return tellgInternal();
	}

	bool CompoundInputFileStream::eof(bool giveStub)
	{
		if (in == nullptr) return true;
		if (it == fileEntries.end()) return true;
		bool isStub = it->file == nullptr;
		KASM_ASSERT(it->file->stream == in, "WEEE WOOO WEEE WOO");
		while (in->peek() == std::istream::traits_type::eof() || tellgInternal() > it->end || isStub)
		{
			in->clear();

			if (!isStub && it->uid != 0 && giveStub)
			{
				auto i = it;
				i++;
				fileEntries.insert(i, { nullptr, 0, 0, 0, it->uid });
				it->uid = 0;
				it++;
				return false;
			}

			if (eoFCallback != nullptr && it->uid != 0)
			{
				eoFCallback(it->uid);
			}

			it++;
			if (it == fileEntries.end())
			{
				in = nullptr;
				return true;
			}

			isStub = it->file == nullptr;
			if (giveStub && isStub && it->uid != 0) return false;
			if (isStub) continue;

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

	void CompoundInputFileStream::setCallback(void(*aEoFCallback)(unsigned))
	{
		eoFCallback = aEoFCallback;
	}

	void CompoundInputFileStream::reset()
	{
		files.clear();
		fileEntries.clear();
		it = fileEntries.end();
		in = nullptr;
	}

	unsigned CompoundInputFileStream::open(const std::string& aFileName, bool setUid)
	{
		reset();
		return include(aFileName, setUid);
	}

	unsigned CompoundInputFileStream::include(const std::string& aFileName, bool setUid)
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
		return pushEntry(aFileName, setUid);
	}

	unsigned CompoundInputFileStream::pushString(const std::string& str, bool setUid)
	{
		if (!files.count(str))
		{
			std::istream* tmpIn = new std::istringstream(str, std::ios::binary);
			tmpIn->exceptions(std::istringstream::failbit | std::istringstream::badbit);
			files[str] = { tmpIn, std::streampos(str.length()) };
		}

		return pushEntry(str, setUid);
	}

	unsigned CompoundInputFileStream::put(char c, bool setUid)
	{
		return pushString(std::string(1, c));
	}

	std::string& CompoundInputFileStream::getIdentifier()
	{
		return identifier;
	}

	unsigned CompoundInputFileStream::pushEntry(const std::string& entryName, bool setUid)
	{
		FileData& file = files[entryName];

		unsigned newUid = 0;

		if (setUid)
		{
			newUid = ++uid;
		}

		if (eof(true) || it->file == nullptr)
		{
			it = fileEntries.insert(it, { &file, 0, file.length - std::streampos(1), 0, newUid });
		}
		else
		{
			it->end = tellgInternal();
			std::streampos start = it->end + std::streampos(1);
			auto i = it;
			i++;
			fileEntries.insert(i, { &file, start, start + file.length - std::streampos(1), 0, newUid });
			fileEntries.insert(i, { it->file, start + file.length, start + file.length + it->file->length - (it->end - it->start + it->offset) - std::streampos(2), it->end - it->start + it->offset, it->uid });
			
			for (; i != fileEntries.end(); i++)
			{
				i->start += file.length;
				i->end += file.length;
			}
			it->uid = 0;
			it++;
		}

		in = it->file->stream;
		in->seekg(it->offset);

		return it->uid;
	}

	void CompoundInputFileStream::debugPrint() const
	{
		std::cout << "Debug Print Begin:" << std::endl;
		std::streampos cursor = tellgInternal();
		for (auto i : fileEntries)
		{
			if (i.file == nullptr)
			{
				std::cout << "[stub " << i.uid << "]" << std::endl;
				continue;
			}

			std::cout << (i.start == it->start ? "->" : "") << "[" << i.start << ", " << i.end << "]" << std::endl;
			i.file->stream->clear();
			std::streampos sav = i.file->stream->tellg();
			i.file->stream->seekg(i.offset, std::ios::beg);
			
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
