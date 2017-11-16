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

#ifndef GGP_VC_HH
#define GGP_VC_HH

#include "ggp/gcc/gcc.hh"

#include "ggp/gcc/util.hh"

namespace Ggp::Gcc
{

struct VariantChecker
{
  VariantChecker(struct plugin_name_args* plugin_info);

  std::string name;
  CallbackRegistration finish_decl;
  CallbackRegistration start_parse_function;
  CallbackRegistration finish_parse_function;
  CallbackRegistration attributes;
};

} // namespace Ggp::Gcc

#endif /* GGP_VC_HH */
