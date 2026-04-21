#include <ranges>

#include <semantic_analysis/semantic_visitor.hpp>

SemanticVisitor::SemanticVisitor(TypeStore& type_store,
                                 GlobalSymbolTable& symbol_table)
    : type_store_{type_store}, symbol_table_{symbol_table.get_root()} {}

void SemanticVisitor::visit(ast::Program& program) {
  for (auto& definition : program.definitions) {
    std::visit([this](auto& val) { this->visit(val); }, definition);
  }
}

void SemanticVisitor::visit(ast::VariableDefinitionNode& variable_definition) {
  variable_definition.table = symbol_table_;

  tp::TypeId declared_type = tp::no_type_id;

  bool has_type = false;

  if (variable_definition.type.has_value()) {
    auto optional_type =
        tp::get_type(variable_definition.type.value()->identifier->name);

    if (!optional_type.has_value()) {
      throw std::runtime_error("Unknown type");
    }

    has_type = true;
    declared_type = type_store_.new_basic_type(optional_type.value());
  } else {
    declared_type = type_store_.new_var();
  }

  if (variable_definition.value.has_value()) {
    if (has_type) {
      visit(*variable_definition.value.value(), declared_type);
    } else {
      visit(*variable_definition.value.value());
    }

    if (std::holds_alternative<tp::IntLiteral>(
            type_store_.get_type(variable_definition.value.value()->type_id)
                .type)) {

      auto& int_literal = std::get<tp::IntLiteral>(
          type_store_.get_type(variable_definition.value.value()->type_id)
              .type);

      int_literal.parent = type_store_.new_basic_type(tp::I32{});

    } else if (std::holds_alternative<tp::FloatLiteral>(
                   type_store_
                       .get_type(variable_definition.value.value()->type_id)
                       .type)) {

      auto& float_literal = std::get<tp::FloatLiteral>(
          type_store_.get_type(variable_definition.value.value()->type_id)
              .type);

      float_literal.parent = type_store_.new_basic_type(tp::F32{});
    }

    if (!type_store_.unify(
            variable_definition.value.value()->type_id, declared_type)) {
      throw std::runtime_error("Type mismatch in variable definition. Variable "
                               "is initialized with wrong type.");
    }
  }

  if (!variable_definition.type.has_value() &&
      !variable_definition.value.has_value()) {
    throw std::runtime_error("Invalid variable definition");
  }

  Symbol symbol{
      .position = static_cast<tkn::Position>(*variable_definition.name),
      .type = declared_type,
      .scope = symbol_table_,
      .definition = nullptr,
  };

  symbol_table_->insert_variable(
      variable_definition.name->identifier->name, std::move(symbol));

  variable_definition.name->type_id = declared_type;
}

void SemanticVisitor::visit(ast::FunctionDefinitionNode& function_definition) {
  function_definition.table = symbol_table_;

  tp::TypeId return_type = tp::no_type_id;

  if (function_definition.return_type != nullptr) {
    auto optional_type =
        tp::get_type(function_definition.return_type->identifier->name);

    if (!optional_type.has_value()) {
      throw std::runtime_error("Unknown type");
    }

    return_type = type_store_.new_basic_type(optional_type.value());
  } else {
    return_type = type_store_.new_basic_type(tp::Void{});
  }

  SymbolTable* current_scope = symbol_table_;

  symbol_table_ = symbol_table_->create_function_child();

  auto& function_scope =
      std::get<SymbolTable::FunctionScope>(symbol_table_->get_scope());

  function_scope.return_type = return_type;

  std::vector<tp::TypeId> args;

  for (auto& [arg_name, arg_type] : function_definition.argument_list) {
    auto optional_type = tp::get_type(arg_type.identifier->name);

    if (!optional_type.has_value()) {
      throw std::runtime_error("Unknown type");
    }

    tp::TypeId arg_type_id = type_store_.new_basic_type(optional_type.value());

    Symbol symbol{
        .position = static_cast<tkn::Position>(arg_name),
        .type = arg_type_id,
        .scope = symbol_table_,
        .definition = &function_definition,
    };

    arg_name.table = symbol_table_;

    symbol_table_->insert_variable(
        arg_name.identifier->name, std::move(symbol));

    args.push_back(type_store_.new_basic_type(optional_type.value()));
  }

  tp::TypeId function_type =
      type_store_.new_function(return_type, std::move(args));

  Symbol symbol{
      .position = *function_definition.name,
      .type = function_type,
      .scope = current_scope,
      .definition = &function_definition,
  };

  current_scope->insert_function(
      function_definition.name->identifier->name, std::move(symbol));

  visit(*function_definition.body, return_type);

  if (!type_store_.unify(function_definition.body->type_id, return_type)) {
    throw std::runtime_error("Type mismatch in function definition. Return "
                             "type mismatch type of block.");
  }

  symbol_table_ = current_scope;

  function_definition.name->type_id = function_type;
}

