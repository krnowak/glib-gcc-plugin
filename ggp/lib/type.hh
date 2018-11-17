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

/*< check: GGP_LIB_TYPE_HH_CHECK >*/
/*< lib: util.hh >*/
/*< lib: variant.hh >*/
/*< stl: cstdint >*/
/*< stl: string >*/
/*< stl: variant >*/
/*< stl: vector >*/

#ifndef GGP_LIB_TYPE_HH
#define GGP_LIB_TYPE_HH

#define GGP_LIB_TYPE_HH_CHECK_VALUE GGP_LIB_TYPE_HH_CHECK

namespace Ggp::Lib
{

enum class Signedness : std::uint8_t
{
  Signed,
  Unsigned,
  Any,
};

struct Integral
{
  std::string name;
  std::vector<std::string> additional_names;
  std::vector<std::string> accidental_names;
  std::uint8_t size_in_bytes;
  Signedness signedness;
};

struct Real
{
  std::string name;
  std::vector<std::string> additional_names;
  std::uint8_t size_in_bytes;
};

GGP_LIB_TRIVIAL_TYPE_WITH_OPS(VariantTypeUnspecified);

GGP_LIB_VARIANT_STRUCT(TypeInfo,
                       VariantTypeUnspecified,
                       VariantType);

struct VariantTyped
{
  std::string name;
  TypeInfo info;
};

GGP_LIB_VARIANT_STRUCT(PlainType,
                       Integral,
                       Real,
                       VariantTyped);

GGP_LIB_VARIANT_STRUCT(Const,
                       Value<Pointer>,
                       PlainType);

GGP_LIB_VARIANT_STRUCT(Pointer,
                       Value<Pointer>,
                       Const,
                       PlainType);


struct NullablePointer : Pointer
{};

// For types we don't support (restrict, volatile, _Atomic qualifiers,
// function types, etc…)
GGP_LIB_TRIVIAL_TYPE_WITH_OPS(Meh);

GGP_LIB_VARIANT_STRUCT(Type,
                       Const,
                       Pointer,
                       NullablePointer,
                       PlainType,
                       Meh);

struct Types
{
  Type for_new;
  Type for_get;
};

auto operator==(Types const& lhs, Types const& rhs) -> bool {
  return lhs.for_new == rhs.for_new && lhs.for_get == rhs.for_get;
}

GGP_LIB_TRIVIAL_NEQ_OP(Types);

std::vector<Types>
expected_types_for_format (VariantFormat const& format);

bool
type_is_convertible_to_type (Type const& from, Type const& to);

} // namespace Ggp::Lib

#else

#if GGP_LIB_TYPE_HH_CHECK_VALUE != GGP_LIB_TYPE_HH_CHECK
#error "This non standalone header file was included from two different wrappers."
#endif

#endif /* GGP_LIB_TYPE_HH */
