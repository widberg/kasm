#pragma once

#include <fstream>
#include <istream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace kasm
{
	class CompoundInputFileStream
	{
	public:
		CompoundInputFileStream(const std::string& aFileName);
		~CompoundInputFileStream();

		char peek();
		void ignore();
		void seekg(const std::streampos& streamPos);
		std::streampos tellg();
		bool eof();
		void get(char& c);
		void read(char* buffer, unsigned size);

		bool include(const std::string& aFileName);
		bool pushString(const std::string& str);
		bool put(char c);

		std::string& getIdentifier();
	private:
		std::streampos tellgInternal() const;
		void pushEntry(const std::string& entryName);
		void debugPrint() const;

		struct FileData
		{
			std::istream* stream;
			std::streampos length;
		};

		struct FileEntry
		{
			FileData* file;
			std::streampos start;
			std::streampos end;
			std::streampos offset;
		};

		std::istream* in;
		std::string identifier;

		std::unordered_map<std::string, FileData> files;
		std::list<FileEntry> fileEntries;
		std::list<FileEntry>::iterator it;
	};
}
