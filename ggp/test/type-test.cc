/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2018, 2019 Krzesimir Nowak
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

#include "ggp/test/generated/type.hh"
#include "ggp/test/generated/variant.hh"

#include "ggp/test/test-print.hh"

#include "catch.hpp"

using namespace Ggp::Lib;

namespace
{

using namespace std::string_literals;

auto
tfs (char const* str) -> std::vector<Types>
{
  auto v {VariantFormat::from_string (str)};

  REQUIRE(v);

  return expected_types_for_format (*v);
}

auto
integral_type (Integral const& i) -> std::vector<Types>
{
  return {Types {{{PlainType {{i}}}}, {{NullablePointer {PlainType {{i}}}}}}};
}

auto
real_type (Real const& r) -> std::vector<Types>
{
  return {Types {{{PlainType {{r}}}}, {{NullablePointer {PlainType {{r}}}}}}};
}

auto
string_type () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}};
}

auto
custom_variant(TypeInfo const &ti) -> std::vector<Types>
{
  return {Types {{Pointer {{PlainType {{VariantTyped {"GVariant"s, ti}}}}}}, {NullablePointer {{Pointer {{PlainType {{VariantTyped {"GVariant"s, ti}}}}}}}}}};
}

auto
any_type_variant() -> std::vector<Types>
{
  return custom_variant (TypeInfo {{VariantType {{Leaf::any_type}}}});
}

auto
any_basic_variant() -> std::vector<Types>
{
  return custom_variant (TypeInfo {{VariantType {{Leaf::any_basic}}}});
}

auto
any_tuple_variant() -> std::vector<Types>
{
  return custom_variant (TypeInfo {{VariantType {{Leaf::any_tuple}}}});
}

auto
unspecified_variant () -> std::vector<Types>
{
  return custom_variant (TypeInfo {{variant_type_unspecified}});
}

auto
variant_variant() -> std::vector<Types>
{
  return custom_variant (TypeInfo {{VariantType {{Leaf::variant}}}});
}

auto
array_of_strings() -> std::vector<Types>
{
  return {Types {{NullablePointer {{PlainType {{VariantTyped {"GVariantBuilder", {{VariantType {{VT::Array {VariantType {{Leaf::StringType {{Leaf::string_}}}}}}}}}}}}}}}, {{NullablePointer {{Pointer {{PlainType {{VariantTyped {"GVariantIter", {{VariantType {{VT::Array {VariantType {{Leaf::StringType {{Leaf::string_}}}}}}}}}}}}}}}}}}}};
}

auto
array_of_any_basics() -> std::vector<Types>
{
  return {Types {{Pointer {{PlainType {{VariantTyped {"GVariantBuilder", {{VariantType {{VT::Array {VariantType {{Leaf::any_basic}}}}}}}}}}}}}, {{NullablePointer {{Pointer {{PlainType {{VariantTyped {"GVariantIter", {{VariantType {{VT::Array {VariantType {{Leaf::any_basic}}}}}}}}}}}}}}}}}};
}

auto
pointer_to_string () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
convenience_a_amp_a_y () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}}};
}

auto
convenience_a_amp_o () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}}};
}

auto
convenience_a_amp_s () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}}};
}

auto
convenience_a_a_y () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
convenience_a_s () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
convenience_a_o () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
convenience_a_y () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}};
}

auto
convenience_amp_a_y () -> std::vector<Types>
{
  return {Types {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}};
}

} // anonymous namespace

// TODO: See "gvariant format strings" in devhelp for information
// about expected types for a format
TEST_CASE ("format to types", "[type]")
{
  SECTION ("basic type formats")
  {
    CHECK (tfs ("b") == integral_type (type_gboolean ()));
    CHECK (tfs ("y") == integral_type (type_guchar ()));
    CHECK (tfs ("n") == integral_type (type_gint16 ()));
    CHECK (tfs ("q") == integral_type (type_guint16 ()));
    CHECK (tfs ("i") == integral_type (type_gint32 ()));
    CHECK (tfs ("u") == integral_type (type_guint32 ()));
    CHECK (tfs ("x") == integral_type (type_gint64 ()));
    CHECK (tfs ("t") == integral_type (type_guint64 ()));
    CHECK (tfs ("h") == integral_type (type_handle ()));
    CHECK (tfs ("d") == real_type (type_gdouble ()));
  }

  SECTION ("string type formats")
  {
    CHECK (tfs ("s") == string_type ());
    CHECK (tfs ("o") == string_type ());
    CHECK (tfs ("g") == string_type ());
  }

  SECTION ("variant type formats")
  {
    CHECK (tfs ("v") == unspecified_variant ());
  }

  SECTION ("array formats")
  {
    CHECK (tfs ("as") == array_of_strings ());
    CHECK (tfs ("a?") == array_of_any_basics ());
  }

  SECTION ("at variant type formats")
  {
    CHECK (tfs ("@v") == variant_variant ());
    CHECK (tfs ("r") == any_tuple_variant ());
    CHECK (tfs ("r") == tfs ("@r"));
    CHECK (tfs ("*") == any_type_variant ());
    CHECK (tfs ("*") == tfs ("@*"));
    CHECK (tfs ("?") == any_basic_variant ());
    CHECK (tfs ("?") == tfs ("@?"));
  }

  SECTION ("pointer formats")
  {
    CHECK (tfs ("&s") == pointer_to_string ());
    CHECK (tfs ("&o") == pointer_to_string ());
    CHECK (tfs ("&g") == pointer_to_string ());
  }

  SECTION ("convenience formats")
  {
    CHECK (tfs ("^a&ay") == convenience_a_amp_a_y ());
    CHECK (tfs ("^a&o") == convenience_a_amp_o ());
    CHECK (tfs ("^a&s") == convenience_a_amp_s ());
    CHECK (tfs ("^aay") == convenience_a_a_y ());
    CHECK (tfs ("^as") == convenience_a_s ());
    CHECK (tfs ("^ao") == convenience_a_o ());
    CHECK (tfs ("^ay") == convenience_a_y ());
    CHECK (tfs ("^&ay") == convenience_amp_a_y ());
  }

  SECTION ("maybe formats")
  {
    // TODO
  }

  SECTION ("entry formats")
  {
    // TODO
  }

  SECTION ("tuple formats")
  {
    // TODO
  }
}
