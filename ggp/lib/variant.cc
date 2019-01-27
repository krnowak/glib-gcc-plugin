/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2017, 2018, 2019 Krzesimir Nowak
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
/*< stl: optional >*/
/*< stl: sstream >*/

namespace Ggp::Lib
{

namespace
{

struct ParseState
{
  auto
  take_one () -> std::optional<char>;
  auto
  take_back () -> void;
  auto
  get_rest () -> std::string_view;

  auto
  error (std::string reason) -> VariantParseErrorCascade;
  auto
  error (std::string reason, VariantParseErrorCascade& error) -> VariantParseErrorCascade;

  std::string_view string;
  std::size_t offset;
};

auto
ParseState::take_one () -> std::optional<char>
{
  if (this->offset < this->string.size())
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

auto
ParseState::take_back () -> void
{
  if (this->offset > 0)
  {
    --this->offset;
  }
}

auto
ParseState::get_rest () -> std::string_view
{
  if (this->offset < this->string.size())
  {
    return this->string.substr (this->offset);
  }
  else
  {
    return {};
  }
}

auto
ParseState::error (std::string reason) -> VariantParseErrorCascade
{
  return {{{{this->offset, std::move(reason)}}}};
}

auto
ParseState::error (std::string reason, VariantParseErrorCascade& error) -> VariantParseErrorCascade
{
  VariantParseErrorCascade e;

  using std::swap;

  e.errors.swap (error.errors);
  auto parse_error {VariantParseError{this->offset, std::move (reason)}};
  e.errors.emplace_back(std::move (parse_error));

  return e;
}

template <typename ParsedP>
struct ParseResult
{
  ParsedP parsed;
  ParseState state;
};

auto
parse_leaf_basic_char (char c) -> std::optional<Leaf::Basic>
{
  switch (c)
  {
  case 'b':
    return {{Leaf::bool_}};
  case 'y':
    return {{Leaf::byte_}};
  case 'n':
    return {{Leaf::i16}};
  case 'q':
    return {{Leaf::u16}};
  case 'i':
    return {{Leaf::i32}};
  case 'u':
    return {{Leaf::u32}};
  case 'x':
    return {{Leaf::i64}};
  case 't':
    return {{Leaf::u64}};
  case 'h':
    return {{Leaf::handle}};
  case 'd':
    return {{Leaf::double_}};
  default:
    return {};
  }
}

auto
parse_leaf_string_type_char (char c) -> std::optional<Leaf::StringType>
{
  switch (c)
  {
  case 's':
    return {{Leaf::string_}};
  case 'o':
    return {{Leaf::object_path}};
  case 'g':
    return {{Leaf::signature}};
  default:
    return {};
  }
}

using ParseTypeResult = ParseResult<VariantType>;

auto
parse_single_type (ParseState state) -> VariantResult<ParseTypeResult>;

using ParseEntryKeyTypeResult = ParseResult<VT::EntryKeyType>;

auto
parse_entry_key_type (ParseState state) -> VariantResult<ParseEntryKeyTypeResult>
{
  auto maybe_c {state.take_one ()};

  if (!maybe_c)
  {
    return {{state.error ("expected entry key type, got premature end of a string")}};
  }

  auto maybe_basic_type {parse_leaf_basic_char (*maybe_c)};

  if (maybe_basic_type)
  {
    return {{ParseEntryKeyTypeResult{{*maybe_basic_type}, state}}};
  }

  auto maybe_string_type {parse_leaf_string_type_char (*maybe_c)};

  if (maybe_string_type)
  {
    return {{ParseEntryKeyTypeResult{{*maybe_string_type}, state}}};
  }

  if (*maybe_c == '?')
  {
    return {{ParseEntryKeyTypeResult{{Leaf::any_basic}, state}}};
  }

  std::ostringstream oss;

  oss << "expected either a basic type, a string type or ?, got " << *maybe_c;
  return {{state.error (oss.str ())}};
}

using ParseEntryTypeResult = ParseResult<VT::Entry>;

auto
parse_entry_type (ParseState state) -> VariantResult<ParseEntryTypeResult>
{
  auto maybe_first_result {parse_entry_key_type (state)};
  if (!maybe_first_result)
  {
    return {{state.error ("failed to parse first type for an entry", maybe_first_result.get_failure ())}};
  }
  auto maybe_second_result {parse_single_type (maybe_first_result->state)};
  if (!maybe_second_result)
  {
    return {{state.error ("failed to parse second type for an entry", maybe_second_result.get_failure ())}};
  }
  auto maybe_c {maybe_second_result->state.take_one ()};
  if (!maybe_c)
  {
    return {{state.error ("expected '}', got premature end of a string")}};
  }
  if (*maybe_c != '}')
  {
    std::ostringstream oss;

    oss << "expected '}', got " << *maybe_c;
    return {{state.error (oss.str ())}};
  }
  return {{ParseEntryTypeResult{{std::move (maybe_first_result->parsed), std::move (maybe_second_result->parsed)}, maybe_second_result->state}}};
}

using ParseTupleTypeResult = ParseResult<VT::Tuple>;

auto
parse_tuple_type (ParseState state) -> VariantResult<ParseTupleTypeResult>
{
  std::vector<VariantType> types {};
  for (;;)
  {
    auto maybe_c {state.take_one ()};
    if (!maybe_c)
    {
      return {{state.error ("expected either a type or ')', got premature end of a string")}};
    }
    if (*maybe_c == ')')
    {
      return {{ParseTupleTypeResult{std::move (types), state}}};
    }
    state.take_back ();
    auto maybe_result {parse_single_type (state)};
    if (!maybe_result)
    {
      std::ostringstream oss;

      oss << "failed to parse type number " << types.size() + 1 << " of a tuple";
      return {{state.error (oss.str(), maybe_result.get_failure ())}};
    }
    state = std::move (maybe_result->state);
    types.push_back (std::move (maybe_result->parsed));
  }
}

auto
parse_single_type (ParseState state) -> VariantResult<ParseTypeResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {{state.error ("expected a type, got premature end of a string")}};
  }

