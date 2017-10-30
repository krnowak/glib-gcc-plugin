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

#ifndef GGP_TYPE_HH
#define GGP_TYPE_HH

#include "ggp-gcc.hh"

#include "ggp-util.hh"
#include "ggp-variant.hh"

#include <variant>

namespace Ggp
{

struct Pointer;

// TODO: This probably won't be enough. We will another variant like:
//
// std::variant<Integral, Real, VariantTyped>
//
// Integral could store a size in bits, signedness and probably a list
// of names from most expected to least expected.
//
// Real would be similar, but for floating point variables.
//
// VariantTyped probably can hold some extra data about GVariant,
// GVariantBuilder or GVariantIter
using TypeName = std::string;

using Const = std::variant<Util::Value<Pointer>, TypeName>;

struct Pointer
{
  std::variant<Util::Value<Pointer>, Const, TypeName> type;
};

// TODO: Add NullablePointer for maybes and some arrays

using Type = std::variant<Const, Pointer, TypeName>;

struct Types
{
  Type for_new;
  Type for_get;
};

std::vector<Types>
expected_types_for_format (VariantFormat const& format);

} // namespace Ggp

#endif /* GGP_TYPE_HH */
