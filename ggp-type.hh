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

struct Const;
struct Pointer;

using TypeName = std::string;

struct Const
{
  std::variant<Util::Value<Pointer>, TypeName> type;
};

struct Pointer
{
  std::variant<Util::Value<Const>, TypeName> type;
};

using Type = std::variant<Const, Pointer, TypeName>;

std::vector<Type>
expected_types_for_format (VariantFormat const& format);

} // namespace Ggp

#endif /* GGP_TYPE_HH */