  auto maybe_basic_type {parse_leaf_basic_char (*maybe_c)};

  if (maybe_basic_type)
  {
    return {{ParseTypeResult{{*maybe_basic_type}, state}}};
  }

  auto maybe_string_type {parse_leaf_string_type_char (*maybe_c)};

  if (maybe_string_type)
  {
    return {{ParseTypeResult{{*maybe_string_type}, state}}};
  }

  switch (*maybe_c)
  {
  case '{':
    {
      auto maybe_result {parse_entry_type (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse entry type", maybe_result.get_failure ())}};
      }
      return {{ParseTypeResult{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_type (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse tuple type", maybe_result.get_failure ())}};
      }
      return {{ParseTypeResult{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  case 'm':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse maybe type", maybe_result.get_failure ())}};
      }
      return {{ParseTypeResult{{VT::Maybe {std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}}};
    }
  case 'a':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse array type", maybe_result.get_failure ())}};
      }
      return {{ParseTypeResult{{VT::Array {std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}}};
    }
  case '*':
    return {{ParseTypeResult{{Leaf::any_type}, std::move (state)}}};
  case 'r':
    return {{ParseTypeResult{{Leaf::any_tuple}, std::move (state)}}};
  case 'v':
    return {{ParseTypeResult{{Leaf::variant}, std::move (state)}}};
  case '?':
    return {{ParseTypeResult{{Leaf::any_basic}, std::move (state)}}};
  default:
    {
      std::ostringstream oss;

      oss << "failed to parse type, expected '{', '(', 'm', 'a', '*', 'r', 'v', '?', a basic type or a string type, got " << *maybe_c;
      return {{state.error (oss.str())}};
    }
  }
}

} // anonymous namespace

/* static */ auto
VariantType::from_string (std::string_view const& string) -> VariantResult<VariantType>
{
  auto state {ParseState{string, 0}};
  auto maybe_result {parse_single_type (state)};

  if (!maybe_result)
  {
    return {{state.error ("failed to parse variant type", maybe_result.get_failure ())}};
  }
  if (auto rest {maybe_result->state.get_rest()}; !rest.empty ())
  {
    std::ostringstream oss;

    oss << "string contains more than one complete type: " << rest;
    return {{maybe_result->state.error(oss.str())}};
  }

  return {std::move (maybe_result->parsed)};
}

namespace
{

auto
maybe_is_definite (VT::Maybe const& maybe) -> bool
{
  return maybe.pointed_type->is_definite ();
}

auto
tuple_is_definite (VT::Tuple const& tuple) -> bool
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

auto
array_is_definite (VT::Array const& array) -> bool
{
  return array.element_type->is_definite ();
}

auto
entry_key_type_is_definite (VT::EntryKeyType const& entry_key_type) -> bool
{
  auto vh {VisitHelper {
    [](Leaf::AnyBasic const&) { return false; },
    [](Leaf::StringType const&) { return true; },
    [](Leaf::Basic const&) { return true; },
  }};

  return std::visit (vh, entry_key_type.v);
}

auto
entry_is_definite (VT::Entry const& entry) -> bool
{
  return entry_key_type_is_definite (entry.key) && entry.value->is_definite ();
}

} // anonymous namespace

auto
VariantType::is_definite () const -> bool
{
  auto vh {VisitHelper {
    [](Leaf::Basic const&) { return true; },
    [](Leaf::AnyBasic const&) { return false; },
    [](Leaf::StringType const&) { return true; },
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

auto
VariantType::get_class () const -> VC::Class
{
  auto vh {VisitHelper {
    [](Leaf::Basic const&) { return VC::Class {{VC::basic}}; },
    [](Leaf::AnyBasic const&) { return VC::Class {{VC::basic}}; },
    [](Leaf::StringType const&) { return VC::Class {{VC::basic}}; },
    [](VT::Maybe const&) { return VC::Class {{VC::maybe}}; },
    [](VT::Tuple const&) { return VC::Class {{VC::tuple}}; },
    [](VT::Array const&) { return VC::Class {{VC::array}}; },
    [](VT::Entry const&) { return VC::Class {{VC::entry}}; },
    [](Leaf::Variant const&) { return VC::Class {{VC::variant}}; },
    [](Leaf::AnyTuple const&) { return VC::Class {{VC::tuple}}; },
    [](Leaf::AnyType const&) { return VC::Class {{VC::any}}; },
  }};

  return std::visit (vh, this->v);
}

namespace
{

using ParsePointerFormatResult = ParseResult<VF::Pointer>;

auto
parse_pointer_format (ParseState state) -> VariantResult<ParsePointerFormatResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {{state.error ("expected pointer format, got premature end of a string")}};
  }
  switch (*maybe_c)
  {
  case 's':
    return {{ParsePointerFormatResult{{Leaf::string_}, std::move (state)}}};
  case 'o':
    return {{ParsePointerFormatResult{{Leaf::object_path}, std::move (state)}}};
  case 'g':
    return {{ParsePointerFormatResult{{Leaf::signature}, std::move (state)}}};
  default:
    {
      std::ostringstream oss;

      oss << "expected 's' or 'o' or 'g', got '" << *maybe_c << "'";

      return {{state.error (oss.str())}};
    }
  }
}

using ParseConvenienceFormatResult = ParseResult<VF::Convenience>;

// TODO: this is just ew.
auto
parse_convenience_format (ParseState state) -> VariantResult<ParseConvenienceFormatResult>
{
  auto maybe_c {state.take_one ()};

  if (!maybe_c)
  {
    return {{state.error ("expected convenience format, got premature end of a string")}};
  }

  switch (*maybe_c)
  {
    // a
  case 'a':
    {
      maybe_c = state.take_one ();

      if (!maybe_c)
      {
        return {{state.error ("expected convenience format, got premature end of a string in the middle for the format")}};
      }

      switch (*maybe_c)
      {
        // a&
      case '&':
        {
          maybe_c = state.take_one ();

          if (!maybe_c)
          {
            return {{state.error ("expected convenience format, got premature end of a string in the middle for the format")}};
          }

          switch (*maybe_c)
          {
            // a&a
          case 'a':
            {
              maybe_c = state.take_one ();

              if (!maybe_c)
              {
                return {{state.error ("expected convenience format, got premature end of a string in the middle for the format")}};
              }

              switch (*maybe_c)
              {
                // a&ay
              case 'y':
                return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::byte_string_array}, {VF::Convenience::Kind::constant}}, state}}};
              default:
                {
                  std::ostringstream oss;

                  oss << "expected 'y', got '" << *maybe_c << "'";

                  return {{state.error (oss.str ())}};
                }
              }
            }
            // a&o
          case 'o':
            return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::object_path_array}, {VF::Convenience::Kind::constant}}, state}}};
            // a&s
          case 's':
            return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::string_array}, {VF::Convenience::Kind::constant}}, state}}};
          default:
            {
              std::ostringstream oss;

              oss << "expected 'o' or 's', got '" << *maybe_c << "'";

              return {{state.error (oss.str ())}};
            }
          }
        }
        // aa
      case 'a':
        {
          maybe_c = state.take_one ();

          if (!maybe_c)
          {
            return {{state.error ("expected convenience format, got premature end of a string in the middle for the format")}};
          }

          switch (*maybe_c)
          {
            // aay
          case 'y':
            return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::byte_string_array}, {VF::Convenience::Kind::duplicated}}, state}}};
          default:
            {
              std::ostringstream oss;

              oss << "expected 'y', got '" << *maybe_c << "'";

              return {{state.error (oss.str ())}};
            }
          }
        }
        // as
      case 's':
        return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::string_array}, {VF::Convenience::Kind::duplicated}}, state}}};
        // ao
      case 'o':
        return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::object_path_array}, {VF::Convenience::Kind::duplicated}}, state}}};
        // ay
      case 'y':
        return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::byte_string}, {VF::Convenience::Kind::duplicated}}, state}}};
      default:
        {
          std::ostringstream oss;

          oss << "expected 's', 'o' or 'y', got '" << *maybe_c << "'";

          return {{state.error (oss.str ())}};
        }
      }
    }
    // &
  case '&':
    {
      maybe_c = state.take_one ();

      if (!maybe_c)
      {
        return {{state.error ("expected convenience format, got premature end of a string in the middle for the format")}};
      }

      switch (*maybe_c)
      {
        // &a
      case 'a':
        {
          maybe_c = state.take_one ();

          if (!maybe_c)
          {
            return {{state.error ("expected convenience format, got premature end of a string in the middle for the format")}};
          }

          switch (*maybe_c)
          {
            // &ay
          case 'y':
            return {{ParseConvenienceFormatResult{{{VF::Convenience::Type::byte_string}, {VF::Convenience::Kind::constant}}, state}}};
          default:
            {
              std::ostringstream oss;

              oss << "expected 'y', got '" << *maybe_c << "'";

              return {{state.error (oss.str ())}};
            }
          }
        }
      default:
        {
          std::ostringstream oss;

          oss << "expected 'a', got '" << *maybe_c << "'";

          return {{state.error (oss.str ())}};
        }
      }
    }
  default:
    {
      std::ostringstream oss;

      oss << "expected 'a' or '&', got '" << *maybe_c << "'";

      return {{state.error (oss.str ())}};
    }
  }
}

