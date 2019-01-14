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

#ifndef GGP_TEST_TEST_PRINT_HH
#define GGP_TEST_TEST_PRINT_HH

#include <iosfwd>
#include <optional>
#include <vector>

namespace Ggp::Lib
{

class VariantType;
class VariantFormat;
class Types;

auto
operator<< (std::ostream& os, std::optional<VariantType> const& maybe_variant_type) -> std::ostream&;

auto
operator<< (std::ostream& os, std::optional<VariantFormat> const& maybe_variant_format) -> std::ostream&;

auto
operator<< (std::ostream& os, std::vector<Types> const& types) -> std::ostream&;

} // namespace Ggp::Lib

#endif /* GGP_TEST_TEST_PRINT_HH */
