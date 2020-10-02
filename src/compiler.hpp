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
		UNARY_OPERATOR,
		VARIABLE_DECLARATION,
		IDENTIFIER,
		TYPE
	};

	enum class BinaryOperator
	{
		ADD,
		ASSIGNMENT
	};

	enum class UnaryOperator
	{
		NEGATE
	};

	enum class Type
	{
		VOID,
		TYPE,
		U8
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
				delete asFunctionDefinition.identifier;
				delete asFunctionDefinition.arguments;
				delete asFunctionDefinition.body;
				break;
			case ASTNodeType::BINARY_OPERATOR:
				delete asBinaryOperator.lhs;
				delete asBinaryOperator.rhs;
				break;
			case ASTNodeType::UNARY_OPERATOR:
				delete asUnaryOperator.rhs;
				break;
			case ASTNodeType::VARIABLE_DECLARATION:
				delete asVariableDeclaration.identifier;
				delete asVariableDeclaration.type;
				break;
			case ASTNodeType::IDENTIFIER:
				break;
			case ASTNodeType::TYPE:
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
				ASTNode* identifier;
				ASTNode* type;
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
			struct
			{
				ASTNode* identifier;
				ASTNode* type;
			} asVariableDeclaration;
			struct
			{
				std::string identifier;
			} asIdentifier;
			struct
			{
				Type type;
			} asType;
		};
	};

	ASTNode* makeCompoundStatement(const std::vector<ASTNode*>& statements) const;
	ASTNode* makeReturn(ASTNode* expression) const;
	ASTNode* makeLiteral(std::uint32_t value) const;
	ASTNode* makeFunctionDefinition(ASTNode* identifier, ASTNode* type, ASTNode*, ASTNode* body) const;
	ASTNode* makeBinaryOperator(BinaryOperator op, ASTNode* lhs, ASTNode* rhs) const;
	ASTNode* makeUnaryOperator(UnaryOperator op, ASTNode* rhs) const;
	ASTNode* makeVariableDeclaration(ASTNode* identifier, ASTNode* type) const;
	ASTNode* makeIdentifier(const std::string& identifier) const;
	ASTNode* makeType(Type type) const;

	void Compiler::prettyPrint(const ASTNode* astNode, unsigned int level = 0) const;
	void codeGeneration(const ASTNode* astNode);
};
