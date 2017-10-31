/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2017 Krzesimir Nowak
 *
 * gcc-glib-plugin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * gcc-glib-plugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * gcc-glib-plugin. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ggp-variant.hh"

#include <algorithm>
#include <iterator>

namespace Ggp
{

namespace
{

template <typename ParsedP>
struct ParseResult
{
  ParsedP parsed;
  std::string_view rest;
};

using ParseBasicTypeResult = ParseResult<VT::Basic>;

std::optional<ParseBasicTypeResult>
parse_basic_type (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }

  auto rest {string.substr (1)};
  switch (string.front ())
  {
  case 'b':
    return {{VT::Basic::Bool, rest}};
  case 'y':
    return {{VT::Basic::Byte, rest}};
  case 'n':
    return {{VT::Basic::I16, rest}};
  case 'q':
    return {{VT::Basic::U16, rest}};
  case 'i':
    return {{VT::Basic::I32, rest}};
  case 'u':
    return {{VT::Basic::U32, rest}};
  case 'x':
    return {{VT::Basic::I64, rest}};
  case 't':
    return {{VT::Basic::U64, rest}};
  case 'h':
    return {{VT::Basic::Handle, rest}};
  case 'd':
    return {{VT::Basic::Double, rest}};
  case 's':
    return {{VT::Basic::String, rest}};
  case 'o':
    return {{VT::Basic::ObjectPath, rest}};
  case 'g':
    return {{VT::Basic::Signature, rest}};
  case '?':
    return {{VT::Basic::Any, rest}};
  default:
    return {};
  }
}

using ParseTypeResult = ParseResult<VariantType>;

std::optional<ParseTypeResult>
parse_single_type (std::string_view const& string);

using ParseEntryTypeResult = ParseResult<VT::Entry>;

std::optional<ParseEntryTypeResult>
parse_entry_type (std::string_view const& string)
{
  auto maybe_first_result {parse_basic_type (string)};
  if (!maybe_first_result)
  {
    return {};
  }
  auto maybe_second_result {parse_single_type (maybe_first_result->rest)};
  if (!maybe_second_result)
  {
    return {};
  }
  if (maybe_second_result->rest.empty ())
  {
    return {};
  }
  if (maybe_second_result->rest.front () != '}')
  {
    return {};
  }
  return {{{std::move (maybe_first_result->parsed), std::move (maybe_second_result->parsed)}, maybe_second_result->rest.substr (1)}};
}

using ParseTupleTypeResult = ParseResult<VT::Tuple>;

std::optional<ParseTupleTypeResult>
parse_tuple_type (std::string_view string)
{
  std::vector<VariantType> types {};
  for (;;)
  {
    if (string.empty ())
    {
      return {};
    }
    if (string.front () == ')')
    {
      return {{{std::move (types)}, string.substr (1)}};
    }
    auto maybe_result {parse_single_type (string)};
    if (!maybe_result)
    {
      return {};
    }
    string = std::move (maybe_result->rest);
    types.push_back (std::move (maybe_result->parsed));
  }
}

std::optional<ParseTypeResult>
parse_single_type (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }

