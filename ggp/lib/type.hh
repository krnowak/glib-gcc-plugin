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

GGP_LIB_STRUCT (Integral,
                std::string, name,
                //std::vector<std::string>, additional_names,
                //std::vector<std::string>, accidental_names,
                std::uint8_t, size_in_bytes,
                Signedness, signedness);

GGP_LIB_STRUCT (Real,
                std::string, name,
                //std::vector<std::string>, additional_names,
                std::uint8_t, size_in_bytes);

auto type_gboolean () -> Integral;
auto type_gchar () -> Integral;
auto type_guchar () -> Integral;
auto type_gint16 () -> Integral;
auto type_guint16 () -> Integral;
auto type_gint32 () -> Integral;
auto type_guint32 () -> Integral;
auto type_gint64 () -> Integral;
auto type_guint64 () -> Integral;
auto type_handle () -> Integral;
auto type_gdouble () -> Real;

GGP_LIB_TRIVIAL_TYPE_WITH_OPS(VariantTypeUnspecified);
inline constexpr VariantTypeUnspecified variant_type_unspecified {};

GGP_LIB_VARIANT_STRUCT(TypeInfo,
                       VariantTypeUnspecified,
                       VariantType);

GGP_LIB_STRUCT (VariantTyped,
                std::string, name,
                TypeInfo, info);

GGP_LIB_VARIANT_STRUCT(PlainType,
                       Integral,
                       Real,
                       VariantTyped);

struct PointerBase;
struct Pointer;

GGP_LIB_VARIANT_STRUCT(Const,
                       Value<Pointer>,
                       PlainType);

GGP_LIB_VARIANT_STRUCT(PointerBase,
                       Value<Pointer>,
                       Const,
                       PlainType);
struct Pointer : PointerBase
{};
GGP_LIB_VARIANT_OPS(Pointer);

// TODO: Replace "using" with the code below when we start needing a
// differentiation between nullable pointers, non-nullable pointers
// and not-yet-introduced dependent pointers.
// struct NullablePointer : PointerBase
// {};
// GGP_LIB_VARIANT_OPS(NullablePointer);
using NullablePointer = Pointer;

// TODO: struct DependentPointer : PointerBase {}; A dependent pointer
// would be a pointer dependent on a value of a special boolean
// parameter (as in GVariant maybe types). If the bool value is true,
// then pointer in "new" format can't be null, but can be null in
// "get" format. When false, it can be whatever, it's ignored. We
// could warn about passing non-null pointers in this case, though.
//
// TODO: Describe out pointers? Usually they are non-null, but for
// maybe types, they may be null. Pointer<typename New, typename Get>?

// For types we don't support (restrict, volatile, _Atomic qualifiers,
// function types, etc…)
GGP_LIB_TRIVIAL_TYPE_WITH_OPS(Meh);

GGP_LIB_VARIANT_STRUCT(Type,
                       Const,
                       Pointer,
                       //NullablePointer,
                       PlainType,
                       Meh);

GGP_LIB_STRUCT (Types,
                Type, for_new,
                Type, for_get);

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
