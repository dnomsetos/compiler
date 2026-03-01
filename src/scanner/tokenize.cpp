#include "scanner/tokenize.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>

bool identifier_filter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '_';
}

bool string_literal_filter(char c) { return c != '"'; }

std::pair<std::string, std::size_t>
read_string(const std::string& code, std::size_t i,
            std::function<bool(char)> filter) {
  std::string name;
  while (i < code.size() && filter(code[i])) {
    name.push_back(code[i]);
    ++i;
  }
  --i;
  return {name, i};
}

std::pair<std::uint64_t, std::size_t> read_number(const std::string& code,
                                                  std::size_t i) {
  std::uint64_t value = 0;
  while (i < code.size() && code[i] >= '0' && code[i] <= '9') {
    value *= 10;
    value += code[i] - '0';
    ++i;
  }
  --i;
  return {value, i};
}

auto tokenize(const std::string& code) -> std::vector<tkn::TokenInfo> {
  std::vector<tkn::TokenInfo> tokens;
  std::size_t current_line = 1;
  std::size_t current_offset = 0;
  for (std::size_t i = 0; i < code.size(); ++i) {
    char c = code[i];
    ++current_offset;
    if (c == '\n') {
      ++current_line;
      current_offset = 0;
      continue;
    } else if (std::isspace(c)) {
      continue;
    }
    if (c >= '0' && c <= '9') {
      auto [value, new_i] = read_number(code, i);
      if (new_i + 1 < code.size() && code[new_i + 1] == '.') {
        auto [value2, new_i2] = read_number(code, new_i + 2);
        tokens.emplace_back(
            tkn::FloatLiteral{
                .value = value + value2 / std::pow(10, new_i2 - new_i - 1),
            },
            tkn::Position{.line = current_line,
                          .offset = current_offset,
                          .size = new_i2 + 1 - i});
        current_offset += new_i2 - i;
        i = new_i2;
        continue;
      }
      tokens.emplace_back(
          tkn::IntLiteral{
              .value = value,
          },
          tkn::Position{.line = current_line,
                        .offset = current_offset,
                        .size = new_i + 1 - i});
      current_offset += new_i - i;
      i = new_i;
    } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
      auto [name, new_i] = read_string(code, i, identifier_filter);
      auto it = std::find_if(
          std::begin(keyword_table), std::end(keyword_table),
          [p = code.c_str() + i, size = name.size()](const auto& pair) {
            return size == std::strlen(pair.first) && pair.first[0] == p[0];
          });
      if (it != std::end(keyword_table)) {
        tokens.emplace_back(it->second, tkn::Position{.line = current_line,
                                                      .offset = current_offset,
                                                      .size = new_i + 1 - i});
        current_offset += new_i - i;
        i = new_i;
        continue;
      }
      tokens.emplace_back(
          tkn::Identifier{
              .name = name,
          },
          tkn::Position{.line = current_line,
                        .offset = current_offset,
                        .size = new_i + 1 - i});
      current_offset += new_i - i;
      i = new_i;
    } else if (c == '"') {
      auto [name, new_i] = read_string(code, i + 1, string_literal_filter);
      tokens.emplace_back(
          tkn::StringLiteral{
              .value = name,
          },
          tkn::Position{.line = current_line,
                        .offset = current_offset,
                        .size = new_i + 2 - i});
      current_offset += new_i + 1 - i;
      i = new_i + 1;
    } else {
      auto it = std::find_if(std::begin(language_symbols_table),
                             std::end(language_symbols_table),
                             [p = code.c_str() + i](const auto& pair) {
                               if (pair.first[0] == p[0]) {
                                 if (pair.first[1] == '\0') {
                                   return true;
                                 } else {
                                   return pair.first[1] == p[1];
                                 }
                               }
                               return false;
                             });
      if (it == std::end(language_symbols_table)) {
        std::cerr << "Unexpected character: " << code[i] << " at line "
                  << current_line << ", offset " << current_offset << "."
                  << std::endl;
        throw std::runtime_error("Unexpected character");
      }
      std::size_t token_size = std::strlen(it->first);
      tokens.emplace_back(it->second, tkn::Position{.line = current_line,
                                                    .offset = current_offset,
                                                    .size = token_size});
      i += token_size - 1;
      current_offset += token_size - 1;
    }
  }
  return tokens;
}
