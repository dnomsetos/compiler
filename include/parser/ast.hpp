#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <scanner/token.hpp>
#include <utility/ast_allocator.hpp>
#include <utility/storage.hpp>
#include <utility/type_tuple.hpp>

namespace ast {

using ASTNode = Storage<tkn::Position>;

struct Type {};

template <typename T>
using pmr_unique_ptr =
    std::unique_ptr<T, alloc::MonotonicBufferResourceDeleter<T>>;

struct LiteralNode : ASTNode {
  pmr_unique_ptr<type_tuple_to_variant_t<tkn::LiteralTuple>> literal;

  LiteralNode(const type_tuple_to_variant_t<tkn::LiteralTuple>& literal,
              const tkn::Position& position);
};

struct IdentifierNode : ASTNode {
  pmr_unique_ptr<tkn::Identifier> identifier;

  IdentifierNode(const tkn::Identifier& identifier,
                 const tkn::Position& position);
};

struct ExpressionNode;

struct FunctionCallNode;

using PrimaryNodeTuple =
    TypeTuple<LiteralNode, IdentifierNode, ExpressionNode, FunctionCallNode>;

using PrimaryNodeVariant = type_tuple_to_variant_t<PrimaryNodeTuple>;

struct FunctionCallNode : ASTNode {
  pmr_unique_ptr<IdentifierNode> name;
  std::pmr::vector<ExpressionNode> arguments;

  FunctionCallNode(pmr_unique_ptr<IdentifierNode>&& name,
                   const tkn::Position& position);
};

struct PrimaryNode : Storage<tkn::Position, Type> {
  pmr_unique_ptr<PrimaryNodeVariant> primary;

  template <typename T>
  PrimaryNode(T&& primary, const tkn::Position& position)
      : Storage<tkn::Position, Type>(position, Type{}),
        primary{alloc::make_unique_pmr<PrimaryNodeVariant>(
            std::in_place_type_t<std::decay_t<decltype(primary)>>{},
            std::forward<T>(primary))} {}
};

struct UnaryNode : Storage<tkn::Position, Type> {
  pmr_unique_ptr<PrimaryNode> primary;
  std::optional<
      pmr_unique_ptr<type_tuple_to_variant_t<tkn::UnaryOperatorTuple>>>
      op;

  UnaryNode(pmr_unique_ptr<PrimaryNode>&& primary,
            const tkn::Position& position)
      : Storage<tkn::Position, Type>(position, Type{}),
        primary{std::move(primary)} {}
};

#define GENERATE_NODE(name, op, type)                                          \
  struct name : Storage<tkn::Position, Type> {                                 \
    pmr_unique_ptr<type> left;                                                 \
    std::pmr::vector<std::pair<op, type>> right;                               \
    name(pmr_unique_ptr<type>&& left, const tkn::Position& position)           \
        : Storage<tkn::Position, Type>(position, Type{}),                      \
          left{std::move(left)}, right{&alloc::mr} {}                          \
  };

GENERATE_NODE(MultiplicationNode,
              type_tuple_to_variant_t<tkn::HighPriorityArithmeticOperatorTuple>,
              UnaryNode);
GENERATE_NODE(AdditionNode,
              type_tuple_to_variant_t<tkn::LowPriorityArithmeticOperatorTuple>,
              MultiplicationNode);
GENERATE_NODE(ComparisonNode,
              type_tuple_to_variant_t<tkn::ComparisonOperatorTuple>,
              AdditionNode);
GENERATE_NODE(EqualityNode, type_tuple_to_variant_t<tkn::EqualityOperatorTuple>,
              ComparisonNode);
GENERATE_NODE(AndNode, tkn::And, EqualityNode);
GENERATE_NODE(XorNode, tkn::Xor, AndNode);
GENERATE_NODE(OrNode, tkn::Or, XorNode);

using BinaryNodeTuple =
    TypeTuple<AndNode, XorNode, OrNode, EqualityNode, ComparisonNode,
              AdditionNode, MultiplicationNode>;

struct AssignmentNode : Storage<tkn::Position, Type> {
  pmr_unique_ptr<IdentifierNode> left;
  pmr_unique_ptr<ExpressionNode> right;

  template <typename T1, typename T2>
  AssignmentNode(T1&& left, T2&& right, const tkn::Position& position)
      : Storage<tkn::Position, Type>(position, Type{}),
        left{std::forward<T1>(left)}, right{std::forward<T2>(right)} {}
};

struct StatementNode;

struct BlockExpressionNode : ASTNode {
  std::pmr::vector<StatementNode> statements;
  std::optional<pmr_unique_ptr<ExpressionNode>> value;

  BlockExpressionNode(const tkn::Position& position);
};

struct LoopExpressionNode : ASTNode {
  std::optional<tkn::Label> label;
  std::pmr::vector<StatementNode> body;

  LoopExpressionNode(std::optional<tkn::Label>&& label,
                     const tkn::Position& position);
};

struct ExpressionStatements;

struct IfExpressionNode : ASTNode {
  pmr_unique_ptr<ExpressionNode> condition;
  pmr_unique_ptr<BlockExpressionNode> body;
  std::pmr::vector<ExpressionStatements> elif_bodies;
  std::optional<pmr_unique_ptr<BlockExpressionNode>> else_body;

