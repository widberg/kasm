#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

class Compiler
{
public:
	Compiler();
	~Compiler();

	void compile(const std::string& sourcePath, const std::string& asmPath);

private:
	void writeLine(const std::string& line);

	std::ofstream asmFile;

	enum class ASTNodeType
	{
		INVALID,
		COMPOUND_STATEMENT,
		RETURN,
		LITERAL,
		FUNCTION_DEFINITION,
		BINARY_OPERATOR,
		UNARY_OPERATOR
	};

	enum class BinaryOperator
	{
		ADD
	};

	enum class UnaryOperator
	{
		NEGATE
	};

	struct ASTNode
	{
		ASTNode()
			: astNodeType(ASTNodeType::INVALID) {};

		~ASTNode()
		{
			switch (astNodeType)
			{
			case ASTNodeType::COMPOUND_STATEMENT:
				for (ASTNode* statement : asCompoundStatement.body)
				{
					delete statement;
				}
				break;
			case ASTNodeType::RETURN:
				delete asReturn.expression;
				break;
			case ASTNodeType::LITERAL:
				break;
			case ASTNodeType::FUNCTION_DEFINITION:
				delete asFunctionDefinition.body;
				break;
			default:
				break;
			}
		};

		ASTNodeType astNodeType;
		union
		{
			struct
			{
				std::vector<ASTNode*> body;
			} asCompoundStatement;
			struct
			{
				ASTNode* expression;
			} asReturn;
			struct
			{
				std::uint32_t value;
			} asLiteral;
			struct
			{
				std::string name;
				ASTNode* arguments;
				ASTNode* body;
			} asFunctionDefinition;
			struct
			{
				BinaryOperator op;
				ASTNode* lhs;
				ASTNode* rhs;
			} asBinaryOperator;
			struct
			{
				UnaryOperator op;
				ASTNode* rhs;
			} asUnaryOperator;
		};
	};

	ASTNode* makeCompoundStatement(const std::vector<ASTNode*>& statements) const;
	ASTNode* makeReturn(ASTNode* expression) const;
	ASTNode* makeLiteral(std::uint32_t value) const;
	ASTNode* makeFunctionDefinition(const std::string& name, ASTNode*, ASTNode* body) const;
	ASTNode* makeBinaryOperator(BinaryOperator op, ASTNode* lhs, ASTNode* rhs) const;
	ASTNode* makeUnaryOperator(UnaryOperator op, ASTNode* rhs) const;

	void Compiler::prettyPrint(const ASTNode* astNode, unsigned int level = 0) const;
	void codeGeneration(const ASTNode* astNode);
};
