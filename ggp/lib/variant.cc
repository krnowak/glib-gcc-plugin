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

struct ParseState
{
  auto take_one () -> std::optional<char>;
  auto take_back () -> void;
  auto get_rest () -> std::string_view;

  std::string_view string;
  std::size_t offset;
};

auto ParseState::take_one () -> std::optional<char>
{
  if (this->offset < this->string.length())
  {
    auto c {this->string[this->offset]};

    ++this->offset;

    return {c};
  }
  else
  {
    return {};
  }
}

auto ParseState::take_back () -> void
{
  if (this->offset > 0)
  {
    --this->offset;
  }
}

auto ParseState::get_rest () -> std::string_view
{
  if (this->string.length() < this->offset)
  {
    return this->string.substr (this->offset);
  }
  else
  {
    return {};
  }
}

template <typename ParsedP>
struct ParseResult
{
  ParsedP parsed;
  ParseState state;
};

using ParseLeafBasicResult = ParseResult<Leaf::Basic>;

auto
parse_leaf_basic (ParseState state) -> std::optional<ParseLeafBasicResult>
{
  auto maybe_c {state.take_one ()};

  if (!maybe_c)
  {
    /* expected basic type, got premature end of a string */
    return {};
  }

  switch (*maybe_c)
  {
  case 'b':
    return {{{Leaf::bool_}, state}};
  case 'y':
    return {{{Leaf::byte_}, state}};
  case 'n':
    return {{{Leaf::i16}, state}};
  case 'q':
    return {{{Leaf::u16}, state}};
  case 'i':
    return {{{Leaf::i32}, state}};
  case 'u':
    return {{{Leaf::u32}, state}};
  case 'x':
    return {{{Leaf::i64}, state}};
  case 't':
    return {{{Leaf::u64}, state}};
  case 'h':
    return {{{Leaf::handle}, state}};
  case 'd':
    return {{{Leaf::double_}, state}};
  case 's':
    return {{{Leaf::string_}, state}};
  case 'o':
    return {{{Leaf::object_path}, state}};
  case 'g':
    return {{{Leaf::signature}, state}};
  case '?':
    return {{{Leaf::any_basic}, state}};
  default:
    /* expected basic type, got X */
    return {};
  }
}

using ParseTypeResult = ParseResult<VariantType>;

auto parse_single_type (ParseState state) -> std::optional<ParseTypeResult>;

using ParseEntryTypeResult = ParseResult<VT::Entry>;

auto parse_entry_type (ParseState state) -> std::optional<ParseEntryTypeResult>
{
  auto maybe_first_result {parse_leaf_basic (state)};
  if (!maybe_first_result)
  {
    /* failed to parse first type for an entry: <reason> */
    return {};
  }
  auto maybe_second_result {parse_single_type (maybe_first_result->state)};
  if (!maybe_second_result)
  {
    /* failed to parse second type for an entry: <reason> */
    return {};
  }
  auto maybe_c {maybe_second_result->state.take_one ()};
  if (!maybe_c)
  {
    /* expected '}', got premature end of a string */
    return {};
  }
  if (*maybe_c != '}')
  {
    /* expected '}', got X */
    return {};
  }
  return {{{std::move (maybe_first_result->parsed), std::move (maybe_second_result->parsed)}, maybe_second_result->state}};
}

using ParseTupleTypeResult = ParseResult<VT::Tuple>;

auto parse_tuple_type (ParseState state) -> std::optional<ParseTupleTypeResult>
{
  std::vector<VariantType> types {};
  for (;;)
  {
    auto maybe_c {state.take_one ()};
    if (!maybe_c)
    {
      /* expected either a type or ')', got premature end of a string */
      return {};
    }
    if (*maybe_c == ')')
    {
      return {{{std::move (types)}, state}};
    }
    auto maybe_result {parse_single_type (state)};
    if (!maybe_result)
    {
      /* failed to parse Nth type of a tuple: <reason> */
      return {};
    }
    state = std::move (maybe_result->state);
    types.push_back (std::move (maybe_result->parsed));
  }
}

