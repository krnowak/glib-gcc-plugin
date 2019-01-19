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

auto
vtfs (char const* str) -> std::optional<VariantType>
{
  auto v {VariantType::from_string (str)};

  if (v)
  {
    return {std::move (*v)};
  }

  return {};
}

template <typename T>
auto
opt (T const& t) -> std::optional<T>
{
  return {t};
}

template <typename T>
auto
basict (T const& t) -> VariantType
{
  return {Leaf::Basic {{t}}};
}

template <typename T>
auto
stringt (T const& t) -> VariantType
{
  return {Leaf::StringType {{t}}};
}

auto none_t {std::optional<VariantType> {}};

} // anonymous namespace

TEST_CASE ("Variant types are parsed", "[variant]")
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

    CHECK (vtfs ("{") == none_t);
    CHECK (vtfs ("{s") == none_t);
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
    CHECK (vtfs ("k") == none_t);
  }
}

namespace
{

auto
dvt (char const* str) -> bool
{
  auto v {VariantType::from_string (str)};

  REQUIRE (v);

  return v->is_definite();
}

}

TEST_CASE ("Definiteness of variants", "[variant]")
{
  SECTION ("definite types")
  {
    SECTION ("basic types")
    {
      CHECK (dvt ("b"));
      CHECK (dvt ("y"));
      CHECK (dvt ("n"));
      CHECK (dvt ("q"));
      CHECK (dvt ("i"));
      CHECK (dvt ("u"));
      CHECK (dvt ("x"));
      CHECK (dvt ("t"));
      CHECK (dvt ("h"));
      CHECK (dvt ("d"));
    }

    SECTION ("string types")
    {
      CHECK (dvt ("s"));
      CHECK (dvt ("o"));
      CHECK (dvt ("g"));
    }

    SECTION ("other leaf types")
    {
      CHECK (dvt ("v"));
    }

    SECTION ("array types")
    {
      CHECK (dvt ("as"));
      CHECK (dvt ("aas"));
    }

    SECTION ("maybe types")
    {
      CHECK (dvt ("ms"));
      CHECK (dvt ("mms"));
    }

    SECTION ("tuple types")
    {
      CHECK (dvt ("()"));
      CHECK (dvt ("(ss)"));
      CHECK (dvt ("(asmv)"));
    }

    SECTION ("entry types")
    {
      CHECK (dvt ("{sv}"));
      CHECK (dvt ("{xmas}"));
    }
  }

  SECTION ("indefinite types")
  {

    SECTION ("other leaf types")
    {
      CHECK (!dvt ("?"));
      CHECK (!dvt ("r"));
      CHECK (!dvt ("*"));
    }

    SECTION ("array types")
    {
      CHECK (!dvt ("a*"));
      CHECK (!dvt ("aar"));
    }

    SECTION ("maybe types")
    {
      CHECK (!dvt ("m?"));
      CHECK (!dvt ("mm*"));
    }

    SECTION ("tuple types")
    {
      CHECK (!dvt ("(r)"));
      CHECK (!dvt ("(ss?)"));
      CHECK (!dvt ("(a*)"));
    }

    SECTION ("entry types")
    {
      CHECK (!dvt ("{?v}"));
      CHECK (!dvt ("{s*}"));
      CHECK (!dvt ("{?*}"));
    }
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

TEST_CASE ("Variant formats are parsed", "[variant]")
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

  SECTION ("convenience formats")
  {
    using ConType = VF::Convenience::Type;
    using ConKind = VF::Convenience::Kind;

    CHECK (vffs ("^a&ay") == opt (VariantFormat {{VF::Convenience {{ConType::byte_string_array}, {ConKind::constant}}}}));
    CHECK (vffs ("^a&o") == opt (VariantFormat {VF::Convenience {{ConType::object_path_array}, {ConKind::constant}}}));
    CHECK (vffs ("^a&s") == opt (VariantFormat {VF::Convenience {{ConType::string_array}, {ConKind::constant}}}));
    CHECK (vffs ("^aay") == opt (VariantFormat {VF::Convenience {{ConType::byte_string_array}, {ConKind::duplicated}}}));
    CHECK (vffs ("^as") == opt (VariantFormat {VF::Convenience {{ConType::string_array}, {ConKind::duplicated}}}));
    CHECK (vffs ("^ao") == opt (VariantFormat {VF::Convenience {{ConType::object_path_array}, {ConKind::duplicated}}}));
    CHECK (vffs ("^ay") == opt (VariantFormat {VF::Convenience {{ConType::byte_string}, {ConKind::duplicated}}}));
    CHECK (vffs ("^&ay") == opt (VariantFormat {VF::Convenience {{ConType::byte_string}, {ConKind::constant}}}));

    CHECK (vffs ("^") == none_f);
    CHECK (vffs ("^x") == none_f);
    CHECK (vffs ("^a") == none_f);
    CHECK (vffs ("^ax") == none_f);
    CHECK (vffs ("^a&") == none_f);
    CHECK (vffs ("^a&x") == none_f);
    CHECK (vffs ("^a&a") == none_f);
    CHECK (vffs ("^a&ax") == none_f);
    CHECK (vffs ("^aa") == none_f);
    CHECK (vffs ("^aax") == none_f);
    CHECK (vffs ("^&") == none_f);
    CHECK (vffs ("^&x") == none_f);
    CHECK (vffs ("^&a") == none_f);
    CHECK (vffs ("^&ax") == none_f);
  }

  SECTION ("maybe formats")
  {
    using ConType = VF::Convenience::Type;
    using ConKind = VF::Convenience::Kind;

    CHECK (vffs ("mas") == opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{VT::Array {{stringt (Leaf::string_)}}}}}}}}));
    CHECK (vffs ("ms") == opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{Leaf::StringType {{Leaf::string_}}}}}}}}));
    CHECK (vffs ("mv") ==  opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{Leaf::variant}}}}}}));
    CHECK (vffs ("m@s") ==  opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{VF::AtVariantType {{Leaf::StringType {{Leaf::string_}}}}}}}}}}));
    CHECK (vffs ("m*") ==  opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_type}}}}}}}}));
    CHECK (vffs ("m?") ==  opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_basic}}}}}}}}));
    CHECK (vffs ("mr") ==  opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{VF::AtVariantType {{Leaf::any_tuple}}}}}}}}));
    CHECK (vffs ("m&s") ==  opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{VF::Pointer {{Leaf::string_}}}}}}}}));
    CHECK (vffs ("m^as") ==  opt (VariantFormat {{VF::Maybe {{VF::MaybePointer {{VF::Convenience {{ConType::string_array}, {ConKind::duplicated}}}}}}}}));
    CHECK (vffs ("mb") == opt (VariantFormat {{VF::Maybe {{VF::MaybeBool {{Leaf::Basic {{Leaf::bool_}}}}}}}}));
    CHECK (vffs ("m{bs}") == opt (VariantFormat {{VF::Maybe {{VF::MaybeBool {{VF::Entry {{{Leaf::Basic {{Leaf::bool_}}}}, {stringf (Leaf::string_)}}}}}}}}));
    CHECK (vffs ("m(bb)") == opt (VariantFormat {{VF::Maybe {{VF::MaybeBool {{VF::Tuple {{basicf (Leaf::bool_), basicf (Leaf::bool_)}}}}}}}}));
    CHECK (vffs ("mmb") == opt (VariantFormat {{VF::Maybe {{VF::MaybeBool {{VF::Maybe {{VF::MaybeBool {{Leaf::Basic {{Leaf::bool_}}}}}}}}}}}}));

    CHECK (vffs ("m") == none_f);
    CHECK (vffs ("ma") == none_f);
    CHECK (vffs ("m@") == none_f);
    CHECK (vffs ("m^") == none_f);
    CHECK (vffs ("m&") == none_f);
    CHECK (vffs ("m{") == none_f);
    CHECK (vffs ("m(") == none_f);
    CHECK (vffs ("mm") == none_f);
    CHECK (vffs ("mk") == none_f);
  }

  SECTION ("entry formats")
  {
    CHECK (vffs ("{bb}") == opt (VariantFormat {{VF::Entry {{{Leaf::Basic {{Leaf::bool_}}}}, basicf (Leaf::bool_)}}}));
    CHECK (vffs ("{sb}") == opt (VariantFormat {{VF::Entry {{{Leaf::StringType {{Leaf::string_}}}}, basicf (Leaf::bool_)}}}));
    CHECK (vffs ("{@bb}") == opt (VariantFormat {{VF::Entry {{{VF::AtEntryKeyType {{{Leaf::Basic {{Leaf::bool_}}}}}}}, basicf (Leaf::bool_)}}}));
    CHECK (vffs ("{?b}") == opt (VariantFormat {{VF::Entry {{{VF::AtEntryKeyType {{{Leaf::any_basic}}}}}, basicf (Leaf::bool_)}}}));
    CHECK (vffs ("{&sb}") == opt (VariantFormat {{VF::Entry {{{VF::Pointer {{Leaf::string_}}}}, basicf (Leaf::bool_)}}}));

    CHECK (vffs ("{") == none_f);
    CHECK (vffs ("{s") == none_f);
    CHECK (vffs ("{ss") == none_f);
    CHECK (vffs ("{ss)") == none_f);
    CHECK (vffs ("{**}") == none_f);
    CHECK (vffs ("{@**}") == none_f);
    CHECK (vffs ("{&xx}") == none_f);
  }

  SECTION ("tuple formats")
  {
    CHECK (vffs ("()") == opt (VariantFormat {{VF::Tuple {{}}}}));
    CHECK (vffs ("(b)") == opt (VariantFormat {{VF::Tuple {{basicf (Leaf::bool_)}}}}));
    CHECK (vffs ("(bb)") == opt (VariantFormat {{VF::Tuple {{basicf (Leaf::bool_), basicf (Leaf::bool_)}}}}));

    CHECK (vffs ("(") == none_f);
    CHECK (vffs ("(b") == none_f);
    CHECK (vffs ("(}") == none_f);
  }

  SECTION ("other formats")
  {
    CHECK (vffs ("ss") == none_f);
  }
}