void SemanticVisitor::visit(ast::StatementNode& statement) {
  statement.table = symbol_table_;
  std::visit([this](auto& val) { this->visit(val); }, *statement.node);
}

void SemanticVisitor::visit(ast::ReturnStatementNode& return_statement) {
  return_statement.table = symbol_table_;

  SymbolTable* nearest_function = symbol_table_->find_nearest_function();

  if (nearest_function == nullptr) {
    throw std::runtime_error("Return statement outside of function");
  }

  tp::TypeId value_type = tp::no_type_id;

  if (return_statement.value.has_value()) {
    visit(*return_statement.value.value(),
          std::get<SymbolTable::FunctionScope>(nearest_function->get_scope())
              .return_type);

    value_type = return_statement.value.value()->type_id;
  } else {
    value_type = type_store_.new_basic_type(tp::Void{});
  }

  auto& function_scope =
      std::get<SymbolTable::FunctionScope>(nearest_function->get_scope());

  if (!type_store_.unify(value_type, function_scope.return_type)) {
    throw std::runtime_error("Type mismatch in return statement. Function "
                             "return type mismatch type in return statement.");
  }
}

void SemanticVisitor::visit(ast::BreakStatementNode& break_statement) {
  break_statement.table = symbol_table_;

  SymbolTable* desired_loop = nullptr;

  if (break_statement.label.has_value()) {
    desired_loop =
        symbol_table_->find_loop_by_label(break_statement.label.value().name);
  } else {
    desired_loop = symbol_table_->find_nearest_loop();
  }

  if (desired_loop == nullptr) {
    throw std::runtime_error("Break statement outside of loop");
  }

  auto& loop_context =
      std::get<SymbolTable::LoopScope>(desired_loop->get_scope());

  tp::TypeId expected_type = loop_context.expected_type;

  tp::TypeId value_type = tp::no_type_id;

  if (break_statement.value.has_value()) {
    visit(*break_statement.value.value(), expected_type);

    value_type = break_statement.value.value()->type_id;
  } else {
    value_type = type_store_.new_basic_type(tp::Void{});
  }

  if (loop_context.result_type == tp::no_type_id) {
    loop_context.result_type = value_type;
  } else if (!type_store_.unify(value_type, loop_context.result_type)) {
    throw std::runtime_error("Type mismatch in break statement. Mismatch types "
                             "of expression in break statements.");
  }
}

void SemanticVisitor::visit(ast::ContinueStatementNode& continue_statement) {
  continue_statement.table = symbol_table_;

  SymbolTable* desired_loop = nullptr;

  if (continue_statement.label.has_value()) {
    desired_loop = symbol_table_->find_loop_by_label(
        continue_statement.label.value().name);
  } else {
    desired_loop = symbol_table_->find_nearest_loop();
  }

  if (desired_loop == nullptr) {
    throw std::runtime_error("Continue statement outside of loop");
  }
}

