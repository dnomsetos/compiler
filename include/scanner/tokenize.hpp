#pragma once

#include <deque>

#include <scanner/token.hpp>

inline const std::pair<const char*, tkn::token_variant_t> keyword_table[] = {
    {"fn", tkn::Fn{}},         {"let", tkn::Let{}},
    {"const", tkn::Const{}},   {"static", tkn::Static{}},
    {"if", tkn::If{}},         {"else", tkn::Else{}},
    {"true", tkn::True{}},     {"false", tkn::False{}},
    {"break", tkn::Break{}},   {"continue", tkn::Continue{}},
    {"return", tkn::Return{}}, {"loop", tkn::Loop{}},
    {"as", tkn::As{}},         {"mut", tkn::Mut{}},
};

inline const std::pair<const char*, tkn::token_variant_t>
    language_symbols_table[] = {
        {"->", tkn::Arrow{}},      {">>", tkn::RightShift{}},
        {"<<", tkn::LeftShift{}},  {"&&", tkn::LogicalAnd{}},
        {"||", tkn::LogicalOr{}},  {"!=", tkn::NotEqual{}},
        {"==", tkn::Equal{}},      {">=", tkn::GreaterEqual{}},
        {"<=", tkn::LessEqual{}},  {">", tkn::Greater{}},
        {"<", tkn::Less{}},        {"+", tkn::Plus{}},
        {"-", tkn::Minus{}},       {"*", tkn::Asterisk{}},
        {"/", tkn::Divide{}},      {"%", tkn::Mod{}},
        {"&", tkn::Ampersand{}},   {"|", tkn::BitwiseOr{}},
        {"^", tkn::BitwiseXor{}},  {";", tkn::Semicolon{}},
        {":", tkn::Colon{}},       {"(", tkn::LeftParent{}},
        {")", tkn::RightParent{}}, {"{", tkn::LeftBrace{}},
        {"}", tkn::RightBrace{}},  {"=", tkn::Assignment{}},
        {"!", tkn::Not{}},         {",", tkn::Comma{}},
};

inline const std::pair<char, char> escape_table[] = {
    {'n', '\n'},  {'t', '\t'}, {'\\', '\\'}, {'"', '"'},
    {'0', '\0'},  {'r', '\r'}, {'f', '\f'},  {'b', '\b'},
    {'\'', '\''}, {'a', '\a'}, {'v', '\v'},
};

struct DummyConverter {};

struct StringLiteralConverter {};

auto tokenize(const std::string& code) -> std::deque<tkn::TokenInfo>;
