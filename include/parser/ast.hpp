#pragma once

#include <optional>
#include <vector>

#include <scanner/token.hpp>
#include <utility/allocator.hpp>
#include <utility/storage.hpp>
#include <utility/type_tuple.hpp>

class SymbolTable;

struct SymbolTableInfo {
  SymbolTable* table = nullptr;
};

namespace tp {

using TypeId = std::size_t;

inline constexpr TypeId no_type_id = -1;

} // namespace tp

namespace ast {

struct TypeIdWrapper {
  TypeIdWrapper();

  tp::TypeId type_id;
};

using ASTNode = Storage<tkn::Position, SymbolTableInfo>;

using ASTTypeNode = Storage<tkn::Position, SymbolTableInfo, TypeIdWrapper>;

using TrimmedLiteralTuple = TypeTuple<tkn::IntLiteral, tkn::FloatLiteral,
                                      tkn::BoolLiteral, tkn::CharLiteral>;

using TrimmedLiteralVariant = type_tuple_to_variant_t<TrimmedLiteralTuple>;

struct LiteralNode : ASTTypeNode {
  alloc::pmr_unique_ptr<TrimmedLiteralVariant> literal;

  LiteralNode(const TrimmedLiteralVariant& literal,
              const tkn::Position& position);
};

struct IdentifierNode : ASTTypeNode {
  alloc::pmr_unique_ptr<tkn::Identifier> identifier;

  IdentifierNode(const tkn::Identifier& identifier,
                 const tkn::Position& position);
};

struct ExpressionNode;

struct FunctionCallNode;

using PrimaryNodeTuple =
    TypeTuple<LiteralNode, IdentifierNode, ExpressionNode, FunctionCallNode>;

using PrimaryNodeVariant = type_tuple_to_variant_t<PrimaryNodeTuple>;

struct FunctionCallNode : ASTTypeNode {
  alloc::pmr_unique_ptr<IdentifierNode> name;
  std::pmr::vector<ExpressionNode> arguments;

  FunctionCallNode(alloc::pmr_unique_ptr<IdentifierNode>&& name,
                   const tkn::Position& position);
};

struct PrimaryNode : ASTTypeNode {
  alloc::pmr_unique_ptr<PrimaryNodeVariant> primary;

  template <typename T>
  PrimaryNode(T&& primary, const tkn::Position& position)
      : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
        primary{alloc::make_unique_pmr<PrimaryNodeVariant>(
            std::in_place_type_t<std::decay_t<decltype(primary)>>{},
            std::forward<T>(primary))} {}
};

struct UnaryNode : ASTTypeNode {
  alloc::pmr_unique_ptr<PrimaryNode> primary;
  std::optional<
      alloc::pmr_unique_ptr<type_tuple_to_variant_t<tkn::UnaryOperatorTuple>>>
      op;

  UnaryNode(alloc::pmr_unique_ptr<PrimaryNode>&& primary,
            const tkn::Position& position);
};

struct CastNode : ASTTypeNode {
  alloc::pmr_unique_ptr<UnaryNode> expression;
  std::optional<alloc::pmr_unique_ptr<IdentifierNode>> type;

  CastNode(alloc::pmr_unique_ptr<UnaryNode>&& expression,
           const tkn::Position& position);
};

#define GENERATE_NODE(name, op, type)                                          \
  struct name : ASTTypeNode {                                                  \
    alloc::pmr_unique_ptr<type> left;                                          \
    std::pmr::vector<std::pair<op, type>> right;                               \
    name(alloc::pmr_unique_ptr<type>&& left, const tkn::Position& position)    \
        : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),           \
          left{std::move(left)}, right{&alloc::mr} {}                          \
  };

GENERATE_NODE(MultiplicationNode,
              type_tuple_to_variant_t<tkn::HighPriorityArithmeticOperatorTuple>,
              CastNode);
GENERATE_NODE(AdditionNode,
              type_tuple_to_variant_t<tkn::LowPriorityArithmeticOperatorTuple>,
              MultiplicationNode);
