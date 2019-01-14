/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2019 Krzesimir Nowak
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

#include "ggp/test/test-print.hh"
#include "ggp/test/generated/variant.hh"
#include "ggp/test/generated/variant-print.hh"

#include <ostream>

namespace Ggp::Lib
{

namespace
{

template <typename T>
auto
print_op (std::ostream& os, std::optional<T> const& maybe_t) -> std::ostream&
{
  if (maybe_t)
  {
    os << '<' << *maybe_t << '>';
  }
  else
  {
    os << "none";
  }

  return os;
}

} // anonymous namespace

auto
operator<< (std::ostream& os, std::optional<VariantType> const& maybe_variant_type) -> std::ostream&
{
  return print_op (os, maybe_variant_type);
}

auto
operator<< (std::ostream& os, std::optional<VariantFormat> const& maybe_variant_format) -> std::ostream&
{
  return print_op (os, maybe_variant_format);
}

auto
operator<< (std::ostream& os, std::vector<Types> const&) -> std::ostream&
{
  // TODO
  os << "<some types>";
  return os;
}

} // namespace Ggp::Lib
