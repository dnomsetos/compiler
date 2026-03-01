#pragma once

#include <string_view>
#include <vector>

#include "token.hpp"

static const std::pair<const char*, tkn::token_variant_t> keyword_table[] = {
    {"fn", tkn::Fn{}},
    {"var", tkn::Var{}},
    {"if", tkn::If{}},
    {"else", tkn::Else{}},
};

static const std::pair<const char*, tkn::token_variant_t>
    language_symbols_table[] = {
        {"->", tkn::Arrow{}},      {"!=", tkn::NotEqual{}},
        {"==", tkn::Equal{}},      {"+", tkn::Plus{}},
        {"-", tkn::Minus{}},       {"*", tkn::Multiply{}},
        {"/", tkn::Divide{}},      {"%", tkn::Mod{}},
        {"&", tkn::And{}},         {"|", tkn::Or{}},
        {"^", tkn::Xor{}},         {";", tkn::Semicolon{}},
        {":", tkn::Colon{}},       {"(", tkn::LeftParent{}},
        {")", tkn::RightParent{}}, {"{", tkn::LeftBrace{}},
        {"}", tkn::RightBrace{}},
};

auto tokenize(const std::string& code) -> std::vector<tkn::TokenInfo>;
