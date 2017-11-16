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

#include "ggp/gcc/tc.hh"

#if 0
static tree
g_tuple_handler (tree *node,
                 tree  name,
                 tree  args,
                 int   flags,
                 bool *no_add_attrs)
{
  if (!stdarg_p (*node))
    {
      warning (OPT_Wattributes,
               "%qE attribute only applies to variadic functions", name);
      *no_add_attrs = true;
    }

  return NULL_TREE;
}

static struct attribute_spec g_tuple_attribute_spec = {"g_tuple", 1, -1, false, true, true, g_tuple_handler, false};

static void
g_tuple_register_attributes (void *gcc_data,
                             void *user_data)
{
  register_attribute (&g_tuple_attribute_spec);
}

static void
g_tuple_start_parse_function (void *gcc_data,
                              void *user_data)
{
  tree fndef = (tree) gcc_data;
  warning (0, "Start fndef %s",
           IDENTIFIER_POINTER (DECL_NAME (fndef)));
}

static void
g_tuple_finish_parse_function (void *gcc_data,
                               void *user_data)
{
  tree fndef = (tree) gcc_data;
  warning (0, "Finish fndef %s",
           IDENTIFIER_POINTER (DECL_NAME (fndef)));
}

#endif

namespace Ggp::Gcc
{

namespace {

static void
ggp_tc_finish_decl (void* /* gcc_data */,
                    void* /* user_data */)
{
  //warning (0, "ggp_tc_finish_decl");
}

static void
ggp_tc_start_parse_function (void* /* gcc_data */,
                             void* /* user_data */)
{
  //warning (0, "ggp_tc_start_parse_function");
}

static void
ggp_tc_finish_parse_function (void* /* gcc_data */,
                              void* /* user_data */)
{
  //warning (0, "ggp_tc_finish_parse_function");
}

static void
ggp_tc_attributes (void* /* gcc_data */,
                   void* /* user_data */)
{
  //warning (0, "ggp_tc_attributes");
}

} // namespace

TupleChecker::TupleChecker (struct plugin_name_args* plugin_info)
  : name {subplugin_name (plugin_info, "tc")},
    finish_decl {name, PLUGIN_FINISH_DECL, ggp_tc_finish_decl, this},
    start_parse_function {name, PLUGIN_START_PARSE_FUNCTION, ggp_tc_start_parse_function, this},
    finish_parse_function {name, PLUGIN_FINISH_PARSE_FUNCTION, ggp_tc_finish_parse_function, this},
    attributes {name, PLUGIN_ATTRIBUTES, ggp_tc_attributes, this}
{}

} // namespace Ggp::Gcc
