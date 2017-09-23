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

#include "ggp-main.hh"
#include "ggp-util.hh"
#include "ggp-vc.hh"

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
  GgpMain *main = static_cast<GgpMain *> (user_data);

  ggp_variant_checker_free (main->vc);
  GGP_UTIL_UNREGISTER_CALLBACK (main->name, PLUGIN_FINISH);
  delete[] main->name;
  delete main;
}

void
ggp_main_setup (struct plugin_name_args *plugin_info)
{
  GgpMain *main = new GgpMain;
  main->name = ggp_util_subplugin_name (plugin_info, "main");
  main->vc = ggp_variant_checker_new (plugin_info);

  register_callback (main->name,
                     PLUGIN_FINISH,
                     main_finish,
                     main);
}
