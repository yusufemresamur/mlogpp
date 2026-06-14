#pragma once
#ifndef MLOGPP_JSON_ESCAPE_HPP_
#define MLOGPP_JSON_ESCAPE_HPP_

#include <format>
#include <string>
#include <string_view>

namespace mlogpp::detail {

// Escapes a string for embedding inside a JSON string value.
//
// Rules follow RFC 8259 §7 <https://www.rfc-editor.org/rfc/rfc8259#section-7>:
//   U+0022 QUOTATION MARK        -> \"
//   U+005C REVERSE SOLIDUS       -> '\\'
//   U+0008 BACKSPACE             -> \b
//   U+000C FORM FEED             -> \f
//   U+000A LINE FEED             -> \n
//   U+000D CARRIAGE RETURN       -> \r
//   U+0009 CHARACTER TABULATION  -> \t
//   U+0000–U+001F (other)        -> \uXXXX
//   All other code units are passed through unchanged.
[[nodiscard]] inline std::string JsonEscape(std::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char const c : s) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20)
          out += std::format("\\u{:04x}", static_cast<unsigned char>(c));
        else
          out += c;
    }
  }
  return out;
}

}  // namespace mlogpp::detail

#endif  // MLOGPP_JSON_ESCAPE_HPP_