namespace
{

auto
vf2t (char const* str) -> VariantType {
  auto v {VariantFormat::from_string (str)};

  REQUIRE (v);

  return v->to_type ();
}

auto
vt (char const* str) -> VariantType {
  auto v {VariantType::from_string (str)};

  REQUIRE (v);

  return *v;
}

} // anonymous namespace

TEST_CASE ("Variant formats are converted to variant types", "[variant]")
{
  SECTION ("basic formats")
  {
    CHECK (vf2t ("b") == vt ("b"));
    CHECK (vf2t ("y") == vt ("y"));
    CHECK (vf2t ("n") == vt ("n"));
    CHECK (vf2t ("q") == vt ("q"));
    CHECK (vf2t ("i") == vt ("i"));
    CHECK (vf2t ("u") == vt ("u"));
    CHECK (vf2t ("x") == vt ("x"));
    CHECK (vf2t ("t") == vt ("t"));
    CHECK (vf2t ("h") == vt ("h"));
    CHECK (vf2t ("d") == vt ("d"));
  }

  SECTION ("string type formats")
  {
    CHECK (vf2t ("s") == vt ("s"));
    CHECK (vf2t ("o") == vt ("o"));
    CHECK (vf2t ("g") == vt ("g"));
  }

  SECTION ("variant formats")
  {
    CHECK (vf2t ("v") == vt ("v"));
  }

  SECTION ("array formats")
  {
    CHECK (vf2t ("as") == vt ("as"));
  }

  SECTION ("at variant type formats")
  {
    CHECK (vf2t ("r") == vt ("r"));
    CHECK (vf2t ("*") == vt ("*"));
    CHECK (vf2t ("?") == vt ("?"));
    CHECK (vf2t ("@r") == vt ("r"));
    CHECK (vf2t ("@*") == vt ("*"));
    CHECK (vf2t ("@?") == vt ("?"));
    CHECK (vf2t ("@s") == vt ("s"));
    CHECK (vf2t ("@v") == vt ("v"));
  }

  SECTION ("pointer formats")
  {
    CHECK (vf2t ("&s") == vt ("s"));
    CHECK (vf2t ("&o") == vt ("o"));
    CHECK (vf2t ("&g") == vt ("g"));
  }

  SECTION ("convenience formats")
  {
    CHECK (vf2t ("^a&ay") == vt ("aay"));
    CHECK (vf2t ("^a&o") == vt ("ao"));
    CHECK (vf2t ("^a&s") == vt ("as"));
    CHECK (vf2t ("^aay") == vt ("aay"));
    CHECK (vf2t ("^as") == vt ("as"));
    CHECK (vf2t ("^ao") == vt ("ao"));
    CHECK (vf2t ("^ay") == vt ("ay"));
    CHECK (vf2t ("^&ay") == vt ("ay"));
  }

  SECTION ("maybe formats")
  {
    CHECK (vf2t ("mai") == vt ("mai"));
    CHECK (vf2t ("ms") == vt ("ms"));
    CHECK (vf2t ("mv") == vt ("mv"));
    CHECK (vf2t ("m*") == vt ("m*"));
    CHECK (vf2t ("m@s") == vt ("ms"));
    CHECK (vf2t ("m&s") == vt ("ms"));
    CHECK (vf2t ("m^as") == vt ("mas"));
    CHECK (vf2t ("mb") == vt ("mb"));
    CHECK (vf2t ("m{?*}") == vt ("m{?*}"));
    CHECK (vf2t ("m(bb)") == vt ("m(bb)"));
    CHECK (vf2t ("mmb") == vt ("mmb"));
  }

  SECTION ("entry formats")
  {
    CHECK (vf2t ("{bs}") == vt ("{bs}"));
    CHECK (vf2t ("{ss}") == vt ("{ss}"));
    CHECK (vf2t ("{?s}") == vt ("{?s}"));
    CHECK (vf2t ("{@bs}") == vt ("{bs}"));
    CHECK (vf2t ("{@ss}") == vt ("{ss}"));
    CHECK (vf2t ("{&ss}") == vt ("{ss}"));
    CHECK (vf2t ("{&os}") == vt ("{os}"));
    CHECK (vf2t ("{&gs}") == vt ("{gs}"));
  }

  SECTION ("tuple formats")
  {
    CHECK (vf2t ("()") == vt ("()"));
    CHECK (vf2t ("(@s&s)") == vt ("(ss)"));
  }
}