  auto rest {string.substr (1)};
  switch (string.front ())
  {
  case '{':
    {
      auto maybe_result {parse_entry_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  case 'm':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{VT::Maybe {std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case 'a':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{VT::Array {std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case '*':
    return {{Leaf::AnyType {}, std::move (rest)}};
  case 'r':
    return {{Leaf::AnyTuple {}, std::move (rest)}};
  case 'v':
    return {{Leaf::Variant {}, std::move (rest)}};
  default:
    {
      auto maybe_result {parse_basic_type (string)};

      if (!maybe_result)
      {
        return {};
      }

      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  }
}

} // anonymous namespace

std::optional<VariantType>
parse_variant_type_string (std::string_view const& string)
{
  auto maybe_result {parse_single_type (string)};

  if (!maybe_result)
  {
    return {};
  }
  if (!maybe_result->rest.empty ())
  {
    return {};
  }

  return {std::move (maybe_result->parsed)};
}

namespace
{

using ParsePointerFormatResult = ParseResult<VF::Pointer>;

std::optional<ParsePointerFormatResult>
parse_pointer_format (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }
  auto rest {string.substr (1)};
  switch (string.front ())
  {
  case 's':
    return {{{Leaf::String {}}, std::move (rest)}};
  case 'o':
    return {{{Leaf::ObjectPath {}}, std::move (rest)}};
  case 'g':
    return {{{Leaf::Signature {}}, std::move (rest)}};
  default:
    return {};
  }
}

using ParseBasicFormatResult = ParseResult<VF::BasicFormat>;

std::optional<ParseBasicFormatResult>
parse_basic_format (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }
  auto rest {string.substr (1)};
  switch (string.front ())
  {
  case '@':
    {
      auto maybe_result {parse_basic_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{VF::AtBasicType {std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case '&':
    {
      auto maybe_result {parse_pointer_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  default:
    {
      auto maybe_result {parse_basic_type (string)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  }
}

using ParseConvenienceFormatResult = ParseResult<VF::Convenience>;

std::optional<ParseConvenienceFormatResult>
parse_convenience_format (std::string_view const& string)
{
  auto len {4u};
  auto sub {string.substr (0, len)};
  if (sub == "a&ay")
  {
    return {{{VF::Convenience::Type::ByteStringArray, VF::Convenience::Kind::Constant}, string.substr (len)}};
  }

  len = 3;
  sub = string.substr (0, len);
  if (sub == "aay")
  {
    return {{{VF::Convenience::Type::ByteStringArray, VF::Convenience::Kind::Duplicated}, string.substr (len)}};
  }
  if (sub == "&ay")
  {
    return {{{VF::Convenience::Type::ByteString, VF::Convenience::Kind::Constant}, string.substr (len)}};
  }
  if (sub == "a&o")
  {
    return {{{VF::Convenience::Type::ObjectPathArray, VF::Convenience::Kind::Constant}, string.substr (len)}};
  }
  if (sub == "a&s")
  {
    return {{{VF::Convenience::Type::StringArray, VF::Convenience::Kind::Constant}, string.substr (len)}};
  }

  len = 2;
  sub = string.substr (0, len);
  if (sub == "as")
  {
    return {{{VF::Convenience::Type::StringArray, VF::Convenience::Kind::Duplicated}, string.substr (len)}};
  }
  if (sub == "ao")
  {
    return {{{VF::Convenience::Type::ObjectPathArray, VF::Convenience::Kind::Duplicated}, string.substr (len)}};
  }
  if (sub == "ay")
  {
    return {{{VF::Convenience::Type::ByteString, VF::Convenience::Kind::Duplicated}, string.substr (len)}};
  }
  return {};
}

using ParseTupleFormat = ParseResult<VF::Tuple>;
using ParseFormatResult = ParseResult<VariantFormat>;

std::optional<ParseFormatResult>
parse_single_format (std::string_view const& string);

std::optional<ParseTupleFormat>
parse_tuple_format (std::string_view string)
{
  std::vector<VariantFormat> formats {};

  for (;;)
  {
    if (string.empty ())
    {
      return {};
    }
    if (string.front () == ')')
    {
      ParseTupleFormat f {{std::move (formats)}, string.substr (1)};
      return {std::move (f)};
    }
    auto maybe_result {parse_single_format (string)};
    if (!maybe_result)
    {
      return {};
    }
    string = std::move (maybe_result->rest);
    formats.push_back (std::move (maybe_result->parsed));
  }
}

using ParseEntryFormatResult = ParseResult<VF::Entry>;

std::optional<ParseEntryFormatResult>
parse_entry_format (std::string_view const& string)
{
  auto maybe_first_result {parse_basic_format (string)};
  if (!maybe_first_result)
  {
    return {};
  }
  auto maybe_second_result {parse_single_format (maybe_first_result->rest)};
  if (!maybe_second_result)
  {
    return {};
  }
  if (maybe_second_result->rest.empty ())
  {
    return {};
  }
  if (maybe_second_result->rest.front () != '}')
  {
    return {};
  }
  return {{{std::move (maybe_first_result->parsed), std::move (maybe_second_result->parsed)}, maybe_second_result->rest.substr (1)}};
}

using ParseMaybeFormatResult = ParseResult<VF::Maybe>;

std::optional<ParseMaybeFormatResult>
parse_maybe_format (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }
  auto rest {string.substr (1)};
  switch (string.front ())
  {
    // pointer maybes
  case 'a':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::MaybePointer {VT::Array {std::move (maybe_result->parsed)}}}, std::move (maybe_result->rest)}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::MaybePointer {VF::AtVariantType {std::move (maybe_result->parsed)}}}, std::move (maybe_result->rest)}};
    }
  case 'v':
    return {{{VF::MaybePointer {VF::AtVariantType {Leaf::Variant {}}}}, std::move (rest)}};
  case '*':
    return {{{VF::MaybePointer {VF::AtVariantType {Leaf::AnyType {}}}}, std::move (rest)}};
  case 'r':
    return {{{VF::MaybePointer {VF::AtVariantType {Leaf::AnyTuple {}}}}, std::move (rest)}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::MaybePointer {std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::MaybePointer {std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
    // bool maybes
  case '{':
    {
      auto maybe_result {parse_entry_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::MaybeBool {std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (std::move (rest))};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::MaybeBool {std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::MaybeBool {std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
    // basic maybes, need to decide whether a pointer or bool
  default:
    {
      auto maybe_result {parse_basic_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      switch (maybe_result->parsed)
      {
      case VT::Basic::String:
        return {{{VF::MaybePointer {VF::BasicMaybePointer {Leaf::String {}}}}, std::move (maybe_result->rest)}};
      case VT::Basic::ObjectPath:
        return {{{VF::MaybePointer {VF::BasicMaybePointer {Leaf::ObjectPath {}}}}, std::move (maybe_result->rest)}};
      case VT::Basic::Signature:
        return {{{VF::MaybePointer {VF::BasicMaybePointer {Leaf::Signature {}}}}, std::move (maybe_result->rest)}};
      case VT::Basic::Any:
        return {{{VF::MaybePointer {VF::BasicMaybePointer {Leaf::AnyBasic {}}}}, std::move (maybe_result->rest)}};
      case VT::Basic::Bool:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::Bool}}, std::move (maybe_result->rest)}};
      case VT::Basic::Byte:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::Byte}}, std::move (maybe_result->rest)}};
      case VT::Basic::I16:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::I16}}, std::move (maybe_result->rest)}};
      case VT::Basic::U16:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::U16}}, std::move (maybe_result->rest)}};
      case VT::Basic::I32:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::I32}}, std::move (maybe_result->rest)}};
      case VT::Basic::U32:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::U32}}, std::move (maybe_result->rest)}};
      case VT::Basic::I64:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::I64}}, std::move (maybe_result->rest)}};
      case VT::Basic::U64:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::U64}}, std::move (maybe_result->rest)}};
      case VT::Basic::Handle:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::Handle}}, std::move (maybe_result->rest)}};
      case VT::Basic::Double:
        return {{{VF::MaybeBool {VF::BasicMaybeBool::Double}}, std::move (maybe_result->rest)}};
      }
      return {};
    }
  }
}

