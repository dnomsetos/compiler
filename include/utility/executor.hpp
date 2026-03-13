#pragma once

#include <unordered_map>

#include <parser/ast.hpp>

std::ostream& operator<<(std::ostream& out, const Dummy&);

template <typename T> struct IsVariant : std::false_type {};

template <typename... Ts>
struct IsVariant<std::variant<Ts...>> : std::true_type {};

template <typename T> constexpr bool is_variant_v = IsVariant<T>::value;

calc_result_t
execute_program(const ast::Program& program,
                std::unordered_map<std::string, calc_result_t>& variables);

void execute_statement(
    const ast::StatementNode& statement,
    std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t execute_block_expression(
    const ast::BlockExpressionNode& expression,
    std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t execute_if_expression(
    const ast::IfExpressionNode& expression,
    std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_expression(const ast::ExpressionNode& expression,
                   std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_or(const ast::OrNode& expression,
           std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_xor(const ast::XorNode& expression,
            std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_and(const ast::AndNode& expression,
            std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_equality(const ast::EqualityNode& expression,
                 std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_comparison(const ast::ComparisonNode& expression,
                   std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t execute_multiplication(
    const ast::MultiplicationNode& expression,
    std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_addition(const ast::AdditionNode& expression,
                 std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_unary(const ast::UnaryNode& expression,
              std::unordered_map<std::string, calc_result_t>& variables);

calc_result_t
execute_primary(const ast::PrimaryNode& expression,
                std::unordered_map<std::string, calc_result_t>& variables);
