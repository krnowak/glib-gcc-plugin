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

auto tfs(char const* str) -> std::vector<Types>
{
  auto v {VariantFormat::from_string (str)};

  REQUIRE(v);

  return expected_types_for_format (*v);
}

// single integral type
auto sit(Integral const& i) -> std::vector<Types>
{
  return {Types{{{PlainType{{i}}}}, {{NullablePointer{PlainType{{i}}}}}}};
}

// single real type
auto srt(Real const& r) -> std::vector<Types>
{
  return {Types{{{PlainType{{r}}}}, {{NullablePointer{PlainType{{r}}}}}}};
}

// string types
auto st() -> std::vector<Types>
{
  return {Types{{Pointer{{Const{{PlainType{{type_gchar ()}}}}}}}, {NullablePointer{{Pointer{{PlainType{{type_gchar ()}}}}}}}}};
}

// variant types
auto vt(TypeInfo const &ti) -> std::vector<Types>
{
  return {Types{{Pointer{{PlainType{{VariantTyped{"GVariant"s, ti}}}}}}, {Pointer{{Pointer{{PlainType{{VariantTyped{"GVariant"s, ti}}}}}}}}}};
}

auto avt() -> std::vector<Types>
{
  return vt (TypeInfo{{VariantType{{Leaf::any_type}}}});
}

auto bvt() -> std::vector<Types>
{
  return vt (TypeInfo{{VariantType{{Leaf::any_basic}}}});
}

auto tvt() -> std::vector<Types>
{
  return vt (TypeInfo{{VariantType{{Leaf::any_tuple}}}});
}

auto vvt() -> std::vector<Types>
{
  return vt (TypeInfo{{VariantType{{Leaf::variant}}}});
}

} // anonymous namespace

TEST_CASE ("format to types", "[type]")
{
  SECTION ("basic types")
  {
    CHECK (tfs ("b") == sit(type_gboolean ()));
    CHECK (tfs ("y") == sit(type_guchar ()));
    CHECK (tfs ("n") == sit(type_gint16 ()));
    CHECK (tfs ("q") == sit(type_guint16 ()));
    CHECK (tfs ("i") == sit(type_gint32 ()));
    CHECK (tfs ("u") == sit(type_guint32 ()));
    CHECK (tfs ("x") == sit(type_gint64 ()));
    CHECK (tfs ("t") == sit(type_guint64 ()));
    CHECK (tfs ("h") == sit(type_handle ()));
    CHECK (tfs ("d") == srt(type_gdouble ()));
    CHECK (tfs ("s") == st ());
    CHECK (tfs ("o") == st ());
    CHECK (tfs ("g") == st ());
  }

  SECTION ("at types")
  {
    CHECK (tfs ("v") == vvt ());
    CHECK (tfs ("v") == tfs ("@v"));
    CHECK (tfs ("r") == tvt ());
    CHECK (tfs ("r") == tfs ("@r"));
    CHECK (tfs ("*") == avt ());
    CHECK (tfs ("*") == tfs ("@*"));
    CHECK (tfs ("?") == bvt ());
    CHECK (tfs ("?") == tfs ("@?"));
  }

  SECTION ("container types")
  {
  }

  SECTION ("pointer types")
  {
  }

  SECTION ("convenience types")
  {
  }
  // TODO: See "gvariant format strings" in devhelp for information
  // about expected types for a format
}
