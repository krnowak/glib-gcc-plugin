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

#include "ggp/test/generated/variant.hh"
#include "ggp/test/test-print.hh"

#include "catch.hpp"

using namespace Ggp::Lib;

namespace
{

std::optional<VariantType>
vtfs (char const* str)
{
  auto v {VariantType::from_string (str)};

  if (v)
  {
    return {std::move (*v)};
  }

  return {};
}

template <typename T>
std::optional<T>
opt (T const& t)
{
  return {t};
}

template <typename T>
VariantType
basict (T const& t)
{
  return {Leaf::Basic {{t}}};
}

template <typename T>
VariantType
stringt (T const& t)
{
  return {Leaf::StringType {{t}}};
}

auto none_t {std::optional<VariantType> {}};

} // anonymous namespace

// TODO: coverage
TEST_CASE ("Variant types are parsed", "[variant-types]")
{
  SECTION ("basic types")
  {
    CHECK (vtfs ("b") == opt (basict (Leaf::bool_)));
    CHECK (vtfs ("y") == opt (basict (Leaf::byte_)));
    CHECK (vtfs ("n") == opt (basict (Leaf::i16)));
    CHECK (vtfs ("q") == opt (basict (Leaf::u16)));
    CHECK (vtfs ("i") == opt (basict (Leaf::i32)));
    CHECK (vtfs ("u") == opt (basict (Leaf::u32)));
    CHECK (vtfs ("x") == opt (basict (Leaf::i64)));
    CHECK (vtfs ("t") == opt (basict (Leaf::u64)));
    CHECK (vtfs ("h") == opt (basict (Leaf::handle)));
    CHECK (vtfs ("d") == opt (basict (Leaf::double_)));
  }

  SECTION ("string types")
  {
    CHECK (vtfs ("s") == opt (stringt (Leaf::string_)));
    CHECK (vtfs ("o") == opt (stringt (Leaf::object_path)));
    CHECK (vtfs ("g") == opt (stringt (Leaf::signature)));
  }

  SECTION ("other leaf types")
  {
    CHECK (vtfs ("v") == opt (VariantType {{Leaf::variant}}));
    CHECK (vtfs ("*") == opt (VariantType {{Leaf::any_type}}));
    CHECK (vtfs ("r") == opt (VariantType {{Leaf::any_tuple}}));
    CHECK (vtfs ("?") == opt (VariantType {{Leaf::any_basic}}));
  }

  SECTION ("array types")
  {
    CHECK (vtfs ("as") == opt (VariantType {{VT::Array {{stringt (Leaf::string_)}}}}));
    CHECK (vtfs ("aas") == opt (VariantType {{VT::Array {{VariantType {{VT::Array {{stringt (Leaf::string_)}}}}}}}}));

    CHECK (vtfs ("a") == none_t);
    CHECK (vtfs ("aa") == none_t);
  }

  SECTION ("maybe types")
  {
    CHECK (vtfs ("ms") == opt (VariantType {{VT::Maybe {{stringt (Leaf::string_)}}}}));
    CHECK (vtfs ("mms") == opt (VariantType {{VT::Maybe {{VariantType {{VT::Maybe {{stringt (Leaf::string_)}}}}}}}}));
    CHECK (vtfs ("mas") == opt (VariantType {{VT::Maybe {{VariantType {{VT::Array {{stringt (Leaf::string_)}}}}}}}}));
    CHECK (vtfs ("ams") == opt (VariantType {{VT::Array {{VariantType {{VT::Maybe {{stringt (Leaf::string_)}}}}}}}}));

    CHECK (vtfs ("m") == none_t);
    CHECK (vtfs ("mm") == none_t);
  }

  SECTION ("tuple types")
  {
    CHECK (vtfs ("()") == opt (VariantType {{VT::Tuple {{}}}}));
    CHECK (vtfs ("(s)") == opt (VariantType {{VT::Tuple {{stringt (Leaf::string_)}}}}));
    CHECK (vtfs ("(ss)") == opt (VariantType {{VT::Tuple {{stringt (Leaf::string_), stringt (Leaf::string_)}}}}));
    CHECK (vtfs ("(ssmas)") == opt (VariantType {{VT::Tuple {{stringt (Leaf::string_), stringt (Leaf::string_), VariantType {{VT::Maybe {{VariantType {{VT::Array {{stringt (Leaf::string_)}}}}}}}}}}}}));

    CHECK (vtfs ("(") == none_t);
    CHECK (vtfs ("(}") == none_t);
    CHECK (vtfs ("(a)") == none_t);
  }

  SECTION ("entry types")
  {
    CHECK (vtfs ("{ss}") == opt (VariantType {{VT::Entry {{{Leaf::StringType{{Leaf::string_}}}}, {stringt (Leaf::string_)}}}}));
    CHECK (vtfs ("{xmas}") == opt (VariantType {{VT::Entry {{{Leaf::Basic{{Leaf::i64}}}}, {VariantType {{VT::Maybe {{VariantType {{VT::Array {{stringt (Leaf::string_)}}}}}}}}}}}}));
    CHECK (vtfs ("{?*}") == opt (VariantType {{VT::Entry {{{Leaf::any_basic}}, {VariantType {{Leaf::any_type}}}}}}));
    CHECK (vtfs ("a{?*}") == opt (VariantType {{VT::Array {{VariantType {{VT::Entry {{{Leaf::any_basic}}, {VariantType {{Leaf::any_type}}}}}}}}}}));

    CHECK (vtfs ("{}") == none_t);
    CHECK (vtfs ("{s}") == none_t);
    CHECK (vtfs ("{rr}}") == none_t);
    CHECK (vtfs ("{sk}}") == none_t);
    CHECK (vtfs ("{si") == none_t);
    CHECK (vtfs ("{sii}") == none_t);
  }

  SECTION ("other")
  {
    CHECK (vtfs ("((this)(msg){is}a(good)(gvariant)(string)(right?))") != none_t);

    CHECK (vtfs ("ss") == none_t);
  }
}

