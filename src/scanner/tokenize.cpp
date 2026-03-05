#include "scanner/tokenize.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

bool identifier_filter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '_';
}

bool string_literal_filter(char c) { return c != '"'; }

template <typename Converter>
auto read_string(const std::string& code, std::size_t i, bool (*filter)(char))
    -> std::pair<std::string, std::size_t> {
  std::string name;
  while (i < code.size() && filter(code[i])) {
    if constexpr (std::is_same_v<Converter, StringLiteralConverter>) {
      if (code[i] == '\\') {
        ++i;
        if (i < code.size()) {
          auto result =
              std::find_if(std::begin(escape_table), std::end(escape_table),
                           [f = code[i]](auto c) { return c.first == f; });
          if (result != std::end(escape_table)) {
            name.push_back(result->second);
            ++i;
            continue;
          } else {
            std::cerr << "Unknown escape sequence: \\" << code[i] << std::endl;
            throw std::runtime_error("Unknown escape sequence");
          }
        } else {
          std::cerr << "Unterminated string" << std::endl;
          throw std::runtime_error("Unterminated string");
        }
        continue;
      } else if (code[i] == '\n') {
        std::cerr << "Multiline string literals are not supported" << std::endl;
        throw std::runtime_error("Multiline string literals are not supported");
      }
    }
    name.push_back(code[i]);
    ++i;
  }
  return {name, i - 1};
}

auto read_number(const std::string& code, std::size_t i)
    -> std::pair<std::uint64_t, std::size_t> {
  std::uint64_t value = 0;
  while (i < code.size() && code[i] >= '0' && code[i] <= '9') {
    value *= 10;
    value += code[i] - '0';
    ++i;
  }
  return {value, i - 1};
}

auto tokenize(const std::string& code) -> std::deque<tkn::TokenInfo> {
  std::deque<tkn::TokenInfo> tokens;
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
      auto [name, new_i] =
          read_string<DummyConverter>(code, i, identifier_filter);
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
      auto [name, new_i] = read_string<StringLiteralConverter>(
          code, i + 1, string_literal_filter);
      if (new_i + 1 >= code.size() || code[new_i + 1] != '"') {
        std::cerr << "Unterminated string literal at line " << current_line
                  << ", offset " << current_offset << "." << std::endl;
        throw std::runtime_error("Unterminated string literal");
      }
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
  tokens.emplace_back(tkn::EOFToken{},
                      tkn::Position{.line = current_line,
                                    .offset = current_offset + 1,
                                    .size = 0});
  return tokens;
}