auto parse_single_type (ParseState state) -> std::optional<ParseTypeResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    /* expected a type, got premature end of a string */
    return {};
  }

  switch (*maybe_c)
  {
  case '{':
    {
      auto maybe_result {parse_entry_type (state)};
      if (!maybe_result)
      {
        /* failed to parse entry type: <reason> */
        return {};
      }
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_type (state)};
      if (!maybe_result)
      {
        /* failed to parse tuple type: <reason> */
        return {};
      }
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}};
    }
  case 'm':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        /* failed to parse maybe type: <reason> */
        return {};
      }
      return {{{{VT::Maybe {std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}};
    }
  case 'a':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        /* failed to parse array type: <reason> */
        return {};
      }
      return {{{{VT::Array {std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}};
    }
  case '*':
    return {{{{Leaf::any_type}}, std::move (state)}};
  case 'r':
    return {{{{Leaf::any_tuple}}, std::move (state)}};
  case 'v':
    return {{{{Leaf::variant}}, std::move (state)}};
  default:
    {
      state.take_back();

      auto maybe_result {parse_leaf_basic (state)};

      if (!maybe_result)
      {
        /* failed to parse type, expected '{', '(', 'm', 'a', '*', 'r', 'v' or a basic type, got X */
        return {};
      }

      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}};
    }
  }
}

} // anonymous namespace

/* static */ std::optional<VariantType>
VariantType::from_string (std::string_view const& string)
{
  auto state {ParseState{string, 0}};
  auto maybe_result {parse_single_type (state)};

  if (!maybe_result)
  {
    /* failed to parse variant type: <reason> */
    return {};
  }
  if (auto rest {maybe_result->state.get_rest()}; !rest.empty ())
  {
    /* string contains more than one complete type: <rest> */
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

auto parse_pointer_format (ParseState state) -> std::optional<ParsePointerFormatResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    /* expected pointer type, got premature end of a string */
    return {};
  }
  switch (*maybe_c)
  {
  case 's':
    return {{{{Leaf::string_}}, std::move (state)}};
  case 'o':
    return {{{{Leaf::object_path}}, std::move (state)}};
  case 'g':
    return {{{{Leaf::signature}}, std::move (state)}};
  default:
    /* expected 's' or 'o' or 'g', got X */
    return {};
  }
}

using ParseBasicFormatResult = ParseResult<VF::BasicFormat>;

auto parse_basic_format (ParseState state) -> std::optional<ParseBasicFormatResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {};
  }
  switch (*maybe_c)
  {
  case '@':
    {
      auto maybe_result {parse_leaf_basic (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::AtBasicType {std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}};
    }
  case '&':
    {
      auto maybe_result {parse_pointer_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}};
    }
  default:
    {
      state.take_back ();
      auto maybe_result {parse_leaf_basic (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}};
    }
  }
}

using ParseConvenienceFormatResult = ParseResult<VF::Convenience>;

auto parse_convenience_format (ParseState state) -> std::optional<ParseConvenienceFormatResult>
{
  auto maybe_c {state.take_one ()};

  if (!maybe_c)
  {
    return {};
  }

  switch (*maybe_c)
  {
    // a
  case 'a':
    {
      maybe_c = state.take_one ();

      if (!maybe_c)
      {
        return {};
      }

      switch (*maybe_c)
      {
        // a&
      case '&':
        {
          maybe_c = state.take_one ();

          if (!maybe_c)
          {
            return {};
          }

          switch (*maybe_c)
          {
            // a&a
          case 'a':
            {
              maybe_c = state.take_one ();

              if (!maybe_c)
              {
                return {};
              }

              switch (*maybe_c)
              {
                // a&ay
              case 'y':
                return {{{{{VF::Convenience::Type::byte_string_array}}, {{VF::Convenience::Kind::constant}}}, state}};
              default:
                return {};
              }
            }
            // a&o
          case 'o':
            return {{{{{VF::Convenience::Type::object_path_array}}, {{VF::Convenience::Kind::constant}}}, state}};
            // a&s
          case 's':
            return {{{{{VF::Convenience::Type::string_array}}, {{VF::Convenience::Kind::constant}}}, state}};
          default:
            return {};
          }
        }
        // aa
      case 'a':
        {
          maybe_c = state.take_one ();

          if (!maybe_c)
          {
            return {};
          }

          switch (*maybe_c)
          {
            // aay
          case 'y':
            return {{{{{VF::Convenience::Type::byte_string_array}}, {{VF::Convenience::Kind::duplicated}}}, state}};
          default:
            return {};
          }
        }
        // as
      case 's':
        return {{{{{VF::Convenience::Type::string_array}}, {{VF::Convenience::Kind::duplicated}}}, state}};
        // ao
      case 'o':
        return {{{{{VF::Convenience::Type::object_path_array}}, {{VF::Convenience::Kind::duplicated}}}, state}};
        // ay
      case 'y':
        return {{{{{VF::Convenience::Type::byte_string}}, {{VF::Convenience::Kind::duplicated}}}, state}};
      default:
        return {};
      }
    }
    // &
  case '&':
    {
      maybe_c = state.take_one ();

      if (!maybe_c)
      {
        return {};
      }

      switch (*maybe_c)
      {
        // &a
      case 'a':
        {
          maybe_c = state.take_one ();

          if (!maybe_c)
          {
            return {};
          }

          switch (*maybe_c)
          {
            // &ay
          case 'y':
            return {{{{{VF::Convenience::Type::byte_string}}, {{VF::Convenience::Kind::constant}}}, state}};
          default:
            return {};
          }
        }
      default:
        return {};
      }
    }
  default:
    return {};
  }
}