namespace
{

std::optional<VariantFormat>
vffs (char const* str)
{
  auto v {VariantFormat::from_string (str)};

  if (v)
  {
    return {std::move (*v)};
  }

  return {};
}

template <typename T>
VariantFormat
basicf (T const& t)
{
  return {Leaf::Basic {{t}}};
}

template <typename T>
VariantFormat
stringf (T const& t)
{
  return {Leaf::StringType {{t}}};
}

auto none_f {std::optional<VariantFormat> {}};

} // anonymous namespace

TEST_CASE ("Variant formats are parsed", "[variant-formats]")
{
  SECTION ("basic formats")
  {
    CHECK (vffs ("b") == opt (basicf (Leaf::bool_)));
    CHECK (vffs ("y") == opt (basicf (Leaf::byte_)));
    CHECK (vffs ("n") == opt (basicf (Leaf::i16)));
    CHECK (vffs ("q") == opt (basicf (Leaf::u16)));
    CHECK (vffs ("i") == opt (basicf (Leaf::i32)));
    CHECK (vffs ("u") == opt (basicf (Leaf::u32)));
    CHECK (vffs ("x") == opt (basicf (Leaf::i64)));
    CHECK (vffs ("t") == opt (basicf (Leaf::u64)));
    CHECK (vffs ("h") == opt (basicf (Leaf::handle)));
    CHECK (vffs ("d") == opt (basicf (Leaf::double_)));
  }

  SECTION ("string type formats")
  {
    CHECK (vffs ("s") == opt (stringf (Leaf::string_)));
    CHECK (vffs ("o") == opt (stringf (Leaf::object_path)));
    CHECK (vffs ("g") == opt (stringf (Leaf::signature)));

    CHECK (vffs ("k") == none_f);
  }

  SECTION ("variant formats")
  {
    CHECK (vffs ("v") == opt (VariantFormat {{Leaf::variant}}));
  }

  SECTION ("array formats")
  {
    CHECK (vffs ("as") == opt (VariantFormat {{VT::Array {{stringt (Leaf::string_)}}}}));
    CHECK (vffs ("aas") == opt (VariantFormat {{VT::Array {{VariantType {{VT::Array {{stringt (Leaf::string_)}}}}}}}}));

    CHECK (vffs ("a") == none_f);
    CHECK (vffs ("aa") == none_f);
  }

  SECTION ("at variant type formats")
  {
    CHECK (vffs ("r") == opt (VariantFormat {{VF::AtVariantType {{{Leaf::any_tuple}}}}}));
    CHECK (vffs ("*") == opt (VariantFormat {{VF::AtVariantType {{{Leaf::any_type}}}}}));
    CHECK (vffs ("?") == opt (VariantFormat {{VF::AtVariantType {{{Leaf::any_basic}}}}}));

    CHECK (vffs ("@s") == opt (VariantFormat {{VF::AtVariantType {stringt (Leaf::string_)}}}));
    CHECK (vffs ("@v") == opt (VariantFormat {{VF::AtVariantType {Leaf::variant}}}));

    CHECK (vffs ("@") == none_f);

    CHECK (vffs ("r") == vffs ("@r"));
    CHECK (vffs ("*") == vffs ("@*"));
    CHECK (vffs ("?") == vffs ("@?"));
  }

  SECTION ("pointer formats")
  {
    CHECK (vffs ("&s") == opt (VariantFormat {{VF::Pointer {{Leaf::string_}}}}));
    CHECK (vffs ("&o") == opt (VariantFormat {{VF::Pointer {{Leaf::object_path}}}}));
    CHECK (vffs ("&g") == opt (VariantFormat {{VF::Pointer {{Leaf::signature}}}}));

    CHECK (vffs ("&i") == none_f);
    CHECK (vffs ("&ay") == none_f);
    CHECK (vffs ("&") == none_f);
  }

  // TODO: Convenience formats
  // TODO: Maybe formats
  // TODO: Tuple formats
  // TODO: Entry formats
}
