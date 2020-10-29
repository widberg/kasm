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
		CompoundInputFileStream();
		CompoundInputFileStream(const std::string& aFileName, void(*aEoFCallback)(unsigned) = nullptr);
		~CompoundInputFileStream();

		char peek();
		void ignore();
		void seekg(const std::streampos& streamPos);
		std::streampos tellg();
		bool eof(bool giveStub = false);
		void get(char& c);
		void read(char* buffer, unsigned size);
		void setCallback(void(*aEoFCallback)(unsigned));

		void reset();
		unsigned open(const std::string& aFileName, bool setUid = false);
		unsigned include(const std::string& aFileName, bool setUid = false);
		unsigned pushString(const std::string& str, bool setUid = false);
		unsigned put(char c, bool setUid = false);

		std::string& getIdentifier();
	private:
		std::streampos tellgInternal() const;
		unsigned pushEntry(const std::string& entryName, bool setUid = false);
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
			unsigned uid;
		};

		std::istream* in;
		std::string identifier;
		unsigned uid = 0;
		void(*eoFCallback)(unsigned);

		std::unordered_map<std::string, FileData> files;
		std::list<FileEntry> fileEntries;
		std::list<FileEntry>::iterator it;
	};
}
