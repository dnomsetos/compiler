#include "scanner/token.hpp"
#include "utility/ast_allocator.hpp"
#include <parser/ast.hpp>
#include <variant>

namespace ast {

LiteralNode::LiteralNode(
    const type_tuple_to_variant_t<tkn::LiteralTuple>& literal,
    const Position& position)
    : ASTNode(position),
      literal{
          alloc::make_unique_pmr<type_tuple_to_variant_t<tkn::LiteralTuple>>(
              literal)} {}

IdentifierNode::IdentifierNode(const tkn::Identifier& identifier,
                               const tkn::Position& position)
    : ASTNode(position),
      identifier{alloc::make_unique_pmr<tkn::Identifier>(identifier)} {}

FunctionCallNode::FunctionCallNode(pmr_unique_ptr<IdentifierNode>&& name,
                                   const tkn::Position& position)
    : ASTNode(position), name{std::move(name)}, arguments{&alloc::mr} {}

ExpressionNode::ExpressionNode(
    std::variant<OrNode, AssignmentNode, IfExpressionNode,
                 BlockExpressionNode>&& node,
    const tkn::Position& position)
    : Storage<tkn::Position, Type>(position, Type{}),
      node{alloc::make_unique_pmr<std::variant<
          OrNode, AssignmentNode, IfExpressionNode, BlockExpressionNode>>(
          std::move(node))} {}

ExpressionStatements::ExpressionStatements(
    pmr_unique_ptr<ExpressionNode>&& expr,
    pmr_unique_ptr<BlockExpressionNode>&& block, const tkn::Position& position)
    : ASTNode(position), expr{std::move(expr)}, block{std::move(block)} {}

BlockExpressionNode::BlockExpressionNode(const tkn::Position& position)
    : ASTNode(position), statements{&alloc::mr}, value{std::nullopt} {}

IfExpressionNode::IfExpressionNode(pmr_unique_ptr<ExpressionNode>&& condition,
                                   pmr_unique_ptr<BlockExpressionNode>&& body,
                                   const tkn::Position& position)
    : ASTNode(position), condition{std::move(condition)}, body{std::move(body)},
      elif_bodies{&alloc::mr} {}

StatementNode::StatementNode(
    std::variant<ExpressionNode, VariableDefinitionNode>&& node,
    const tkn::Position& position)
    : ASTNode(position),
      node{alloc::make_unique_pmr<
          std::variant<ExpressionNode, VariableDefinitionNode>>(
          std::move(node))} {}

VariableDefinitionNode::VariableDefinitionNode(
    pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position), name{std::move(name)} {}

FunctionDefinitionNode::FunctionDefinitionNode(
    pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position), name{std::move(name)} {}

} // namespace ast
