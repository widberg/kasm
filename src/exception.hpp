#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>
#include <string>

namespace kasm
{
	class Exception : public std::exception
	{
	public:
		Exception(const std::string& message_) noexcept : message(message_) {}
		virtual ~Exception() noexcept {}

		virtual const char* what() const noexcept { return message.c_str(); }
	private:
		std::string message;
	};
} // namespace kasm

#endif // !EXCEPTION_HPP
