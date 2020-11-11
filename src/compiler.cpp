#include "compiler.hpp"

#include <exception>
#include <iostream>

#include "common.hpp"

namespace kasm
{
	Compiler::Compiler()
	{
	}

	Compiler::~Compiler()
	{
	}

	void Compiler::compile(const std::string& sourcePath, const std::string& asmPath)
	{
		symbolTable.clear();

		std::ifstream sourceFile(sourcePath);
		asmFile.open(asmPath);

		ASTNode* ast =
			makeCompoundNode({
				makeFunctionDefinition(makeIdentifier("entry"), makeType(Type::U8), makeCompoundNode({}), // entry() : u8
				makeCompoundNode({ // {
					makeVariableDeclaration(makeIdentifier("x"), makeType(Type::U8)), // x : u8;
					makeBinaryOperator(BinaryOperator::ASSIGNMENT, makeIdentifier("x"), makeLiteral(2)), // x = 2;
					makeBinaryOperator(BinaryOperator::ASSIGNMENT, makeIdentifier("x"), makeBinaryOperator(BinaryOperator::ADD, makeBinaryOperator(BinaryOperator::ADD, makeIdentifier("x"), makeLiteral(3)), makeBinaryOperator(BinaryOperator::ADD, makeIdentifier("x"), makeLiteral(3)))), // x = x + x + 3;
					makeReturn(makeBinaryOperator(BinaryOperator::ADD, makeIdentifier("x"), makeFunctionCall(makeIdentifier("test"), makeCompoundNode({ makeIdentifier("x") })))), // return x + 5;
				})), // }

				makeFunctionDefinition(makeIdentifier("test"), makeType(Type::U8), makeCompoundNode({ makeVariableDeclaration(makeIdentifier("x"), makeType(Type::U8)) }), // u8 test(x : u8)
				makeCompoundNode({ // {
					makeBinaryOperator(BinaryOperator::ASSIGNMENT, makeIdentifier("x"), makeBinaryOperator(BinaryOperator::ADD, makeIdentifier("x"), makeLiteral(3))), // x = x + 3;
					makeReturn(makeBinaryOperator(BinaryOperator::ADD, makeIdentifier("x"), makeLiteral(5))), // return x + 5;
				})), // }
			});

		prettyPrint(ast);
		semanticAnalysis(ast);
		codeGeneration(ast);

		delete ast;
	}

	void Compiler::writeLine(const std::string& line)
	{
		asmFile << line << std::endl;
		std::cout << line << std::endl;
	}

	Compiler::ASTNode* Compiler::makeCompoundNode(const std::vector<ASTNode*>& body) const
	{
		ASTNode* astNode = new ASTNode;
		astNode->astNodeType = ASTNodeType::COMPOUND_NODE;
		new (&astNode->asCompoundNode.body) std::vector<ASTNode*>(body);
		return astNode;
	}

	Compiler::ASTNode* Compiler::makeReturn(ASTNode* expression) const
	{
		ASTNode* astNode = new ASTNode;
		astNode->astNodeType = ASTNodeType::RETURN;
		astNode->asReturn.condition = expression;
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
		astNode->asFunctionDefinition.entry = identifier->asIdentifier.identifier == "entry";
		return astNode;
	}


