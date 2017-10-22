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

namespace Ggp
{

namespace
{

struct ParseTypeResult
{
  VariantType type;
  std::string_view rest;
};

struct ParseBasicTypeResult
{
  VT::Basic basic;
  std::string_view rest;
};

std::optional<ParseBasicTypeResult>
parse_basic_type (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }

  auto rest = string.substr (1);
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

std::optional<ParseTypeResult>
parse_single_type (std::string_view const& string)
{
  if (string.empty ())
  {
    return {};
  }

  auto rest = string.substr (1);
  switch (string.front ())
  {
  case '{':
    {
      auto maybe_first_result {parse_basic_type (rest)};
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
      return {{VT::Entry {std::move (maybe_first_result->basic), std::move (maybe_second_result->type)}, maybe_second_result->rest.substr (1)}};
    }
  case '(':
    {
      std::vector<VariantType> types {};
      for (;;)
      {
        if (rest.empty ())
        {
          return {};
        }
        if (rest.front () == ')')
        {
          return {{VT::Tuple {std::move (types)}, rest.substr (1)}};
        }
        auto maybe_result {parse_single_type (rest)};
        if (!maybe_result)
        {
          return {};
        }
        rest = std::move (maybe_result->rest);
        types.push_back (std::move (maybe_result->type));
      }
    }
  case 'm':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{VT::Maybe {std::move (maybe_result->type)}, std::move (maybe_result->rest)}};
    }
  case 'a':
    {
      auto maybe_result {parse_single_type (rest)};
      if (!maybe_result)
      {
        return {};
      }
      return {{VT::Array {std::move (maybe_result->type)}, std::move (maybe_result->rest)}};
    }
  case '*':
    return {{VT::AnyType {}, std::move (rest)}};
  case 'r':
    return {{VT::AnyTuple {}, std::move (rest)}};
  case 'v':
    return {{VT::Variant {}, std::move (rest)}};
  default:
    {
      auto maybe_result {parse_basic_type (string)};

      if (!maybe_result)
      {
        return {};
      }

      return {{std::move (maybe_result->basic), std::move (maybe_result->rest)}};
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

  return {std::move (maybe_result->type)};
}

namespace
{

struct ParseBasicFormatResult
{
  VF::BasicFormat basic;
  std::string_view rest;
};

std::optional<ParseBasicFormatResult>
parse_basic_format (std::string_view const& /* string */)
{
  // TODO
  return {};
}

struct ParseFormatResult
{
  VariantFormat format;
  std::string_view rest;
};

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
  case '@':
    {
      auto maybe_result = parse_single_type (rest);
      if (!maybe_result)
      {
        return {};
      }
      return {{VF::AtVariantType{std::move (maybe_result->type)}, std::move (maybe_result->rest)}};
    }
  case '&':
    {
      if (rest.empty ())
      {
        return {};
      }
      auto pointer_rest = rest.substr (1);
      switch (rest.front ())
      {
      case 's':
        return {{VF::Pointer::String, std::move (pointer_rest)}};
      case 'o':
        return {{VF::Pointer::ObjectPath, std::move (pointer_rest)}};
      case 'g':
        return {{VF::Pointer::Signature, std::move (pointer_rest)}};
      default:
        return {};
      }
    }
  case '^':
    {
      auto s4 = rest.substr(0, 4);
      if (s4 == "a&ay")
      {
        return {{VF::Convenience {VF::Convenience::Type::ByteStringArray, VF::Convenience::Kind::Constant}, rest.substr (4)}};
      }
      auto s3 = rest.substr(0, 3);
      if (s3 == "aay")
      {
        return {{VF::Convenience {VF::Convenience::Type::ByteStringArray, VF::Convenience::Kind::Duplicated}, rest.substr (3)}};
      }
      if (s3 == "&ay")
      {
        return {{VF::Convenience {VF::Convenience::Type::ByteString, VF::Convenience::Kind::Constant}, rest.substr (3)}};
      }
      if (s3 == "a&o")
      {
        return {{VF::Convenience {VF::Convenience::Type::ObjectPathArray, VF::Convenience::Kind::Constant}, rest.substr (3)}};
      }
      if (s3 == "a&s")
      {
        return {{VF::Convenience {VF::Convenience::Type::StringArray, VF::Convenience::Kind::Constant}, rest.substr (3)}};
      }
      auto s2 = rest.substr(0, 2);
      if (s2 == "as")
      {
        return {{VF::Convenience {VF::Convenience::Type::StringArray, VF::Convenience::Kind::Duplicated}, rest.substr (2)}};
      }
      if (s2 == "ao")
      {
        return {{VF::Convenience {VF::Convenience::Type::ObjectPathArray, VF::Convenience::Kind::Duplicated}, rest.substr (2)}};
      }
      if (s2 == "ay")
      {
        return {{VF::Convenience {VF::Convenience::Type::ByteString, VF::Convenience::Kind::Duplicated}, rest.substr (2)}};
      }
      return {};
    }
  case 'm':
    {
      // TODO
      return {};
    }
  case '(':
    {
      std::vector<VariantFormat> formats {};
      for (;;)
      {
        if (rest.empty ())
        {
          return {};
        }
        if (rest.front () == ')')
        {
          return {{VF::Tuple {std::move (formats)}, rest.substr (1)}};
        }
        auto maybe_result {parse_single_format (rest)};
        if (!maybe_result)
        {
          return {};
        }
        rest = std::move (maybe_result->rest);
        formats.push_back (std::move (maybe_result->format));
      }
      return {};
    }
  case '{':
    {
      auto maybe_first_result {parse_basic_format (rest)};
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
      return {{VF::Entry {std::move (maybe_first_result->basic), std::move (maybe_second_result->format)}, maybe_second_result->rest.substr (1)}};
    }
  default:
    {
      auto maybe_result = parse_single_type (rest);
      if (!maybe_result)
      {
        return {};
      }
      return {{std::move (maybe_result->type), std::move (maybe_result->rest)}};
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

  return {std::move (maybe_result->format)};
}

VariantType
variant_format_to_type (VariantFormat const& /* format */)
{
  // TODO
  return VariantType {VT::Basic::I32};
}

} // namespace Ggp
