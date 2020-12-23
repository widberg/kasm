#ifndef NODE_HPP
#define NODE_HPP

#include <string>

#include "program.hpp"
#include "specification.hpp"

namespace kasm
{
	class Node
	{
	public:
		virtual ~Node() {}

		virtual void write(Program& program) const = 0;
	private:

	};

	class StatementNode : public Node
	{
	public:
		virtual ~StatementNode() {}
	private:
	};

	class InstructionNode : public StatementNode
	{
	public:
		InstructionNode(Instruction instruction_) : instruction(instruction_) {}
		virtual ~InstructionNode() {}

		virtual void write(Program& program) const { program.write(instruction); }
	private:
		Instruction instruction;
	};

	class DirectiveNode : public StatementNode
	{
	public:
		virtual ~DirectiveNode() {}
	private:

	};

	class LabeledStatementNode : public StatementNode
	{
	public:
		InstructionNode(const std::string& label_, StatementNode* statement_) : label(label_), statement(statement_) {}
		virtual ~LabeledStatementNode() {}

		virtual void write(Program& program) const { statement->write(); }
	private:
		std::string label;
		StatementNode* statement;
	};
} // namespace kasm

#endif // !NODE_HPP
