#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
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
		COMPOUND_NODE,
		RETURN,
		LITERAL,
		FUNCTION_DEFINITION,
		BINARY_OPERATOR,
		UNARY_OPERATOR,
		VARIABLE_DECLARATION,
		IDENTIFIER,
		TYPE,
		IF_THEN_ELSE,
		WHILE,
		DO_WHILE,
		FOR
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
			case ASTNodeType::COMPOUND_NODE:
				for (ASTNode* statement : asCompoundNode.body)
				{
					delete statement;
				}
				break;
			case ASTNodeType::RETURN:
				delete asReturn.condition;
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
			case ASTNodeType::IF_THEN_ELSE:
				delete asIfThenElse.condition;
				delete asIfThenElse.trueBody;
				delete asIfThenElse.falseBody;
				break;
			case ASTNodeType::WHILE:
				delete asWhile.condition;
				delete asWhile.body;
				break;
			case ASTNodeType::DO_WHILE:
				delete asDoWhile.condition;
				delete asDoWhile.body;
				break;
			case ASTNodeType::FOR:
				delete asFor.initialize;
				delete asFor.condition;
				delete asFor.increment;
				delete asFor.body;
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
			} asCompoundNode;
			struct
			{
				ASTNode* condition;
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
			struct
			{
				ASTNode* condition;
				ASTNode* trueBody;
				ASTNode* falseBody;
			} asIfThenElse;
			struct
			{
				ASTNode* condition;
				ASTNode* body;
			} asWhile;
			struct
			{
				ASTNode* condition;
				ASTNode* body;
			} asDoWhile;
			struct
			{
				ASTNode* initialize;
				ASTNode* condition;
				ASTNode* increment;
				ASTNode* body;
			} asFor;
		};
	};

	ASTNode* makeCompoundNode(const std::vector<ASTNode*>& nodes) const;
	ASTNode* makeReturn(ASTNode* expression) const;
	ASTNode* makeLiteral(std::uint32_t value) const;
	ASTNode* makeFunctionDefinition(ASTNode* identifier, ASTNode* type, ASTNode*, ASTNode* body) const;
	ASTNode* makeBinaryOperator(BinaryOperator op, ASTNode* lhs, ASTNode* rhs) const;
	ASTNode* makeUnaryOperator(UnaryOperator op, ASTNode* rhs) const;
	ASTNode* makeVariableDeclaration(ASTNode* identifier, ASTNode* type) const;
	ASTNode* makeIdentifier(const std::string& identifier) const;
	ASTNode* makeType(Type type) const;
	ASTNode* makeIfThenElse(ASTNode* condition, ASTNode* trueBody, ASTNode* falseBody);
	ASTNode* makeWhile(ASTNode* condition, ASTNode* body);
	ASTNode* makeDoWhile(ASTNode* condition, ASTNode* body);
	ASTNode* makeFor(ASTNode* initialize, ASTNode* condition, ASTNode* increment, ASTNode* body);

	std::unordered_map<std::string, ASTNode*> symbolTable;

	void prettyPrint(const ASTNode* astNode, unsigned int level = 0) const;
	void semanticAnalysis(const ASTNode* astNode);
	void optimize(const ASTNode* astNode);
	void codeGeneration(const ASTNode* astNode);
};
