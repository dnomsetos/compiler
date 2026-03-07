#include "scanner/token.hpp"
#include "scanner/tokenize.hpp"
#include <gtest/gtest.h>

using namespace tkn;

static void expect_tokens_eq(const std::deque<TokenInfo>& actual,
                             const std::deque<TokenInfo>& expected) {
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

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 123}, Position(1, 1, 3)},
      TokenInfo{EOFToken{}, Position(1, 4, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, LargeInteger) {
  std::string code = "4294967296";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 4294967296ULL}, Position(1, 1, 10)},
      TokenInfo{EOFToken{}, Position(1, 11, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, IdentifierAndKeyword) {
  std::string code = "fn main";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Fn{}, Position(1, 1, 2)},
      TokenInfo{Identifier{.name = "main"}, Position(1, 4, 4)},
      TokenInfo{EOFToken{}, Position(1, 8, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, MultipleSymbolsMultiChar) {
  std::string code = "-> != == + - * / % & | ^ ; : ( ) { }";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Arrow{}, Position(1, 1, 2)},
      TokenInfo{NotEqual{}, Position(1, 4, 2)},
      TokenInfo{Equal{}, Position(1, 7, 2)},
      TokenInfo{Plus{}, Position(1, 10, 1)},
      TokenInfo{Minus{}, Position(1, 12, 1)},
      TokenInfo{Multiply{}, Position(1, 14, 1)},
      TokenInfo{Divide{}, Position(1, 16, 1)},
      TokenInfo{Mod{}, Position(1, 18, 1)},
      TokenInfo{And{}, Position(1, 20, 1)},
      TokenInfo{Or{}, Position(1, 22, 1)},
      TokenInfo{Xor{}, Position(1, 24, 1)},
      TokenInfo{Semicolon{}, Position(1, 26, 1)},
      TokenInfo{Colon{}, Position(1, 28, 1)},
      TokenInfo{LeftParent{}, Position(1, 30, 1)},
      TokenInfo{RightParent{}, Position(1, 32, 1)},
      TokenInfo{LeftBrace{}, Position(1, 34, 1)},
      TokenInfo{RightBrace{}, Position(1, 36, 1)},
      TokenInfo{EOFToken{}, Position(1, 37, 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, PositionTrackingNewlines) {
  std::string code = "1\nx";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 1}, Position(1, 1, 1)},
      TokenInfo{Identifier{.name = "x"}, Position(2, 1, 1)},
      TokenInfo{EOFToken{}, Position(2, 2, 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, OffsetCountsSpacesAfterNewline) {
  std::string code = "\n 42";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 42}, Position(2, 2, 2)},
      TokenInfo{EOFToken{}, Position(2, 4, 0)},
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

  std::deque<TokenInfo> expected{
      TokenInfo{Identifier{.name = "var_name123"}, Position(1, 1, 11)},
      TokenInfo{EOFToken{}, Position(1, 12, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, KeywordsVarIfElse) {
  std::string code = "var if else";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Var{}, Position(1, 1, 3)},
      TokenInfo{If{}, Position(1, 5, 2)},
      TokenInfo{Else{}, Position(1, 8, 4)},
      TokenInfo{EOFToken{}, Position(1, 12, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, FloatLiteralSimple) {
  std::string code = "12.34";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{FloatLiteral{.value = 12.34}, Position(1, 1, 5)},
      TokenInfo{EOFToken{}, Position(1, 6, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, FloatAndIntSequence) {
  std::string code = "1 2.5 3";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 1}, Position(1, 1, 1)},
      TokenInfo{FloatLiteral{.value = 2.5}, Position(1, 3, 3)},
      TokenInfo{IntLiteral{.value = 3}, Position(1, 7, 1)},
      TokenInfo{EOFToken{}, Position(1, 8, 0)},
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

  std::deque<TokenInfo> expected{
      TokenInfo{Identifier{.name = "a"}, Position(1, 1, 1)},
      TokenInfo{Plus{}, Position(1, 2, 1)},
      TokenInfo{Identifier{.name = "b"}, Position(1, 3, 1)},
      TokenInfo{Equal{}, Position(1, 4, 2)},
      TokenInfo{Identifier{.name = "c"}, Position(1, 6, 1)},
      TokenInfo{EOFToken{}, Position(1, 7, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(TokenizerMoreTests, SmallSnippetCombinedTokens) {
  std::string code = "fn add -> var;";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Fn{}, Position(1, 1, 2)},
      TokenInfo{Identifier{.name = "add"}, Position(1, 4, 3)},
      TokenInfo{Arrow{}, Position(1, 8, 2)},
      TokenInfo{Var{}, Position(1, 11, 3)},
      TokenInfo{Semicolon{}, Position(1, 14, 1)},
      TokenInfo{EOFToken{}, Position(1, 15, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, SimpleString) {
  std::string code = R"("hello")";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = std::string("hello")},
                Position(1, 1, code.size())},
      TokenInfo{EOFToken{}, Position(1, 8, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, EmptyString) {
  std::string code = R"("")";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = std::string("")},
                Position(1, 1, code.size())},
      TokenInfo{EOFToken{}, Position(1, 3, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, StringWithSpacesAndSymbols) {
  std::string code = R"(" a b !@# ")";
  auto tokens = tokenize(code);

  std::string expected_value = " a b !@# ";

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = expected_value},
                Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, MultipleStringTokensSeparatedBySpace) {
  std::string code = R"("one" "two")";
  auto tokens = tokenize(code);

  std::size_t first_size = 5;
  std::size_t second_size = 5;

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = std::string("one")},
                Position(1, 1, first_size)},
      TokenInfo{StringLiteral{.value = std::string("two")},
                Position(1, static_cast<std::size_t>(1 + first_size + 1),
                         second_size)},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, EscapedCharacters) {
  std::string code = R"("a\"b\\c\n")";
  std::cout << "code: " << code << std::endl;
  std::string expected_value = "a\"b\\c\n";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = expected_value},
                Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, UnterminatedStringThrows) {
  std::string code = "\"hello";
  EXPECT_THROW(tokenize(code), std::runtime_error);
}

TEST(StringLiteralTests, EscapedQuoteAtEnd) {
  std::string code = "\"say \\\"hi\\\"\"";
  std::string expected_value = "say \"hi\"";

  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = expected_value},
                Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(StringLiteralTests, StringContainingEscapedBackslashAndQuote) {
  std::string code = "\"path\\\\to\\\\\\\"file\\\"\"";
  std::string expected_value = "path\\to\\\"file\"";

  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = expected_value},
                Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)}};

  expect_tokens_eq(tokens, expected);
}
