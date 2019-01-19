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
string_variant() -> std::vector<Types>
{
  return custom_variant (TypeInfo {{VariantType {{Leaf::StringType {{Leaf::string_}}}}}});
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

auto
maybe_string_array () -> std::vector<Types>
{
  return {Types {{NullablePointer {{PlainType {{VariantTyped {"GVariantBuilder", {{VariantType {{VT::Array {VariantType {{Leaf::StringType {{Leaf::string_}}}}}}}}}}}}}}}, {{NullablePointer {{Pointer {{PlainType {{VariantTyped {"GVariantIter", {{VariantType {{VT::Array {VariantType {{Leaf::StringType {{Leaf::string_}}}}}}}}}}}}}}}}}}}};
}

auto
maybe_string () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}};
}

auto
maybe_custom_variant(TypeInfo const &ti) -> std::vector<Types>
{
  return {Types {{NullablePointer {{PlainType {{VariantTyped {"GVariant"s, ti}}}}}}, {NullablePointer {{Pointer {{PlainType {{VariantTyped {"GVariant"s, ti}}}}}}}}}};
}

auto
maybe_unspecified_variant () -> std::vector<Types>
{
  return maybe_custom_variant (TypeInfo {{variant_type_unspecified}});
}

auto
maybe_any_type_variant() -> std::vector<Types>
{
  return maybe_custom_variant (TypeInfo {{VariantType {{Leaf::any_type}}}});
}

auto
maybe_any_basic_variant() -> std::vector<Types>
{
  return maybe_custom_variant (TypeInfo {{VariantType {{Leaf::any_basic}}}});
}

auto
maybe_any_tuple_variant() -> std::vector<Types>
{
  return maybe_custom_variant (TypeInfo {{VariantType {{Leaf::any_tuple}}}});
}

auto
maybe_variant_variant() -> std::vector<Types>
{
  return maybe_custom_variant (TypeInfo {{VariantType {{Leaf::variant}}}});
}

auto
maybe_string_variant() -> std::vector<Types>
{
  return maybe_custom_variant (TypeInfo {{VariantType {{Leaf::StringType {{Leaf::string_}}}}}});
}

auto
maybe_pointer_to_string () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
maybe_convenience_a_amp_a_y () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}}};
}

auto
maybe_convenience_a_amp_o () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}}};
}

auto
maybe_convenience_a_amp_s () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}}};
}

auto
maybe_convenience_a_a_y () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
maybe_convenience_a_s () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
maybe_convenience_a_o () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}}, {NullablePointer {{Pointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
maybe_convenience_a_y () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}};
}

auto
maybe_convenience_amp_a_y () -> std::vector<Types>
{
  return {Types {{NullablePointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{Const {{PlainType {{type_gchar ()}}}}}}}}}}};
}

auto
special_maybe_bool_types () -> Types
{
  return {{{PlainType {{type_gboolean ()}}}}, {NullablePointer {{PlainType {{type_gboolean ()}}}}}};
}

auto
maybe_integral_type (Integral const& i) -> std::vector<Types>
{
  return {special_maybe_bool_types (), Types {{{PlainType {{i}}}}, {{NullablePointer {PlainType {{i}}}}}}};
}

auto
maybe_real_type (Real const& r) -> std::vector<Types>
{
  return {special_maybe_bool_types (), Types {{{PlainType {{r}}}}, {{NullablePointer {PlainType {{r}}}}}}};
}

auto
maybe_any_basic_to_any_type_entry () -> std::vector<Types>
{
  auto any_basic_variant_types {any_basic_variant ()};
  auto any_type_variant_types {any_type_variant ()};

  return {special_maybe_bool_types (), any_basic_variant_types[0], any_type_variant_types[0]};
}

auto
maybe_bool_bool_tuple () -> std::vector<Types>
{
  auto bool_types {integral_type (type_gboolean ())};
  return {special_maybe_bool_types (), bool_types[0], bool_types[0]};
}

auto
maybe_maybe_string () -> std::vector<Types>
{
  return {special_maybe_bool_types (), Types {{NullablePointer {{Const {{PlainType {{type_gchar ()}}}}}}}, {NullablePointer {{Pointer {{PlainType {{type_gchar ()}}}}}}}}};
}

auto maybe_maybe_bool () -> std::vector<Types>
{
  auto bool_types {integral_type (type_gboolean ())};

  return {special_maybe_bool_types (), special_maybe_bool_types (), bool_types[0]};
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
    CHECK (tfs ("@s") == string_variant ());
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
    CHECK (tfs ("mas") == maybe_string_array ());
    CHECK (tfs ("ms") == maybe_string ());
    CHECK (tfs ("mv") == maybe_unspecified_variant ());
    CHECK (tfs ("m*") == maybe_any_type_variant ());
    CHECK (tfs ("m@*") == tfs ("m*"));
    CHECK (tfs ("m?") == maybe_any_basic_variant ());
    CHECK (tfs ("m@?") == tfs ("m?"));
    CHECK (tfs ("mr") == maybe_any_tuple_variant ());
    CHECK (tfs ("m@r") == tfs ("mr"));
    CHECK (tfs ("mv") == maybe_variant_variant ());
    CHECK (tfs ("m@v") == tfs ("mv"));
    CHECK (tfs ("ms") == maybe_string_variant ());
    CHECK (tfs ("m@s") == tfs ("ms"));
    CHECK (tfs ("m&s") == maybe_pointer_to_string ());
    CHECK (tfs ("m&o") == maybe_pointer_to_string ());
    CHECK (tfs ("m&g") == maybe_pointer_to_string ());
    CHECK (tfs ("m^a&ay") == maybe_convenience_a_amp_a_y ());
    CHECK (tfs ("m^a&o") == maybe_convenience_a_amp_o ());
    CHECK (tfs ("m^a&s") == maybe_convenience_a_amp_s ());
    CHECK (tfs ("m^aay") == maybe_convenience_a_a_y ());
    CHECK (tfs ("m^as") == maybe_convenience_a_s ());
    CHECK (tfs ("m^ao") == maybe_convenience_a_o ());
    CHECK (tfs ("m^ay") == maybe_convenience_a_y ());
    CHECK (tfs ("m^&ay") == maybe_convenience_amp_a_y ());
    CHECK (tfs ("mb") == maybe_integral_type (type_gboolean ()));
    CHECK (tfs ("md") == maybe_real_type (type_gdouble ()));
    CHECK (tfs ("m{?*}") == maybe_any_basic_to_any_type_entry ());
    CHECK (tfs ("m(bb)") == maybe_bool_bool_tuple ());
    CHECK (tfs ("mms") == maybe_maybe_string ());
    CHECK (tfs ("mmb") == maybe_maybe_bool ());
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
