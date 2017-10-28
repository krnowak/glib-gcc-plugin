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

#include "ggp-main.hh"
#include "ggp-util.hh"
#include "ggp-tc.hh"
#include "ggp-vc.hh"

namespace Ggp
{

namespace {

struct Main
{
  Main (struct plugin_name_args* plugin_info);

  std::string name;
  VariantChecker vc;
  TupleChecker tc;
  Util::CallbackRegistration finish_unit;
};

void
main_finish (void* /* gcc_data */,
             void* user_data)
{
  delete static_cast<Main*> (user_data);
}

Main::Main (struct plugin_name_args* plugin_info)
  : name {Util::subplugin_name (plugin_info, "main")},
    vc {plugin_info},
    tc {plugin_info},
    finish_unit {name, PLUGIN_FINISH_UNIT, main_finish, this}
{}

} // namespace

void
main_setup (struct plugin_name_args* plugin_info)
{
  new Main (plugin_info);
}

} // namespace Ggp
