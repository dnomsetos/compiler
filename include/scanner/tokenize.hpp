#pragma once

#include <deque>

#include <scanner/token.hpp>

static const std::pair<const char*, tkn::token_variant_t> keyword_table[] = {
    {"fn", tkn::Fn{}},
    {"var", tkn::Var{}},
    {"if", tkn::If{}},
    {"else", tkn::Else{}},
};

static const std::pair<const char*, tkn::token_variant_t>
    language_symbols_table[] = {
        {"->", tkn::Arrow{}},      {"!=", tkn::NotEqual{}},
        {"==", tkn::Equal{}},      {">=", tkn::GreaterEqual{}},
        {"<=", tkn::LessEqual{}},  {">", tkn::Greater{}},
        {"<", tkn::Less{}},        {"+", tkn::Plus{}},
        {"-", tkn::Minus{}},       {"*", tkn::Multiply{}},
        {"/", tkn::Divide{}},      {"%", tkn::Mod{}},
        {"&", tkn::And{}},         {"|", tkn::Or{}},
        {"^", tkn::Xor{}},         {";", tkn::Semicolon{}},
        {":", tkn::Colon{}},       {"(", tkn::LeftParent{}},
        {")", tkn::RightParent{}}, {"{", tkn::LeftBrace{}},
        {"}", tkn::RightBrace{}},  {"=", tkn::Assignment{}},
        {"!", tkn::Not{}},
};

static const std::pair<char, char> escape_table[] = {
    {'n', '\n'},  {'t', '\t'}, {'\\', '\\'}, {'"', '"'},
    {'0', '\0'},  {'r', '\r'}, {'f', '\f'},  {'b', '\b'},
    {'\'', '\''}, {'a', '\a'}, {'v', '\v'},
};

struct DummyConverter {};

struct StringLiteralConverter {};

auto tokenize(const std::string& code) -> std::deque<tkn::TokenInfo>;
