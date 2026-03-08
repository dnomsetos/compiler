#pragma once

#include <unordered_map>

#include <parser/ast.hpp>

struct Dummy {};

using calc_result_t =
    std::variant<std::int64_t, double, bool, std::string, Dummy>;

inline const std::pair<std::string, calc_result_t> default_value_table[] = {
    {"int", 0},
    {"float", 0.0},
    {"bool", false},
    {"str", ""},
};

calc_result_t
execute_program(const ast::Program& program,
                std::unordered_map<std::string, calc_result_t>& variables);

void execute_statement(
    const ast::StatementNode& statement,
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