void SemanticVisitor::visit(ast::ExpressionNode& expression,
                            tp::TypeId expected_type) {
  expression.table = symbol_table_;

  std::visit(
      [this, expected_type](auto& val) { this->visit(val, expected_type); },
      *expression.node);

  expression.type_id = std::visit(
      [](auto&& val) -> tp::TypeId { return val.type_id; }, *expression.node);

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expression.type_id, expected_type)) {

    throw std::runtime_error(
        "Type mismatch in expression. Expected type mismatch real type.");
  }
}

void SemanticVisitor::visit(ast::BlockExpressionNode& block,
                            tp::TypeId expected_type) {
  block.table = symbol_table_;

  SymbolTable* current_scope = symbol_table_;
  symbol_table_ = symbol_table_->create_simple_child();

  for (auto& statement : block.statements) {
    visit(statement);
  }

  tp::TypeId result_type = tp::no_type_id;

  if (block.value.has_value()) {
    visit(*block.value.value(), expected_type);

    result_type = block.value.value()->type_id;
  } else {
    result_type = type_store_.new_basic_type(tp::Void{});
  }

  block.type_id = result_type;

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expected_type, result_type)) {
    throw std::runtime_error(
        "Type mismatch in block. Expected type mismatch real type.");
  }

  symbol_table_ = current_scope;
}

void SemanticVisitor::visit(ast::IfExpressionNode& if_expression,
                            tp::TypeId expected_type) {
  if_expression.table = symbol_table_;

  tp::TypeId condition_type = type_store_.new_basic_type(tp::Bool{});

  visit(*if_expression.condition, condition_type);

  visit(*if_expression.body, expected_type);

  tp::TypeId if_type = if_expression.body->type_id;

  if_expression.type_id = if_type;

  for (auto& expr_stmts : if_expression.elif_bodies) {
    visit(*expr_stmts.expr, condition_type);

    visit(*expr_stmts.block, expected_type);

    tp::TypeId elif_type = expr_stmts.block->type_id;

    if (!type_store_.unify(if_type, elif_type)) {
      throw std::runtime_error("Type mismatch in else if block. Type of else "
                               "if block mismatch type of if block.");
    }
  }

  if (if_expression.else_body.has_value()) {
    visit(*if_expression.else_body.value(), expected_type);

    tp::TypeId else_type = if_expression.else_body.value()->type_id;

    if (!type_store_.unify(if_type, else_type)) {
      throw std::runtime_error("Type mismatch in else block. Type of else "
                               "block mismatch type of of block.");
    }
  }

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(if_type, expected_type)) {
    throw std::runtime_error(
        "Type mismatch in if expression. Expected type mismatch real type.");
  }
}

void SemanticVisitor::visit(ast::LoopExpressionNode& loop,
                            tp::TypeId expected_type) {
  loop.table = symbol_table_;

  SymbolTable* current_scope = symbol_table_;
  if (loop.label.has_value()) {
    symbol_table_ = symbol_table_->create_loop_child(
        loop.label.value().name, expected_type);
  } else {
    symbol_table_ = symbol_table_->create_loop_child(expected_type);
  }

  for (auto& statement : loop.body) {
    visit(statement);
  }

  auto loop_scope =
      std::get<SymbolTable::LoopScope>(symbol_table_->get_scope());

  if (loop_scope.result_type == tp::no_type_id) {
    loop_scope.result_type = type_store_.new_basic_type(tp::Void{});
  }

  loop.type_id = loop_scope.result_type;

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expected_type, loop.type_id)) {
    throw std::runtime_error("Type mismatch in loop expression. Expected "
                             "type mismatch real type.");
  }

  symbol_table_ = current_scope;
}

