#include "compiler.hpp"

#include <exception>
#include <iostream>

Compiler::Compiler()
{
}

Compiler::~Compiler()
{
}

void Compiler::compile(const std::string& sourcePath, const std::string& asmPath)
{
	std::ifstream sourceFile(sourcePath);
	asmFile.open(asmPath);

	const ASTNode* ast =
		makeCompoundStatement({
			makeFunctionDefinition("entry", makeCompoundStatement({}), makeCompoundStatement({
				makeReturn(makeBinaryOperator(BinaryOperator::ADD, makeLiteral(3), makeLiteral(5)))
			}))
		});

	prettyPrint(ast);

	codeGeneration(ast);

	delete ast;
}

void Compiler::writeLine(const std::string& line)
{
	asmFile << line << std::endl;
	std::cout << line << std::endl;
}

Compiler::ASTNode* Compiler::makeCompoundStatement(const std::vector<ASTNode*>& body) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::COMPOUND_STATEMENT;
	new (&astNode->asCompoundStatement.body) std::vector<ASTNode*>(body);
	return astNode;
}

Compiler::ASTNode* Compiler::makeReturn(ASTNode* expression) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::RETURN;
	astNode->asReturn.expression = expression;
	return astNode;
}

Compiler::ASTNode* Compiler::makeLiteral(std::uint32_t value) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::LITERAL;
	astNode->asLiteral.value = value;
	return astNode;
}

Compiler::ASTNode* Compiler::makeFunctionDefinition(const std::string& name, ASTNode* arguments, ASTNode* body) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::FUNCTION_DEFINITION;
	new (&astNode->asFunctionDefinition.name) std::string(name);
	astNode->asFunctionDefinition.arguments = arguments;
	astNode->asFunctionDefinition.body = body;
	return astNode;
}

Compiler::ASTNode* Compiler::makeBinaryOperator(BinaryOperator op, ASTNode* lhs, ASTNode* rhs) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::BINARY_OPERATOR;
	astNode->asBinaryOperator.op = op;
	astNode->asBinaryOperator.lhs = lhs;
	astNode->asBinaryOperator.rhs = rhs;
	return astNode;
}

Compiler::ASTNode* Compiler::makeUnaryOperator(UnaryOperator op, ASTNode* rhs) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::UNARY_OPERATOR;
	astNode->asUnaryOperator.op = op;
	astNode->asUnaryOperator.rhs = rhs;
	return astNode;
}

void Compiler::prettyPrint(const ASTNode* astNode, unsigned int level) const
{
	for (int i = 0; i < level; i++)
	{
		std::cout << "    ";
	}

	switch (astNode->astNodeType)
	{
	case ASTNodeType::INVALID:
		throw std::exception("Illegal ASTNodeType");
		break;
	case ASTNodeType::COMPOUND_STATEMENT:
		std::cout << "COMPOUND_STATEMENT:" << std::endl;
		for (ASTNode* statement : astNode->asCompoundStatement.body)
		{
			prettyPrint(statement, level + 1);
		}
		break;
	case ASTNodeType::RETURN:
		std::cout << "RETURN:" << std::endl;
		prettyPrint(astNode->asReturn.expression, level + 1);
		break;
	case ASTNodeType::LITERAL:
		std::cout << "LITERAL(" << astNode->asLiteral.value << ");" << std::endl;
		break;
	case ASTNodeType::FUNCTION_DEFINITION:
		std::cout << "FUNCTION_DEFINITION:" << std::endl;
		std::cout << "\tARGUMENTS:" << std::endl;
		prettyPrint(astNode->asFunctionDefinition.arguments, level + 2);
		std::cout << "\tBODY:" << std::endl;
		prettyPrint(astNode->asFunctionDefinition.body, level + 2);
		break;
	case ASTNodeType::BINARY_OPERATOR:
		std::cout << "BINARY_OPERATOR(";
		switch (astNode->asBinaryOperator.op)
		{
		case BinaryOperator::ADD:
			std::cout << "+";
			break;
		default:
			break;
		}
		std::cout << "):" << std::endl;
		prettyPrint(astNode->asBinaryOperator.lhs, level + 1);
		prettyPrint(astNode->asBinaryOperator.rhs, level + 1);
		break;
	case ASTNodeType::UNARY_OPERATOR:
		std::cout << "UNARY_OPERATOR(";
		switch (astNode->asUnaryOperator.op)
		{
		case UnaryOperator::NEGATE:
			std::cout << "-";
			break;
		default:
			break;
		}
		std::cout << "):" << std::endl;
		prettyPrint(astNode->asUnaryOperator.rhs, level + 1);
		break;
	default:
		break;
	}
}

void Compiler::codeGeneration(const ASTNode* astNode)
{
	switch (astNode->astNodeType)
	{
	case ASTNodeType::INVALID:
		throw std::exception("Illegal ASTNodeType");
		break;
	case ASTNodeType::COMPOUND_STATEMENT:
		for (ASTNode* statement : astNode->asCompoundStatement.body)
		{
			codeGeneration(statement);
		}
		break;
	case ASTNodeType::RETURN:
		codeGeneration(astNode->asReturn.expression);
		writeLine("\tadd a0 t0 zero");
		writeLine("\tli v0 0");
		writeLine("\tsys");
		break;
	case ASTNodeType::LITERAL:
		writeLine("\tli t0 " + std::to_string(astNode->asLiteral.value));
		break;
	case ASTNodeType::FUNCTION_DEFINITION:
		writeLine(astNode->asFunctionDefinition.name + ":");
		codeGeneration(astNode->asFunctionDefinition.body);
		break;
	case ASTNodeType::BINARY_OPERATOR:
		switch (astNode->asBinaryOperator.op)
		{
		case BinaryOperator::ADD:
			codeGeneration(astNode->asBinaryOperator.lhs);
			writeLine("\tadd t1 t0 zero");
			codeGeneration(astNode->asBinaryOperator.rhs);
			writeLine("\tadd t0 t0 t1");
			break;
		default:
			break;
		}
		break;
	case ASTNodeType::UNARY_OPERATOR:
		switch (astNode->asUnaryOperator.op)
		{
		case UnaryOperator::NEGATE:
			codeGeneration(astNode->asUnaryOperator.rhs);
			break;
		default:
			break;
		}
		break;
		break;
	default:
		break;
	}
}
