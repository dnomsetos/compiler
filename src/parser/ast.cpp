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

ExpressionNode::ExpressionNode(std::variant<OrNode, AssignmentNode>&& node,
                               const tkn::Position& position)
    : Storage<tkn::Position, Type>(position, Type{}),
      node{alloc::make_unique_pmr<std::variant<OrNode, AssignmentNode>>(
          std::move(node))} {}

ExpressionStatements::ExpressionStatements(ExpressionNode&& expr)
    : expr{std::move(expr)}, statements{&alloc::mr} {}

IfStatementNode::IfStatementNode(pmr_unique_ptr<ExpressionNode>&& condition,
                                 const tkn::Position& position)
    : ASTNode(position), condition{std::move(condition)}, body{&alloc::mr},
      elif_bodies{&alloc::mr}, else_body{&alloc::mr} {}

StatementNode::StatementNode(std::variant<ExpressionNode, IfStatementNode,
                                          VariableDefinitionNode>&& node,
                             const tkn::Position& position)
    : ASTNode(position),
      node{alloc::make_unique_pmr<std::variant<ExpressionNode, IfStatementNode,
                                               VariableDefinitionNode>>(
          std::move(node))} {}

VariableDefinitionNode::VariableDefinitionNode(
    pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position), name{std::move(name)} {}

FunctionDefinitionNode::FunctionDefinitionNode(
    pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position), name{std::move(name)}, body{&alloc::mr} {}

} // namespace ast
