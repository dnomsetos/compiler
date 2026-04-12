#include <parser/ast.hpp>

namespace ast {

LiteralNode::LiteralNode(
    const type_tuple_to_variant_t<tkn::LiteralTuple>& literal,
    const Position& position)
    : ASTNode(position, tp::Type{}),
      literal{
          alloc::make_unique_pmr<type_tuple_to_variant_t<tkn::LiteralTuple>>(
              literal)} {}

IdentifierNode::IdentifierNode(const tkn::Identifier& identifier,
                               const tkn::Position& position)
    : ASTNode(position, tp::Type{}),
      identifier{alloc::make_unique_pmr<tkn::Identifier>(identifier)} {}

FunctionCallNode::FunctionCallNode(alloc::pmr_unique_ptr<IdentifierNode>&& name,
                                   const tkn::Position& position)
    : ASTNode(position, tp::Type{}), name{std::move(name)},
      arguments{&alloc::mr} {}

ExpressionNode::ExpressionNode(ExpressionNodeVariant&& node,
                               const tkn::Position& position)
    : Storage<tkn::Position, tp::Type>(position, tp::Type{}),
      node{alloc::make_unique_pmr<ExpressionNodeVariant>(std::move(node))} {}

ExpressionStatements::ExpressionStatements(
    alloc::pmr_unique_ptr<ExpressionNode>&& expr,
    alloc::pmr_unique_ptr<BlockExpressionNode>&& block)
    : expr{std::move(expr)}, block{std::move(block)} {}

BlockExpressionNode::BlockExpressionNode(const tkn::Position& position)
    : ASTNode(position, tp::Type{}), statements{&alloc::mr},
      value{std::nullopt} {}

LoopExpressionNode::LoopExpressionNode(std::optional<tkn::Label>&& label,
                                       const tkn::Position& position)
    : ASTNode(position, tp::Type{}), label{std::move(label)} {}

IfExpressionNode::IfExpressionNode(
    alloc::pmr_unique_ptr<ExpressionNode>&& condition,
    alloc::pmr_unique_ptr<BlockExpressionNode>&& body,
    const tkn::Position& position)
    : ASTNode(position, tp::Type{}), condition{std::move(condition)},
      body{std::move(body)}, elif_bodies{&alloc::mr} {}

StatementNode::StatementNode(StatementNodeVariant&& node,
                             const tkn::Position& position)
    : ASTNode(position, tp::Type{}),
      node{alloc::make_unique_pmr<StatementNodeVariant>(std::move(node))} {}

VariableDefinitionNode::VariableDefinitionNode(
    alloc::pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position, tp::Type{}), name{std::move(name)} {}

BreakStatementNode::BreakStatementNode(const tkn::Position& position)
    : ASTNode(position, tp::Type{}) {}

ContinueStatementNode::ContinueStatementNode(const tkn::Position& position)
    : ASTNode(position, tp::Type{}) {}

ReturnStatementNode::ReturnStatementNode(const tkn::Position& position)
    : ASTNode(position, tp::Type{}) {}

FunctionDefinitionNode::FunctionDefinitionNode(
    alloc::pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position, tp::Type{}), name{std::move(name)} {}

} // namespace ast
