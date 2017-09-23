/* This file is part of gcc-glib-plugin.
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

#include <stdio.h>

/* Do not even think about sorting those includes! */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "diagnostic.h"

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

static struct plugin_info g_tuple_plugin_info = { "0.1", "GTuple for varargs" };
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

int plugin_is_GPL_compatible;
static struct plugin_info ggp_plugin_info = { "0.1", "GLib GCC plugin" };

typedef struct GgpMain_ GgpMain;

struct GgpMain_
{
  char *name;
  GgpVariantChecker *vc;
};

static void
main_finish (void *gcc_data,
             void *user_data)
{
  GgpMain *main = user_data;

  ggp_variant_checker_free (main->vc);
  GGP_UTIL_UNREGISTER_CALLBACK (main->name, PLUGIN_FINISH);
  free (main->name);
  free (main);
}

int plugin_init (struct plugin_name_args   *plugin_info,
                 struct plugin_gcc_version *version)
{
  GgpMain *main;

  if (!plugin_default_version_check (version, &gcc_version))
    {
      puts ("This GCC plugin is for version " GGP_UTIL_STR (GCCPLUGIN_VERSION_MAJOR)
            "." GGP_UTIL_STR (GCCPLUGIN_VERSION_MINOR));
      return 1;
    }

  register_callback (plugin_info->base_name,
                     PLUGIN_INFO,
                     NULL /* callback */,
                     &ggp_plugin_info);

  main = malloc (sizeof (GgpMain));
  main->name = ggp_util_subplugin_name (plugin_info, "main");
  main->vc = ggp_variant_checker_new (plugin_info);

  register_callback (global_main.name,
		     PLUGIN_FINISH,
		     main_finish,
		     main);


  #if 0
  register_callback (plugin_info->base_name,
                     PLUGIN_ATTRIBUTES,
                     g_tuple_register_attributes,
                     NULL /* user data */);
  register_callback (plugin_info->base_name,
                     PLUGIN_START_PARSE_FUNCTION,
                     g_tuple_start_parse_function,
                     NULL /* user data */);
  register_callback (plugin_info->base_name,
                     PLUGIN_FINISH_PARSE_FUNCTION,
                     g_tuple_finish_parse_function,
                     NULL /* user data */);
  #endif

  return 0;
}
