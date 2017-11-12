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

/*< lib: variant.hh >*/
/*< stl: algorithm >*/
/*< stl: iterator >*/

namespace Ggp::Lib
{

namespace
{

template <typename ParsedP>
struct ParseResult
{
  ParsedP parsed;
  std::string_view rest;
};

using ParseLeafBasicResult = ParseResult<Leaf::Basic>;

std::optional<ParseLeafBasicResult>
parse_leaf_basic (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }

  auto rest {string.substr (1)};
  switch (string.front ())
  {
  case 'b':
    return {{{Leaf::bool_}, rest}};
  case 'y':
    return {{{Leaf::byte_}, rest}};
  case 'n':
    return {{{Leaf::i16}, rest}};
  case 'q':
    return {{{Leaf::u16}, rest}};
  case 'i':
    return {{{Leaf::i32}, rest}};
  case 'u':
    return {{{Leaf::u32}, rest}};
  case 'x':
    return {{{Leaf::i64}, rest}};
  case 't':
    return {{{Leaf::u64}, rest}};
  case 'h':
    return {{{Leaf::handle}, rest}};
  case 'd':
    return {{{Leaf::double_}, rest}};
  case 's':
    return {{{Leaf::string_}, rest}};
  case 'o':
    return {{{Leaf::object_path}, rest}};
  case 'g':
    return {{{Leaf::signature}, rest}};
  case '?':
    return {{{Leaf::any_basic}, rest}};
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
  auto maybe_first_result {parse_leaf_basic (string)};
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
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  case 'm':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VT::Maybe {std::move (maybe_result->parsed)}}}, std::move (maybe_result->rest)}};
    }
  case 'a':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VT::Array {std::move (maybe_result->parsed)}}}, std::move (maybe_result->rest)}};
    }
  case '*':
    return {{{{Leaf::any_type}}, std::move (rest)}};
  case 'r':
    return {{{{Leaf::any_tuple}}, std::move (rest)}};
  case 'v':
    return {{{{Leaf::variant}}, std::move (rest)}};
  default:
    {
      auto maybe_result {parse_leaf_basic (string)};

      if (!maybe_result)
      {
        return {};
      }

      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  }
}

} // anonymous namespace

/* static */ std::optional<VariantType>
VariantType::from_string (std::string_view const& string)
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

bool
basic_is_definite (Leaf::Basic const& basic)
{
  auto vh {VisitHelper {
    [](Leaf::AnyBasic const&) { return false; },
    [](auto const&) { return true; },
  }};
  return std::visit (vh, basic.v);
}

bool
maybe_is_definite (VT::Maybe const& maybe)
{
  return maybe.pointed_type->is_definite ();
}

bool
tuple_is_definite (VT::Tuple const& tuple)
{
  for (auto const& type : tuple.types)
  {
    if (!type.is_definite ())
    {
      return false;
    }
  }
  return true;
}

bool
array_is_definite (VT::Array const& array)
{
  return array.element_type->is_definite ();
}

bool
entry_is_definite (VT::Entry const& entry)
{
  return basic_is_definite (entry.key) && entry.value->is_definite ();
}

} // anonymous namespace

