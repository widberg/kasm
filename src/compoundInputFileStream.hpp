#pragma once

#include <fstream>
#include <istream>
#include <stack>
#include <sstream>
#include <string>

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
		void popFile();

		struct StreamRestorationData
		{
			std::string fileName;
			std::streampos position;
			bool isFile;
		};

		std::istream* in;
		std::string fileName;
		std::streampos totalPos;
		bool isFile;

		std::string identifier;

		std::stack<StreamRestorationData> streamRestorationDataStack;
	};
}
