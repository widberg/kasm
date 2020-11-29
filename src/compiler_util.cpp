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
		in.open(sourcePath);
		loc.begin.filename = &in.getIdentifier();
		loc.end.filename = &in.getIdentifier();
		cyy::parser parser(*this);
		int status = parser.parse();

		asmFile.open(asmPath);
		prettyPrint(astRoot);
		semanticAnalysis(astRoot);
		codeGeneration(astRoot);

		delete astRoot;
	}

	void Compiler::writeLine(const std::string& line)
	{
		asmFile << line << std::endl;
		std::cout << line << std::endl;
	}

	std::uint32_t Compiler::getSizeOfType(ast::Type type)
	{
		switch (type)
		{
		case ast::Type::VOID:
			return 0;
		case ast::Type::TYPE:
			return 0;
		case ast::Type::U8:
			return 1;
		case ast::Type::U32:
			return 4;
		case ast::Type::S8:
			return 1;
		case ast::Type::S32:
			return 4;
		default:
			return 0;
		}
	}

	ast::Node* Compiler::negate(ast::Node* astNode)
	{
		switch (astNode->astNodeType)
		{
		case ast::NodeType::BINARY_OPERATOR:
			astNode->asBinaryOperator.lhs = negate(astNode->asBinaryOperator.lhs);
			astNode->asBinaryOperator.rhs = negate(astNode->asBinaryOperator.rhs);
			switch (astNode->asBinaryOperator.op)
			{
			case ast::BinaryOperator::LOGICAL_AND:
				astNode->asBinaryOperator.op = ast::BinaryOperator::LOGICAL_OR;
				break;
			case ast::BinaryOperator::LOGICAL_OR:
				astNode->asBinaryOperator.op = ast::BinaryOperator::LOGICAL_AND;
				break;
			case ast::BinaryOperator::LESS_THAN:
				astNode->asBinaryOperator.op = ast::BinaryOperator::GREATER_THAN_OR_EQUAL;
				break;
			case ast::BinaryOperator::GREATER_THAN:
				astNode->asBinaryOperator.op = ast::BinaryOperator::LESS_THAN_OR_EQUAL;
				break;
			case ast::BinaryOperator::EQUAL:
				astNode->asBinaryOperator.op = ast::BinaryOperator::NOT_EQUAL;
				break;
			case ast::BinaryOperator::NOT_EQUAL:
				astNode->asBinaryOperator.op = ast::BinaryOperator::EQUAL;
				break;
			case ast::BinaryOperator::LESS_THAN_OR_EQUAL:
				astNode->asBinaryOperator.op = ast::BinaryOperator::LESS_THAN;
				break;
			case ast::BinaryOperator::GREATER_THAN_OR_EQUAL:
				astNode->asBinaryOperator.op = ast::BinaryOperator::LESS_THAN;
				break;
			default:
				astNode = ast::makeUnaryOperator(ast::UnaryOperator::LOGICAL_NOT, astNode);
				break;
			}
			break;
		case ast::NodeType::UNARY_OPERATOR:
			if (astNode->asUnaryOperator.op == ast::UnaryOperator::LOGICAL_NOT)
			{
				ast::Node* tmp = astNode;
				astNode = astNode->asUnaryOperator.rhs;
				tmp->astNodeType = ast::NodeType::EMPTY;
				delete tmp;
			}
			else
			{
				astNode = ast::makeUnaryOperator(ast::UnaryOperator::LOGICAL_NOT, astNode);
			}
			break;
		default:
			astNode = ast::makeUnaryOperator(ast::UnaryOperator::LOGICAL_NOT, astNode);
			break;
		}
		return astNode;
	}

	void Compiler::prettyPrint(const ast::Node* astNode, unsigned int level)
	{
		for (unsigned int i = 0; i < level; i++)
		{
			std::cout << "    ";
		}

		switch (astNode->astNodeType)
		{
		case ast::NodeType::EMPTY:
			std::cout << "EMPTY" << std::endl;
			break;
		case ast::NodeType::COMPOUND:
			std::cout << "COMPOUND:" << std::endl;
			prettyPrint(astNode->asCompound.first, level + 1);
			prettyPrint(astNode->asCompound.second, level + 1);
			break;
		case ast::NodeType::RETURN:
			std::cout << "RETURN:" << std::endl;
			prettyPrint(astNode->asReturn.condition, level + 1);
			break;
		case ast::NodeType::LITERAL:
			std::cout << "LITERAL(" << astNode->asLiteral.value << ");" << std::endl;
			break;
		case ast::NodeType::FUNCTION_DEFINITION:
			std::cout << "FUNCTION_DEFINITION:" << std::endl;
			prettyPrint(astNode->asFunctionDefinition.identifier, level + 1);
			prettyPrint(astNode->asFunctionDefinition.type, level + 1);
			prettyPrint(astNode->asFunctionDefinition.arguments, level + 1);
			prettyPrint(astNode->asFunctionDefinition.body, level + 1);
			break;
		case ast::NodeType::FUNCTION_CALL:
			std::cout << "FUNCTION_CALL:" << std::endl;
			prettyPrint(astNode->asFunctionCall.identifier, level + 1);
			prettyPrint(astNode->asFunctionCall.arguments, level + 1);
			break;
		case ast::NodeType::BINARY_OPERATOR:
			std::cout << "BINARY_OPERATOR(";
			switch (astNode->asBinaryOperator.op)
			{
			case ast::BinaryOperator::ADD:
				std::cout << "+";
				break;
			case ast::BinaryOperator::ASSIGNMENT:
				std::cout << "=";
				break;
			default:
				break;
			}
			std::cout << "):" << std::endl;
			prettyPrint(astNode->asBinaryOperator.lhs, level + 1);
			prettyPrint(astNode->asBinaryOperator.rhs, level + 1);
			break;
		case ast::NodeType::UNARY_OPERATOR:
			std::cout << "UNARY_OPERATOR(";
			switch (astNode->asUnaryOperator.op)
			{
			case ast::UnaryOperator::NEGATE:
				std::cout << "-";
				break;
			default:
				break;
			}
			std::cout << "):" << std::endl;
			prettyPrint(astNode->asUnaryOperator.rhs, level + 1);
			break;
		case ast::NodeType::VARIABLE_DECLARATION:
			std::cout << "VARIABLE_DECLARATION:" << std::endl;
			prettyPrint(astNode->asVariableDeclaration.identifier, level + 1);
			prettyPrint(astNode->asVariableDeclaration.type, level + 1);
			break;
		case ast::NodeType::IDENTIFIER:
			std::cout << "IDENTIFIER(" << astNode->asIdentifier.identifier << ");" << std::endl;
			break;
		case ast::NodeType::TYPE:
			std::cout << "TYPE(";
			switch (astNode->asType.type)
			{
			case ast::Type::VOID:
				std::cout << "void";
				break;
			case ast::Type::TYPE:
				std::cout << "type";
				break;
			case ast::Type::U8:
				std::cout << "u8";
				break;
			default:
				break;
			}
			std::cout << ");" << std::endl;
			break;
		case ast::NodeType::IF_THEN_ELSE:
			std::cout << "IF_THEN_ELSE:" << std::endl;
			prettyPrint(astNode->asIfThenElse.condition, level + 1);
			prettyPrint(astNode->asIfThenElse.trueBody, level + 1);
			prettyPrint(astNode->asIfThenElse.falseBody, level + 1);
			break;
		case ast::NodeType::IF_THEN:
			std::cout << "IF_THEN:" << std::endl;
			prettyPrint(astNode->asIfThen.condition, level + 1);
			prettyPrint(astNode->asIfThen.trueBody, level + 1);
			break;
		case ast::NodeType::WHILE:
			std::cout << "WHILE:" << std::endl;
			prettyPrint(astNode->asWhile.condition, level + 1);
			prettyPrint(astNode->asWhile.body, level + 1);
			break;
		case ast::NodeType::DO_WHILE:
			std::cout << "DO_WHILE:" << std::endl;
			prettyPrint(astNode->asDoWhile.condition, level + 1);
			prettyPrint(astNode->asDoWhile.body, level + 1);
			break;
		case ast::NodeType::FOR:
			std::cout << "FOR:" << std::endl;
			prettyPrint(astNode->asFor.initialize, level + 1);
			prettyPrint(astNode->asFor.condition, level + 1);
			prettyPrint(astNode->asFor.increment, level + 1);
			prettyPrint(astNode->asFor.body, level + 1);
			break;
		case ast::NodeType::STRING_LITERAL:
			std::cout << "STRING_LITERAL(\"" + astNode->asStringLiteral.str + "\");" << std::endl;
			break;
		case ast::NodeType::ASM:
			std::cout << "ASM(\"" + astNode->asASM.source + "\");" << std::endl;
			break;
		default:
			throw std::exception("Illegal ast::NodeType");
			break;
		}
	}

	void Compiler::semanticAnalysis(ast::Node* astNode)
	{
		static bool entry = false;
		switch (astNode->astNodeType)
		{
		case ast::NodeType::EMPTY:
			break;
		case ast::NodeType::COMPOUND:
			semanticAnalysis(astNode->asCompound.first);
			semanticAnalysis(astNode->asCompound.second);
			break;
		case ast::NodeType::RETURN:
			semanticAnalysis(astNode->asReturn.condition);
			astNode->asReturn.entry = entry;
			break;
		case ast::NodeType::LITERAL:
			break;
		case ast::NodeType::FUNCTION_DEFINITION:
			entry = astNode->asFunctionDefinition.entry;
			semanticAnalysis(astNode->asFunctionDefinition.body);
			break;
		case ast::NodeType::FUNCTION_CALL:
			semanticAnalysis(astNode->asFunctionCall.arguments);
			break;
		case ast::NodeType::BINARY_OPERATOR:
			switch (astNode->asBinaryOperator.op)
			{
			case ast::BinaryOperator::ADD:
				semanticAnalysis(astNode->asBinaryOperator.lhs);
				semanticAnalysis(astNode->asBinaryOperator.rhs);
				break;
			case ast::BinaryOperator::ASSIGNMENT:
				semanticAnalysis(astNode->asBinaryOperator.rhs);
				break;
			default:
				break;
			}
			break;
		case ast::NodeType::UNARY_OPERATOR:
			switch (astNode->asUnaryOperator.op)
			{
			case ast::UnaryOperator::NEGATE:
				semanticAnalysis(astNode->asUnaryOperator.rhs);
				break;
			case ast::UnaryOperator::INDIRECTION:
				semanticAnalysis(astNode->asUnaryOperator.rhs);
				break;
			case ast::UnaryOperator::ADDRESS_OF:
				semanticAnalysis(astNode->asUnaryOperator.rhs);
				break;
			case ast::UnaryOperator::LOGICAL_NOT:
				semanticAnalysis(astNode->asUnaryOperator.rhs);
				break;
			default:
				break;
			}
			break;
		case ast::NodeType::VARIABLE_DECLARATION:
			switch (astNode->asVariableDeclaration.type->asType.type)
			{
			case ast::Type::U8:
				break;
			default:
				break;
			}
			break;
		case ast::NodeType::IDENTIFIER:
			break;
		case ast::NodeType::STRING_LITERAL:
			break;
		case ast::NodeType::ASM:
			break;
		case ast::NodeType::WHILE:
			semanticAnalysis(astNode->asWhile.condition);
			semanticAnalysis(astNode->asWhile.body);
			break;
		case ast::NodeType::IF_THEN:
			semanticAnalysis(astNode->asIfThen.condition);
			semanticAnalysis(astNode->asIfThen.trueBody);
			break;
		case ast::NodeType::IF_THEN_ELSE:
			semanticAnalysis(astNode->asIfThenElse.condition);
			semanticAnalysis(astNode->asIfThenElse.trueBody);
			semanticAnalysis(astNode->asIfThenElse.falseBody);
			break;
		case ast::NodeType::FOR:
			semanticAnalysis(astNode->asFor.initialize);
			semanticAnalysis(astNode->asFor.condition);
			semanticAnalysis(astNode->asFor.increment);
			semanticAnalysis(astNode->asFor.body);
			break;
		default:
			throw std::exception("Illegal ast::NodeType");
			break;
		}
	}

	void Compiler::optimize(const ast::Node* astNode)
	{
		// heck^2
	}

	void Compiler::codeGeneration(const ast::Node* astNode)
	{
		static std::unordered_map<std::string, std::uint32_t> localVariables;
		static std::uint32_t uid = 0;
		static bool lval = false;

		switch (astNode->astNodeType)
		{
		case ast::NodeType::EMPTY:
			break;
		case ast::NodeType::COMPOUND:
			codeGeneration(astNode->asCompound.first);
			codeGeneration(astNode->asCompound.second);
			break;
		case ast::NodeType::RETURN:
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
		case ast::NodeType::LITERAL:
			writeLine("\tli $t0, " + std::to_string(astNode->asLiteral.value));
			break;
		case ast::NodeType::FUNCTION_DEFINITION:
			{
				writeLine(astNode->asFunctionDefinition.identifier->asIdentifier.identifier + ":");
				if (!astNode->asFunctionDefinition.entry)
				{
					writeLine("\tenter");
				}
				else
				{
					writeLine("\tcopy $fp, $sp");
				}

				std::uint32_t i = 4;

				ast::Node* node = astNode->asFunctionDefinition.arguments;
				while (node->astNodeType == ast::NodeType::COMPOUND || node->astNodeType == ast::NodeType::VARIABLE_DECLARATION)
				{
					localVariables[astNode->asFunctionDefinition.identifier->asIdentifier.identifier] = i;
					writeLine("\taddi $sp, $sp, -4");
					writeLine("\tsw $" + std::to_string(Register::A0 + i) + ", -" + std::to_string(i) + "($fp)");
					i += INSTRUCTION_SIZE;

					if (node->astNodeType == ast::NodeType::COMPOUND)
					{
						node = node->asCompound.second;
					}
					else
					{
						break;
					}
				}

				codeGeneration(astNode->asFunctionDefinition.body);

				writeLine("\tret");

				localVariables.clear();
			}
			break;
		case ast::NodeType::FUNCTION_CALL:
			{
				std::uint32_t argReg = Register::A0;
				ast::Node* node = astNode->asFunctionCall.arguments;
				while (node->astNodeType != ast::NodeType::EMPTY)
				{
					codeGeneration(node->astNodeType == ast::NodeType::COMPOUND ? node->asCompound.first : node);

					writeLine("\tcopy $" + std::to_string(argReg) + ", $t0");
					argReg++;

					if (node->astNodeType == ast::NodeType::COMPOUND)
					{
						node = node->asCompound.second;
					}
					else
					{
						break;
					}
				}

				writeLine("\tcall " + astNode->asFunctionCall.identifier->asIdentifier.identifier);
			}
			break;
		case ast::NodeType::IF_THEN_ELSE:
			{
				std::uint32_t endLabel = uid++;
				std::uint32_t elseLabel = uid++;
				codeGeneration(astNode->asIfThenElse.condition);
				writeLine("\tbeqz $t0, _" + std::to_string(elseLabel));
				codeGeneration(astNode->asIfThenElse.trueBody);
				writeLine("\tb _" + std::to_string(endLabel));
				writeLine("_" + std::to_string(elseLabel) + ":");
				codeGeneration(astNode->asIfThenElse.falseBody);
				writeLine("_" + std::to_string(endLabel) + ":");
			}
			break;
		case ast::NodeType::IF_THEN:
			{
				std::uint32_t endLabel = uid++;
				codeGeneration(astNode->asIfThen.condition);
				writeLine("\tbeqz $t0, _" + std::to_string(endLabel));
				codeGeneration(astNode->asIfThen.trueBody);
				writeLine("_" + std::to_string(endLabel) + ":");
			}
			break;
		case ast::NodeType::WHILE:
			{
				std::uint32_t topLabel = uid++;
				std::uint32_t bottomLabel = uid++;
				writeLine("\tb _" + std::to_string(bottomLabel));
				writeLine("_" + std::to_string(topLabel) + ":");
				codeGeneration(astNode->asWhile.body);
				writeLine("_" + std::to_string(bottomLabel) + ":");
				codeGeneration(astNode->asWhile.condition);
				writeLine("\tbne $t0, $zero, _" + std::to_string(topLabel));
			}
			break;
		case ast::NodeType::DO_WHILE:
			{
				std::uint32_t topLabel = uid++;
				writeLine("_" + std::to_string(topLabel) + ":");
				codeGeneration(astNode->asWhile.body);
				codeGeneration(astNode->asWhile.condition);
				writeLine("\tbne $t0, $zero, _" + std::to_string(topLabel));
			}
			break;
		case ast::NodeType::FOR:
			{
				std::uint32_t topLabel = uid++;
				std::uint32_t bottomLabel = uid++;
				std::uint32_t endLabel = uid++;
				codeGeneration(astNode->asFor.initialize);
				codeGeneration(astNode->asFor.condition);
				writeLine("\tbeqz $t0, _" + std::to_string(endLabel));
				writeLine("_" + std::to_string(topLabel) + ":");
				codeGeneration(astNode->asFor.body);
				writeLine("_" + std::to_string(bottomLabel) + ":");
				codeGeneration(astNode->asFor.increment);
				codeGeneration(astNode->asFor.condition);
				writeLine("\tbne $t0, $zero, _" + std::to_string(topLabel));
				writeLine("_" + std::to_string(endLabel) + ":");
			}
			break;
		case ast::NodeType::BINARY_OPERATOR:
			switch (astNode->asBinaryOperator.op)
			{
			case ast::BinaryOperator::ADD:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tadd $t0, $t1, $t0");
				break;
			case ast::BinaryOperator::MODULUS:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\trem $t0, $t1, $t0");
				break;
			case ast::BinaryOperator::ASSIGNMENT:
				lval = true;
				codeGeneration(astNode->asBinaryOperator.lhs);
				lval = false;
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tsw $t0, ($t1)");
				break;
			case ast::BinaryOperator::LOGICAL_AND:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tsne $t0, $t0, $zero");
				writeLine("\tsne $t1, $t1, $zero");
				writeLine("\tand $t0, $t1, $t0");
				break;
			case ast::BinaryOperator::EQUAL:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tseq $t0, $t1, $t0");
				break;
			case ast::BinaryOperator::NOT_EQUAL:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tsne $t0, $t1, $t0");
				break;
			case ast::BinaryOperator::LESS_THAN:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tslt $t0, $t1, $t0");
				break;
			case ast::BinaryOperator::LESS_THAN_OR_EQUAL:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tslt $t2, $t1, $t0");
				writeLine("\tseq $t0, $t0, $t1");
				writeLine("\tor $t0, $t0, $t2");
				break;
			case ast::BinaryOperator::GREATER_THAN:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tslt $t2, $t0, $t1");
				writeLine("\tseq $t0, $t0, $t1");
				writeLine("\tor $t0, $t0, $t2");
				writeLine("\txori $t0, $t0, 1");
				break;
			case ast::BinaryOperator::GREATER_THAN_OR_EQUAL:
				codeGeneration(astNode->asBinaryOperator.lhs);
				writeLine("\tsw $t0, 0($gp)");
				writeLine("\taddi $gp, $gp, 4");
				codeGeneration(astNode->asBinaryOperator.rhs);
				writeLine("\taddi $gp, $gp, -4");
				writeLine("\tlw $t1, 0($gp)");
				writeLine("\tslt $t0, $t0, $t1");
				writeLine("\txori $t0, $t0, 1");
				break;
			default:
				break;
			}
			break;
		case ast::NodeType::UNARY_OPERATOR:
			switch (astNode->asUnaryOperator.op)
			{
			case ast::UnaryOperator::NEGATE:
				codeGeneration(astNode->asUnaryOperator.rhs);
				writeLine("sub $t0, $zero, $t0");
				break;
			case ast::UnaryOperator::INDIRECTION:
				codeGeneration(astNode->asUnaryOperator.rhs);
				writeLine("lw $t0, ($t0)");
				break;
			case ast::UnaryOperator::ADDRESS_OF:
				if (localVariables.count(astNode->asUnaryOperator.rhs->asIdentifier.identifier))
				{
					writeLine("addi $t0, $fp, -" + std::to_string(localVariables[astNode->asUnaryOperator.rhs->asIdentifier.identifier]));
				}
				else
				{
					writeLine("la $t0, " + astNode->asUnaryOperator.rhs->asIdentifier.identifier);
				}
				break;
			case ast::UnaryOperator::LOGICAL_NOT:
				codeGeneration(astNode->asUnaryOperator.rhs);
				writeLine("seq $t0, $t0, $zero");
				break;
			default:
				break;
			}
			break;
		case ast::NodeType::VARIABLE_DECLARATION:
			writeLine(".data");
			writeLine(astNode->asVariableDeclaration.identifier->asIdentifier.identifier + ":");
			writeLine("\t.space " + std::to_string(getSizeOfType(astNode->asVariableDeclaration.type->asType.type)));
			writeLine(".text");
			if (lval)
			{
				writeLine("la $t0, " + astNode->asVariableDeclaration.identifier->asIdentifier.identifier);
			}
			break;
		case ast::NodeType::IDENTIFIER:
			if (lval)
			{
				if (localVariables.count(astNode->asIdentifier.identifier))
				{
					writeLine("\tadd $t0, -" + std::to_string(localVariables[astNode->asIdentifier.identifier]) + ", $fp");
				}
				else
				{
					writeLine("\tla $t0, " + astNode->asIdentifier.identifier);
				}
			}
			else
			{
				if (localVariables.count(astNode->asIdentifier.identifier))
				{
					writeLine("\tlw $t0, -" + std::to_string(localVariables[astNode->asIdentifier.identifier]) + "($fp)");
				}
				else
				{
					writeLine("\tla $t0, " + astNode->asIdentifier.identifier);
					writeLine("\tlw $t0, ($t0)");
				}
			}
			break;
		case ast::NodeType::STRING_LITERAL:
			{
				std::uint32_t stringLabel = uid++;
				writeLine(".data");
				writeLine("_" + std::to_string(stringLabel) + ":");
				writeLine("\t.asciiz \"" + astNode->asStringLiteral.str + "\"");
				writeLine(".text");
				writeLine("\tla $t0, _" + std::to_string(stringLabel));
			}
			break;
		case ast::NodeType::ASM:
			writeLine(astNode->asASM.source);
			break;
		default:
			throw std::exception("Illegal ast::NodeType");
			break;
		}
	}
}
