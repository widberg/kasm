#include "assembler.hpp"

namespace kasm
{
	bool Assembler::isIdentifierDefined(const std::string& identifier)
	{
		return labelLocations.count(identifier) | macros.count(identifier);
	}

	bool Assembler::resolveAddress(AddressData& address, bool mustResolve)
	{
		switch (address.type)
		{
		case AddressType::DirectAddressAbsolute:
			if (labelLocations.count(address.label))
			{
				address.instructionData.directAddressAbsolute = labelLocations.at(address.label);
				return true;
			}
			break;
		case AddressType::DirectAddressOffset:
			if (labelLocations.count(address.label))
			{
				address.instructionData.directAddressOffset = static_cast<std::int32_t>(labelLocations.at(address.label)) - address.position;
				return true;
			}
			break;
		case AddressType::IndirectAddressOffset:
			if (address.label.empty())
			{
				address.instructionData.register1 = address.reg;
				address.instructionData.directAddressOffset = static_cast<std::int32_t>(address.offset) - address.position;
				return true;
			}
			else if (labelLocations.count(address.label))
			{
				address.instructionData.register1 = address.reg;
				address.instructionData.directAddressOffset = static_cast<std::int32_t>(labelLocations.at(address.label)) + address.offset - address.position;
				return true;
			}
			break;
		case AddressType::DirectAddressAbsoluteWord:
		case AddressType::DirectAddressAbsoluteByte:
		case AddressType::DirectAddressAbsoluteLoad:
			if (labelLocations.count(address.label))
			{
				address.instructionData.instruction = labelLocations.at(address.label);
				return true;
			}
			break;
		default:
			break;
		}

		if (mustResolve)
		{
			throw std::exception(std::string("Unresolved Label: " + address.label).c_str());
		}

		unresolvedAddressLocations.push_back(address);

		return false;
	}

	void Assembler::defineLabel(const std::string& name, std::uint32_t location)
	{
		if (isIdentifierDefined(name))
		{
			throw std::exception(std::string("Redefined Identifier: " + name).c_str());
		}

		labelLocations[name] = location;
	}

	void Assembler::saveSymbolTable(const std::string& symbolTablePath)
	{
		std::ofstream symbolTableFile(symbolTablePath, std::ios::binary);

		for (auto symbol : labelLocations)
		{
			std::uint8_t labelSize = symbol.first.size();
			symbolTableFile.write(reinterpret_cast<char*>(&labelSize), sizeof(labelSize));
			symbolTableFile.write(symbol.first.c_str(), labelSize);
			symbolTableFile.write(reinterpret_cast<char*>(&symbol.second), sizeof(symbol.second));
		}
	}

	void Assembler::defineMacro(const std::string& name, const std::string& value)
	{
		if (isIdentifierDefined(name))
		{
			throw std::exception(std::string("Redefined Identifier: " + name).c_str());
		}

		macros[name] = value;
	}
}
