#include "catch.hpp"

#include "ggp/test/generated/type.hh"

using namespace Ggp::Lib;

namespace
{

auto tfs(char const* str) -> Types
{
  auto v {VariantFormat::from_string (str)};

  REQUIRE(v.has_value());

  // TODO
}

} // anonymous namespace

TEST_CASE ("format to types", "[type]")
{
  SECTION ("basic types")
  {
    using namespace std::string_literals;

    CHECK (tfs ("b") == Types{{{PlainType{{Integral{"gboolean"s, {}, {"gint"s, "int"s}, sizeof(int), Signedness::Any}}}}}, {{NullablePointer{PlainType{{Integral{"gboolean"s, {}, {"gint"s, "int"s}, sizeof(int), Signedness::Any}}}}}}});
  }
  // TODO: See "gvariant format strings" in devhelp for information
  // about expected types for a format
}