void SemanticVisitor::visit(ast::AssignmentNode& assignment,
                            tp::TypeId expected_type) {
  assignment.table = symbol_table_;

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(
          expected_type, type_store_.new_basic_type(tp::Void{}))) {
    throw std::runtime_error("Type mismatch. Expected non void in assignment.");
  }

  auto symbol = symbol_table_->get_variable_symbol_in_position(
      assignment.left->identifier->name,
      static_cast<tkn::Position>(*assignment.left));

  visit(*assignment.right, symbol->type);

  assignment.type_id = type_store_.new_basic_type(tp::Void{});
}

template <typename BinaryNode, bool NeedUnify>
  requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
void SemanticVisitor::binary_node_helper(
    BinaryNode& binary_node,
    std::function<bool(const tp::TypeVariant&)> type_check,
    tp::TypeId expected_type) {
  binary_node.table = symbol_table_;

  tp::TypeId left_type = tp::no_type_id;

  if (binary_node.right.empty()) {
    visit(*binary_node.left, expected_type);

    left_type = binary_node.left->type_id;

    binary_node.type_id = left_type;

    if (expected_type != tp::no_type_id &&
        !type_store_.unify(left_type, expected_type)) {

      throw std::runtime_error(
          "Type mismatch in empty binary node. Type mismatch expected type.");
    }

    return;
  }

  if constexpr (is_in_type_tuple_v<BinaryNode, ast::ClosedOperatorNodeTuple>) {
    visit(*binary_node.left, expected_type);
  } else {
    visit(*binary_node.left);
  }

  left_type = type_store_.resolve(binary_node.left->type_id);

  if (!type_check(type_store_.get_type(left_type).type)) {

    throw std::runtime_error(
        "Type mismatch in binary node. Arguments type check failed.");
  }

  if (expected_type != tp::no_type_id) {
    binary_node.type_id = expected_type;
  } else {
    if constexpr (is_in_type_tuple_v<BinaryNode,
                                     ast::ClosedOperatorNodeTuple>) {
      binary_node.type_id = left_type;
    } else {
      throw std::runtime_error("Failed to infer type of binary node");
    }
  }

  for (auto& [op, expr] : binary_node.right) {

    if constexpr (NeedUnify) {
      visit(expr, left_type);

      if (!type_store_.unify(left_type, expr.type_id)) {
        throw std::runtime_error(
            "Type mismatch in binary node. One of the type of right "
            "exppressions mismatch left expression type.");
      }
    } else {
      visit(expr);

      if (!type_check(type_store_.get_type(expr.type_id).type)) {
        throw std::runtime_error(
            "Type mismatch in binary node. One of the type of right "
            "exppressions failed type check.");
      }
    }
  }
}

template <typename LogicalNode>
  requires is_in_type_tuple_v<LogicalNode, ast::LogicalBinaryNodeTuple>
void SemanticVisitor::visit(LogicalNode& logical_node,
                            tp::TypeId expected_type) {
  if (!logical_node.right.empty() && expected_type != tp::no_type_id &&
      !type_store_.unify(
          expected_type, type_store_.new_basic_type(tp::Bool{}))) {
    throw std::runtime_error(
        "Type mismatch in logical node. Expected type is not bool.");
  }

  binary_node_helper<LogicalNode, true>(
      logical_node,
      [](const tp::TypeVariant& val) -> bool {
        return std::holds_alternative<tp::Bool>(val);
      },
      expected_type);
}

void SemanticVisitor::visit(ast::ComparisonNode& comparison_node,
                            tp::TypeId expected_type) {
  if (!comparison_node.right.empty() && expected_type != tp::no_type_id &&
      !type_store_.unify(
          expected_type, type_store_.new_basic_type(tp::Bool{}))) {
    throw std::runtime_error(
        "Type mismatch in comparison node. Expected type is not bool.");
  }

  binary_node_helper<ast::ComparisonNode, true>(
      comparison_node, [](const tp::TypeVariant&) -> bool { return true; },
      expected_type);
}

