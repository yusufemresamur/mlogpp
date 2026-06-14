#include "mlogpp/format/json_escape.hpp"
#include <gtest/gtest.h>
#include <string_view>

// TODO (yusufemresamur): review AI generated tests

namespace mlogpp::detail {

TEST(JsonEscapeTest, EmptyStringIsUnchanged) { EXPECT_EQ(JsonEscape(""), ""); }

TEST(JsonEscapeTest, PlainAsciiIsUnchanged) {
  EXPECT_EQ(JsonEscape("hello world 123"), "hello world 123");
}

// ── RFC 8259 §7 named escape sequences ───────────────────────────────────────

TEST(JsonEscapeTest, QuotationMarkIsEscaped) {
  EXPECT_EQ(JsonEscape("\""), "\\\"");
}

TEST(JsonEscapeTest, ReverseSolidusIsEscaped) {
  EXPECT_EQ(JsonEscape("\\"), "\\\\");
}

TEST(JsonEscapeTest, BackspaceIsEscaped) { EXPECT_EQ(JsonEscape("\b"), "\\b"); }

TEST(JsonEscapeTest, FormFeedIsEscaped) { EXPECT_EQ(JsonEscape("\f"), "\\f"); }

TEST(JsonEscapeTest, LineFeedIsEscaped) { EXPECT_EQ(JsonEscape("\n"), "\\n"); }

TEST(JsonEscapeTest, CarriageReturnIsEscaped) {
  EXPECT_EQ(JsonEscape("\r"), "\\r");
}

TEST(JsonEscapeTest, TabIsEscaped) { EXPECT_EQ(JsonEscape("\t"), "\\t"); }

// ── Generic \uXXXX for remaining control characters ──────────────────────────

TEST(JsonEscapeTest, NulIsEscapedAsUnicodeEscape) {
  EXPECT_EQ(JsonEscape(std::string_view("\0", 1)), "\\u0000");
}

TEST(JsonEscapeTest, ControlCharSohIsEscaped) {
  EXPECT_EQ(JsonEscape("\x01"), "\\u0001");
}

TEST(JsonEscapeTest, ControlCharUnitSeparatorIsEscaped) {
  // U+001F is the last control character below 0x20.
  EXPECT_EQ(JsonEscape("\x1f"), "\\u001f");
}

// ── Characters that must NOT be escaped ──────────────────────────────────────

TEST(JsonEscapeTest, SpaceIsNotEscaped) { EXPECT_EQ(JsonEscape(" "), " "); }

TEST(JsonEscapeTest, HighBytePassesThroughUnchanged) {
  // UTF-8 encoded é (U+00E9): bytes 0xC3 0xA9 — both ≥ 0x80, no escaping.
  EXPECT_EQ(JsonEscape("\xc3\xa9"), "\xc3\xa9");
}

// ── Combined inputs
// ───────────────────────────────────────────────────────────

TEST(JsonEscapeTest, AllNamedEscapesInSequence) {
  std::string const input{'"', '\\', '\b', '\f', '\n', '\r', '\t'};
  EXPECT_EQ(JsonEscape(input), R"(\"\\\b\f\n\r\t)");
}

TEST(JsonEscapeTest, MixedStringWithEscapesAndPlainText) {
  // Input: say "hello"<newline>world\end
  EXPECT_EQ(JsonEscape("say \"hello\"\nworld\\end"),
            R"(say \"hello\"\nworld\\end)");
}

TEST(JsonEscapeTest, StringWithEmbeddedNul) {
  // NUL byte surrounded by plain text.
  std::string const input = std::string("ab") + '\0' + "cd";
  EXPECT_EQ(JsonEscape(input), "ab\\u0000cd");
}

TEST(JsonEscapeTest, MultipleQuotesAndBackslashes) {
  EXPECT_EQ(JsonEscape("\"\\\"\\"), "\\\"\\\\\\\"\\\\");
}

}  // namespace mlogpp::detail