bool
VariantType::is_definite () const
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return basic_is_definite (basic); },
    [](VT::Maybe const& maybe) { return maybe_is_definite (maybe); },
    [](VT::Tuple const& tuple) { return tuple_is_definite (tuple); },
    [](VT::Array const& array) { return array_is_definite (array); },
    [](VT::Entry const& entry) { return entry_is_definite (entry); },
    [](Leaf::Variant const&) { return true; },
    [](Leaf::AnyTuple const&) { return false; },
    [](Leaf::AnyType const&) { return false; },
  }};
  return std::visit (vh, this->v);
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
    return {{{{Leaf::string_}}, std::move (rest)}};
  case 'o':
    return {{{{Leaf::object_path}}, std::move (rest)}};
  case 'g':
    return {{{{Leaf::signature}}, std::move (rest)}};
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
      auto maybe_result {parse_leaf_basic (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::AtBasicType {std::move (maybe_result->parsed)}}}, std::move (maybe_result->rest)}};
    }
  case '&':
    {
      auto maybe_result {parse_pointer_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  default:
    {
      auto maybe_result {parse_leaf_basic (string)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
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
    return {{{{{VF::Convenience::Type::byte_string_array}}, {{VF::Convenience::Kind::constant}}}, string.substr (len)}};
  }

  len = 3;
  sub = string.substr (0, len);
  if (sub == "aay")
  {
    return {{{{{VF::Convenience::Type::byte_string_array}}, {{VF::Convenience::Kind::duplicated}}}, string.substr (len)}};
  }
  if (sub == "&ay")
  {
    return {{{{{VF::Convenience::Type::byte_string}}, {{VF::Convenience::Kind::constant}}}, string.substr (len)}};
  }
  if (sub == "a&o")
  {
    return {{{{{VF::Convenience::Type::object_path_array}}, {{VF::Convenience::Kind::constant}}}, string.substr (len)}};
  }
  if (sub == "a&s")
  {
    return {{{{{VF::Convenience::Type::string_array}}, {{VF::Convenience::Kind::constant}}}, string.substr (len)}};
  }

  len = 2;
  sub = string.substr (0, len);
  if (sub == "as")
  {
    return {{{{{VF::Convenience::Type::string_array}}, {{VF::Convenience::Kind::duplicated}}}, string.substr (len)}};
  }
  if (sub == "ao")
  {
    return {{{{{VF::Convenience::Type::object_path_array}}, {{VF::Convenience::Kind::duplicated}}}, string.substr (len)}};
  }
  if (sub == "ay")
  {
    return {{{{{VF::Convenience::Type::byte_string}}, {{VF::Convenience::Kind::duplicated}}}, string.substr (len)}};
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
      return {{{{VF::MaybePointer {{VT::Array {{std::move (maybe_result->parsed)}}}}}}, std::move (maybe_result->rest)}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybePointer {{VF::AtVariantType {std::move (maybe_result->parsed)}}}}}, std::move (maybe_result->rest)}};
    }
  case 'v':
    return {{{{VF::MaybePointer {{VF::AtVariantType {{Leaf::variant}}}}}}, std::move (rest)}};
  case '*':
    return {{{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_type}}}}}}, std::move (rest)}};
  case 'r':
    return {{{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_tuple}}}}}}, std::move (rest)}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybePointer {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->rest)}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybePointer {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->rest)}};
    }
    // bool maybes
  case '{':
    {
      auto maybe_result {parse_entry_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->rest)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (std::move (rest))};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->rest)}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->rest)}};
    }
    // basic maybes, need to decide whether a pointer or bool
  default:
    {
      auto maybe_result {parse_leaf_basic (rest)};
      if (!maybe_result)
      {
        return {};
      }
      auto vh {VisitHelper {
        [](Leaf::String const& basic) { return VF::Maybe {{VF::MaybePointer {{VF::BasicMaybePointer {{basic}}}}}}; },
        [](Leaf::ObjectPath const& basic) { return VF::Maybe {{VF::MaybePointer {{VF::BasicMaybePointer {{basic}}}}}}; },
        [](Leaf::Signature const& basic) { return VF::Maybe {{VF::MaybePointer {{VF::BasicMaybePointer {{basic}}}}}}; },
        [](Leaf::AnyBasic const& basic) { return VF::Maybe {{VF::MaybePointer {{VF::BasicMaybePointer {{basic}}}}}}; },
        [](Leaf::Bool const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::Byte const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::I16 const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::U16 const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::I32 const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::U32 const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::I64 const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::U64 const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::Handle const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
        [](Leaf::Double const& basic) { return VF::Maybe {{VF::MaybeBool {{VF::BasicMaybeBool {{basic}}}}}}; },
      }};
      return {{std::visit (vh, maybe_result->parsed.v), std::move (maybe_result->rest)}};
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
      return {{{VT::Array {std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::AtVariantType {std::move (maybe_result->parsed)}}, std::move (maybe_result->rest)}};
    }
  case 'v':
    return {{{VF::AtVariantType {Leaf::variant}}, std::move (rest)}};
  case 'r':
    return {{{VF::AtVariantType {Leaf::any_tuple}}, std::move (rest)}};
  case '*':
    return {{{VF::AtVariantType {Leaf::any_type}}, std::move (rest)}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (std::move (rest))};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  case '{':
    {
      auto maybe_result {parse_entry_format (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  default:
    {
      auto maybe_result {parse_leaf_basic (string)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->rest)}};
    }
  }
}

} // anonymous namespace

/* static */ std::optional<VariantFormat>
VariantFormat::from_string (std::string_view const& string)
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
  return {{Leaf::Basic {generalize<Leaf::Basic::V> (bmp.v)}}};
}