template <typename BitwiseNode>
  requires is_in_type_tuple_v<BitwiseNode, ast::BitwiseBinaryNodeTuple>
void SemanticVisitor::visit(BitwiseNode& bitwise_node,
                            tp::TypeId expected_type) {
  if (!bitwise_node.right.empty() && expected_type != tp::no_type_id) {
    tp::TypeVariant& type_variant = type_store_.get_type(expected_type).type;

    if (!std::visit(
            [](auto&& type) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::IntegerTypeTuple> ||
                     std::is_same_v<std::decay_t<decltype(type)>,
                                    tp::IntLiteral>;
            },
            type_variant)) {
      throw std::runtime_error(
          "Type mismatch in bitwise node. Expected type is not integer.");
    }
  }

  binary_node_helper<BitwiseNode, true>(
      bitwise_node,
      [](const tp::TypeVariant& val) -> bool {
        return std::visit(
            [](auto&& type) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::IntegerTypeTuple> ||
                     std::is_same_v<std::decay_t<decltype(type)>,
                                    tp::IntLiteral>;
            },
            val);
      },
      expected_type);
}

void SemanticVisitor::visit(ast::ShiftNode& shift_node,
                            tp::TypeId expected_type) {
  if (!shift_node.right.empty() && expected_type != tp::no_type_id) {
    tp::TypeVariant& type_variant = type_store_.get_type(expected_type).type;

    if (!std::visit(
            [](auto&& type) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::IntegerTypeTuple> ||
                     std::is_same_v<std::decay_t<decltype(type)>,
                                    tp::IntLiteral>;
            },
            type_variant)) {
      throw std::runtime_error(
          "Type mismatch in shift node. Expected type is not integer.");
    }
  }

  binary_node_helper<ast::ShiftNode, false>(
      shift_node,
      [](const tp::TypeVariant& val) -> bool {
        return std::visit(
            [](auto&& type) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::IntegerTypeTuple> ||
                     std::is_same_v<std::decay_t<decltype(type)>,
                                    tp::IntLiteral>;
            },
            val);
      },
      expected_type);
}

template <typename ArithmeticNode>
  requires is_in_type_tuple_v<ArithmeticNode, ast::ArithmeticBinaryNodeTuple>
void SemanticVisitor::visit(ArithmeticNode& arithmetic_node,
                            tp::TypeId expected_type) {
  if (!arithmetic_node.right.empty() && expected_type != tp::no_type_id) {
    tp::TypeVariant& type_variant = type_store_.get_type(expected_type).type;

    if (!std::visit(
            [](auto&& type) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::IntegerTypeTuple> ||
                     is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::FloatingPointTypeTuple> ||
                     is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::LiteralTypeTuple>;
            },
            type_variant)) {
      throw std::runtime_error("Type mismatch in arithmetic node. Expected "
                               "type is not integer or floating point.");
    }
  }

  binary_node_helper<ArithmeticNode, true>(
      arithmetic_node,
      [](const tp::TypeVariant& val) -> bool {
        return std::visit(
            [](auto&& type) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::IntegerTypeTuple> ||
                     is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::FloatingPointTypeTuple> ||
                     is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::LiteralTypeTuple>;
            },
            val);
      },
      expected_type);
}

