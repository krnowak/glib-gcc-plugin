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

#include "ggp/gcc/gcc.hh"

#include "ggp/gcc/main.hh"

#include "ggp/gcc/generated/util.hh"

#include <stdio.h>

#include "plugin-version.h"

int plugin_is_GPL_compatible;
static struct plugin_info ggp_plugin_info = { "0.1", "GLib GCC plugin" };

int
plugin_init (struct plugin_name_args* plugin_info,
             struct plugin_gcc_version* version)
{
  if (!plugin_default_version_check (version, &gcc_version))
  {
    puts ("This GCC plugin is for version " GGP_LIB_UTIL_STR (GCCPLUGIN_VERSION_MAJOR)
          "." GGP_LIB_UTIL_STR (GCCPLUGIN_VERSION_MINOR));
    return 1;
  }

  register_callback (plugin_info->base_name,
                     PLUGIN_INFO,
                     NULL /* callback */,
                     &ggp_plugin_info);
  Ggp::Gcc::main_setup (plugin_info);

  return 0;
}
