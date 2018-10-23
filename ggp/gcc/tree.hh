/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2018 Krzesimir Nowak
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

#ifndef GGP_TREE_HH
#define GGP_TREE_HH

#include "ggp/gcc/gcc.hh"

#include <functional>

namespace Ggp::Gcc
{

auto iterate_tree(tree root, std::function<void(tree)> callback) -> void;

} // namespace Ggp::Gcc

#endif /* GGP_TREE_HH */