void SemanticVisitor::visit(ast::CastNode& cast_node,
                            tp::TypeId expected_type) {
  cast_node.table = symbol_table_;

  if (!cast_node.type.has_value()) {
    visit(*cast_node.expression, expected_type);

    cast_node.type_id = cast_node.expression->type_id;
    if (expected_type != tp::no_type_id &&
        !type_store_.unify(expected_type, cast_node.type_id)) {
      throw std::runtime_error("Type mismatch in empty cast node. Type of "
                               "expression mismatch expected type.");
    }

    return;
  }

  visit(*cast_node.expression);

  auto optional_cast_type =
      tp::get_type(cast_node.type.value()->identifier->name);

  if (!optional_cast_type.has_value()) {
    throw std::runtime_error("Unknown type");
  }

  cast_node.type_id = type_store_.new_basic_type(optional_cast_type.value());

  if (std::holds_alternative<tp::IntLiteral>(
          type_store_.get_type(cast_node.expression->type_id).type) ||
      std::holds_alternative<tp::FloatLiteral>(
          type_store_.get_type(cast_node.expression->type_id).type)) {

    if (!type_store_.unify(cast_node.expression->type_id, cast_node.type_id)) {
      throw std::runtime_error(
          "Type mismatch in cast node. Invalid literal type.");
    }
  }

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expected_type, cast_node.type_id)) {
    throw std::runtime_error(
        "Type mismatch in cast node. Cast type mismatch expected type.");
  }

  tp::TypeId expression_type =
      type_store_.resolve(cast_node.expression->type_id);

  tp::Type real_expression_type = type_store_.get_type(expression_type);

  if (std::holds_alternative<tp::VariableType>(real_expression_type.type)) {
    if (!type_store_.unify(cast_node.type_id, expression_type)) {
      throw std::runtime_error("Logic error");
    }
  } else {
    if (!std::visit(
            [](auto&& cast_type, auto&& expression_type) -> bool {
              using cast_type_t = std::decay_t<decltype(cast_type)>;
              using expression_type_t = std::decay_t<decltype(expression_type)>;

              if constexpr (is_in_type_tuple_v<cast_type_t,
                                               tp::NumericTypeTuple>) {

                return is_in_type_tuple_v<expression_type_t,
                                          tp::NumericTypeTuple>;

              } else if constexpr (is_in_type_tuple_v<cast_type_t,
                                                      tp::BooleanTypeTuple>) {

                return is_in_type_tuple_v<expression_type_t,
                                          tp::BooleanTypeTuple>;

              } else if constexpr (is_in_type_tuple_v<cast_type_t,
                                                      tp::CharTypeTuple>) {

                return is_in_type_tuple_v<expression_type_t, tp::CharTypeTuple>;
              } else {
                return false;
              }
            },
            optional_cast_type.value(), real_expression_type.type)) {

      throw std::runtime_error(
          "Type mismatch in cast node. Cast type mismatch.");
    }
  }
}

void SemanticVisitor::visit(ast::UnaryNode& unary_node,
                            tp::TypeId expected_type) {
  unary_node.table = symbol_table_;

  visit(*unary_node.primary, expected_type);

  unary_node.type_id = unary_node.primary->type_id;

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expected_type, unary_node.type_id)) {
    throw std::runtime_error(
        "Type mismatch in unary node. Type mismatch with expected type.");
  }

  if (!unary_node.op.has_value()) {
    return;
  }

  auto& real_type = type_store_.get_type(unary_node.type_id);

  if (std::holds_alternative<tkn::Not>(*unary_node.op.value())) {
    if (!std::holds_alternative<tp::Bool>(real_type.type)) {
      throw std::runtime_error(
          "Type mismatch in unary node. Operator ! applicable only to bool.");
    }
  } else {
    if (!std::visit(
            [](auto&& type) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::IntegerTypeTuple> ||
                     is_in_type_tuple_v<std::decay_t<decltype(type)>,
                                        tp::FloatingPointTypeTuple> ||
                     std::is_same_v<std::decay_t<decltype(type)>,
                                    tp::IntLiteral>;
            },
            real_type.type)) {
      throw std::runtime_error("Type mismatch in unary node. Operator - "
                               "applicable only to int or float.");
    }
  }
}

void SemanticVisitor::visit(ast::PrimaryNode& primary_node,
                            tp::TypeId expected_type) {
  primary_node.table = symbol_table_;

  std::visit(
      [this, expected_type](auto& val) { this->visit(val, expected_type); },
      *primary_node.primary);

  primary_node.type_id =
      std::visit([](auto&& val) -> tp::TypeId { return val.type_id; },
                 *primary_node.primary);

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expected_type, primary_node.type_id)) {
    throw std::runtime_error(
        "Type mismatch in primary node. Type mismatch with expected type.");
  }
}