  IfExpressionNode(pmr_unique_ptr<ExpressionNode>&& condition,
                   pmr_unique_ptr<BlockExpressionNode>&& body,
                   const tkn::Position& position);
};

using ExpressionNodeTuple = TypeTuple<OrNode, AssignmentNode, IfExpressionNode,
                                      BlockExpressionNode, LoopExpressionNode>;

using ExpressionNodeVariant = type_tuple_to_variant_t<ExpressionNodeTuple>;

struct ExpressionNode : Storage<tkn::Position, Type> {
  pmr_unique_ptr<ExpressionNodeVariant> node;

  ExpressionNode(ExpressionNodeVariant&& node, const tkn::Position& position);
};

struct ExpressionStatements : ASTNode {
  pmr_unique_ptr<ExpressionNode> expr;
  pmr_unique_ptr<BlockExpressionNode> block;

  ExpressionStatements(pmr_unique_ptr<ExpressionNode>&& expr,
                       pmr_unique_ptr<BlockExpressionNode>&& block,
                       const tkn::Position& position);
};

struct VariableDefinitionNode : ASTNode {
  pmr_unique_ptr<IdentifierNode> name;
  std::optional<pmr_unique_ptr<IdentifierNode>> type;
  std::optional<pmr_unique_ptr<ExpressionNode>> value;

  VariableDefinitionNode(pmr_unique_ptr<IdentifierNode>&& name,
                         const tkn::Position& position);
};

struct BreakStatementNode : ASTNode {
  std::optional<tkn::Label> label;
  std::optional<pmr_unique_ptr<ExpressionNode>> value;

  BreakStatementNode(const tkn::Position& position);
};

struct ContinueStatementNode : ASTNode {
  std::optional<tkn::Label> label;

  ContinueStatementNode(const tkn::Position& position);
};

struct ReturnStatementNode : ASTNode {
  std::optional<pmr_unique_ptr<ExpressionNode>> value;

  ReturnStatementNode(const tkn::Position& position);
};

using InterruptNodeTuple =
    TypeTuple<BreakStatementNode, ContinueStatementNode, ReturnStatementNode>;

static_assert(type_tuple_index_v<BreakStatementNode, InterruptNodeTuple> == 0);
static_assert(type_tuple_index_v<ContinueStatementNode, InterruptNodeTuple> ==
              1);
static_assert(type_tuple_index_v<ReturnStatementNode, InterruptNodeTuple> == 2);

using InterruptNodeVariant = type_tuple_to_variant_t<InterruptNodeTuple>;

using StatementNodeTuple =
    TypeTuple<ExpressionNode, VariableDefinitionNode, BlockExpressionNode,
              IfExpressionNode, LoopExpressionNode, BreakStatementNode,
              ContinueStatementNode, ReturnStatementNode>;

using StatementNodeVariant = type_tuple_to_variant_t<StatementNodeTuple>;

struct StatementNode : ASTNode {
  pmr_unique_ptr<StatementNodeVariant> node;

  StatementNode(StatementNodeVariant&& node, const tkn::Position& position);
};

struct FunctionDefinitionNode : ASTNode {
  pmr_unique_ptr<IdentifierNode> name;
  std::pmr::vector<std::pair<IdentifierNode, IdentifierNode>> argument_list;
  pmr_unique_ptr<IdentifierNode> return_type;
  pmr_unique_ptr<BlockExpressionNode> body;

  FunctionDefinitionNode(pmr_unique_ptr<IdentifierNode>&& name,
                         const tkn::Position& position);
};

using DefinitionTuple =
    TypeTuple<FunctionDefinitionNode, VariableDefinitionNode>;

using DefinitionNodeVariant = type_tuple_to_variant_t<DefinitionTuple>;

struct Program {
  std::pmr::vector<DefinitionNodeVariant> definitions;

  Program() : definitions{&alloc::mr} {}
};

template <typename T> struct NodeName;

#define CREATE_NODE_NAME(node, name)                                           \
  template <> struct NodeName<node> {                                          \
    static constexpr const char* value = name;                                 \
  };

CREATE_NODE_NAME(LiteralNode, "literal")
CREATE_NODE_NAME(IdentifierNode, "identifier")
CREATE_NODE_NAME(FunctionCallNode, "function call")
CREATE_NODE_NAME(AssignmentNode, "assignment")
CREATE_NODE_NAME(OrNode, "or")
CREATE_NODE_NAME(XorNode, "xor")
CREATE_NODE_NAME(AndNode, "and")
CREATE_NODE_NAME(EqualityNode, "equality")
CREATE_NODE_NAME(ComparisonNode, "comparison")
CREATE_NODE_NAME(AdditionNode, "addition")
CREATE_NODE_NAME(MultiplicationNode, "multiplication")
CREATE_NODE_NAME(UnaryNode, "unary")
CREATE_NODE_NAME(PrimaryNode, "primary")
CREATE_NODE_NAME(ExpressionNode, "expression")
CREATE_NODE_NAME(StatementNode, "statement")
CREATE_NODE_NAME(IfExpressionNode, "if expression")
CREATE_NODE_NAME(VariableDefinitionNode, "variable definition")
CREATE_NODE_NAME(FunctionDefinitionNode, "function definition")

} // namespace ast
