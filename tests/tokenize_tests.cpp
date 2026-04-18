#include "scanner/token.hpp"
#include <gtest/gtest.h>
#include <scanner/tokenize.hpp>
#include <string>

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

TEST(SimpleTokenizerTests, IdentifierAndKeyword) {
  std::string code = "fn main";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Fn{}, Position(1, 1, 2)},
      TokenInfo{Identifier{.name = "main"}, Position(1, 4, 4)},
      TokenInfo{EOFToken{}, Position(1, 8, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(SimpleTokenizerTests, LabelsAndCharLiterals) {
  std::string code = "'a' 'aboba 'b' 'bebra";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{CharLiteral{.value = 'a'}, Position(1, 1, 3)},
      TokenInfo{Label{.name = "aboba"}, Position(1, 5, 6)},
      TokenInfo{CharLiteral{.value = 'b'}, Position(1, 12, 3)},
      TokenInfo{Label{.name = "bebra"}, Position(1, 16, 6)},
      TokenInfo{EOFToken{}, Position(1, 22, 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(SimpleTokenizerTests, MultipleSymbolsMultiChar) {
  std::string code = "-> != == + - * / % & | ^ ; : ( ) { } << >> && ||";
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
      TokenInfo{BitwiseAnd{}, Position(1, 20, 1)},
      TokenInfo{BitwiseOr{}, Position(1, 22, 1)},
      TokenInfo{BitwiseXor{}, Position(1, 24, 1)},
      TokenInfo{Semicolon{}, Position(1, 26, 1)},
      TokenInfo{Colon{}, Position(1, 28, 1)},
      TokenInfo{LeftParent{}, Position(1, 30, 1)},
      TokenInfo{RightParent{}, Position(1, 32, 1)},
      TokenInfo{LeftBrace{}, Position(1, 34, 1)},
      TokenInfo{RightBrace{}, Position(1, 36, 1)},
      TokenInfo{LeftShift{}, Position(1, 38, 2)},
      TokenInfo{RightShift{}, Position(1, 41, 2)},
      TokenInfo{LogicalAnd{}, Position(1, 44, 2)},
      TokenInfo{LogicalOr{}, Position(1, 47, 2)},
      TokenInfo{EOFToken{}, Position(1, 49, 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(SimpleTokenizerTests, PositionTrackingNewlines) {
  std::string code = "1\nx";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 1}, Position(1, 1, 1)},
      TokenInfo{Identifier{.name = "x"}, Position(2, 1, 1)},
      TokenInfo{EOFToken{}, Position(2, 2, 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(SimpleTokenizerTests, UnknownCharacterThrows) {
  std::string code = "$";
  EXPECT_THROW(tokenize(code), std::runtime_error);
}

TEST(SimpleTokenizerTests, IdentifierWithUnderscore) {
  std::string code = "var_name123";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Identifier{.name = "var_name123"}, Position(1, 1, 11)},
      TokenInfo{EOFToken{}, Position(1, 12, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(SimpleTokenizerTests, KeywordsVarIfElse) {
  std::string code = "let if else";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Let{}, Position(1, 1, 3)},
      TokenInfo{If{}, Position(1, 5, 2)},
      TokenInfo{Else{}, Position(1, 8, 4)},
      TokenInfo{EOFToken{}, Position(1, 12, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(SimpleTokenizerTests, MixedNoSpaces) {
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

TEST(SimpleTokenizerTests, SmallSnippetCombinedTokens) {
  std::string code = "fn add -> let;";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Fn{}, Position(1, 1, 2)},
      TokenInfo{Identifier{.name = "add"}, Position(1, 4, 3)},
      TokenInfo{Arrow{}, Position(1, 8, 2)},
      TokenInfo{Let{}, Position(1, 11, 3)},
      TokenInfo{Semicolon{}, Position(1, 14, 1)},
      TokenInfo{EOFToken{}, Position(1, 15, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, OffsetCountsSpacesAfterNewline) {
  std::string code = "\n 42";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 42}, Position(2, 2, 2)},
      TokenInfo{EOFToken{}, Position(2, 4, 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, IntegerLiteralSingle) {
  std::string code = "123";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 123}, Position(1, 1, 3)},
      TokenInfo{EOFToken{}, Position(1, 4, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, LargeInteger) {
  std::string code = "4294967296";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 4294967296ULL}, Position(1, 1, 10)},
      TokenInfo{EOFToken{}, Position(1, 11, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, FloatAndIntSequence) {
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

TEST(LiteralTests, FloatMultipleDotsThrows) {
  std::string code = "1.2.3";
  EXPECT_THROW(tokenize(code), std::runtime_error);
}

TEST(LiteralTests, FloatLiteralSimple) {
  std::string code = "12.34";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{FloatLiteral{.value = 12.34}, Position(1, 1, 5)},
      TokenInfo{EOFToken{}, Position(1, 6, 0)},
  };
  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, SimpleString) {
  // code:"hello"
  std::string code = "\"hello\"";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{StringLiteral{.value = std::string("hello")},
                Position(1, 1, code.size())},
      TokenInfo{EOFToken{}, Position(1, 8, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, EmptyString) {
  // code:""
  std::string code = "\"\"";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{
          StringLiteral{.value = std::string("")}, Position(1, 1, code.size())},
      TokenInfo{EOFToken{}, Position(1, 3, 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, StringWithSpacesAndSymbols) {
  // code:" a b !@# "
  std::string code = "\" a b !@# \"";
  auto tokens = tokenize(code);

  std::string expected_value = " a b !@# ";

  std::deque<TokenInfo> expected{
      TokenInfo{
          StringLiteral{.value = expected_value}, Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, MultipleStringTokensSeparatedBySpace) {
  // code:"one" "two"
  std::string code = "\"one\" \"two\"";
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

TEST(LiteralTests, EscapedCharacters) {
  // code:"a\"b\\c\n"
  std::string code = "\"a\\\"b\\\\c\\n\"";
  std::string expected_value = "a\"b\\c\n";
  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{
          StringLiteral{.value = expected_value}, Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, UnterminatedStringThrows) {
  // code:"hello
  std::string code = "\"hello";
  EXPECT_THROW(tokenize(code), std::runtime_error);
}

TEST(LiteralTests, EscapedQuoteAtEnd) {
  // code:"say \"hi\""
  std::string code = "\"say \\\"hi\\\"\"";
  std::string expected_value = "say \"hi\"";

  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{
          StringLiteral{.value = expected_value}, Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, StringContainingEscapedBackslashAndQuote) {
  // code:"path\\to\\\"file\""
  std::string code = "\"path\\\\to\\\\\\\"file\\\"\"";
  std::string expected_value = "path\\to\\\"file\"";

  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{
          StringLiteral{.value = expected_value}, Position(1, 1, code.size())},
      TokenInfo{EOFToken{},
                Position(1, static_cast<std::size_t>(code.size() + 1), 0)}};

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, CharLiteralSimple) {
  // code:'a', '\n', '\\'
  std::string code = "\'a\', \'\\n\', \'\\\\\'";

  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{CharLiteral{.value = 'a'}, Position(1, 1, 3)},
      TokenInfo{Comma{}, Position(1, 4, 1)},
      TokenInfo{CharLiteral{.value = '\n'}, Position(1, 6, 4)},
      TokenInfo{Comma{}, Position(1, 10, 1)},
      TokenInfo{CharLiteral{.value = '\\'}, Position(1, 12, 4)},
      TokenInfo{EOFToken{}, Position(1, 16, 0)},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, BoolLiteralSimple) {
  // code:let a : bool = true;
  //      let b : bool = false;
  std::string code = "let a : bool = true;\nlet b : bool = false;";

  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{Let{}, Position(1, 1, 3)},
      TokenInfo{Identifier{.name = "a"}, Position(1, 5, 1)},
      TokenInfo{Colon{}, Position{1, 7, 1}},
      TokenInfo{Identifier{.name = "bool"}, Position(1, 9, 4)},
      TokenInfo{Assignment{}, Position{1, 14, 1}},
      TokenInfo{BoolLiteral{.value = true}, Position(1, 16, 4)},
      TokenInfo{Semicolon{}, Position(1, 20, 1)},
      TokenInfo{Let{}, Position(2, 1, 3)},
      TokenInfo{Identifier{.name = "b"}, Position(2, 5, 1)},
      TokenInfo{Colon{}, Position{2, 7, 1}},
      TokenInfo{Identifier{.name = "bool"}, Position(2, 9, 4)},
      TokenInfo{Assignment{}, Position{2, 14, 1}},
      TokenInfo{BoolLiteral{.value = false}, Position(2, 16, 5)},
      TokenInfo{Semicolon{}, Position(2, 21, 1)},
      TokenInfo{EOFToken{}, Position{2, 22, 0}},
  };

  expect_tokens_eq(tokens, expected);
}

TEST(LiteralTests, ComplexLiterals) {
  // code:42 3.14 "println!(\"Hello, world!\n\")" true '\\' false '\n'
  std::string code = "42 3.14 \"println!(\\\"Hello, world!\\n\\\")\" true "
                     "\'\\\\\' false \'\\n\'";

  auto tokens = tokenize(code);

  std::deque<TokenInfo> expected{
      TokenInfo{IntLiteral{.value = 42}, Position(1, 1, 2)},
      TokenInfo{FloatLiteral{.value = 3.14}, Position(1, 4, 4)},
      TokenInfo{StringLiteral{.value = "println!(\"Hello, world!\n\")"},
                Position(1, 9, 31)},
      TokenInfo{BoolLiteral{.value = true}, Position(1, 41, 4)},
      TokenInfo{CharLiteral{.value = '\\'}, Position(1, 46, 4)},
      TokenInfo{BoolLiteral{.value = false}, Position(1, 51, 5)},
      TokenInfo{CharLiteral{.value = '\n'}, Position(1, 57, 4)},
      TokenInfo{EOFToken{}, Position(1, 61, 0)},
  };

  expect_tokens_eq(tokens, expected);
}