	Compiler::ASTNode* Compiler::makeFunctionCall(ASTNode* identifier, ASTNode* arguments) const
	{
		ASTNode* astNode = new ASTNode;
		astNode->astNodeType = ASTNodeType::FUNCTION_CALL;
		astNode->asFunctionCall.identifier = identifier;
		astNode->asFunctionCall.arguments = arguments;
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

	Compiler::ASTNode* Compiler::makeIfThenElse(ASTNode* condition, ASTNode* trueBody, ASTNode* falseBody)
	{
		ASTNode* astNode = new ASTNode;
		astNode->astNodeType = ASTNodeType::IF_THEN_ELSE;
		astNode->asIfThenElse.condition = condition;
		astNode->asIfThenElse.trueBody = trueBody;
		astNode->asIfThenElse.falseBody = falseBody;
		return astNode;
	}

	Compiler::ASTNode* Compiler::makeWhile(ASTNode* condition, ASTNode* body)
	{
		ASTNode* astNode = new ASTNode;
		astNode->astNodeType = ASTNodeType::WHILE;
		astNode->asWhile.condition = condition;
		astNode->asWhile.body = body;
		return astNode;
	}

	Compiler::ASTNode* Compiler::makeDoWhile(ASTNode* condition, ASTNode* body)
	{
		ASTNode* astNode = new ASTNode;
		astNode->astNodeType = ASTNodeType::DO_WHILE;
		astNode->asDoWhile.condition = condition;
		astNode->asDoWhile.body = body;
		return astNode;
	}

	Compiler::ASTNode* Compiler::makeFor(ASTNode* initialize, ASTNode* condition, ASTNode* increment, ASTNode* body)
	{
		ASTNode* astNode = new ASTNode;
		astNode->astNodeType = ASTNodeType::FOR;
		astNode->asFor.initialize = initialize;
		astNode->asFor.condition = condition;
		astNode->asFor.increment = increment;
		astNode->asFor.body = body;
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
		case ASTNodeType::COMPOUND_NODE:
			std::cout << "COMPOUND_NODE:" << std::endl;
			for (ASTNode* node : astNode->asCompoundNode.body)
			{
				prettyPrint(node, level + 1);
			}
			break;
		case ASTNodeType::RETURN:
			std::cout << "RETURN:" << std::endl;
			prettyPrint(astNode->asReturn.condition, level + 1);
			break;
		case ASTNodeType::LITERAL:
			std::cout << "LITERAL(" << astNode->asLiteral.value << ");" << std::endl;
			break;
		case ASTNodeType::FUNCTION_DEFINITION:
			std::cout << "FUNCTION_DEFINITION:" << std::endl;
			prettyPrint(astNode->asFunctionDefinition.identifier, level + 1);
			prettyPrint(astNode->asFunctionDefinition.type, level + 1);
			prettyPrint(astNode->asFunctionDefinition.arguments, level + 1);
			prettyPrint(astNode->asFunctionDefinition.body, level + 1);
			break;
		case ASTNodeType::FUNCTION_CALL:
			std::cout << "FUNCTION_CALL:" << std::endl;
			prettyPrint(astNode->asFunctionCall.identifier, level + 1);
			prettyPrint(astNode->asFunctionCall.arguments, level + 1);
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
		case ASTNodeType::IF_THEN_ELSE:
			std::cout << "IF_THEN_ELSE:" << std::endl;
			prettyPrint(astNode->asIfThenElse.condition, level + 1);
			prettyPrint(astNode->asIfThenElse.trueBody, level + 1);
			prettyPrint(astNode->asIfThenElse.falseBody, level + 1);
			break;
		case ASTNodeType::WHILE:
			std::cout << "WHILE:" << std::endl;
			prettyPrint(astNode->asWhile.condition, level + 1);
			prettyPrint(astNode->asWhile.body, level + 1);
			break;
		case ASTNodeType::DO_WHILE:
			std::cout << "DO_WHILE:" << std::endl;
			prettyPrint(astNode->asDoWhile.condition, level + 1);
			prettyPrint(astNode->asDoWhile.body, level + 1);
			break;
		case ASTNodeType::FOR:
			std::cout << "FOR:" << std::endl;
			prettyPrint(astNode->asFor.initialize, level + 1);
			prettyPrint(astNode->asFor.condition, level + 1);
			prettyPrint(astNode->asFor.increment, level + 1);
			prettyPrint(astNode->asFor.body, level + 1);
			break;
		default:
			throw std::exception("Illegal ASTNodeType");
			break;
		}
	}

	void Compiler::semanticAnalysis(ASTNode* astNode)
	{
		static bool entry = false;
		switch (astNode->astNodeType)
		{
		case ASTNodeType::COMPOUND_NODE:
			for (ASTNode* node : astNode->asCompoundNode.body)
			{
				semanticAnalysis(node);
			}
			break;
		case ASTNodeType::RETURN:
			semanticAnalysis(astNode->asReturn.condition);
			astNode->asReturn.entry = entry;
			break;
		case ASTNodeType::LITERAL:
			break;
		case ASTNodeType::FUNCTION_DEFINITION:
			entry = astNode->asFunctionDefinition.entry;
			semanticAnalysis(astNode->asFunctionDefinition.body);
			break;
		case ASTNodeType::FUNCTION_CALL:
			semanticAnalysis(astNode->asFunctionCall.arguments);
			break;
		case ASTNodeType::BINARY_OPERATOR:
			switch (astNode->asBinaryOperator.op)
			{
			case BinaryOperator::ADD:
				semanticAnalysis(astNode->asBinaryOperator.lhs);
				semanticAnalysis(astNode->asBinaryOperator.rhs);
				break;
			case BinaryOperator::ASSIGNMENT:
				semanticAnalysis(astNode->asBinaryOperator.rhs);
				break;
			default:
				break;
			}
			break;
		case ASTNodeType::UNARY_OPERATOR:
			switch (astNode->asUnaryOperator.op)
			{
			case UnaryOperator::NEGATE:
				semanticAnalysis(astNode->asUnaryOperator.rhs);
				break;
			case UnaryOperator::INDIRECTION:
				semanticAnalysis(astNode->asUnaryOperator.rhs);
				break;
			case UnaryOperator::NOT:
				semanticAnalysis(astNode->asUnaryOperator.rhs);
				break;
			default:
				break;
			}
			break;
		case ASTNodeType::VARIABLE_DECLARATION:
			switch (astNode->asVariableDeclaration.type->asType.type)
			{
			case Type::U8:
				break;
			default:
				break;
			}
			break;
		case ASTNodeType::IDENTIFIER:
			break;
		default:
			throw std::exception("Illegal ASTNodeType");
			break;
		}
	}

