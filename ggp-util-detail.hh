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

#ifndef GGP_UTIL_DETAIL_HH
#define GGP_UTIL_DETAIL_HH

#include "ggp-gcc.hh"

#include <type_traits>
#include <variant>

namespace Ggp
{

namespace Util
{

namespace Detail
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

} // namespace Detail

} // namespace Util

} // namespace Ggp

#endif /* GGP_UTIL_DETAIL_HH */
