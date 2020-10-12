#pragma once

#include <fstream>
#include <stack>
#include <string>

namespace kasm
{
	class CompoundInputFileStream
	{
	public:
		CompoundInputFileStream(const std::string& aFileName);

		char peek();
		void ignore();
		void seekg(const std::streampos& streamPos);
		std::streampos tellg();
		bool eof();
		void get(char& c);
		void read(char* buffer, unsigned size);

		bool include(const std::string& aFileName);
	private:
		void popFile();

		struct StreamRestorationData
		{
			std::string fileName;
			std::streampos position;
			int peekChar;
		};

		std::ifstream in;
		std::string fileName;
		std::streampos totalPos;

		bool isEof;

		std::stack<StreamRestorationData> streamRestorationDataStack;
	};
}
