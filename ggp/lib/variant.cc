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
/*< stl: sstream >*/

namespace Ggp::Lib
{

namespace
{

struct ParseState
{
  auto take_one () -> std::optional<char>;
  auto take_back () -> void;
  auto get_rest () -> std::string_view;

  auto error (std::string reason) -> VariantParseErrorCascade;
  auto error (std::string reason, VariantParseErrorCascade& error) -> VariantParseErrorCascade;

  std::string_view string;
  std::size_t offset;
};

auto ParseState::take_one () -> std::optional<char>
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

auto ParseState::take_back () -> void
{
  if (this->offset > 0)
  {
    --this->offset;
  }
}

auto ParseState::get_rest () -> std::string_view
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

auto ParseState::error (std::string reason) -> VariantParseErrorCascade
{
  return {{{{this->offset, std::move(reason)}}}};
}

auto ParseState::error (std::string reason, VariantParseErrorCascade& error) -> VariantParseErrorCascade
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

using ParseLeafBasicResult = ParseResult<Leaf::Basic>;

auto
parse_leaf_basic (ParseState state) -> VariantResult<ParseLeafBasicResult>
{
  auto maybe_c {state.take_one ()};

  if (!maybe_c)
  {
    return {{state.error ("expected basic type, got premature end of a string")}};
  }

  switch (*maybe_c)
  {
  case 'b':
    return {{ParseLeafBasicResult{{Leaf::bool_}, state}}};
  case 'y':
    return {{ParseLeafBasicResult{{Leaf::byte_}, state}}};
  case 'n':
    return {{ParseLeafBasicResult{{Leaf::i16}, state}}};
  case 'q':
    return {{ParseLeafBasicResult{{Leaf::u16}, state}}};
  case 'i':
    return {{ParseLeafBasicResult{{Leaf::i32}, state}}};
  case 'u':
    return {{ParseLeafBasicResult{{Leaf::u32}, state}}};
  case 'x':
    return {{ParseLeafBasicResult{{Leaf::i64}, state}}};
  case 't':
    return {{ParseLeafBasicResult{{Leaf::u64}, state}}};
  case 'h':
    return {{ParseLeafBasicResult{{Leaf::handle}, state}}};
  case 'd':
    return {{ParseLeafBasicResult{{Leaf::double_}, state}}};
  case 's':
    return {{ParseLeafBasicResult{{Leaf::string_}, state}}};
  case 'o':
    return {{ParseLeafBasicResult{{Leaf::object_path}, state}}};
  case 'g':
    return {{ParseLeafBasicResult{{Leaf::signature}, state}}};
  case '?':
    return {{ParseLeafBasicResult{{Leaf::any_basic}, state}}};
  default:
    {
      std::ostringstream oss;

      oss << "expected basic type, got '" << *maybe_c << "'";

      return {{state.error (oss.str())}};
    }
  }
}

using ParseTypeResult = ParseResult<VariantType>;

auto parse_single_type (ParseState state) -> VariantResult<ParseTypeResult>;

using ParseEntryTypeResult = ParseResult<VT::Entry>;

auto parse_entry_type (ParseState state) -> VariantResult<ParseEntryTypeResult>
{
  auto maybe_first_result {parse_leaf_basic (state)};
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

auto parse_tuple_type (ParseState state) -> VariantResult<ParseTupleTypeResult>
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

auto parse_single_type (ParseState state) -> VariantResult<ParseTypeResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {{state.error ("expected a type, got premature end of a string")}};
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
  default:
    {
      state.take_back();

      auto maybe_result {parse_leaf_basic (state)};

      if (!maybe_result)
      {
        std::ostringstream oss;

        oss << "failed to parse type, expected '{', '(', 'm', 'a', '*', 'r', 'v' or a basic type, got " << *maybe_c;
        return {{state.error (oss.str())}};
      }

      return {{ParseTypeResult{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  }
}

} // anonymous namespace

/* static */ VariantResult<VariantType>
VariantType::from_string (std::string_view const& string)
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

auto parse_pointer_format (ParseState state) -> VariantResult<ParsePointerFormatResult>
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

using ParseBasicFormatResult = ParseResult<VF::BasicFormat>;

auto parse_basic_format (ParseState state) -> VariantResult<ParseBasicFormatResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {{state.error ("expected basic format, got premature end of a string")}};
  }
  switch (*maybe_c)
  {
  case '@':
    {
      auto maybe_result {parse_leaf_basic (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse basic format", maybe_result.get_failure ())}};
      }
      return {{ParseBasicFormatResult{{VF::AtBasicType {std::move (maybe_result->parsed)}}, std::move (maybe_result->state)}}};
    }
  case '&':
    {
      auto maybe_result {parse_pointer_format (state)};
      if (!maybe_result)
      {
        return {{state.error ("failed to parse basic format", maybe_result.get_failure ())}};
      }
      return {{ParseBasicFormatResult{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  default:
    {
      state.take_back ();
      auto maybe_result {parse_leaf_basic (state)};
      if (!maybe_result)
      {
        std::ostringstream oss;

        oss << "failed to parse basic format, expected '@', '&' or a basic type, got " << *maybe_c;
        return {{state.error (oss.str())}};
      }
      return {{ParseBasicFormatResult{{std::move (maybe_result->parsed)}, std::move (maybe_result->state)}}};
    }
  }
}

using ParseConvenienceFormatResult = ParseResult<VF::Convenience>;

auto parse_convenience_format (ParseState state) -> VariantResult<ParseConvenienceFormatResult>
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
                return {{{{{VF::Convenience::Type::byte_string_array}}, {{VF::Convenience::Kind::constant}}}, state}};
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
            return {{{{{VF::Convenience::Type::object_path_array}}, {{VF::Convenience::Kind::constant}}}, state}};
            // a&s
          case 's':
            return {{{{{VF::Convenience::Type::string_array}}, {{VF::Convenience::Kind::constant}}}, state}};
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
            return {{{{{VF::Convenience::Type::byte_string_array}}, {{VF::Convenience::Kind::duplicated}}}, state}};
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
        return {{{{{VF::Convenience::Type::string_array}}, {{VF::Convenience::Kind::duplicated}}}, state}};
        // ao
      case 'o':
        return {{{{{VF::Convenience::Type::object_path_array}}, {{VF::Convenience::Kind::duplicated}}}, state}};
        // ay
      case 'y':
        return {{{{{VF::Convenience::Type::byte_string}}, {{VF::Convenience::Kind::duplicated}}}, state}};
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
            return {{{{{VF::Convenience::Type::byte_string}}, {{VF::Convenience::Kind::constant}}}, state}};
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

auto parse_single_format (ParseState state) -> VariantResult<ParseFormatResult>;

auto parse_tuple_format (ParseState state) -> VariantResult<ParseTupleFormat>
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
    auto maybe_result {parse_single_format (state)};
    if (!maybe_result)
    {
      std::ostringstream oss;

      oss << "failed to parse format number " << types.size() + 1 << " of a tuple";
      return {{state.error (oss.str(), maybe_result.get_failure ())}};
    }
    state = std::move (maybe_result->state);
    formats.push_back (std::move (maybe_result->parsed));
  }
}

using ParseEntryFormatResult = ParseResult<VF::Entry>;

auto parse_entry_format (ParseState state) -> VariantResult<ParseEntryFormatResult>
{
  auto maybe_first_result {parse_basic_format (state)};
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

auto parse_maybe_format (ParseState state) -> VariantResult<ParseMaybeFormatResult>
{
  auto maybe_c {state.take_one()};
  if (!maybe_c)
  {
    return {{state.error ("expected either a maybe format, got premature end of a string")}};
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
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{VF::AtVariantType {{Leaf::variant}}}}}, std::move (state)}}};
  case '*':
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_type}}}}}, std::move (state)}}};
  case 'r':
    return {{ParseMaybeFormatResult{{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_tuple}}}}}, std::move (state)}}};
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
    // basic maybes, need to decide whether a pointer or bool
  default:
    {
      state.take_back ();
      auto maybe_result {parse_leaf_basic (state)};
      if (!maybe_result)
      {
        return {{state.error("expected 'a', '@', 'v', '*', 'r', '&', '^', '{', '(', 'm' or a basic type", maybe_result.get_failure ())}};
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
      return {{ParseMaybeFormatResult{std::visit (vh, maybe_result->parsed.v), std::move (maybe_result->state)}}};
    }
  }
}

auto parse_single_format (ParseState state) -> VariantResult<ParseFormatResult>
{
  auto maybe_c {state.take_one ()};
  if (!maybe_c)
  {
    return {{state.error ("expected a format, got premature end of a string")}};
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
    return {{ParseFormatResult{VF::AtVariantType {Leaf::variant}, std::move (state)}}};
  case 'r':
    return {{ParseFormatResult{VF::AtVariantType {Leaf::any_tuple}, std::move (state)}}};
  case '*':
    return {{ParseFormatResult{VF::AtVariantType {Leaf::any_type}, std::move (state)}}};
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
      state.take_back ();
      auto maybe_result {parse_leaf_basic (state)};
      if (!maybe_result)
      {
        std::ostringstream oss;

        oss << "failed to parse type, expected 'a', '@', 'v', 'r', '*', '&', '^', 'm', '(', '{' or a basic type, got " << *maybe_c;
        return {{state.error (oss.str())}};
      }
      return {{ParseFormatResult{std::move (maybe_result->parsed), std::move (maybe_result->state)}}};
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
