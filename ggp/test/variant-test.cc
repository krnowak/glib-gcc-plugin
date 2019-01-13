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

/*

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

auto none_t {std::optional<VariantType> {}};

} // anonymous namespace

TEST_CASE ("Variant types are parsed", "[variant-types]")
{
  SECTION ("basic types")
  {
    CHECK (vtfs ("b") == opt (basict (Leaf::Bool {})));
    CHECK (vtfs ("y") == opt (basict (Leaf::Byte {})));
    CHECK (vtfs ("n") == opt (basict (Leaf::I16 {})));
    CHECK (vtfs ("q") == opt (basict (Leaf::U16 {})));
    CHECK (vtfs ("i") == opt (basict (Leaf::I32 {})));
    CHECK (vtfs ("u") == opt (basict (Leaf::U32 {})));
    CHECK (vtfs ("x") == opt (basict (Leaf::I64 {})));
    CHECK (vtfs ("t") == opt (basict (Leaf::U64 {})));
    CHECK (vtfs ("h") == opt (basict (Leaf::Handle {})));
    CHECK (vtfs ("d") == opt (basict (Leaf::Double {})));
    CHECK (vtfs ("s") == opt (basict (Leaf::String {})));
    CHECK (vtfs ("o") == opt (basict (Leaf::ObjectPath {})));
    CHECK (vtfs ("g") == opt (basict (Leaf::Signature {})));

    CHECK (vtfs ("k") == none_t);
  }

  SECTION ("other leaf types")
  {
    CHECK (vtfs ("v") == opt (VariantType {{Leaf::Variant {}}}));
    CHECK (vtfs ("*") == opt (VariantType {{Leaf::AnyType {}}}));
    CHECK (vtfs ("r") == opt (VariantType {{Leaf::AnyTuple {}}}));
    CHECK (vtfs ("?") == opt (VariantType {{Leaf::AnyBasic {}}}));
  }

  SECTION ("array types")
  {
    CHECK (vtfs ("as") == opt (VariantType {{VT::Array {{basict (Leaf::String {})}}}}));
    CHECK (vtfs ("aas") == opt (VariantType {{VT::Array {{VariantType {{VT::Array {{basict (Leaf::String {})}}}}}}}}));

    CHECK (vtfs ("a") == none_t);
    CHECK (vtfs ("aa") == none_t);
  }

  SECTION ("maybe types")
  {
    CHECK (vtfs ("ms") == opt (VariantType {{VT::Maybe {{basict (Leaf::String {})}}}}));
    CHECK (vtfs ("mms") == opt (VariantType {{VT::Maybe {{VariantType {{VT::Maybe {{basict (Leaf::String {})}}}}}}}}));
    CHECK (vtfs ("mas") == opt (VariantType {{VT::Maybe {{VariantType {{VT::Array {{basict (Leaf::String {})}}}}}}}}));
    CHECK (vtfs ("ams") == opt (VariantType {{VT::Array {{VariantType {{VT::Maybe {{basict (Leaf::String {})}}}}}}}}));

    CHECK (vtfs ("m") == none_t);
    CHECK (vtfs ("mm") == none_t);
  }

  SECTION ("tuple types")
  {
    CHECK (vtfs ("()") == opt (VariantType {{VT::Tuple {{}}}}));
    CHECK (vtfs ("(s)") == opt (VariantType {{VT::Tuple {{basict (Leaf::String {})}}}}));
    CHECK (vtfs ("(ss)") == opt (VariantType {{VT::Tuple {{basict (Leaf::String {}), basict (Leaf::String {})}}}}));
    CHECK (vtfs ("(ssmas)") == opt (VariantType {{VT::Tuple {{basict (Leaf::String {}), basict (Leaf::String {}), VariantType {{VT::Maybe {{VariantType {{VT::Array {{basict (Leaf::String {})}}}}}}}}}}}}));

    CHECK (vtfs ("(") == none_t);
    CHECK (vtfs ("(}") == none_t);
    CHECK (vtfs ("(a)") == none_t);
  }

  SECTION ("entry types")
  {
    CHECK (vtfs ("{ss}") == opt (VariantType {{VT::Entry {{{Leaf::String {}}}, {basict (Leaf::String {})}}}}));
    CHECK (vtfs ("{xmas}") == opt (VariantType {{VT::Entry {{{Leaf::I64 {}}}, {VariantType {{VT::Maybe {{VariantType {{VT::Array {{basict (Leaf::String {})}}}}}}}}}}}}));
    CHECK (vtfs ("{?*}") == opt (VariantType {{VT::Entry {{{Leaf::AnyBasic {}}}, {VariantType {{Leaf::AnyType {}}}}}}}));
    CHECK (vtfs ("a{?*}") == opt (VariantType {{VT::Array {{VariantType {{VT::Entry {{{Leaf::AnyBasic {}}}, {VariantType {{Leaf::AnyType {}}}}}}}}}}}));

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

auto none_f {std::optional<VariantFormat> {}};

} // anonymous namespace

TEST_CASE ("Variant formats are parsed", "[variant-formats]")
{
  SECTION ("basic formats")
  {
    CHECK (vffs ("b") == opt (basicf (Leaf::Bool {})));
    CHECK (vffs ("y") == opt (basicf (Leaf::Byte {})));
    CHECK (vffs ("n") == opt (basicf (Leaf::I16 {})));
    CHECK (vffs ("q") == opt (basicf (Leaf::U16 {})));
    CHECK (vffs ("i") == opt (basicf (Leaf::I32 {})));
    CHECK (vffs ("u") == opt (basicf (Leaf::U32 {})));
    CHECK (vffs ("x") == opt (basicf (Leaf::I64 {})));
    CHECK (vffs ("t") == opt (basicf (Leaf::U64 {})));
    CHECK (vffs ("h") == opt (basicf (Leaf::Handle {})));
    CHECK (vffs ("d") == opt (basicf (Leaf::Double {})));
    CHECK (vffs ("s") == opt (basicf (Leaf::String {})));
    CHECK (vffs ("o") == opt (basicf (Leaf::ObjectPath {})));
    CHECK (vffs ("g") == opt (basicf (Leaf::Signature {})));
    CHECK (vffs ("?") == opt (basicf (Leaf::AnyBasic {})));

    CHECK (vffs ("k") == none_f);
  }

  SECTION ("array formats")
  {
    CHECK (vffs ("as") == opt (VariantFormat {{VT::Array {{basict (Leaf::String {})}}}}));
    CHECK (vffs ("aas") == opt (VariantFormat {{VT::Array {{VariantType {{VT::Array {{basict (Leaf::String {})}}}}}}}}));

    CHECK (vffs ("a") == none_f);
    CHECK (vffs ("aa") == none_f);
  }

  SECTION ("at variant type formats")
  {
    CHECK (vffs ("v") == opt (VariantFormat {{VF::AtVariantType {{{Leaf::Variant {}}}}}}));
    CHECK (vffs ("r") == opt (VariantFormat {{VF::AtVariantType {{{Leaf::AnyTuple {}}}}}}));
    CHECK (vffs ("*") == opt (VariantFormat {{VF::AtVariantType {{{Leaf::AnyType {}}}}}}));

    CHECK (vffs ("@s") == opt (VariantFormat {{VF::AtVariantType {basict (Leaf::String {})}}}));

    CHECK (vffs ("@") == none_f);

    CHECK (vffs ("r") == vffs ("@r"));
    CHECK (vffs ("*") == vffs ("@*"));
    CHECK (vffs ("?") == vffs ("@?"));
    CHECK (vffs ("v") == vffs ("@v"));
  }

  SECTION ("pointer formats")
  {
    CHECK (vffs ("&s") == opt (VariantFormat {{VF::Pointer {{Leaf::String {}}}}}));
    CHECK (vffs ("&o") == opt (VariantFormat {{VF::Pointer {{Leaf::ObjectPath {}}}}}));
    CHECK (vffs ("&g") == opt (VariantFormat {{VF::Pointer {{Leaf::Signature {}}}}}));

    CHECK (vffs ("&i") == none_f);
    CHECK (vffs ("&ay") == none_f);
    CHECK (vffs ("&") == none_f);
  }

  // TODO: Convenience formats
  // TODO: Maybe formats
  // TODO: Tuple formats
  // TODO: Entry formats
}
*/