VariantType
basic_maybe_bool_to_variant_type (VF::BasicMaybeBool const& bmb)
{
  return {{Leaf::Basic {generalize<Leaf::Basic::V> (bmb.v)}}};
}

Leaf::Basic
pointer_to_basic_type (VF::Pointer pointer)
{
  return {generalize<Leaf::Basic::V> (pointer.v)};
}

VariantType
convenience_to_variant_type (VF::Convenience const& convenience)
{
  auto vh {VisitHelper {
    [](VF::Convenience::Type::StringArray const&) { return VariantType {{VT::Array {{VariantType {{Leaf::Basic {{Leaf::string_}}}}}}}}; },
    [](VF::Convenience::Type::ObjectPathArray const&) { return VariantType {{VT::Array {{VariantType {{Leaf::Basic {{Leaf::object_path}}}}}}}}; },
    [](VF::Convenience::Type::ByteString const&) { return VariantType {{VT::Array {{VariantType {{Leaf::Basic {{Leaf::byte_}}}}}}}}; },
    [](VF::Convenience::Type::ByteStringArray const&) { return VariantType {{VT::Array {{VariantType {{VT::Array {{VariantType {{Leaf::Basic {Leaf::byte_}}}}}}}}}}}; },
  }};
  return std::visit (vh, convenience.type.v);
}

VariantType
pointer_to_variant_type (VF::Pointer const& pointer)
{
  return {{pointer_to_basic_type (pointer)}};
}

VariantType
maybe_pointer_to_variant_type (VF::MaybePointer const& mp)
{
  auto vh {VisitHelper {
    [](VT::Array const& array) { return VariantType {{array}}; },
    [](VF::AtVariantType const& avt) { return avt.type; },
    [](VF::BasicMaybePointer const& bmp) { return basic_maybe_pointer_to_variant_type (bmp); },
    [](VF::Pointer const& pointer) { return pointer_to_variant_type (pointer); },
    [](VF::Convenience const& convenience) { return convenience_to_variant_type (convenience); },
  }};

  return {VT::Maybe {std::visit (vh, mp.v)}};
}

Leaf::Basic
basic_format_to_basic_type (VF::BasicFormat const& basic_format)
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return basic; },
    [](VF::AtBasicType const& at) { return at.basic; },
    [](VF::Pointer const& pointer) { return pointer_to_basic_type (pointer); },
  }};
  return std::visit (vh, basic_format.v);
}

VariantType
entry_to_variant_type (VF::Entry const& entry)
{
  return {{VT::Entry {basic_format_to_basic_type (entry.key), entry.value->to_type ()}}};
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
                    return format.to_type ();
                  });
  return {{VT::Tuple {std::move (types)}}};
}

VariantType
maybe_to_variant_type (VF::Maybe const& maybe);

VariantType
maybe_bool_to_variant_type (VF::MaybeBool const& mb)
{
  auto vh {VisitHelper {
    [](VF::BasicMaybeBool const& bmb) { return basic_maybe_bool_to_variant_type (bmb); },
    [](VF::Entry const& entry) { return entry_to_variant_type (entry); },
    [](VF::Tuple const& tuple) { return tuple_to_variant_type (tuple); },
    [](VF::Maybe const& maybe) { return maybe_to_variant_type (maybe); },
  }};
  return {VT::Maybe {std::visit (vh, mb.v)}};
}

VariantType
maybe_to_variant_type (VF::Maybe const& maybe)
{
  auto vh {VisitHelper {
    [](VF::MaybePointer const& mp) { return maybe_pointer_to_variant_type (mp); },
    [](VF::MaybeBool const& mb) { return maybe_bool_to_variant_type (mb); },
  }};
  return std::visit (vh, maybe.v);
}

} // anonymous namespace

VariantType
VariantFormat::to_type () const
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return VariantType {{basic}}; },
    [](VT::Array const& array) { return VariantType {{array}}; },
    [](VF::AtVariantType const& avt) { return avt.type; },
    [](VF::Pointer const &pointer) { return pointer_to_variant_type (pointer); },
    [](VF::Convenience const& convenience) { return convenience_to_variant_type (convenience); },
    [](VF::Maybe const& maybe) { return maybe_to_variant_type (maybe); },
    [](VF::Tuple const& tuple) { return tuple_to_variant_type (tuple); },
    [](VF::Entry const& entry) { return entry_to_variant_type (entry); },
  }};
  return std::visit (vh, this->v);
}

} // namespace Ggp::Lib