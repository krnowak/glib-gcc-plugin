#include "ggp-main.hh"
#include "ggp-util.hh"

#include <stdio.h>

#include "gcc-plugin.h"
#include "plugin-version.h"

int plugin_is_GPL_compatible;
static struct plugin_info ggp_plugin_info = { "0.1", "GLib GCC plugin" };

int
plugin_init (struct plugin_name_args   *plugin_info,
             struct plugin_gcc_version *version)
{
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
  ggp_main_setup (plugin_info);

  return 0;
}