	void Compiler::optimize(const ASTNode* astNode)
	{
		// heck^2
	}

	void Compiler::codeGeneration(const ASTNode* astNode)
	{
		static std::unordered_map<std::string, std::uint32_t> localVariables;

		switch (astNode->astNodeType)
		{
		case ASTNodeType::COMPOUND_NODE:
			for (ASTNode* node : astNode->asCompoundNode.body)
			{
				codeGeneration(node);
			}
			break;
		case ASTNodeType::RETURN:
			codeGeneration(astNode->asReturn.condition);
			if (astNode->asReturn.entry)
			{
				writeLine("\tcopy $a0, $t0");
				writeLine("\tli $v0, 0");
				writeLine("\tsys");
			}
			else
			{
				writeLine("\tret");
			}
			break;
		case ASTNodeType::LITERAL:
			writeLine("\tli $t0, " + std::to_string(astNode->asLiteral.value));
			break;
		case ASTNodeType::FUNCTION_DEFINITION:
			{
				writeLine(astNode->asFunctionDefinition.identifier->asIdentifier.identifier + ":");
				if (!astNode->asFunctionDefinition.entry)
				{
					writeLine("\tpushw $ra");
					writeLine("\tpushw $fp");
				}
				writeLine("\tcopy $fp, $sp");

				std::uint32_t i = 4;
				for (ASTNode* node : astNode->asFunctionDefinition.arguments->asCompoundNode.body)
				{
					localVariables[astNode->asFunctionDefinition.identifier->asIdentifier.identifier] = i;
					writeLine("\tsw $" + std::to_string(Register::A0 + i) + ", -" + std::to_string(i) + "($fp)");
					i += INSTRUCTION_SIZE;
				}

				codeGeneration(astNode->asFunctionDefinition.body);

				localVariables.clear();
			}
			break;
		case ASTNodeType::FUNCTION_CALL:
			{
				std::uint32_t argReg = Register::A0;
				for (ASTNode* node : astNode->asFunctionCall.arguments->asCompoundNode.body)
				{
					codeGeneration(node);
					writeLine("\tcopy $" + std::to_string(argReg) + ", $t0");
					argReg++;
				}
				writeLine("\tcall " + astNode->asFunctionCall.identifier->asIdentifier.identifier);
			}
			break;
		case ASTNodeType::BINARY_OPERATOR:
			switch (astNode->asBinaryOperator.op)
			{
			case BinaryOperator::ADD:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tpushw $t0");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\tpopw $t1");
				writeLine("\tadd $t0, $t0, $t1");
				break;
			case BinaryOperator::ASSIGNMENT:
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\tsb $t0, " + astNode->asBinaryOperator.lhs->asIdentifier.identifier);
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
				writeLine("sub $t0, $zero, $t0");
				break;
			case UnaryOperator::INDIRECTION:
				codeGeneration(astNode->asUnaryOperator.rhs);
				writeLine("lw $t0, $t0");
				break;
			case UnaryOperator::NOT:
				codeGeneration(astNode->asUnaryOperator.rhs);
				writeLine("nor $t0, $t0, $zero");
				break;
			default:
				break;
			}
			break;
		case ASTNodeType::VARIABLE_DECLARATION:
			writeLine(".data");
			writeLine(astNode->asVariableDeclaration.identifier->asIdentifier.identifier + ":");
			switch (astNode->asVariableDeclaration.type->asType.type)
			{
			case Type::U8:
				writeLine("\t.space 1");
				break;
			default:
				break;
			}
			writeLine(".text");
			break;
		case ASTNodeType::IDENTIFIER:
			if (localVariables.count(astNode->asIdentifier.identifier))
			{
				writeLine("\tlw $t0, -" + std::to_string(localVariables[astNode->asIdentifier.identifier]) + "($fp)");
			}
			else
			{
				writeLine("\tlb $t0, " + astNode->asIdentifier.identifier);
			}
			break;
		default:
			throw std::exception("Illegal ASTNodeType");
			break;
		}
	}
}
