#include "compoundInputFileStream.hpp"

namespace kasm
{
	CompoundInputFileStream::CompoundInputFileStream(const std::string& aFileName)
	{
		include(aFileName);
		isEof = false;
	}

	char CompoundInputFileStream::peek()
	{
		if (isEof) return '\n';
		int c = in.peek();
		if (c == -1)
		{
			c = '\n';
		}
		return c;
	}

	void CompoundInputFileStream::ignore()
	{
		if (isEof) return;
		do
		{
			if (in.eof())
			{
				popFile();
				if (isEof) return;
			}
			in.ignore();
		} while (in.eof());
	}

	void CompoundInputFileStream::seekg(const std::streampos& streamPos)
	{
		if (isEof) return;
		while (streamPos <= totalPos)
		{
			popFile();
			if (isEof) return;
		}
		in.seekg(streamPos - totalPos, std::ios::beg);
	}

	std::streampos CompoundInputFileStream::tellg()
	{
		if (isEof) return 0;
		return totalPos + in.tellg();
	}

	bool CompoundInputFileStream::eof()
	{
		return isEof;
	}

	void CompoundInputFileStream::get(char& c)
	{
		if (isEof) c = '\n';
		c = in.get();
		if (in.eof())
		{
			popFile();
			c = '\n';
			if (isEof) return;
		}
	}

	void CompoundInputFileStream::read(char* buffer, unsigned size)
	{
		if (isEof) return;
		in.read(buffer, size);
	}

	bool CompoundInputFileStream::include(const std::string& aFileName)
	{
		if (in.is_open())
		{
			totalPos += in.tellg();
			streamRestorationDataStack.push({ fileName, in.tellg(), in.peek() });
			in.close();
		}
		fileName = aFileName;
		in.open(fileName, std::ios::binary);
		in.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		return true;
	}

	void CompoundInputFileStream::popFile()
	{
		if (!streamRestorationDataStack.empty())
		{
			StreamRestorationData streamRestorationData = streamRestorationDataStack.top();
			streamRestorationDataStack.pop();
			totalPos -= streamRestorationData.position;
			in.close();
			fileName = streamRestorationData.fileName;
			in.open(fileName, std::ios::binary);
			in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			in.seekg(streamRestorationData.position, std::ios::beg);
		}
		else
		{
			isEof = true;
		}
	}
}
