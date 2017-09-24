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

#include "ggp-util.hh"

#include "diagnostic.h"

#include "errors.h"

namespace Ggp
{

namespace Util
{

namespace
{

const char*
get_event_name (int event)
{
  switch (event)
    {
#define DEFEVENT(name) case name: return #name;
#include "plugin.def"
#undef DEFEVENT
    default: return "UNKNOWN_OR_DYNAMIC_EVENT";
    }
}

const char*
get_code_name (int code)
{
  switch (code)
    {
    case 0: return "PLUGEVT_SUCCESS";
    case 1: return "PLUGEVT_NO_EVENTS";
    case 2: return "PLUGEVT_NO_SUCH_EVENT";
    case 3: return "PLUGEVT_NO_CALLBACK";
    default: return "UNKNOWN_CODE";
    }
}

} // namespace

std::string
subplugin_name (struct plugin_name_args* plugin_info,
                const char* suffix)
{
  std::string subplugin_name {plugin_info->base_name};

  subplugin_name += '-';
  subplugin_name += suffix;

  return subplugin_name;
}

CallbackRegistration::CallbackRegistration (const std::string& plugin_name,
                                            int event,
                                            plugin_callback_func callback,
                                            void* user_data)
  : plugin_name {plugin_name},
    event {event}
{
  ::register_callback (plugin_name.c_str (),
                       event,
                       callback,
                       user_data);
}

CallbackRegistration::~CallbackRegistration ()
{
  int code = ::unregister_callback (plugin_name.c_str (), event);
  if (code != PLUGEVT_SUCCESS)
    {
      error ("failed to unregister callback for %s"
             " event for plugin %s (code %d - %s)",
             get_event_name (event),
             plugin_name.c_str (),
             code,
             get_code_name (code));
    }
}

} // namespace Util

} // namespace Ggp
