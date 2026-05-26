#include <iostream>
#include <ranges>
#include <stdexcept>
#include <unordered_set>

#include <borrow_check/control_flow_graph.hpp>

namespace bc_ir {

ControlFlowGraph::ControlFlowGraph(TypeStore& type_store)
    : type_store_(type_store) {}

auto ControlFlowGraph::find_loop(const std::string& label)
    -> ControlFlowGraph::LoopContext& {

  if (label == "") {
    return loop_stack_.back();
  }

  for (auto&& loop : std::views::reverse(loop_stack_)) {
    if (loop.label.has_value() && loop.label.value() == label) {
      return loop;
    }
  }

  throw std::runtime_error("Loop label " + label + " not found");
}

void ControlFlowGraph::visit(const ast::Program& program) {
  for (const auto& definition : program.definitions) {
    std::visit([&](auto& val) { visit(val); }, definition);
  }
}

void ControlFlowGraph::visit(
    const ast::FunctionDefinitionNode& function_definition) {

  const Symbol* function_symbol =
      function_definition.table->get_function_symbol(
          function_definition.name->identifier->name);

  Function function{.symbol = function_symbol,
                    .entry = alloc::make_shared_pmr<BasicBlock>(),
                    .name = function_definition.name->identifier->name};

  function.blocks.push_back(function.entry.get());

  function.entry->name = "entry";
  current_block_ = function.entry.get();

  functions_.push_back(std::move(function));

  for (auto&& [i, value] :
       std::views::enumerate(function_definition.argument_list)) {

    auto&& [arg, type] = value;
    std::string& arg_name = arg.identifier->name;

    const Symbol* symbol = arg.table->get_variable_symbol(arg_name);

    if (type_store_.is_reference_type(symbol->type)) {
      std::string placeholder_name =
          "%" + function_definition.name->identifier->name + ".arg" +
          std::to_string(i);

      Symbol phantom{.position = symbol->position,
                     .type = symbol->type,
                     .scope = symbol->scope,
                     .symbol_info = symbol->symbol_info};

      function.arg_placeholders.push_back(
          FunctionArgPlaceholder{.index = static_cast<std::size_t>(i),
                                 .phantom_symbol = std::move(phantom),
                                 .param_name = placeholder_name});

      const Symbol* phantom_ptr =
          &function.arg_placeholders.back().phantom_symbol;

      current_block_->instructions.emplace_back(Alloca{
          .position = arg, .variable = phantom_ptr, .name = placeholder_name});

      scopes_[arg.table].push_back(
          &std::get<Alloca>(std::get<AccessKindVariant>(
              current_block_->instructions.back().inst)));

      AccessKindVariant* phantom_access = &std::get<AccessKindVariant>(
          current_block_->instructions.back().inst);

      global_variables_.emplace(placeholder_name, phantom_access);

      if (type_store_.is_mutable_reference_type(symbol->type)) {
        current_block_->instructions.emplace_back(
            BorrowMut{.position = arg,
                      .reference = symbol,
                      .resource = phantom_ptr,
                      .name = arg_name,
                      .resource_name = placeholder_name});
      } else {
        current_block_->instructions.emplace_back(
            BorrowShared{.position = arg,
                         .reference = symbol,
                         .resource = phantom_ptr,
                         .name = arg_name,
                         .resource_name = placeholder_name});
      }
    } else {
      current_block_->instructions.emplace_back(
          Alloca{.position = arg, .variable = symbol, .name = arg_name});

      scopes_[arg.table].push_back(
          &std::get<Alloca>(std::get<AccessKindVariant>(
              current_block_->instructions.back().inst)));
    }

    variables_.emplace(
        std::make_pair(arg_name, arg.table),
        &std::get<AccessKindVariant>(current_block_->instructions.back().inst));
  }

  visit(*function_definition.body);

  current_block_->terminator = alloc::make_unique_pmr<Terminator>(ReturnInst{});

  current_block_ = nullptr;
}

void ControlFlowGraph::visit(
    const ast::VariableDefinitionNode& variable_definition) {

  const std::string& name = variable_definition.name->identifier->name;

  const Symbol* symbol =
      variable_definition.name->table->get_variable_symbol(name);

  if (current_block_ == nullptr) {
    return;
  }

  if (type_store_.is_reference_type(symbol->type)) {

    if (type_store_.is_mutable_reference_type(symbol->type)) {
      current_block_->instructions.emplace_back(
          BorrowMut{.position = variable_definition,
                    .reference = symbol,
                    .resource = nullptr,
                    .name = name,
                    .resource_name = ""});
    } else {
      current_block_->instructions.emplace_back(
          BorrowShared{.position = variable_definition,
                       .reference = symbol,
                       .resource = nullptr,
                       .name = name,
                       .resource_name = ""});
    }

    variables_.emplace(
        std::make_pair(name, variable_definition.name->table),
        &std::get<AccessKindVariant>(current_block_->instructions.back().inst));

    if (!variable_definition.value.has_value()) {
      return;
    }

    auto& right_unary_node =
        *std::get<ast::LogicalOrNode>(*variable_definition.value.value()->node)
             .left->left->left->left->left->left->left->left->left->expression;

    auto& borrow_inst = current_block_->instructions.back();

    if (!right_unary_node.op.has_value()) {
      auto& ident =
          std::get<ast::IdentifierNode>(*right_unary_node.primary->primary);

      auto& var = *variables_.at({ident.identifier->name, ident.table});

      if (std::holds_alternative<BorrowMut>(var)) {
        auto& borrow_mut_inst =
            std::get<BorrowMut>(std::get<AccessKindVariant>(borrow_inst.inst));

        borrow_mut_inst.resource = std::get<BorrowMut>(var).resource;
        borrow_mut_inst.resource_name = std::get<BorrowMut>(var).resource_name;

      } else {
        auto& borrow_shared_inst = std::get<BorrowShared>(
            std::get<AccessKindVariant>(borrow_inst.inst));

        borrow_shared_inst.resource = std::get<BorrowShared>(var).resource;
        borrow_shared_inst.resource_name =
            std::get<BorrowShared>(var).resource_name;
      }
      return;
    }

    auto& ident =
        std::get<ast::IdentifierNode>(*right_unary_node.primary->primary);

    if (!std::holds_alternative<tkn::Ampersand>(*right_unary_node.op.value())) {
      throw std::runtime_error("Expect reference, but without ampersand");
    }

    auto& var = variables_.at({ident.identifier->name, ident.table});

    if (!std::holds_alternative<Alloca>(*var)) {
      throw std::runtime_error("References to references are not allowed");
    }

    auto* alloca_symbol = std::get<Alloca>(*var).variable;

    std::visit(
        [&](auto&& val) {
          using T = std::decay_t<decltype(val)>;

          if constexpr (std::is_same_v<T, Alloca>) {
            throw std::logic_error("Invariant break");
          } else {
            val.resource = alloca_symbol;
            val.resource_name = std::get<Alloca>(*var).name;
          }
        },
        std::get<AccessKindVariant>(borrow_inst.inst));
  } else {
    current_block_->instructions.emplace_back(Alloca{
        .position = variable_definition, .variable = symbol, .name = name});

    scopes_[variable_definition.table].push_back(&std::get<Alloca>(
        std::get<AccessKindVariant>(current_block_->instructions.back().inst)));

    variables_.emplace(
        std::make_pair(name, variable_definition.name->table),
        &std::get<AccessKindVariant>(current_block_->instructions.back().inst));

    if (variable_definition.value.has_value()) {
      visit(*variable_definition.value.value());

      current_block_->instructions.emplace_back(
          Write{.position = *variable_definition.name,
                .target = &std::get<Alloca>(
                    *variables_.at({name, variable_definition.name->table}))});
    }
  }
}

void ControlFlowGraph::visit(const ast::StatementNode& statement) {
  std::visit(
      [&](auto& val) {
        using T = std::decay_t<decltype(val)>;

        if (std::is_same_v<T, ast::FunctionDefinitionNode>) {
          throw std::runtime_error("Unexpected function definition");
        }

        visit(val);
      },
      *statement.node);
}

void ControlFlowGraph::visit(const ast::BreakStatementNode& break_stmt) {

  ControlFlowGraph::LoopContext& loop = find_loop(
      break_stmt.label.has_value() ? break_stmt.label.value().name : "");

  if (break_stmt.value.has_value()) {
    visit(*break_stmt.value.value());
  }

  for (auto* alloca : scopes_[break_stmt.table]) {
    current_block_->instructions.push_back(
        Drop{.position = break_stmt, .target = alloca});
  }

  auto target = loop.finish.lock();

  target->incoming_edges.push_back(current_block_->shared_from_this());
  current_block_->terminator = alloc::make_unique_pmr<Terminator>(
      UnconditionalBranchInst{.target = target});

  current_block_ = nullptr;
}

void ControlFlowGraph::visit(const ast::ContinueStatementNode& continue_stmt) {

  ControlFlowGraph::LoopContext& loop = find_loop(
      continue_stmt.label.has_value() ? continue_stmt.label.value().name : "");

  for (auto* alloca : scopes_[continue_stmt.table]) {
    current_block_->instructions.push_back(
        Drop{.position = continue_stmt, .target = alloca});
  }

  auto target = loop.body.lock();

  target->incoming_edges.push_back(current_block_->shared_from_this());
  current_block_->terminator = alloc::make_unique_pmr<Terminator>(
      UnconditionalBranchInst{.target = target});

  current_block_ = nullptr;
}

void ControlFlowGraph::visit(const ast::ReturnStatementNode& return_stmt) {

  if (return_stmt.value.has_value()) {
    visit(*return_stmt.value.value());
  }

  current_block_->terminator = alloc::make_unique_pmr<Terminator>(ReturnInst{});

  current_block_ = nullptr;
}

void ControlFlowGraph::visit(const ast::ExpressionNode& expression) {
  std::visit([&](auto& val) { visit(val); }, *expression.node);
}

void ControlFlowGraph::visit(const ast::BlockExpressionNode& expression) {
  if (expression.statements.empty() && !expression.value.has_value()) {
    return;
  }

  SymbolTable* scope;

  if (!expression.statements.empty()) {
    scope = expression.statements.front().table;
  } else {
    scope = expression.value.value()->table;
  }

  for (const auto& statement : expression.statements) {
    visit(statement);

    if (current_block_ == nullptr) {
      return;
    }
  }

  if (expression.value.has_value()) {
    visit(*expression.value.value());
  }

  for (auto* alloca : scopes_[scope]) {
    current_block_->instructions.push_back(
        Drop{.position = expression, .target = alloca});
  }
}

void ControlFlowGraph::visit(const ast::IfExpressionNode& expression) {

  auto merge_block = alloc::make_shared_pmr<BasicBlock>();
  functions_.back().blocks.push_back(merge_block.get());

  merge_block->name = "if.merge";

  auto build_branch =
      [&](const ast::ExpressionNode& cond, const ast::BlockExpressionNode& body,
          const std::string& suffix) -> alloc::pmr_shared_ptr<BasicBlock> {
    visit(cond);

    auto body_block = alloc::make_shared_pmr<BasicBlock>();
    auto next_block = alloc::make_shared_pmr<BasicBlock>();
    body_block->name = suffix + ".body";
    next_block->name = suffix + ".next";

    functions_.back().blocks.push_back(body_block.get());
    functions_.back().blocks.push_back(next_block.get());

    body_block->incoming_edges.push_back(current_block_->shared_from_this());
    next_block->incoming_edges.push_back(current_block_->shared_from_this());

    SwitchInst sw;
    sw.cases.push_back(body_block);
    sw.cases.push_back(next_block);
    current_block_->terminator =
        alloc::make_unique_pmr<Terminator>(std::move(sw));

    current_block_ = body_block.get();
    visit(body);

    if (current_block_ != nullptr) {
      merge_block->incoming_edges.push_back(current_block_->shared_from_this());
      current_block_->terminator = alloc::make_unique_pmr<Terminator>(
          UnconditionalBranchInst{.target = merge_block});
    }

    return next_block;
  };

  auto next_block = build_branch(*expression.condition, *expression.body, "if");

  for (std::size_t i = 0; i < expression.elif_bodies.size(); ++i) {
    current_block_ = next_block.get();
    const auto& elif = expression.elif_bodies[i];
    next_block =
        build_branch(*elif.expr, *elif.block, "elif" + std::to_string(i));
  }

  current_block_ = next_block.get();
  if (expression.else_body.has_value()) {
    visit(*expression.else_body.value());
    if (current_block_ != nullptr) {
      merge_block->incoming_edges.push_back(current_block_->shared_from_this());
      current_block_->terminator = alloc::make_unique_pmr<Terminator>(
          UnconditionalBranchInst{.target = merge_block});
    }
  } else {
    merge_block->incoming_edges.push_back(current_block_->shared_from_this());
    current_block_->terminator = alloc::make_unique_pmr<Terminator>(
        UnconditionalBranchInst{.target = merge_block});
  }

  current_block_ = merge_block.get();
}

void ControlFlowGraph::visit(const ast::LoopExpressionNode& loop) {

  if (loop.body.empty()) {
    throw std::runtime_error("Infinity loop");
  }

  LoopContext loop_context;

  loop_context.label = loop.label.has_value()
                           ? std::optional<std::string>(loop.label.value().name)
                           : std::nullopt;

  auto body_block = alloc::make_shared_pmr<BasicBlock>();
  auto finish_block = alloc::make_shared_pmr<BasicBlock>();

  functions_.back().blocks.push_back(body_block.get());
  functions_.back().blocks.push_back(finish_block.get());

  body_block->name = loop_context.label.has_value()
                         ? loop_context.label.value() + ".body"
                         : "loop.body";
  finish_block->name = loop_context.label.has_value()
                           ? loop_context.label.value() + ".finish"
                           : "loop.finish";

  loop_context.body = body_block;
  loop_context.finish = finish_block;

  body_block->terminator = alloc::make_unique_pmr<Terminator>(
      UnconditionalBranchInst{.target = body_block});

  body_block->incoming_edges.push_back(current_block_->shared_from_this());
  current_block_->terminator = alloc::make_unique_pmr<Terminator>(
      UnconditionalBranchInst{.target = body_block});

  loop_stack_.push_back(loop_context);

  current_block_ = body_block.get();

  auto* scope = loop.body.front().table;

  for (auto& statement : loop.body) {
    visit(statement);

    if (current_block_ == nullptr) {
      break;
    }
  }

  for (auto* alloca : scopes_[scope]) {
    current_block_->instructions.push_back(
        Drop{.position = loop, .target = alloca});
  }

  if (current_block_ != nullptr) {
    body_block->incoming_edges.push_back(current_block_->shared_from_this());
    current_block_->terminator = alloc::make_unique_pmr<Terminator>(
        UnconditionalBranchInst{.target = body_block});
  }

  loop_stack_.pop_back();

  current_block_ = finish_block.get();
}

void ControlFlowGraph::visit(const ast::AssignmentNode& assignment) {
  if (!type_store_.is_reference_type(assignment.right->type_id)) {
    visit(*assignment.right);

    if (std::holds_alternative<ast::IdentifierNode>(*assignment.left->lvalue)) {

      auto& identifier =
          std::get<ast::IdentifierNode>(*assignment.left->lvalue);

      auto* init_table =
          identifier.table->get_variable_symbol(identifier.identifier->name)
              ->scope;

      auto& var = *variables_.at({identifier.identifier->name, init_table});

      if (!std::holds_alternative<Alloca>(var)) {
        throw std::runtime_error("Variable is not a value");
      }

      current_block_->instructions.emplace_back(Write{
          .position = *assignment.left, .target = &std::get<Alloca>(var)});

      return;
    }

    auto& deref =
        std::get<ast::LvalueDereferenceNode>(*assignment.left->lvalue);

    if (std::holds_alternative<ast::FunctionCallNode>(*deref.reference)) {
      throw std::runtime_error("Cannot assign to function call");
    }

    auto& identifier = std::get<ast::IdentifierNode>(*deref.reference);

    SymbolTable* init_table =
        identifier.table->get_variable_symbol(identifier.identifier->name)
            ->scope;

    auto& var = *variables_.at({identifier.identifier->name, init_table});

    if (std::holds_alternative<Alloca>(var)) {
      throw std::runtime_error("Cannot borrow to value");
    }

    if (std::holds_alternative<BorrowShared>(var)) {
      throw std::runtime_error("Cannot assign to immutable reference");
    } else if (std::holds_alternative<Alloca>(var)) {
      std::logic_error("Incorrect variable type");
    } else {
      current_block_->instructions.emplace_back(WriteRef{
          .position = *assignment.left, .target = &std::get<BorrowMut>(var)});
    }

    return;
  }

  auto& right_unary_node =
      std::get<ast::LogicalOrNode>(*assignment.right->node)
          .left->left->left->left->left->left->left->left->left->expression;

  auto& lident = std::get<ast::IdentifierNode>(*assignment.left->lvalue);

  SymbolTable* init_table =
      lident.table->get_variable_symbol(lident.identifier->name)->scope;

  auto& lvar = *variables_.at({lident.identifier->name, init_table});

  if (!right_unary_node->op.has_value()) {
    auto& ident =
        std::get<ast::IdentifierNode>(*right_unary_node->primary->primary);

    SymbolTable* init_table =
        ident.table->get_variable_symbol(ident.identifier->name)->scope;

    auto& var = *variables_.at({ident.identifier->name, init_table});

    if (std::holds_alternative<Alloca>(var)) {
      throw std::logic_error("Value but expect reference");
    }

    if (std::holds_alternative<Alloca>(lvar)) {
      throw std::logic_error("Value but expect reference");
    }

    if (std::holds_alternative<BorrowShared>(lvar)) {
      if (!std::holds_alternative<BorrowShared>(var)) {
        throw std::runtime_error("Cannot assign to immutable reference");
      } else {
        current_block_->instructions.emplace_back(BorrowShared{
            .position = assignment,
            .reference = std::get<BorrowShared>(lvar).reference,
            .resource = std::get<BorrowShared>(var).resource,
            .name = std::get<BorrowShared>(lvar).name,
            .resource_name = std::get<BorrowShared>(var).resource_name});
      }
    } else {
      if (!std::holds_alternative<BorrowMut>(var)) {
        throw std::runtime_error(
            "Cannot borrow mutable place from immutable reference");
      } else {
        current_block_->instructions.emplace_back(
            BorrowMut{.position = assignment,
                      .reference = std::get<BorrowMut>(lvar).reference,
                      .resource = std::get<BorrowMut>(var).resource,
                      .name = std::get<BorrowMut>(lvar).name,
                      .resource_name = std::get<BorrowMut>(var).resource_name});
      }
    }

    return;
  }

  if (!std::holds_alternative<tkn::Ampersand>(*right_unary_node->op.value())) {
    throw std::runtime_error("Cannot assign value to reference");
  }

  auto& src =
      std::get<ast::IdentifierNode>(*right_unary_node->primary->primary);

  SymbolTable* src_init_table =
      src.table->get_variable_symbol(src.identifier->name)->scope;

  auto& var = *variables_.at({src.identifier->name, src_init_table});

  if (!std::holds_alternative<Alloca>(var)) {
    throw std::runtime_error("Attempt to get reference to reference");
  }

  if (right_unary_node->is_mut_ref) {
    current_block_->instructions.emplace_back(
        BorrowMut{.position = lident,
                  .reference = std::get<BorrowMut>(lvar).reference,
                  .resource = std::get<Alloca>(var).variable,
                  .name = std::get<BorrowMut>(lvar).name,
                  .resource_name = std::get<Alloca>(var).name});
  } else {
    current_block_->instructions.emplace_back(
        BorrowShared{.position = lident,
                     .reference = std::get<BorrowShared>(lvar).reference,
                     .resource = std::get<Alloca>(var).variable,
                     .name = std::get<BorrowShared>(lvar).name,
                     .resource_name = std::get<Alloca>(var).name});
  }
}

template <typename BinaryNode>
  requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
void ControlFlowGraph::visit(const BinaryNode& binary_node) {

  visit(*binary_node.left);

  for (auto& [op, rhs_expr] : binary_node.right) {
    visit(rhs_expr);
  }
}

void ControlFlowGraph::visit(const ast::CastNode& cast_node) {
  visit(*cast_node.expression);
}

void ControlFlowGraph::visit(const ast::UnaryNode& unary_node) {

  auto& node = *unary_node.primary->primary;

  if (std::holds_alternative<ast::ExpressionNode>(node)) {
    visit(std::get<ast::ExpressionNode>(node));
    return;
  }

  if (std::holds_alternative<ast::LiteralNode>(node)) {
    return;
  }

  if (std::holds_alternative<ast::FunctionCallNode>(node)) {
    visit(std::get<ast::FunctionCallNode>(node));
    return;
  }

  if (!unary_node.op.has_value()) {
    return;
  }

  auto& identifier = std::get<ast::IdentifierNode>(node);

  if (std::holds_alternative<tkn::Ampersand>(*unary_node.op.value())) {
    throw std::runtime_error("Unexpected ampersand");
  } else if (std::holds_alternative<tkn::Asterisk>(*unary_node.op.value())) {
    SymbolTable* init_table =
        identifier.table->get_variable_symbol(identifier.identifier->name)
            ->scope;
    auto& var = *variables_.at({identifier.identifier->name, init_table});

    if (std::holds_alternative<BorrowShared>(var)) {
      auto& ref = std::get<BorrowShared>(var);

      current_block_->instructions.emplace_back(
          ReadImmutRef{.position = unary_node, .target = &ref});
    } else if (std::holds_alternative<BorrowMut>(var)) {
      auto& ref = std::get<BorrowMut>(var);

      current_block_->instructions.emplace_back(
          ReadMutRef{.position = unary_node, .target = &ref});
    } else {
      throw std::runtime_error("Try to dereference value");
    }
  }
}

void ControlFlowGraph::visit(const ast::FunctionCallNode& function_call) {

  for (const auto& arg : function_call.arguments) {
    visit(arg);
  }

  const Symbol* symbol = function_call.name->table->get_function_symbol(
      function_call.name->identifier->name);

  std::vector<ArgBinding> bindings;

  const auto* fn_info = std::get_if<FunctionInfo>(&symbol->symbol_info.info);

  if (fn_info && fn_info->definition) {
    const auto& param_list = fn_info->definition->argument_list;

    for (std::size_t i = 0;
         i < param_list.size() && i < function_call.arguments.size(); ++i) {

      const auto& [param_id, param_type_node] = param_list[i];
      const Symbol* param_sym =
          param_id.table->get_variable_symbol(param_id.identifier->name);

      if (!type_store_.is_reference_type(param_sym->type)) {
        continue;
      }

      const ast::ExpressionNode& expr = function_call.arguments[i];
      const auto* unary = &*std::get<ast::LogicalOrNode>(*expr.node)
                                .left->left->left->left->left->left->left->left
                                ->left->expression;

      ArgBinding binding;
      binding.param = param_sym;
      binding.param_name = param_id.identifier->name;
      binding.is_mut = type_store_.is_mutable_reference_type(param_sym->type);

      if (!unary->op.has_value()) {
        const auto& ident =
            std::get<ast::IdentifierNode>(*unary->primary->primary);

        SymbolTable* init_table =
            ident.table->get_variable_symbol(ident.identifier->name)->scope;

        const auto& var = *variables_.at({ident.identifier->name, init_table});

        if (std::holds_alternative<BorrowShared>(var)) {
          binding.resource = std::get<BorrowShared>(var).resource;
          binding.resource_name = std::get<BorrowShared>(var).resource_name;
        } else if (std::holds_alternative<BorrowMut>(var)) {
          binding.resource = std::get<BorrowMut>(var).resource;
          binding.resource_name = std::get<BorrowMut>(var).resource_name;
        } else {
          throw std::runtime_error(
              "Expected reference variable for reference parameter");
        }

      } else if (std::holds_alternative<tkn::Ampersand>(*unary->op.value())) {
        const auto& ident =
            std::get<ast::IdentifierNode>(*unary->primary->primary);

        SymbolTable* init_table =
            ident.table->get_variable_symbol(ident.identifier->name)->scope;

        const auto& var = *variables_.at({ident.identifier->name, init_table});

        if (!std::holds_alternative<Alloca>(var)) {
          throw std::runtime_error("Cannot borrow reference to reference");
        }

        binding.resource = std::get<Alloca>(var).variable;
        binding.resource_name = std::get<Alloca>(var).name;

      } else {
        continue;
      }

      bindings.push_back(std::move(binding));
    }
  }

  current_block_->instructions.emplace_back(
      FunctionCallInst{.position = function_call,
                       .symbol = symbol,
                       .arg_bindings = std::move(bindings),
                       .name = function_call.name->identifier->name});
}

void ControlFlowGraph::print() const {

  auto print_block = [](const BasicBlock& block, std::ostream& out) {
    out << "[block: " << block.name << "]\n";

    for (const auto& instr : block.instructions) {
      out << "  ";
      std::visit(
          [&](const auto& val) {
            using T = std::decay_t<decltype(val)>;

            if constexpr (std::is_same_v<T, AccessKindVariant>) {
              std::visit(
                  [&](const auto& access) {
                    using A = std::decay_t<decltype(access)>;
                    if constexpr (std::is_same_v<A, Alloca>) {
                      if (!access.name.empty() && access.name[0] == '%') {
                        out << "alloca.phantom " << access.name;
                      } else {
                        out << "alloca " << access.name;
                      }
                    } else if constexpr (std::is_same_v<A, BorrowShared>) {
                      out << "borrow.shared " << access.name << " from "
                          << access.resource_name;
                    } else if constexpr (std::is_same_v<A, BorrowMut>) {
                      out << "borrow.mut " << access.name << " from "
                          << access.resource_name;
                    }
                  },
                  val);
            } else if constexpr (std::is_same_v<T, Read>) {
              out << "read " << val.target->name;
            } else if constexpr (std::is_same_v<T, Write>) {
              if (val.target) {
                out << "write " << val.target->name;
              } else {
                out << "write <null>";
              }
            } else if constexpr (std::is_same_v<T, ReadImmutRef>) {
              out << "read.immut_ref " << val.target->name;
            } else if constexpr (std::is_same_v<T, ReadMutRef>) {
              out << "read.mut_ref " << val.target->name;
            } else if constexpr (std::is_same_v<T, WriteRef>) {
              out << "write.ref " << val.target->name;
            } else if constexpr (std::is_same_v<T, Drop>) {
              out << "drop " << val.target->name;
            } else if constexpr (std::is_same_v<T, FunctionCallInst>) {
              out << "call " << val.name;
              for (const auto& b : val.arg_bindings) {
                out << "\n    " << b.param_name
                    << (b.is_mut ? " <-mut- " : " <-ref- ") << b.resource_name;
              }
            }
          },
          instr.inst);
      out << "\n";
    }

    if (block.terminator) {
      out << "  terminator: ";
      std::visit(
          [&](const auto& term) {
            using T = std::decay_t<decltype(term)>;
            if constexpr (std::is_same_v<T, UnconditionalBranchInst>) {
              out << "br -> " << (term.target ? term.target->name : "<null>");
            } else if constexpr (std::is_same_v<T, SwitchInst>) {
              out << "switch [";
              for (const auto& c : term.cases) {
                out << (c ? c->name : "<null>") << " ";
              }
              out << "]";
            } else if constexpr (std::is_same_v<T, ReturnInst>) {
              out << "ret";
            }
          },
          block.terminator->terminator);
      out << "\n";
    }
    out << "\n";
  };

  for (const auto& function : functions_) {
    std::cout << "fn " << function.name << "(";
    if (function.symbol) {
      const auto& arg_list =
          std::get<FunctionInfo>(function.symbol->symbol_info.info)
              .definition->argument_list;
      for (std::size_t pi = 0; pi < arg_list.size(); ++pi) {
        if (pi > 0)
          std::cout << ", ";
        const auto& [arg_id, arg_type] = arg_list[pi];
        std::cout << arg_id.identifier->name;
      }
    }
    std::cout << ")";

    if (!function.arg_placeholders.empty()) {
      std::cout << "  // phantoms: ";
      for (const auto& ph : function.arg_placeholders) {
        std::cout << ph.param_name << " ";
      }
    }
    std::cout << ":\n";

    if (!function.entry) {
      std::cout << "  <no entry block>\n\n";
      continue;
    }

    std::unordered_set<const BasicBlock*> visited;
    std::deque<const BasicBlock*> queue;

    queue.push_back(function.entry.get());

    while (!queue.empty()) {
      const BasicBlock* block = queue.front();
      queue.pop_front();

      if (!block || visited.count(block)) {
        continue;
      }
      visited.insert(block);

      print_block(*block, std::cout);

      if (block->terminator) {
        std::visit(
            [&](const auto& term) {
              using T = std::decay_t<decltype(term)>;
              if constexpr (std::is_same_v<T, UnconditionalBranchInst>) {
                queue.push_back(term.target.get());
              } else if constexpr (std::is_same_v<T, SwitchInst>) {
                for (const auto& c : term.cases) {
                  queue.push_back(c.get());
                }
              }
            },
            block->terminator->terminator);
      }
    }
  }
}

auto ControlFlowGraph::get_functions() const -> const std::deque<Function>& {
  return functions_;
}

} // namespace bc_ir
