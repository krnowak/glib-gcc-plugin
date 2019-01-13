/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2017, 2018 Krzesimir Nowak
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

#ifndef GGP_LIB_DETAIL_UTIL_HH
#define GGP_LIB_DETAIL_UTIL_HH

#ifndef GGP_LIB_UTIL_HH
#error "Do not include this file directly."
#endif

#include "ggp/pp/pp.hh"

#define GGP_LIB_DETAIL_UTIL_STR_HELP_(x) #x

#define GGP_LIB_DETAIL_STRUCT_FIELDS_FUNC(type, name) type name;
#define GGP_LIB_DETAIL_STRUCT_COMPARE_FIELDS_FUNC(type, name) (lhs.name == rhs.name) &&

#define GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_5(func, array_of_tuples) PP_APPLY(func, array_of_tuples)
#define GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_4(func, array_of_tuples) GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_5(func, array_of_tuples)
#define GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_3(func, tuple_of_tuples) GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_4(func, PP_ARRAY_FROM_TUPLE(tuple_of_tuples))
#define GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_2(func, tuple_of_tuples) GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_3(func, tuple_of_tuples)
#define GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_1(func, inner_tuple_size, ...) GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_2(func, PP_TUPLE_OF_N_TUPLES(inner_tuple_size, __VA_ARGS__))
#define GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES(func, inner_tuple_size, ...) GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES_EX_1(func, inner_tuple_size, __VA_ARGS__)

#define GGP_LIB_DETAIL_STRUCT_FIELDS(...) GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES(GGP_LIB_DETAIL_STRUCT_FIELDS_FUNC, 2, __VA_ARGS__)

#define GGP_LIB_DETAIL_STRUCT_EQ_OP(...) GGP_LIB_DETAIL_APPLY_FUNC_ON_TUPLE_OF_TUPLES(GGP_LIB_DETAIL_STRUCT_COMPARE_FIELDS_FUNC, 2, __VA_ARGS__)

#define GGP_LIB_DETAIL_DROP_FIRST_FROM_PAIRS(...)

namespace Ggp::Lib::Detail
{

template <typename A, typename B, typename... C>
struct contains;

template <typename A, typename B, typename... C>
inline constexpr bool contains_v = contains<A, B, C...>::value;

template <typename A, typename B, typename... C>
struct contains
{
  static constexpr bool value = std::is_same_v<A, B> ? true : contains_v<A, C...>;
};

template <typename A, typename B>
struct contains<A, B>
{
  static constexpr bool const value = std::is_same_v<A, B>;
};

template <typename T, typename V>
struct type_in_variant;

template <typename T, typename V>
inline constexpr bool type_in_variant_v = type_in_variant<T, V>::value;

template <typename T, typename... VT>
struct type_in_variant<T, std::variant<VT...>>
{
  static constexpr bool value = contains_v<T, VT...>;
};

template <typename T, typename V>
constexpr void
std_variant_type_check ()
{
  static_assert (type_in_variant_v<T, V>);
}

template <typename T>
struct is_std_variant_impl : std::false_type
{};

template <typename... T>
struct is_std_variant_impl<std::variant<T...>> : std::true_type
{};

template <typename T>
struct is_std_variant : is_std_variant_impl<std::decay_t<T>>
{};

template <typename T>
inline constexpr bool is_std_variant_v = is_std_variant<T>::value;

} // namespace Ggp::Lib::Detail

#endif /* GGP_LIB_DETAIL_UTIL_HH */