using ParseTupleFormat = ParseResult<VF::Tuple>;
using ParseFormatResult = ParseResult<VariantFormat>;

auto
parse_single_format (ParseState state) -> VariantResult<ParseFormatResult>;

auto
parse_tuple_format (ParseState state) -> VariantResult<ParseTupleFormat>
{
  std::vector<VariantFormat> formats {};

  for (;;)
  {
    auto maybe_c {state.take_one ()};
    if (!maybe_c)
    {
      return {{state.error ("expected either a format or ')', got premature end of a string")}};
    }
    if (*maybe_c == ')')
    {
      return {{ParseTupleFormat{std::move (formats), state}}};
    }
    state.take_back ();
    auto maybe_result {parse_single_format (state)};
    if (!maybe_result)
    {
      std::ostringstream oss;

      oss << "failed to parse format number " << formats.size() + 1 << " of a tuple";
      return {{state.error (oss.str(), maybe_result.get_failure ())}};
    }
    state = std::move (maybe_result->state);
    formats.push_back (std::move (maybe_result->parsed));
  }
}

using ParseEntryKeyFormatResult = ParseResult<VF::EntryKeyFormat>;

auto
parse_entry_key_format (ParseState state) -> VariantResult<ParseEntryKeyFormatResult>
{
  auto maybe_c {state.take_one ()};

  if (!maybe_c)
  {
    return {{state.error ("expected either a format for entry key, got premature end of a string")}};
  }

  auto maybe_basic_type {parse_leaf_basic_char (*maybe_c)};

  if (maybe_basic_type)
  {
    return {{ParseEntryKeyFormatResult{{*maybe_basic_type}, state}}};
  }

  auto maybe_string_type {parse_leaf_string_type_char (*maybe_c)};

  if (maybe_string_type)
  {
    return {{ParseEntryKeyFormatResult{{*maybe_string_type}, state}}};
  }

  switch (*maybe_c)
  {
  case '@':
    {
      auto maybe_result {parse_entry_key_type (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse entry key format", maybe_result.get_failure ())}};
      }
      return {{ParseEntryKeyFormatResult{{VF::AtEntryKeyType {{std::move (maybe_result->parsed)}}}, maybe_result->state}}};
    }
  case '?':
    return {{ParseEntryKeyFormatResult{{VF::AtEntryKeyType {{{Leaf::any_basic}}}}, state}}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (state)};
      if (!maybe_result)
      {
        return {{state.error("failed to parse entry key format", maybe_result.get_failure ())}};
      }
      return {{ParseEntryKeyFormatResult{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  default:
    {
      std::ostringstream oss;

      oss << "expected '}', got " << *maybe_c;
      return {{state.error (oss.str ())}};
    }
  }
}

using ParseEntryFormatResult = ParseResult<VF::Entry>;

auto
parse_entry_format (ParseState state) -> VariantResult<ParseEntryFormatResult>
{
  auto maybe_first_result {parse_entry_key_format (state)};
  if (!maybe_first_result)
  {
    return {{state.error ("failed to parse first format for an entry", maybe_first_result.get_failure ())}};
  }
  auto maybe_second_result {parse_single_format (maybe_first_result->state)};
  if (!maybe_second_result)
  {
    return {{state.error ("failed to parse second format for an entry", maybe_second_result.get_failure ())}};
  }
  auto maybe_c {maybe_second_result->state.take_one()};
  if (!maybe_c)
  {
    return {{state.error ("expected '}', got premature end of a string")}};
  }
  if (*maybe_c != '}')
  {
    std::ostringstream oss;

    oss << "expected '}', got " << *maybe_c;
    return {{state.error (oss.str ())}};
  }
  return {{ParseEntryFormatResult{{std::move (maybe_first_result->parsed), std::move (maybe_second_result->parsed)}, maybe_second_result->state}}};
}

using ParseMaybeFormatResult = ParseResult<VF::Maybe>;

auto
parse_maybe_format (ParseState state) -> VariantResult<ParseMaybeFormatResult>
{
  auto maybe_c {state.take_one()};
  if (!maybe_c)
  {
    return {{state.error ("expected either a maybe format, got premature end of a string")}};
  }

  auto maybe_basic_type {parse_leaf_basic_char (*maybe_c)};

  if (maybe_basic_type)
  {
    return {{ParseMaybeFormatResult{{VF::MaybeBool {{*maybe_basic_type}}}, std::move (state)}}};
  }

  auto maybe_string_type {parse_leaf_string_type_char (*maybe_c)};

  if (maybe_string_type)
  {
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{*maybe_string_type}}}, std::move (state)}}};
  }

  switch (*maybe_c)
  {
    // pointer maybes
  case 'a':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {{state.error("failed to parse maybe array format", maybe_result.get_failure ())}};
      }
      return {{ParseMaybeFormatResult{{VF::MaybePointer {{VT::Array {{std::move (maybe_result->parsed)}}}}}, std::move (maybe_result->state)}}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {{state.error("failed to parse maybe format", maybe_result.get_failure ())}};
      }
      return {{ParseMaybeFormatResult{{VF::MaybePointer {{VF::AtVariantType {std::move (maybe_result->parsed)}}}}, std::move (maybe_result->state)}}};
    }
  case 'v':
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{Leaf::variant}}}, std::move (state)}}};
  case '*':
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_type}}}}}, std::move (state)}}};
  case 'r':
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_tuple}}}}}, std::move (state)}}};
  case '?':
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_basic}}}}}, std::move (state)}}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (state)};
      if (!maybe_result)
      {
        return {{state.error("failed to parse maybe format", maybe_result.get_failure ())}};
      }
      return {{ParseMaybeFormatResult{{VF::MaybePointer {{std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (state)};
      if (!maybe_result)
      {
        return {{state.error("failed to parse maybe convenience format", maybe_result.get_failure ())}};
      }
      return {{ParseMaybeFormatResult{{VF::MaybePointer {{std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}}};
    }
    // bool maybes
  case '{':
    {
      auto maybe_result {parse_entry_format (state)};
      if (!maybe_result)
      {
        return {{state.error("failed to parse maybe entry format", maybe_result.get_failure ())}};
      }
      return {{ParseMaybeFormatResult{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (std::move (state))};
      if (!maybe_result)
      {
        return {{state.error("failed to parse maybe tuple format", maybe_result.get_failure ())}};
      }
      return {{ParseMaybeFormatResult{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (state)};
      if (!maybe_result)
      {
        return {{state.error("failed to parse maybe maybe format", maybe_result.get_failure ())}};
      }
      return {{ParseMaybeFormatResult{{VF::MaybeBool {{std::move (maybe_result->parsed)}}}, std::move (maybe_result->state)}}};
    }
  default:
    {
      std::ostringstream oss;

      oss << "expected 'a', '@', 'v', '*', 'r', '&', '^', '{', '(', 'm' '?', a basic type or a string type, got " << *maybe_c;
      return {{state.error (oss.str ())}};
    }
  }
}

auto
parse_single_format (ParseState state) -> VariantResult<ParseFormatResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {{state.error ("expected a format, got premature end of a string")}};
  }

  auto maybe_basic_type {parse_leaf_basic_char (*maybe_c)};

  if (maybe_basic_type)
  {
    return {{ParseFormatResult{{*maybe_basic_type}, state}}};
  }

  auto maybe_string_type {parse_leaf_string_type_char (*maybe_c)};

  if (maybe_string_type)
  {
    return {{ParseFormatResult{{*maybe_string_type}, state}}};
  }

  switch (*maybe_c)
  {
  case 'a':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse array format", maybe_result.get_failure ())}};
      }
      return {{ParseFormatResult{VT::Array {std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  case '@':
    {
      auto maybe_result {parse_single_type (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse at format", maybe_result.get_failure ())}};
      }
      return {{ParseFormatResult{VF::AtVariantType {std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  case 'v':
    return {{ParseFormatResult{Leaf::variant, std::move (state)}}};
  case 'r':
    return {{ParseFormatResult{VF::AtVariantType {Leaf::any_tuple}, std::move (state)}}};
  case '*':
    return {{ParseFormatResult{VF::AtVariantType {Leaf::any_type}, std::move (state)}}};
  case '?':
    return {{ParseFormatResult{VF::AtVariantType {Leaf::any_basic}, std::move (state)}}};
  case '&':
    {
      auto maybe_result {parse_pointer_format (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse pointer format", maybe_result.get_failure ())}};
      }
      return {{ParseFormatResult{std::move (maybe_result->parsed), std::move (maybe_result->state)}}};
    }
  case '^':
    {
      auto maybe_result {parse_convenience_format (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse convenience format", maybe_result.get_failure ())}};
      }
      return {{ParseFormatResult{std::move (maybe_result->parsed), std::move (maybe_result->state)}}};
    }
  case 'm':
    {
      auto maybe_result {parse_maybe_format (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse maybe format", maybe_result.get_failure ())}};
      }
      return {{ParseFormatResult{std::move (maybe_result->parsed), std::move (maybe_result->state)}}};
    }
  case '(':
    {
      auto maybe_result {parse_tuple_format (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse tuple format", maybe_result.get_failure ())}};
      }
      return {{ParseFormatResult{std::move (maybe_result->parsed), std::move (maybe_result->state)}}};
    }
  case '{':
    {
      auto maybe_result {parse_entry_format (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse entry format", maybe_result.get_failure ())}};
      }
      return {{ParseFormatResult{std::move (maybe_result->parsed), std::move (maybe_result->state)}}};
    }
  default:
    {
      std::ostringstream oss;

      oss << "failed to parse type, expected 'a', '@', 'v', 'r', '*', '&', '^', 'm', '(', '{', '?', a basic type or a string type, got " << *maybe_c;
      return {{state.error (oss.str())}};
    }
  }
}

} // anonymous namespace

/* static */ auto
VariantFormat::from_string (std::string_view const& string) -> VariantResult<VariantFormat>
{
  auto state {ParseState{string, 0}};
  auto maybe_result {parse_single_format (state)};

  if (!maybe_result)
  {
    return {{state.error ("failed to parse variant format", maybe_result.get_failure ())}};
  }
  if (auto rest {maybe_result->state.get_rest ()}; !rest.empty ())
  {
    std::ostringstream oss;

    oss << "string contains more than one complete format: " << rest;
    return {{maybe_result->state.error(oss.str())}};
  }

  return {std::move (maybe_result->parsed)};
}

namespace
{

auto
convenience_to_variant_type (VF::Convenience const& convenience) -> VariantType
{
  auto vh {VisitHelper {
    [](VF::Convenience::Type::StringArray const&) { return VariantType {{VT::Array {{VariantType {{Leaf::StringType {{Leaf::string_}}}}}}}}; },
    [](VF::Convenience::Type::ObjectPathArray const&) { return VariantType {{VT::Array {{VariantType {{Leaf::StringType {{Leaf::object_path}}}}}}}}; },
    [](VF::Convenience::Type::ByteString const&) { return VariantType {{VT::Array {{VariantType {{Leaf::Basic {{Leaf::byte_}}}}}}}}; },
    [](VF::Convenience::Type::ByteStringArray const&) { return VariantType {{VT::Array {{VariantType {{VT::Array {{VariantType {{Leaf::Basic {Leaf::byte_}}}}}}}}}}}; },
  }};
  return std::visit (vh, convenience.type.v);
}

auto
maybe_pointer_to_variant_type (VF::MaybePointer const& mp) -> VariantType
{
  auto vh {VisitHelper {
    [](VT::Array const& array) { return VariantType {{array}}; },
    [](Leaf::StringType const& string_type) { return VariantType {{string_type}}; },
    [](Leaf::Variant const& variant) { return VariantType {{variant}}; },
    [](VF::AtVariantType const& avt) { return avt.type; },
    [](VF::Pointer const& pointer) { return VariantType {{pointer.string_type}}; },
    [](VF::Convenience const& convenience) { return convenience_to_variant_type (convenience); },
  }};

  return {VT::Maybe {std::visit (vh, mp.v)}};
}

auto
entry_key_format_to_entry_key_type (VF::EntryKeyFormat const& entry_key_format) -> VT::EntryKeyType
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return VT::EntryKeyType{{basic}}; },
    [](Leaf::StringType const& string_type) { return VT::EntryKeyType{{string_type}}; },
    [](VF::AtEntryKeyType const& at) { return at.entry_key_type; },
    [](VF::Pointer const& pointer) { return VT::EntryKeyType{{pointer.string_type}}; },
  }};

  return std::visit (vh, entry_key_format.v);
}

auto
entry_to_variant_type (VF::Entry const& entry) -> VariantType
{
  return {{VT::Entry {entry_key_format_to_entry_key_type (entry.key), entry.value->to_type ()}}};
}

auto
tuple_to_variant_type (VF::Tuple const& tuple) -> VariantType
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

auto
maybe_to_variant_type (VF::Maybe const& maybe) -> VariantType;

auto
maybe_bool_to_variant_type (VF::MaybeBool const& mb) -> VariantType
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return VariantType{{basic}}; },
    [](VF::Entry const& entry) { return entry_to_variant_type (entry); },
    [](VF::Tuple const& tuple) { return tuple_to_variant_type (tuple); },
    [](VF::Maybe const& maybe) { return maybe_to_variant_type (maybe); },
  }};

  return {VT::Maybe {std::visit (vh, mb.v)}};
}

auto
maybe_to_variant_type (VF::Maybe const& maybe) -> VariantType
{
  auto vh {VisitHelper {
    [](VF::MaybePointer const& mp) { return maybe_pointer_to_variant_type (mp); },
    [](VF::MaybeBool const& mb) { return maybe_bool_to_variant_type (mb); },
  }};
  return std::visit (vh, maybe.v);
}

} // anonymous namespace

auto
VariantFormat::to_type () const -> VariantType
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return VariantType {{basic}}; },
    [](Leaf::StringType const& string_type) { return VariantType {{string_type}}; },
    [](Leaf::Variant const& variant) { return VariantType {{variant}}; },
    [](VT::Array const& array) { return VariantType {{array}}; },
    [](VF::AtVariantType const& avt) { return avt.type; },
    [](VF::Pointer const &pointer) { return VariantType {{pointer.string_type}}; },
    [](VF::Convenience const& convenience) { return convenience_to_variant_type (convenience); },
    [](VF::Maybe const& maybe) { return maybe_to_variant_type (maybe); },
    [](VF::Tuple const& tuple) { return tuple_to_variant_type (tuple); },
    [](VF::Entry const& entry) { return entry_to_variant_type (entry); },
  }};
  return std::visit (vh, this->v);
}

} // namespace Ggp::Lib
