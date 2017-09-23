#ifndef GLIB_GCC_PLUGIN_UTIL_HH
#define GLIB_GCC_PLUGIN_UTIL_HH

#include "gcc-plugin.h"

#define GGP_UTIL_STR_HELP_(x) #x
#define GGP_UTIL_STR(x) GGP_UTIL_STR_HELP_(x)

char *
ggp_util_subplugin_name (struct plugin_name_args *plugin_info,
                         const char *suffix);

void
ggp_util_unregister_callback (const char *plugin_name,
                              int event,
                              const char *event_name);

#define GGP_UTIL_UNREGISTER_CALLBACK(name, event) ggp_util_unregister_callback (name, event, #event)

#endif /* GLIB_GCC_PLUGIN_UTIL_HH */
