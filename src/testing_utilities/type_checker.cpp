#include <iostream>

#include <semantic_analysis/symbol_table.hpp>
#include <testing_utilities/type_checker.hpp>

bool TypeChecker::is_concrete(tp::TypeId type_id) const {
  if (type_id == tp::no_type_id) {
    return false;
  }
  if (type_id < tp::basic_type_count) {
    return true;
  }

  return std::visit(
      [](auto&& val) -> bool {
        using T = std::decay_t<decltype(val)>;
        return std::is_same_v<T, tp::ReferenceType> ||
               std::is_same_v<T, tp::FunctionType>;
      },
      type_store_.get_type(type_id).type);
}

void TypeChecker::visit(const ast::IdentifierNode& identifier) {
  if (!is_concrete(identifier.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << identifier
              << "with val " << identifier.type_id << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  const Symbol* symbol =
      identifier.table->get_variable_symbol(identifier.identifier->name);

  if (symbol->type != identifier.type_id) {
    std::cerr << "TypeChecker::visit: type_id mismatch at " << identifier
              << std::endl;
    std::cerr << "symbol type: " << symbol->type << std::endl;
    std::cerr << "identifier type: " << identifier.type_id << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id mismatch");
  }
}

void TypeChecker::visit(const ast::LiteralNode& literal) {
  if (!is_concrete(literal.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << literal
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }
}

void TypeChecker::visit(const ast::LvalueDereferenceNode& lvalue) {
  if (!is_concrete(lvalue.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << lvalue
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }
}

void TypeChecker::visit(const ast::LvalueExpressionNode& lvalue) {
  if (!is_concrete(lvalue.type_id)) {
    std::cerr << "LvalueExpressionNode" << std::endl;
    std::cerr << "TypeChecker::visit: type_id out of range at " << lvalue
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }
}

void TypeChecker::visit(const ast::FunctionCallNode& function_call) {
  if (!is_concrete(function_call.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << function_call
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  for (auto& argument : function_call.arguments) {
    visit(argument);
  }
}

void TypeChecker::visit(const ast::ExpressionNode& expression) {
  std::visit([this](auto&& val) { visit(val); }, *expression.node);
}

void TypeChecker::visit(const ast::BlockExpressionNode& expression) {
  if (!is_concrete(expression.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << expression
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  for (auto& statement : expression.statements) {
    visit(statement);
  }

  if (expression.value.has_value()) {
    visit(*expression.value.value());
  }
}

void TypeChecker::visit(const ast::IfExpressionNode& expression) {
  if (!is_concrete(expression.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << expression
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  visit(*expression.condition);

  visit(*expression.body);

  for (auto& elif_body : expression.elif_bodies) {
    visit(*elif_body.expr);
    visit(*elif_body.block);
  }

  if (expression.else_body.has_value()) {
    visit(*expression.else_body.value());
  }
}

void TypeChecker::visit(const ast::AssignmentNode& assignment) {
  if (!is_concrete(assignment.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << assignment
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  visit(*assignment.left);
  visit(*assignment.right);
}

void TypeChecker::visit(const ast::LoopExpressionNode& loop) {
  if (!is_concrete(loop.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << loop
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  for (auto& statement : loop.body) {
    visit(statement);
  }
}

void TypeChecker::visit(const ast::BreakStatementNode& break_stmt) {
  if (break_stmt.value.has_value()) {
    visit(*break_stmt.value.value());
  }
}

void TypeChecker::visit(const ast::ContinueStatementNode&) {}

void TypeChecker::visit(const ast::ReturnStatementNode& return_stmt) {
  if (return_stmt.value.has_value()) {
    visit(*return_stmt.value.value());
  }
}

template <typename BinaryNode>
  requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
void TypeChecker::visit(const BinaryNode& binary_node) {
  if (!is_concrete(binary_node.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << binary_node
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  visit(*binary_node.left);
  for (auto& [_, right] : binary_node.right) {
    visit(right);
  }
}

void TypeChecker::visit(const ast::CastNode& cast_node) {
  if (!is_concrete(cast_node.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << cast_node
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  visit(*cast_node.expression);
}

void TypeChecker::visit(const ast::UnaryNode& unary_node) {
  if (!is_concrete(unary_node.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << unary_node
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  visit(*unary_node.primary);
}

void TypeChecker::visit(const ast::PrimaryNode& primary_node) {
  if (!is_concrete(primary_node.type_id)) {
    std::cerr << "TypeChecker::visit: type_id out of range at " << primary_node
              << std::endl;
    throw std::runtime_error("TypeChecker::visit: type_id out of range");
  }

  std::visit([this](auto&& val) { visit(val); }, *primary_node.primary);
}

void TypeChecker::visit(const ast::StatementNode& statement) {
  std::visit([this](auto&& val) { visit(val); }, *statement.node);
}

void TypeChecker::visit(
    const ast::VariableDefinitionNode& variable_definition) {
  visit(*variable_definition.name);

  if (variable_definition.value.has_value()) {
    visit(*variable_definition.value.value());
  }
}

void TypeChecker::visit(
    const ast::FunctionDefinitionNode& function_definition) {
  for (auto& [arg, type] : function_definition.argument_list) {
    visit(arg);
  }

  visit(*function_definition.body);
}

void TypeChecker::visit(const ast::Program& program) {
  for (auto& definition : program.definitions) {
    std::visit([this](auto& val) { visit(val); }, definition);
  }
}
