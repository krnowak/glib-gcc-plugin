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

#include "ggp-type.hh"

namespace Ggp
{

namespace
{

std::vector<Type>
foo ()
{
  return {};
}

/*
std::vector<Type>
variant_type_sub_set_mod_to_types (VF::VTMod::VariantTypeSubSet const& vtss)
{
  return {};
}
*/

} // anonymous namespace

std::vector<Type>
expected_types_for_format (VariantFormat const& format)
{
  auto v {Util::VisitHelper {
    [](VF::VTMod::VariantTypeSubSet const&) { return foo (); },
    [](VF::AtVariantType const&) { return foo (); },
    [](VF::Pointer const&) { return foo (); },
    [](VF::Convenience const&) { return foo (); },
    [](VF::Maybe const&) { return foo (); },
    [](VF::Tuple const&) { return foo (); },
    [](VF::Entry const&) { return foo (); },
  }};
  return std::visit (v, format);;
}

} // namespace Ggp
