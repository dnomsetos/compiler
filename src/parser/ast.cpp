#include <parser/ast.hpp>

namespace ast {

TypeIdWrapper::TypeIdWrapper() : type_id{tp::no_type_id} {}

LiteralNode::LiteralNode(const TrimmedLiteralVariant& literal,
                         const Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      literal{alloc::make_unique_pmr<TrimmedLiteralVariant>(literal)} {}

IdentifierNode::IdentifierNode(const tkn::Identifier& identifier,
                               const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      identifier{alloc::make_unique_pmr<tkn::Identifier>(identifier)} {}

FunctionCallNode::FunctionCallNode(alloc::pmr_unique_ptr<IdentifierNode>&& name,
                                   const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      name{std::move(name)}, arguments{&alloc::mr} {}

UnaryNode::UnaryNode(alloc::pmr_unique_ptr<PrimaryNode>&& primary,
                     const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      primary{std::move(primary)} {}

CastNode::CastNode(alloc::pmr_unique_ptr<UnaryNode>&& expression,
                   const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      expression{std::move(expression)} {}

ExpressionNode::ExpressionNode(ExpressionNodeVariant&& node,
                               const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      node{alloc::make_unique_pmr<ExpressionNodeVariant>(std::move(node))} {}

ExpressionStatements::ExpressionStatements(
    alloc::pmr_unique_ptr<ExpressionNode>&& expr,
    alloc::pmr_unique_ptr<BlockExpressionNode>&& block)
    : expr{std::move(expr)}, block{std::move(block)} {}

BlockExpressionNode::BlockExpressionNode(const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      statements{&alloc::mr}, value{std::nullopt} {}

LoopExpressionNode::LoopExpressionNode(std::optional<tkn::Label>&& label,
                                       const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      label{std::move(label)} {}

IfExpressionNode::IfExpressionNode(
    alloc::pmr_unique_ptr<ExpressionNode>&& condition,
    alloc::pmr_unique_ptr<BlockExpressionNode>&& body,
    const tkn::Position& position)
    : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
      condition{std::move(condition)}, body{std::move(body)},
      elif_bodies{&alloc::mr} {}

StatementNode::StatementNode(StatementNodeVariant&& node,
                             const tkn::Position& position)
    : ASTNode(position, SymbolTableInfo{}),
      node{alloc::make_unique_pmr<StatementNodeVariant>(std::move(node))} {}

VariableDefinitionNode::VariableDefinitionNode(
    alloc::pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position, SymbolTableInfo{}), name{std::move(name)},
      is_global(false) {}

BreakStatementNode::BreakStatementNode(const tkn::Position& position)
    : ASTNode(position, SymbolTableInfo{}) {}

ContinueStatementNode::ContinueStatementNode(const tkn::Position& position)
    : ASTNode(position, SymbolTableInfo{}) {}

ReturnStatementNode::ReturnStatementNode(const tkn::Position& position)
    : ASTNode(position, SymbolTableInfo{}) {}

FunctionDefinitionNode::FunctionDefinitionNode(
    alloc::pmr_unique_ptr<IdentifierNode>&& name, const tkn::Position& position)
    : ASTNode(position, SymbolTableInfo{}), name{std::move(name)} {}

} // namespace ast
