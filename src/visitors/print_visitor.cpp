#include <iostream>

#include <parser/ast.hpp>
#include <utility/executor.hpp>
#include <visitors/print_visitor.hpp>

PrintVisitor::PrintVisitor(std::ostream& out, bool skip_empty)
    : tabs_{}, out_{out}, skip_empty_{skip_empty} {}

void PrintVisitor::operator()(const ast::IdentifierNode& identifier) {
  out_ << tabs_ << "identifier: " << identifier.identifier->name << std::endl;
}

void PrintVisitor::operator()(const ast::LiteralNode& literal) {
  std::visit(
      [this](auto&& value) {
        out_ << tabs_ << "literal: ";

        if constexpr (std::is_same_v<std::decay_t<decltype(value)>,
                                     tkn::BoolLiteral>) {
          if (value.value) {
            out_ << "true";
          } else {
            out_ << "false";
          }

          out_ << std::endl;
        } else {
          out_ << value.value << std::endl;
        }
      },
      *literal.literal);
}

void PrintVisitor::operator()(const ast::FunctionCallNode& function_call) {
  out_ << tabs_ << "call function: " << function_call.name->identifier->name
       << std::endl
       << tabs_ << "with arguments:" << std::endl;

  tabs_ += "  ";

  for (const auto& argument : function_call.arguments) {
    operator()(argument);
  }

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::ExpressionNode& expression) {
  out_ << tabs_ << "expression: " << std::endl;

  tabs_ += "  ";

  std::visit(*this, *expression.node);

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::AssignmentNode& assignment) {
  out_ << tabs_ << "assignment: " << std::endl;

  tabs_ += "  ";

  operator()(*assignment.left);

  out_ << tabs_ << "assigned value: " << std::endl;

  tabs_ += "  ";

  operator()(*assignment.right);

  tabs_.pop_back();
  tabs_.pop_back();

  tabs_.pop_back();
  tabs_.pop_back();
}

template <typename BinaryNode>
void PrintVisitor::operator()(const BinaryNode& node) {
  if (skip_empty_) {
    if (node.right.empty()) {
      operator()(*node.left);
      return;
    }
  }
  out_ << tabs_ << ast::NodeName<BinaryNode>::value << ": " << std::endl;

  tabs_ += "  ";

  operator()(*node.left);
  for (auto& [op, in_node] : node.right) {
    out_ << tabs_ << "binary operation: ";
    if constexpr (is_variant_v<std::decay_t<decltype(op)>>) {
      std::visit([this](auto&& value) { out_ << value << std::endl; }, op);
    } else {
      out_ << op << std::endl;
    }
    operator()(in_node);
  }

  tabs_.pop_back();
  tabs_.pop_back();
}

template void PrintVisitor::operator()<ast::OrNode>(const ast::OrNode& or_node);

template void
PrintVisitor::operator()<ast::XorNode>(const ast::XorNode& xor_node);

template void
PrintVisitor::operator()<ast::AndNode>(const ast::AndNode& and_node);

template void PrintVisitor::operator()<ast::ComparisonNode>(
    const ast::ComparisonNode& comparison_node);

template void PrintVisitor::operator()<ast::EqualityNode>(
    const ast::EqualityNode& equality_node);

template void PrintVisitor::operator()<ast::AdditionNode>(
    const ast::AdditionNode& addition_node);

template void PrintVisitor::operator()<ast::MultiplicationNode>(
    const ast::MultiplicationNode& multiplication_node);

void PrintVisitor::operator()(const ast::UnaryNode& unary_node) {
  if (skip_empty_) {
    if (!unary_node.op.has_value()) {
      operator()(*unary_node.primary);
      return;
    }
  }
  out_ << tabs_ << "unary: " << std::endl;

  tabs_ += "  ";

  if (unary_node.op.has_value()) {
    std::cout << "unary operation: " << unary_node.op.value() << std::endl;
  }

  operator()(*unary_node.primary);

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::PrimaryNode& primary_node) {
  if (skip_empty_) {
    std::visit(*this, *primary_node.primary);
    return;
  }
  out_ << tabs_ << "primary: " << std::endl;

  tabs_ += "  ";

  std::visit(*this, *primary_node.primary);

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::StatementNode& statement) {
  out_ << tabs_ << "statement: " << std::endl;

  tabs_ += "  ";

  std::visit(*this, *statement.node);

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::IfStatementNode& if_statement) {
  out_ << tabs_ << "if statement: " << std::endl;

  tabs_ += "  ";

  out_ << tabs_ << "if condition: " << std::endl;
  operator()(*if_statement.condition);

  out_ << tabs_ << "body: " << std::endl;
  tabs_ += "  ";

  for (auto& statement : if_statement.body) {
    operator()(statement);
  }

  tabs_.pop_back();
  tabs_.pop_back();

  if (!if_statement.elif_bodies.empty()) {
    for (auto& expr_stmts : if_statement.elif_bodies) {
      out_ << tabs_ << "else if condition: " << std::endl;
      operator()(expr_stmts.expr);

      out_ << tabs_ << "body: " << std::endl;
      tabs_ += "  ";

      for (auto& statement : expr_stmts.statements) {
        operator()(statement);
      }

      tabs_.pop_back();
      tabs_.pop_back();
    }
  }

  if (!if_statement.else_body.empty()) {
    out_ << tabs_ << "else body: " << std::endl;
    tabs_ += "  ";

    for (auto& statement : if_statement.else_body) {
      operator()(statement);
    }

    tabs_.pop_back();
    tabs_.pop_back();
  }

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::VariableDefinitionNode& var_def) {
  out_ << tabs_ << "variable definition: " << std::endl;

  tabs_ += "  ";

  out_ << tabs_ << "name: " << var_def.name->identifier->name << std::endl;

  if (var_def.type.has_value()) {
    out_ << tabs_ << "type: " << var_def.type.value()->identifier->name
         << std::endl;
  }

  if (var_def.value.has_value()) {
    out_ << tabs_ << "value: " << std::endl;
    operator()(*var_def.value.value());
  }

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::FunctionDefinitionNode& fn_def) {
  out_ << tabs_ << "function definition: " << std::endl;

  tabs_ += "  ";

  out_ << tabs_ << "name: " << fn_def.name->identifier->name << std::endl;

  if (fn_def.argument_list.empty()) {
    out_ << tabs_ << "argument list is empty" << std::endl;
  } else {
    out_ << tabs_ << "argument list: " << std::endl;
    tabs_ += "  ";

    for (auto& [name, type] : fn_def.argument_list) {
      out_ << tabs_ << "argument name: " << name.identifier << std::endl;
      out_ << tabs_ << "argument type: " << type.identifier << std::endl;
    }

    tabs_.pop_back();
    tabs_.pop_back();
  }

  if (fn_def.return_type == nullptr) {
    out_ << tabs_ << "return type is void" << std::endl;
  } else {
    out_ << tabs_ << "return type: " << fn_def.return_type->identifier->name
         << std::endl;
  }

  out_ << tabs_ << "body: " << std::endl;
  tabs_ += "  ";

  for (auto& statement : fn_def.body) {
    operator()(statement);
  }

  tabs_.pop_back();
  tabs_.pop_back();

  tabs_.pop_back();
  tabs_.pop_back();
}

void PrintVisitor::operator()(const ast::Program& program) {
  for (auto& definition : program.definitions) {
    std::visit(*this, definition);
  }
}
