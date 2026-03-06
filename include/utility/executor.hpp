#pragma once

#include <iostream>
#include <unordered_map>

#include <parser/ast.hpp>

struct Dummy {};

using calc_result =
    std::variant<std::int64_t, double, bool, std::string, Dummy>;

calc_result
execute_program(const ast::Program& program,
                std::unordered_map<std::string, calc_result>& variables);

void execute_statement(const ast::StatementNode& statement,
                       std::unordered_map<std::string, calc_result>& variables);
calc_result
execute_expression(const ast::ExpressionNode& expression,
                   std::unordered_map<std::string, calc_result>& variables);

calc_result execute_or(const ast::OrNode& expression,
                       std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_xor(const ast::XorNode& expression,
            std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_and(const ast::AndNode& expression,
            std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_equality(const ast::EqualityNode& expression,
                 std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_comparison(const ast::ComparisonNode& expression,
                   std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_multiplication(const ast::MultiplicationNode& expression,
                       std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_addition(const ast::AdditionNode& expression,
                 std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_unary(const ast::UnaryNode& expression,
              std::unordered_map<std::string, calc_result>& variables);

calc_result
execute_primary(const ast::PrimaryNode& expression,
                std::unordered_map<std::string, calc_result>& variables);
