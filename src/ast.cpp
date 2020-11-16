#include "ast.hpp"

namespace kasm
{
	namespace ast
	{

		Node* makeEmpty()
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::EMPTY;
			return astNode;
		}

		Node* makeCompound(Node* first, Node* second)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::COMPOUND;
			astNode->asCompound.first = first;
			astNode->asCompound.second = second;
			return astNode;
		}

		Node* makeReturn(Node* expression)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::RETURN;
			astNode->asReturn.condition = expression;
			return astNode;
		}

		Node* makeLiteral(std::uint32_t value)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::LITERAL;
			astNode->asLiteral.value = value;
			return astNode;
		}

		Node* makeFunctionDefinition(Node* identifier, Node* type, Node* arguments, Node* body)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::FUNCTION_DEFINITION;
			astNode->asFunctionDefinition.identifier = identifier;
			astNode->asFunctionDefinition.type = type;
			astNode->asFunctionDefinition.arguments = arguments;
			astNode->asFunctionDefinition.body = body;
			astNode->asFunctionDefinition.entry = identifier->asIdentifier.identifier == "entry";
			return astNode;
		}


		Node* makeFunctionCall(Node* identifier, Node* arguments)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::FUNCTION_CALL;
			astNode->asFunctionCall.identifier = identifier;
			astNode->asFunctionCall.arguments = arguments;
			return astNode;
		}

		Node* makeBinaryOperator(BinaryOperator op, Node* lhs, Node* rhs)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::BINARY_OPERATOR;
			astNode->asBinaryOperator.op = op;
			astNode->asBinaryOperator.lhs = lhs;
			astNode->asBinaryOperator.rhs = rhs;
			return astNode;
		}

		Node* makeUnaryOperator(UnaryOperator op, Node* rhs)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::UNARY_OPERATOR;
			astNode->asUnaryOperator.op = op;
			astNode->asUnaryOperator.rhs = rhs;
			return astNode;
		}

		Node* makeVariableDeclaration(Node* identifier, Node* type)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::VARIABLE_DECLARATION;
			astNode->asVariableDeclaration.identifier = identifier;
			astNode->asVariableDeclaration.type = type;
			return astNode;
		}

		Node* makeIdentifier(const std::string& identifier)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::IDENTIFIER;
			new (&astNode->asIdentifier.identifier) std::string(identifier);
			return astNode;
		}

		Node* makeType(Type type)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::TYPE;
			astNode->asType.type = type;
			return astNode;
		}

		Node* makeIfThenElse(Node* condition, Node* trueBody, Node* falseBody)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::IF_THEN_ELSE;
			astNode->asIfThenElse.condition = condition;
			astNode->asIfThenElse.trueBody = trueBody;
			astNode->asIfThenElse.falseBody = falseBody;
			return astNode;
		}

		Node* makeIfThen(Node* condition, Node* trueBody)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::IF_THEN;
			astNode->asIfThen.condition = condition;
			astNode->asIfThen.trueBody = trueBody;
			return astNode;
		}

		Node* makeWhile(Node* condition, Node* body)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::WHILE;
			astNode->asWhile.condition = condition;
			astNode->asWhile.body = body;
			return astNode;
		}

		Node* makeDoWhile(Node* condition, Node* body)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::DO_WHILE;
			astNode->asDoWhile.condition = condition;
			astNode->asDoWhile.body = body;
			return astNode;
		}

		Node* makeFor(Node* initialize, Node* condition, Node* increment, Node* body)
		{
			Node* astNode = new Node;
			astNode->astNodeType = NodeType::FOR;
			astNode->asFor.initialize = initialize;
			astNode->asFor.condition = condition;
			astNode->asFor.increment = increment;
			astNode->asFor.body = body;
			return astNode;
		}
	}
}