std::optional<ParseFormatResult>
parse_single_format (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }

  auto rest {string.substr (1)};
  switch (string.front ())
  {
  case 'a':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{VT::Array {std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{VF::AtVariantType {std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case 'v':
    return {{VF::AtVariantType {Leaf::Variant {}}, std::move (rest)}};
  case 'r':
    return {{VF::AtVariantType {Leaf::AnyTuple {}}, std::move (rest)}};
  case '*':
    return {{VF::AtVariantType {Leaf::AnyType {}}, std::move (rest)}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (std::move (rest))};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  case '{':
    {
      auto maybe_result {parse_entry_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  default:
    {
      auto maybe_result {parse_basic_type (string)};
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->parsed), std::move (maybe_result->rest)}};
    }
  }
}

} // anonymous namespace

std::optional<VariantFormat>
parse_variant_format_string (std::string_view const& string)
{
  auto maybe_result {parse_single_format (string)};

  if (!maybe_result)
  {
    return {};
  }
  if (!maybe_result->rest.empty ())
  {
    return {};
  }

  return {std::move (maybe_result->parsed)};
}

namespace
{

VariantType
basic_maybe_pointer_to_variant_type (VF::BasicMaybePointer const& bmp)
{
  auto v {Util::VisitHelper {
    [](Leaf::String const&) { return VariantType {VT::Basic::String}; },
    [](Leaf::ObjectPath const&) { return VariantType {VT::Basic::ObjectPath}; },
    [](Leaf::Signature const&) { return VariantType {VT::Basic::Signature}; },
    [](Leaf::AnyBasic const&) { return VariantType {VT::Basic::Any}; },
  }};
  return std::visit (v, bmp);
}

VariantType
basic_maybe_bool_to_variant_type (VF::BasicMaybeBool const& bmb)
{
  switch (bmb)
  {
  case VF::BasicMaybeBool::Bool:
    return {VT::Basic::Bool};
  case VF::BasicMaybeBool::Byte:
    return {VT::Basic::Byte};
  case VF::BasicMaybeBool::I16:
    return {VT::Basic::I16};
  case VF::BasicMaybeBool::U16:
    return {VT::Basic::U16};
  case VF::BasicMaybeBool::I32:
    return {VT::Basic::I32};
  case VF::BasicMaybeBool::U32:
    return {VT::Basic::U32};
  case VF::BasicMaybeBool::I64:
    return {VT::Basic::I64};
  case VF::BasicMaybeBool::U64:
    return {VT::Basic::U64};
  case VF::BasicMaybeBool::Handle:
    return {VT::Basic::Handle};
  case VF::BasicMaybeBool::Double:
    return {VT::Basic::Double};
  default:
    gcc_unreachable ();
    return {VT::Basic::Bool};
  }
}

VT::Basic
pointer_to_basic_type (VF::Pointer pointer)
{
  auto v {Util::VisitHelper {
    [](Leaf::String const&) { return VT::Basic::String; },
    [](Leaf::ObjectPath const&) { return VT::Basic::ObjectPath; },
    [](Leaf::Signature const&) { return VT::Basic::Signature; },
  }};
  return std::visit (v, pointer);
}

VariantType
convenience_to_variant_type (VF::Convenience const& convenience)
{
  switch (convenience.type)
  {
  case VF::Convenience::Type::StringArray:
    return {VT::Array {VT::Basic::String}};
  case VF::Convenience::Type::ObjectPathArray:
    return {VT::Array {VT::Basic::ObjectPath}};
  case VF::Convenience::Type::ByteString:
    return {VT::Array {VT::Basic::Byte}};
  case VF::Convenience::Type::ByteStringArray:
    return {VT::Array {VT::Array {VT::Basic::Byte}}};
  default:
    gcc_unreachable();
    return {VT::Array {VT::Basic::String}};
  }
}

VariantType
pointer_to_variant_type (VF::Pointer const& pointer)
{
  return {pointer_to_basic_type (pointer)};
}

VariantType
maybe_pointer_to_variant_type (VF::MaybePointer const& mp)
{
  auto v {Util::VisitHelper {
    [](VT::Array const& array) { return VariantType {array}; },
    [](VF::AtVariantType const& avt) { return avt.type; },
    [](VF::BasicMaybePointer const& bmp) { return basic_maybe_pointer_to_variant_type (bmp); },
    [](VF::Pointer const& pointer) { return pointer_to_variant_type (pointer); },
    [](VF::Convenience const &convenience) { return convenience_to_variant_type (convenience); },
  }};

  return {VT::Maybe {std::visit (v, mp)}};
}

VT::Basic
basic_format_to_basic_type (VF::BasicFormat const& basic_format)
{
  auto v {Util::VisitHelper {
    [](VT::Basic const& basic) { return basic; },
    [](VF::AtBasicType const& at) { return at.basic; },
    [](VF::Pointer const& pointer) { return pointer_to_basic_type (pointer); },
  }};
  return std::visit (v, basic_format);
}

VariantType
entry_to_variant_type (VF::Entry const& entry)
{
  return {VT::Entry {basic_format_to_basic_type (entry.key), variant_format_to_type (entry.value)}};
}

VariantType
tuple_to_variant_type (VF::Tuple const& tuple)
{
  std::vector<VariantType> types {};
  std::transform (tuple.formats.cbegin (),
                  tuple.formats.cend (),
                  std::back_inserter (types),
                  [](VariantFormat const& format)
                  {
                    return variant_format_to_type (format);
                  });
  return {VT::Tuple {std::move (types)}};
}

VariantType
maybe_to_variant_type (VF::Maybe const& maybe);

VariantType
maybe_bool_to_variant_type (VF::MaybeBool const& mp)
{
  auto v {Util::VisitHelper {
    [](VF::BasicMaybeBool const& bmb) { return basic_maybe_bool_to_variant_type (bmb); },
    [](VF::Entry const& entry) { return entry_to_variant_type (entry); },
    [](VF::Tuple const& tuple) { return tuple_to_variant_type (tuple); },
    [](VF::Maybe const& maybe) { return maybe_to_variant_type (maybe); },
  }};
  return {VT::Maybe {std::visit (v, mp)}};
}

VariantType
maybe_to_variant_type (VF::Maybe const& maybe)
{
  auto v {Util::VisitHelper {
    [](VF::MaybePointer const& mp) { return maybe_pointer_to_variant_type (mp); },
    [](VF::MaybeBool const& mb) { return maybe_bool_to_variant_type (mb); },
  }};
  return std::visit (v, maybe.kind);
}

} // anonymous namespace

VariantType
variant_format_to_type (VariantFormat const& format)
{
  auto v {Util::VisitHelper {
    [](VT::Basic const& basic) { return VariantType {basic}; },
    [](VT::Array const& array) { return VariantType {array}; },
    [](VF::AtVariantType const& avt) { return avt.type; },
    [](VF::Pointer pointer) { return pointer_to_variant_type (pointer); },
    [](VF::Convenience const& convenience) { return convenience_to_variant_type (convenience); },
    [](VF::Maybe const& maybe) { return maybe_to_variant_type (maybe); },
    [](VF::Tuple const& tuple) { return tuple_to_variant_type (tuple); },
    [](VF::Entry const& entry) { return entry_to_variant_type (entry); },
  }};
  return std::visit (v, format);
}

} // namespace Ggp
