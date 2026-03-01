#include "scanner/tokenize.hpp"
#include <gtest/gtest.h>

using namespace tkn;

static void expect_tokens_eq(const std::vector<TokenInfo>& actual,
                             const std::vector<TokenInfo>& expected) {
  ASSERT_EQ(actual.size(), expected.size()) << "token count mismatch";
  for (size_t i = 0; i < actual.size(); ++i) {
    EXPECT_EQ(actual[i].token_variant, expected[i].token_variant)
        << "mismatch at token index " << i;
    EXPECT_EQ(actual[i].position, expected[i].position)
        << "mismatch at token index " << i;
  }
}

TEST(TokenizerMoreTests, IntegerLiteralSingle) {
  std::string code = "123";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{TokenInfo{
      IntLiteral{.value = 123}, Position{.line = 1, .offset = 1, .size = 3}}};

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, LargeInteger) {
  std::string code = "4294967296";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 4294967296ULL},
                Position{.line = 1, .offset = 1, .size = 10}}};

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, IdentifierAndKeyword) {
  std::string code = "fn main";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{Fn{}, Position{.line = 1, .offset = 1, .size = 2}},
      TokenInfo{Identifier{.name = "main"},
                Position{.line = 1, .offset = 4, .size = 4}}};

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, MultipleSymbolsMultiChar) {
  std::string code = "-> != == + - * / % & | ^ ; : ( ) { }";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{Arrow{}, Position{.line = 1, .offset = 1, .size = 2}},
      TokenInfo{NotEqual{}, Position{.line = 1, .offset = 4, .size = 2}},
      TokenInfo{Equal{}, Position{.line = 1, .offset = 7, .size = 2}},
      TokenInfo{Plus{}, Position{.line = 1, .offset = 10, .size = 1}},
      TokenInfo{Minus{}, Position{.line = 1, .offset = 12, .size = 1}},
      TokenInfo{Multiply{}, Position{.line = 1, .offset = 14, .size = 1}},
      TokenInfo{Divide{}, Position{.line = 1, .offset = 16, .size = 1}},
      TokenInfo{Mod{}, Position{.line = 1, .offset = 18, .size = 1}},
      TokenInfo{And{}, Position{.line = 1, .offset = 20, .size = 1}},
      TokenInfo{Or{}, Position{.line = 1, .offset = 22, .size = 1}},
      TokenInfo{Xor{}, Position{.line = 1, .offset = 24, .size = 1}},
      TokenInfo{Semicolon{}, Position{.line = 1, .offset = 26, .size = 1}},
      TokenInfo{Colon{}, Position{.line = 1, .offset = 28, .size = 1}},
      TokenInfo{LeftParent{}, Position{.line = 1, .offset = 30, .size = 1}},
      TokenInfo{RightParent{}, Position{.line = 1, .offset = 32, .size = 1}},
      TokenInfo{LeftBrace{}, Position{.line = 1, .offset = 34, .size = 1}},
      TokenInfo{RightBrace{}, Position{.line = 1, .offset = 36, .size = 1}},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, PositionTrackingNewlines) {
  std::string code = "1\nx";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 1},
                Position{.line = 1, .offset = 1, .size = 1}},
      TokenInfo{Identifier{.name = "x"},
                Position{.line = 2, .offset = 1, .size = 1}},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, OffsetCountsSpacesAfterNewline) {
  std::string code = "\n 42";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 42},
                Position{.line = 2, .offset = 2, .size = 2}},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, UnknownCharacterThrows) {
  std::string code = "$";
  EXPECT_THROW(tokenize(code), std::runtime_error);
}

TEST(TokenizerMoreTests, IdentifierWithUnderscore) {
  std::string code = "var_name123";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{Identifier{.name = "var_name123"},
                Position{.line = 1, .offset = 1, .size = 11}},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, KeywordsVarIfElse) {
  std::string code = "var if else";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{Var{}, Position{.line = 1, .offset = 1, .size = 3}},
      TokenInfo{If{}, Position{.line = 1, .offset = 5, .size = 2}},
      TokenInfo{Else{}, Position{.line = 1, .offset = 8, .size = 4}},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, FloatLiteralSimple) {
  std::string code = "12.34";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{FloatLiteral{.value = 12.34},
                Position{.line = 1, .offset = 1, .size = 5}},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, FloatAndIntSequence) {
  std::string code = "1 2.5 3";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 1},
                Position{.line = 1, .offset = 1, .size = 1}},
      TokenInfo{FloatLiteral{.value = 2.5},
                Position{.line = 1, .offset = 3, .size = 3}},
      TokenInfo{IntLiteral{.value = 3},
                Position{.line = 1, .offset = 7, .size = 1}},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, FloatMultipleDotsThrows) {
  std::string code = "1.2.3";
  EXPECT_THROW(tokenize(code), std::runtime_error);
}

TEST(TokenizerMoreTests, MixedNoSpaces) {
  std::string code = "a+b==c";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{Identifier{.name = "a"},
                Position{.line = 1, .offset = 1, .size = 1}},
      TokenInfo{Plus{}, Position{.line = 1, .offset = 2, .size = 1}},
      TokenInfo{Identifier{.name = "b"},
                Position{.line = 1, .offset = 3, .size = 1}},
      TokenInfo{Equal{}, Position{.line = 1, .offset = 4, .size = 2}},
      TokenInfo{Identifier{.name = "c"},
                Position{.line = 1, .offset = 6, .size = 1}},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, SmallSnippetCombinedTokens) {
  std::string code = "fn add -> var;";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{Fn{}, Position{.line = 1, .offset = 1, .size = 2}},
      TokenInfo{Identifier{.name = "add"},
                Position{.line = 1, .offset = 4, .size = 3}},
      TokenInfo{Arrow{}, Position{.line = 1, .offset = 8, .size = 2}},
      TokenInfo{Var{}, Position{.line = 1, .offset = 11, .size = 3}},
      TokenInfo{Semicolon{}, Position{.line = 1, .offset = 14, .size = 1}},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, SimpleString) {
  std::string code = R"("hello")";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = std::string("hello")},
                Position{.line = 1, .offset = 1, .size = code.size()}}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, EmptyString) {
  std::string code = R"("")";
  auto tokens = tokenize(code);

  std::vector<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = std::string("")},
                Position{.line = 1, .offset = 1, .size = code.size()}}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, StringWithSpacesAndSymbols) {
  std::string code = R"(" a b !@# ")";
  auto tokens = tokenize(code);

  std::string expected_value = " a b !@# ";

  std::vector<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = expected_value},
                Position{.line = 1, .offset = 1, .size = code.size()}}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, MultipleStringTokensSeparatedBySpace) {
  std::string code = R"("one" "two")";
  auto tokens = tokenize(code);

  std::size_t first_size = 5;
  std::size_t second_size = 5;

  std::vector<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = std::string("one")},
                Position{.line = 1, .offset = 1, .size = first_size}},
      TokenInfo{StringLiteral{.value = std::string("two")},
                Position{.line = 1,
                         .offset = static_cast<std::size_t>(1 + first_size + 1),
                         .size = second_size}},
  };

  expect_tokens_eq(tokens, expected);
}
