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

#ifndef GGP_UTIL_HH
#define GGP_UTIL_HH

#include <string>

#include "gcc-plugin.h"

#define GGP_UTIL_STR_HELP_(x) #x
#define GGP_UTIL_STR(x) GGP_UTIL_STR_HELP_(x)

namespace Ggp
{

namespace Util
{

std::string
subplugin_name (struct plugin_name_args* plugin_info,
                const char* suffix);

struct CallbackRegistration
{
  CallbackRegistration (const std::string& plugin_name,
			int event,
			plugin_callback_func callback,
			void* user_data);

  ~CallbackRegistration ();

private:
  std::string plugin_name;
  int event;
};

} // namespace Util

} // namespace Ggp

#endif /* GGP_UTIL_HH */