GENERATE_NODE(ShiftNode, type_tuple_to_variant_t<tkn::ShiftOperatorTuple>,
              AdditionNode);
GENERATE_NODE(BitwiseAndNode, tkn::BitwiseAnd, ShiftNode);
GENERATE_NODE(BitwiseXorNode, tkn::BitwiseXor, BitwiseAndNode);
GENERATE_NODE(BitwiseOrNode, tkn::BitwiseOr, BitwiseXorNode);
GENERATE_NODE(ComparisonNode,
              type_tuple_to_variant_t<tkn::ComparisonOperatorTuple>,
              BitwiseOrNode);
GENERATE_NODE(LogicalAndNode, tkn::LogicalAnd, ComparisonNode);
GENERATE_NODE(LogicalOrNode, tkn::LogicalOr, LogicalAndNode);

using ClosedOperatorNodeTuple =
    TypeTuple<BitwiseOrNode, BitwiseXorNode, BitwiseAndNode, ShiftNode,
              AdditionNode, MultiplicationNode>;

using BinaryNodeTuple = TypeTuple<LogicalAndNode, LogicalOrNode, ComparisonNode,
                                  BitwiseAndNode, BitwiseOrNode, BitwiseXorNode,
                                  ShiftNode, AdditionNode, MultiplicationNode>;

using LogicalBinaryNodeTuple = TypeTuple<LogicalAndNode, LogicalOrNode>;

using BitwiseBinaryNodeTuple =
    TypeTuple<BitwiseAndNode, BitwiseOrNode, BitwiseXorNode>;

using ArithmeticBinaryNodeTuple = TypeTuple<AdditionNode, MultiplicationNode>;

struct AssignmentNode : ASTTypeNode {
  alloc::pmr_unique_ptr<IdentifierNode> left;
  alloc::pmr_unique_ptr<ExpressionNode> right;

  template <typename T1, typename T2>
  AssignmentNode(T1&& left, T2&& right, const tkn::Position& position)
      : ASTTypeNode(position, SymbolTableInfo{}, TypeIdWrapper{}),
        left{std::forward<T1>(left)}, right{std::forward<T2>(right)} {}
};

struct StatementNode;

struct BlockExpressionNode : ASTTypeNode {
  std::pmr::vector<StatementNode> statements;
  std::optional<alloc::pmr_unique_ptr<ExpressionNode>> value;

  BlockExpressionNode(const tkn::Position& position);
};

struct LoopExpressionNode : ASTTypeNode {
  std::optional<tkn::Label> label;
  std::pmr::vector<StatementNode> body;

  LoopExpressionNode(std::optional<tkn::Label>&& label,
                     const tkn::Position& position);
};

struct ExpressionStatements;

struct IfExpressionNode : ASTTypeNode {
  alloc::pmr_unique_ptr<ExpressionNode> condition;
  alloc::pmr_unique_ptr<BlockExpressionNode> body;
  std::pmr::vector<ExpressionStatements> elif_bodies;
  std::optional<alloc::pmr_unique_ptr<BlockExpressionNode>> else_body;

  IfExpressionNode(alloc::pmr_unique_ptr<ExpressionNode>&& condition,
                   alloc::pmr_unique_ptr<BlockExpressionNode>&& body,
                   const tkn::Position& position);
};

using ExpressionNodeTuple =
    TypeTuple<LogicalOrNode, AssignmentNode, IfExpressionNode,
              BlockExpressionNode, LoopExpressionNode>;

using ExpressionNodeVariant = type_tuple_to_variant_t<ExpressionNodeTuple>;

struct ExpressionNode : ASTTypeNode {
  alloc::pmr_unique_ptr<ExpressionNodeVariant> node;

  ExpressionNode(ExpressionNodeVariant&& node, const tkn::Position& position);
};

struct ExpressionStatements {
  alloc::pmr_unique_ptr<ExpressionNode> expr;
  alloc::pmr_unique_ptr<BlockExpressionNode> block;

