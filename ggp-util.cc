#include "ggp-util.hh"

#include "errors.h"

char *
ggp_util_subplugin_name (struct plugin_name_args *plugin_info,
                         const char *suffix)
{
  size_t plugin_name_len = strlen (plugin_info->base_name);
  size_t suffix_len = strlen (suffix);
  size_t subplugin_name_len = plugin_name_len + 1 /* dash */ + suffix_len;
  char *subplugin_name = new char[subplugin_name_len + 1 /* terminating zero */];
  memcpy (subplugin_name, plugin_info->base_name, plugin_name_len);
  subplugin_name[plugin_name_len] = '-';
  memcpy (subplugin_name + plugin_name_len + 1 /* dash */, suffix, suffix_len);
  subplugin_name[subplugin_name_len] = '\0';

  return subplugin_name;
}

void
ggp_util_unregister_callback (const char *plugin_name,
                              int event,
                              const char *event_name)
{
  if (unregister_callback (plugin_name, event) != PLUGEVT_SUCCESS)
    {
      internal_error ("failed to unregister callback for %s"
                      " event for plugin %s",
                      event_name,
                      plugin_name);
    }
}
