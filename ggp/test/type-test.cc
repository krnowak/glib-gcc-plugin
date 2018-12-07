#include "catch.hpp"

#include "ggp/test/generated/type.hh"
#include "ggp/test/generated/variant.hh"

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

// variant types
auto bvt() -> std::vector<Types>
{
  return vt (TypeInfo{{VariantType{{Leaf::Basic {{Leaf::any_basic}}}}}});
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
    CHECK (tfs ("v") == bvt ());
    CHECK (tfs ("r") == bvt ());
    CHECK (tfs ("*") == bvt ());
    CHECK (tfs ("?") == bvt ());
    CHECK (tfs ("@b") == bvt ());
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
