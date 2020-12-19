#include <gtest/gtest.h>

#include "lexer.hpp"

TEST(Lexer, Empty)
{
    const std::string source = "";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfFile, token.tokenType);
}

TEST(Lexer, Comment)
{
    const std::string source = "; example comment";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfFile, token.tokenType);
}

TEST(Lexer, Instruction)
{
    const std::string source = "add";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::Instruction, token.tokenType);
    ASSERT_EQ(kasm::OpCode::Add, token.opCode);
}

TEST(Lexer, Register)
{
    const std::string source = "%rax";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::Register, token.tokenType);
    ASSERT_EQ(kasm::Register::RAX, token.reg);
}

TEST(Lexer, Integer)
{
    const std::string source = "$1";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::IntegerLiteral, token.tokenType);
    ASSERT_EQ(1, token.quadWord);
}

TEST(Lexer, String)
{
    const std::string source = "\"\"";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::StringLiteral, token.tokenType);
    ASSERT_EQ("", token.string);
}

TEST(Lexer, CRLF)
{
    const std::string source = "\r\n";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfLine, token.tokenType);
    token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfFile, token.tokenType);
}

TEST(Lexer, CRLFCRLF)
{
    const std::string source = "\r\n\r\n";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfLine, token.tokenType);
    token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfLine, token.tokenType);
    token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfFile, token.tokenType);
}

TEST(Lexer, LF)
{
    const std::string source = "\n";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfLine, token.tokenType);
    token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfFile, token.tokenType);
}

TEST(Lexer, LFLF)
{
    const std::string source = "\n\n";
    kasm::Lexer lexer(source);
    kasm::Token token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfLine, token.tokenType);
    token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfLine, token.tokenType);
    token = lexer.next();
    ASSERT_EQ(kasm::TokenType::EndOfFile, token.tokenType);
}
