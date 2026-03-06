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

template <typename T> using pmr_unique_ptr = std::unique_ptr<T, PmrDeleter<T>>;

struct LiteralNode : Storage<tkn::Position> {
  pmr_unique_ptr<type_tuple_to_variant_t<tkn::LiteralTuple>> literal;

  template <typename T>
  LiteralNode(T&& literal, const tkn::Position& position,
              std::pmr::memory_resource* mr)
      : Storage<tkn::Position>(position),
        literal{make_unique_pmr<type_tuple_to_variant_t<tkn::LiteralTuple>>(
            mr, std::forward<T>(literal))} {}
};

struct IdentifierNode : Storage<tkn::Position> {
  pmr_unique_ptr<tkn::Identifier> identifier;

  IdentifierNode(const tkn::Identifier& identifier,
                 const tkn::Position& position, std::pmr::memory_resource* mr)
      : Storage<tkn::Position>(position),
        identifier{make_unique_pmr<tkn::Identifier>(mr, identifier)} {}
};

struct ExpressionNode;

struct FunctionCallNode;

using PrimaryNodeTuple =
    TypeTuple<LiteralNode, IdentifierNode, ExpressionNode, FunctionCallNode>;

struct PrimaryNode : Storage<tkn::Position, Type> {
  pmr_unique_ptr<type_tuple_to_variant_t<PrimaryNodeTuple>> primary;

  template <typename T>
  PrimaryNode(T&& primary, const tkn::Position& position,
              std::pmr::memory_resource* mr)
      : Storage<tkn::Position, Type>(position, Type{}),
        primary{make_unique_pmr<type_tuple_to_variant_t<PrimaryNodeTuple>>(
            mr, std::in_place_type_t<std::decay_t<decltype(primary)>>{},
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
    name(pmr_unique_ptr<type>&& left, const tkn::Position& position,           \
         std::pmr::memory_resource* mr)                                        \
        : Storage<tkn::Position, Type>(position, Type{}),                      \
          left{std::move(left)}, right{mr} {}                                  \
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

struct AssignmentNode : Storage<tkn::Position, Type> {
  pmr_unique_ptr<IdentifierNode> left;
  pmr_unique_ptr<ExpressionNode> right;

  template <typename T1, typename T2>
  AssignmentNode(T1&& left, T2&& right, const tkn::Position& position)
      : Storage<tkn::Position, Type>(position, Type{}),
        left{std::forward<T1>(left)}, right{std::forward<T2>(right)} {}
};

struct ExpressionNode : Storage<tkn::Position, Type> {
  pmr_unique_ptr<std::variant<OrNode, AssignmentNode>> node;

  template <typename T>
  ExpressionNode(T&& node, const tkn::Position& position,
                 std::pmr::memory_resource* mr)
      : Storage<tkn::Position, Type>(position, Type{}),
        node{make_unique_pmr<std::variant<OrNode, AssignmentNode>>(
            mr, std::forward<T>(node))} {}
};

struct FunctionCallNode : Storage<tkn::Position> {
  pmr_unique_ptr<IdentifierNode> name;
  std::pmr::vector<ExpressionNode> arguments;

  FunctionCallNode(pmr_unique_ptr<IdentifierNode>&& name,
                   const tkn::Position& position, std::pmr::memory_resource* mr)
      : Storage<tkn::Position>(position), name{std::move(name)}, arguments{mr} {
  }
};

struct IfStatementNode;

struct VariableDefinitionNode;

struct StatementNode : Storage<tkn::Position> {
  pmr_unique_ptr<
      std::variant<ExpressionNode, IfStatementNode, VariableDefinitionNode>>
      node;

  template <typename T>
  StatementNode(T&& node, const tkn::Position& position,
                std::pmr::memory_resource* mr)
      : Storage<tkn::Position>(position),
        node{make_unique_pmr<std::variant<ExpressionNode, IfStatementNode,
                                          VariableDefinitionNode>>(
            mr, std::forward<T>(node))} {}
};

struct ExpressionStatements {
  ExpressionNode expr;
  std::pmr::vector<StatementNode> statements;

  ExpressionStatements(ExpressionNode&& expr, std::pmr::memory_resource* mr)
      : expr{std::move(expr)}, statements{mr} {}
};

struct IfStatementNode : Storage<tkn::Position> {
  pmr_unique_ptr<ExpressionNode> condition;
  std::pmr::vector<StatementNode> body;
  std::pmr::vector<ExpressionStatements> elif_bodies;
  std::pmr::vector<StatementNode> else_body;

  template <typename T>
  IfStatementNode(T&& condition, const tkn::Position& position,
                  std::pmr::memory_resource* mr)
      : Storage<tkn::Position>(position), condition{std::forward<T>(condition)},
        body{mr}, elif_bodies{mr}, else_body{mr} {}
};

struct VariableDefinitionNode : Storage<tkn::Position> {
  pmr_unique_ptr<IdentifierNode> name;
  std::optional<pmr_unique_ptr<IdentifierNode>> type;
  std::optional<pmr_unique_ptr<ExpressionNode>> value;

  template <typename T>
  VariableDefinitionNode(T&& name, const tkn::Position& position)
      : Storage<tkn::Position>(position), name{std::forward<T>(name)} {}
};

struct FunctionDefinitionNode : Storage<tkn::Position> {
  pmr_unique_ptr<IdentifierNode> name;
  std::pmr::vector<std::pair<IdentifierNode, IdentifierNode>> argument_lits;
  pmr_unique_ptr<IdentifierNode> return_type;
  std::pmr::vector<StatementNode> body;
  pmr_unique_ptr<ExpressionNode> return_value;

  template <typename T>
  FunctionDefinitionNode(T&& name, const tkn::Position& position,
                         std::pmr::memory_resource* mr)
      : Storage<tkn::Position>(position), name{std::forward<T>(name)},
        argument_lits{mr}, body{mr} {}
};

using DefinitionTuple =
    TypeTuple<FunctionDefinitionNode, VariableDefinitionNode>;

struct Program {
  std::pmr::vector<type_tuple_to_variant_t<DefinitionTuple>> definitions;

  Program(std::pmr::memory_resource* mr) : definitions{mr} {}
};

} // namespace ast