void SemanticVisitor::visit(ast::FunctionCallNode& function_call,
                            tp::TypeId expected_type) {
  function_call.table = symbol_table_;

  auto& function_name = function_call.name->identifier->name;

  const Symbol* symbol = symbol_table_->get_function_symbol_in_position(
      function_name, static_cast<tkn::Position>(function_call));

  const tp::Function& function_info =
      std::get<tp::Function>(type_store_.get_type(symbol->type).type);

  function_call.type_id = function_info.return_type;

  function_call.name->type_id = symbol->type;

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expected_type, function_call.type_id)) {
    throw std::runtime_error("Type mismatch in function call node. Expected "
                             "type mismatch with function return type.");
  }

  for (auto&& [expr, argument_expected_type] :
       std::views::zip(function_call.arguments, function_info.args)) {
    visit(expr, argument_expected_type);
  }
}

void SemanticVisitor::visit(ast::IdentifierNode& identifier_node,
                            tp::TypeId expected_type) {
  identifier_node.table = symbol_table_;

  std::string& identifier = identifier_node.identifier->name;

  if (symbol_table_->check_variable_availability(identifier)) {
    const Symbol* symbol = symbol_table_->get_variable_symbol_in_position(
        identifier, static_cast<tkn::Position>(identifier_node));

    identifier_node.type_id = type_store_.resolve(symbol->type);

    if (expected_type != tp::no_type_id &&
        !type_store_.unify(expected_type, identifier_node.type_id)) {
      throw std::runtime_error("Type mismatch in identifier node. Expected "
                               "type mismatch with variable type.");
    }
  } else if (symbol_table_->check_function_availability(identifier)) {
    const Symbol* symbol = symbol_table_->get_function_symbol_in_position(
        identifier, static_cast<tkn::Position>(identifier_node));

    identifier_node.type_id = symbol->type;

    if (expected_type != tp::no_type_id &&
        type_store_.unify(expected_type, identifier_node.type_id)) {
      throw std::runtime_error("Type mismatch in identifier node. Expected "
                               "type mismatch with function type.");
    }
  } else {
    throw std::runtime_error("Unknown identifier.");
  }
}

void SemanticVisitor::visit(ast::LiteralNode& literal_node,
                            tp::TypeId expected_type) {
  literal_node.table = symbol_table_;

  if (std::holds_alternative<tkn::BoolLiteral>(*literal_node.literal)) {
    literal_node.type_id = type_store_.new_basic_type(tp::Bool{});
  } else if (std::holds_alternative<tkn::IntLiteral>(*literal_node.literal)) {
    literal_node.type_id = type_store_.new_literal_type(tp::IntLiteral{
        .value = std::get<tkn::IntLiteral>(*literal_node.literal).value,
        .parent = tp::no_type_id});
  } else if (std::holds_alternative<tkn::FloatLiteral>(*literal_node.literal)) {
    literal_node.type_id = type_store_.new_literal_type(tp::FloatLiteral{
        .value = std::get<tkn::FloatLiteral>(*literal_node.literal).value,
        .parent = tp::no_type_id});
  } else if (std::holds_alternative<tkn::CharLiteral>(*literal_node.literal)) {
    literal_node.type_id = type_store_.new_basic_type(tp::Char{});
  }

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(literal_node.type_id, expected_type)) {
    throw std::runtime_error("Type mismatch in literal node. Expected type "
                             "is not correspond to literal type.");
  }

  if (expected_type != tp::no_type_id &&
      !type_store_.unify(expected_type, literal_node.type_id)) {
    throw std::runtime_error("Type mismatch in literal node. Expected type "
                             "is not correspond to literal type.");
  }
}
