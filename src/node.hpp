#ifndef NODE_HPP
#define NODE_HPP

#include <iomanip>
#include <iostream>
#include <string>

#include "debug.hpp"
#include "program.hpp"
#include "specification.hpp"

namespace kasm
{
	class Node
	{
	public:
		virtual ~Node() {}

		virtual address_t write(Program& program) const = 0;
		virtual void dump(std::ostream& out) const = 0;
	private:
	};

	class ErrorNode
	{
	public:
		virtual address_t write(Program& program) const { KASM_ASSERT(false, "Why are we trying to write an error node?"); return 0; }
		virtual void dump(std::ostream& out) const { out << "ERROR\n"; };
	};

	class StatementNode : public Node {};

	class InstructionNode : public StatementNode
	{
	public:
		InstructionNode(Instruction instruction_) : instruction(instruction_) {}
		virtual ~InstructionNode() {}

		virtual address_t write(Program& program) const { program.write(instruction); return program.tell() - INSTRUCTION_SIZE; }
		virtual void dump(std::ostream& out) const { out << "\t0x" << std::hex << std::fixed << std::setfill(0) << instruction.data() << std::resetiosflags << "\n"; };
	private:
		Instruction instruction;
	};

	class DirectiveNode : public StatementNode {};

	class LabeledStatementNode : public StatementNode
	{
	public:
		LabeledStatementNode(const std::string& label_, StatementNode* statement_) : label(label_), statement(statement_) {}
		~LabeledStatementNode() { delete statement; }

		virtual address_t write(Program& program) const { return statement->write(program); }
		virtual void dump(std::ostream& out) const { out << label << ":\n"; statement->dump(out); };
	private:
		std::string label;
		StatementNode* statement;
	};
} // namespace kasm

#endif // !NODE_HPP
