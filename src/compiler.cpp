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
			makeFunctionDefinition(makeIdentifier("entry"), makeType(Type::VOID), makeCompoundStatement({}), // void entry()
			makeCompoundStatement({ // {
				makeVariableDeclaration(makeIdentifier("x"), makeType(Type::U8)), // x : u8;
				makeBinaryOperator(BinaryOperator::ASSIGNMENT, makeIdentifier("x"), makeLiteral(2)), // x = 2;
				makeBinaryOperator(BinaryOperator::ASSIGNMENT, makeIdentifier("x"), makeBinaryOperator(BinaryOperator::ADD, makeIdentifier("x"), makeLiteral(3))), // x = x + 3;
				makeReturn(makeBinaryOperator(BinaryOperator::ADD, makeIdentifier("x"), makeLiteral(5))) // return x + 5;
			})) // }
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

Compiler::ASTNode* Compiler::makeFunctionDefinition(ASTNode* identifier, ASTNode* type, ASTNode* arguments, ASTNode* body) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::FUNCTION_DEFINITION;
	astNode->asFunctionDefinition.identifier = identifier;
	astNode->asFunctionDefinition.type = type;
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

Compiler::ASTNode* Compiler::makeVariableDeclaration(ASTNode* identifier, ASTNode* type) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::VARIABLE_DECLARATION;
	astNode->asVariableDeclaration.identifier = identifier;
	astNode->asVariableDeclaration.type = type;
	return astNode;
}

Compiler::ASTNode* Compiler::makeIdentifier(const std::string& identifier) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::IDENTIFIER;
	new (&astNode->asIdentifier.identifier) std::string(identifier);
	return astNode;
}

Compiler::ASTNode* Compiler::makeType(Type type) const
{
	ASTNode* astNode = new ASTNode;
	astNode->astNodeType = ASTNodeType::TYPE;
	astNode->asType.type = type;
	return astNode;
}

void Compiler::prettyPrint(const ASTNode* astNode, unsigned int level) const
{
	for (unsigned int i = 0; i < level; i++)
	{
		std::cout << "    ";
	}

	switch (astNode->astNodeType)
	{
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
		prettyPrint(astNode->asFunctionDefinition.identifier, level + 1);
		prettyPrint(astNode->asFunctionDefinition.arguments, level + 1);
		prettyPrint(astNode->asFunctionDefinition.body, level + 1);
		break;
	case ASTNodeType::BINARY_OPERATOR:
		std::cout << "BINARY_OPERATOR(";
		switch (astNode->asBinaryOperator.op)
		{
		case BinaryOperator::ADD:
			std::cout << "+";
			break;
		case BinaryOperator::ASSIGNMENT:
			std::cout << "=";
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
	case ASTNodeType::VARIABLE_DECLARATION:
		std::cout << "VARIABLE_DECLARATION:" << std::endl;
		prettyPrint(astNode->asVariableDeclaration.identifier, level + 1);
		prettyPrint(astNode->asVariableDeclaration.type, level + 1);
		break;
	case ASTNodeType::IDENTIFIER:
		std::cout << "IDENTIFIER(" << astNode->asIdentifier.identifier << ");" << std::endl;
		break;
	case ASTNodeType::TYPE:
		std::cout << "TYPE(";
		switch (astNode->asType.type)
		{
		case Type::VOID:
			std::cout << "void";
			break;
		case Type::TYPE:
			std::cout << "type";
			break;
		case Type::U8:
			std::cout << "u8";
			break;
		default:
			break;
		}
		std::cout << ");" << std::endl;
		break;
	default:
		throw std::exception("Illegal ASTNodeType");
		break;
	}
}

void Compiler::codeGeneration(const ASTNode* astNode)
{
	switch (astNode->astNodeType)
	{
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
		writeLine(astNode->asFunctionDefinition.identifier->asIdentifier.identifier + ":");
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
		case BinaryOperator::ASSIGNMENT:
			codeGeneration(astNode->asBinaryOperator.rhs);
			codeGeneration(astNode->asBinaryOperator.lhs);
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
	case ASTNodeType::VARIABLE_DECLARATION:
		break;
	case ASTNodeType::IDENTIFIER:
		break;
	default:
		throw std::exception("Illegal ASTNodeType");
		break;
	}
}