using ParseTupleFormat = ParseResult<VF::Tuple>;
using ParseFormatResult = ParseResult<VariantFormat>;

auto parse_single_format (ParseState state) -> std::optional<ParseFormatResult>;

auto parse_tuple_format (ParseState state) -> std::optional<ParseTupleFormat>
{
  std::vector<VariantFormat> formats {};

  for (;;)
  {
    auto maybe_c {state.take_one ()};
    if (!maybe_c)
    {
      return {};
    }
    if (*maybe_c == ')')
    {
      ParseTupleFormat f {{std::move (formats)}, state};
      return {std::move (f)};
    }
    auto maybe_result {parse_single_format (state)};
    if (!maybe_result)
    {
      return {};
    }
    state = std::move (maybe_result->state);
    formats.push_back (std::move (maybe_result->parsed));
  }
}

using ParseEntryFormatResult = ParseResult<VF::Entry>;

auto parse_entry_format (ParseState state) -> std::optional<ParseEntryFormatResult>
{
  auto maybe_first_result {parse_basic_format (state)};
  if (!maybe_first_result)
  {
    return {};
  }
  auto maybe_second_result {parse_single_format (maybe_first_result->state)};
  if (!maybe_second_result)
  {
    return {};
  }
  auto maybe_c {maybe_second_result->state.take_one()};
  if (!maybe_c)
  {
    return {};
  }
  if (*maybe_c != '}')
  {
    return {};
  }
  return {{{std::move (maybe_first_result->parsed), std::move (maybe_second_result->parsed)}, maybe_second_result->state}};
}

using ParseMaybeFormatResult = ParseResult<VF::Maybe>;

auto parse_maybe_format (ParseState state) -> std::optional<ParseMaybeFormatResult>
{
  auto maybe_c {state.take_one()};
  if (!maybe_c)
  {
    return {};
  }
  switch (*maybe_c)
  {
    // pointer maybes
  case 'a':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybePointer {{VT::Array {{std::move (maybe_result->parsed)}}}}}}, std::move (maybe_result->state)}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybePointer {{VF::AtVariantType {std::move (maybe_result->parsed)}}}}}, std::move (maybe_result->state)}};
    }
  case 'v':
    return {{{{VF::MaybePointer {{VF::AtVariantType {{Leaf::variant}}}}}}, std::move (state)}};
  case '*':
    return {{{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_type}}}}}}, std::move (state)}};
  case 'r':
    return {{{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_tuple}}}}}}, std::move (state)}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybePointer {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->state)}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybePointer {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->state)}};
    }
    // bool maybes
  case '{':
    {
      auto maybe_result {parse_entry_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->state)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (std::move (state))};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->state)}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}}, std::move (maybe_result->state)}};
    }
    // basic maybes, need to decide whether a pointer or bool
  default:
    {
      state.take_back ();
      auto maybe_result {parse_leaf_basic (state)};
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
      return {{std::visit (vh, maybe_result->parsed.v), std::move (maybe_result->state)}};
    }
  }
}

auto parse_single_format (ParseState state) -> std::optional<ParseFormatResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {};
  }

  switch (*maybe_c)
  {
  case 'a':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VT::Array {std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{VF::AtVariantType {std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}};
    }
  case 'v':
    return {{{VF::AtVariantType {Leaf::variant}}, std::move (state)}};
  case 'r':
    return {{{VF::AtVariantType {Leaf::any_tuple}}, std::move (state)}};
  case '*':
    return {{{VF::AtVariantType {Leaf::any_type}}, std::move (state)}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}};
    }
  case '{':
    {
      auto maybe_result {parse_entry_format (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}};
    }
  default:
    {
      state.take_back ();
      auto maybe_result {parse_leaf_basic (state)};
      if (!maybe_result)
      {
        return {};
      }
      return {{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}};
    }
  }
}

} // anonymous namespace

/* static */ std::optional<VariantFormat>
VariantFormat::from_string (std::string_view const& string)
{
  auto state {ParseState{string, 0}};
  auto maybe_result {parse_single_format (state)};

  if (!maybe_result)
  {
    return {};
  }
  if (auto rest {maybe_result->state.get_rest ()}; !rest.empty ())
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
