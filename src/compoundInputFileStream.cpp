#include "compoundInputFileStream.hpp"

namespace kasm
{
	CompoundInputFileStream::CompoundInputFileStream(const std::string& aFileName)
		: in(nullptr)
	{
		include(aFileName);
	}

	CompoundInputFileStream::~CompoundInputFileStream()
	{
		delete in;
	}

	char CompoundInputFileStream::peek()
	{
		if (in == nullptr) return '\n';
		int c = in->peek();
		if (c == -1)
		{
			c = '\n';
		}
		return c;
	}

	void CompoundInputFileStream::ignore()
	{
		if (in == nullptr) return;
		do
		{
			if (in->eof())
			{
				popFile();
				if (in == nullptr) return;
			}
			in->ignore();
		} while (in->eof());
	}

	void CompoundInputFileStream::seekg(const std::streampos& streamPos)
	{
		if (in == nullptr) return;
		while (streamPos <= totalPos)
		{
			popFile();
			if (in == nullptr) return;
		}
		in->clear();
		in->seekg(streamPos - totalPos, std::ios::beg);
		in->clear();
	}

	std::streampos CompoundInputFileStream::tellg()
	{
		if (in == nullptr) return 0;
		in->clear();
		return totalPos + in->tellg();
	}

	bool CompoundInputFileStream::eof()
	{
		return in == nullptr;
	}

	void CompoundInputFileStream::get(char& c)
	{
		if (in == nullptr) c = '\n';
		c = in->get();
		if (in->eof())
		{
			popFile();
			c = '\n';
			if (in == nullptr) return;
		}
	}

	void CompoundInputFileStream::read(char* buffer, unsigned size)
	{
		if (in == nullptr) return;
		in->read(buffer, size);
	}

	bool CompoundInputFileStream::include(const std::string& aFileName)
	{
		if (in != nullptr)
		{
			totalPos += in->tellg();
			streamRestorationDataStack.push({ fileName, in->tellg(), isFile });
			delete in;
		}
		isFile = true;
		fileName = aFileName;
		identifier = fileName;
		in = new std::ifstream(fileName, std::ios::binary);
		in->exceptions(std::ifstream::failbit | std::ifstream::badbit);

		return true;
	}

	bool CompoundInputFileStream::pushString(const std::string& str)
	{
		if (in != nullptr)
		{
			totalPos += in->tellg();
			streamRestorationDataStack.push({ fileName, in->tellg(), isFile });
			delete in;
		}
		isFile = false;
		fileName = str;
		in = new std::istringstream(fileName, std::ios::binary);
		in->exceptions(std::ifstream::failbit | std::ifstream::badbit);

		return true;
	}

	bool CompoundInputFileStream::put(char c)
	{
		return pushString(std::string(1, c));
	}

	void CompoundInputFileStream::popFile()
	{
		if (!streamRestorationDataStack.empty())
		{
			StreamRestorationData streamRestorationData = streamRestorationDataStack.top();
			streamRestorationDataStack.pop();
			totalPos -= streamRestorationData.position;
			delete in;
			fileName = streamRestorationData.fileName;
			isFile = streamRestorationData.isFile;
			if (isFile)
			{
				identifier = fileName;
				in = new std::ifstream(fileName, std::ios::binary);
			}
			else
			{
				in = new std::istringstream(fileName, std::ios::binary);
			}
			in->exceptions(std::ifstream::failbit | std::ifstream::badbit);
			in->seekg(streamRestorationData.position, std::ios::beg);
		}
		else
		{
			in = nullptr;
		}
	}

	std::string& CompoundInputFileStream::getIdentifier()
	{
		return identifier;
	}
}