  ExpressionStatements(alloc::pmr_unique_ptr<ExpressionNode>&& expr,
                       alloc::pmr_unique_ptr<BlockExpressionNode>&& block);
};

struct VariableDefinitionNode : ASTNode {
  alloc::pmr_unique_ptr<IdentifierNode> name;
  std::optional<alloc::pmr_unique_ptr<IdentifierNode>> type;
  std::optional<alloc::pmr_unique_ptr<ExpressionNode>> value;
  bool is_global;

  VariableDefinitionNode(alloc::pmr_unique_ptr<IdentifierNode>&& name,
                         const tkn::Position& position);
};

struct BreakStatementNode : ASTNode {
  std::optional<tkn::Label> label;
  std::optional<alloc::pmr_unique_ptr<ExpressionNode>> value;

  BreakStatementNode(const tkn::Position& position);
};

struct ContinueStatementNode : ASTNode {
  std::optional<tkn::Label> label;

  ContinueStatementNode(const tkn::Position& position);
};

struct ReturnStatementNode : ASTNode {
  std::optional<alloc::pmr_unique_ptr<ExpressionNode>> value;

  ReturnStatementNode(const tkn::Position& position);
};

using InterruptNodeTuple =
    TypeTuple<BreakStatementNode, ContinueStatementNode, ReturnStatementNode>;

template <typename T>
concept InterruptNode = is_in_type_tuple_v<T, InterruptNodeTuple>;

static_assert(type_tuple_index_v<BreakStatementNode, InterruptNodeTuple> == 0);
static_assert(type_tuple_index_v<ContinueStatementNode, InterruptNodeTuple> ==
              1);
static_assert(type_tuple_index_v<ReturnStatementNode, InterruptNodeTuple> == 2);

using InterruptNodeVariant = type_tuple_to_variant_t<InterruptNodeTuple>;

struct FunctionDefinitionNode;

using StatementNodeTuple =
    TypeTuple<ExpressionNode, VariableDefinitionNode, FunctionDefinitionNode,
              BlockExpressionNode, IfExpressionNode, LoopExpressionNode,
              BreakStatementNode, ContinueStatementNode, ReturnStatementNode>;

static_assert(is_subset_of_v<InterruptNodeTuple, StatementNodeTuple>);

using StatementNodeVariant = type_tuple_to_variant_t<StatementNodeTuple>;

struct StatementNode : ASTNode {
  alloc::pmr_unique_ptr<StatementNodeVariant> node;

  StatementNode(StatementNodeVariant&& node, const tkn::Position& position);
};

struct FunctionDefinitionNode : ASTNode {
  alloc::pmr_unique_ptr<IdentifierNode> name;
  // first is name, second is type
  std::pmr::vector<std::pair<IdentifierNode, IdentifierNode>> argument_list;
  alloc::pmr_unique_ptr<IdentifierNode> return_type;
  alloc::pmr_unique_ptr<BlockExpressionNode> body;

  FunctionDefinitionNode(alloc::pmr_unique_ptr<IdentifierNode>&& name,
                         const tkn::Position& position);
};

using DefinitionNodeTuple =
    TypeTuple<FunctionDefinitionNode, VariableDefinitionNode>;

using DefinitionNodeVariant = type_tuple_to_variant_t<DefinitionNodeTuple>;

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
CREATE_NODE_NAME(BlockExpressionNode, "block")
CREATE_NODE_NAME(LoopExpressionNode, "loop")
CREATE_NODE_NAME(LogicalOrNode, "logical or")
CREATE_NODE_NAME(LogicalAndNode, "logical and")
CREATE_NODE_NAME(ShiftNode, "shift")
CREATE_NODE_NAME(BitwiseAndNode, "bitwise and")
CREATE_NODE_NAME(BitwiseXorNode, "bitwise xor")
CREATE_NODE_NAME(BitwiseOrNode, "bitwise or")
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
