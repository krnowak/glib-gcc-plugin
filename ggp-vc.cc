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

#include "ggp-vc.hh"

#include "diagnostic.h"

namespace Ggp
{

namespace {

static void
ggp_vc_finish_decl (void* /* gcc_data */,
                    void* /* user_data */)
{
  warning (0, "ggp_vc_finish_decl");
}

static void
ggp_vc_start_parse_function (void* /* gcc_data */,
                             void* /* user_data */)
{
  warning (0, "ggp_vc_start_parse_function");
}

static void
ggp_vc_finish_parse_function (void* /* gcc_data */,
                              void* /* user_data */)
{
  warning (0, "ggp_vc_finish_parse_function");
}

static void
ggp_vc_attributes (void* /* gcc_data */,
                   void* /* user_data */)
{
  warning (0, "ggp_vc_attributes");
}

} // namespace

VariantChecker::VariantChecker (struct plugin_name_args* plugin_info)
  : name {Util::subplugin_name (plugin_info, "vc")},
    finish_decl {name, PLUGIN_FINISH_DECL, ggp_vc_finish_decl, this},
    start_parse_function {name, PLUGIN_START_PARSE_FUNCTION, ggp_vc_start_parse_function, this},
    finish_parse_function {name, PLUGIN_FINISH_PARSE_FUNCTION, ggp_vc_finish_parse_function, this},
    attributes {name, PLUGIN_ATTRIBUTES, ggp_vc_attributes, this}
{}

} // namespace Ggp
