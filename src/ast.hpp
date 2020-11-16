#pragma once

#include <cstdint>
#include <string>

namespace kasm
{
	namespace ast
	{
		enum class NodeType
		{
			INVALID,
			EMPTY,
			COMPOUND,
			RETURN,
			LITERAL,
			FUNCTION_DEFINITION,
			FUNCTION_CALL,
			BINARY_OPERATOR,
			UNARY_OPERATOR,
			VARIABLE_DECLARATION,
			IDENTIFIER,
			TYPE,
			IF_THEN_ELSE,
			IF_THEN,
			WHILE,
			DO_WHILE,
			FOR,
			STRING_LITERAL,
			ASM
		};

		enum class BinaryOperator
		{
			ADD,
			MODULUS,
			ASSIGNMENT,
			LOGICAL_AND,
			LOGICAL_OR,
			LESS_THAN,
			GREATER_THAN,
			EQUAL,
			NOT_EQUAL,
			LESS_THAN_OR_EQUAL,
			GREATER_THAN_OR_EQUAL
		};

		enum class UnaryOperator
		{
			NEGATE,
			INDIRECTION,
			ADDRESS_OF,
			LOGICAL_NOT
		};

		enum class Type
		{
			VOID,
			TYPE,
			U8,
			U32,
			S8,
			S32
		};

		struct Node
		{
			Node()
				: astNodeType(NodeType::INVALID) {};

			~Node()
			{
				switch (astNodeType)
				{
				case NodeType::COMPOUND:
					delete asCompound.first;
					delete asCompound.second;
					break;
				case NodeType::RETURN:
					delete asReturn.condition;
					break;
				case NodeType::LITERAL:
					break;
				case NodeType::FUNCTION_DEFINITION:
					delete asFunctionDefinition.identifier;
					delete asFunctionDefinition.arguments;
					delete asFunctionDefinition.body;
					break;
				case NodeType::FUNCTION_CALL:
					delete asFunctionCall.identifier;
					delete asFunctionCall.arguments;
					break;
				case NodeType::BINARY_OPERATOR:
					delete asBinaryOperator.lhs;
					delete asBinaryOperator.rhs;
					break;
				case NodeType::UNARY_OPERATOR:
					delete asUnaryOperator.rhs;
					break;
				case NodeType::VARIABLE_DECLARATION:
					delete asVariableDeclaration.identifier;
					delete asVariableDeclaration.type;
					break;
				case NodeType::IDENTIFIER:
					asIdentifier.identifier.~basic_string();
					break;
				case NodeType::TYPE:
					break;
				case NodeType::IF_THEN_ELSE:
					delete asIfThenElse.condition;
					delete asIfThenElse.trueBody;
					delete asIfThenElse.falseBody;
					break;
				case NodeType::IF_THEN:
					delete asIfThenElse.condition;
					delete asIfThenElse.trueBody;
					break;
				case NodeType::WHILE:
					delete asWhile.condition;
					delete asWhile.body;
					break;
				case NodeType::DO_WHILE:
					delete asDoWhile.condition;
					delete asDoWhile.body;
					break;
				case NodeType::FOR:
					delete asFor.initialize;
					delete asFor.condition;
					delete asFor.increment;
					delete asFor.body;
					break;
				case NodeType::STRING_LITERAL:
					asStringLiteral.str.~basic_string();
					break;
				case NodeType::ASM:
					asASM.source.~basic_string();
					break;
				default:
					break;
				}
			};

			NodeType astNodeType;
			union
			{
				struct
				{
					Node* first;
					Node* second;
				} asCompound;
				struct
				{
					Node* condition;
					bool entry;
				} asReturn;
				struct
				{
					std::uint32_t value;
				} asLiteral;
				struct
				{
					Node* identifier;
					Node* type;
					Node* arguments;
					Node* body;
					bool entry;
				} asFunctionDefinition;
				struct
				{
					Node* identifier;
					Node* arguments;
				} asFunctionCall;
				struct
				{
					BinaryOperator op;
					Node* lhs;
					Node* rhs;
				} asBinaryOperator;
				struct
				{
					UnaryOperator op;
					Node* rhs;
				} asUnaryOperator;
				struct
				{
					Node* identifier;
					Node* type;
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
					Node* condition;
					Node* trueBody;
					Node* falseBody;
				} asIfThenElse;
				struct
				{
					Node* condition;
					Node* trueBody;
				} asIfThen;
				struct
				{
					Node* condition;
					Node* body;
				} asWhile;
				struct
				{
					Node* condition;
					Node* body;
				} asDoWhile;
				struct
				{
					Node* initialize;
					Node* condition;
					Node* increment;
					Node* body;
				} asFor;
				struct
				{
					std::string str;
				} asStringLiteral;
				struct
				{
					std::string source;
				} asASM;
			};
		};

		Node* makeCompound(Node* first, Node* second);
		Node* makeEmpty();
		Node* makeReturn(Node* expression);
		Node* makeLiteral(std::uint32_t value);
		Node* makeFunctionDefinition(Node* identifier, Node* type, Node* arguments, Node* body);
		Node* makeFunctionCall(Node* identifier, Node* arguments);
		Node* makeBinaryOperator(BinaryOperator op, Node* lhs, Node* rhs);
		Node* makeUnaryOperator(UnaryOperator op, Node* rhs);
		Node* makeVariableDeclaration(Node* identifier, Node* type);
		Node* makeIdentifier(const std::string& identifier);
		Node* makeType(Type type);
		Node* makeIfThenElse(Node* condition, Node* trueBody, Node* falseBody);
		Node* makeIfThen(Node* condition, Node* trueBody);
		Node* makeWhile(Node* condition, Node* body);
		Node* makeDoWhile(Node* condition, Node* body);
		Node* makeFor(Node* initialize, Node* condition, Node* increment, Node* body);
		Node* makeStringLiteral(const std::string& str);
		Node* makeASM(const std::string& source);
	}
}
