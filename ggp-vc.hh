#ifndef GLIB_GCC_PLUGIN_GVARIANT_HH
#define GLIB_GCC_PLUGIN_GVARIANT_HH

#include "gcc-plugin.h"

typedef struct GgpVariantChecker_ GgpVariantChecker;

GgpVariantChecker *
ggp_variant_checker_new (struct plugin_name_args *plugin_info);

void
ggp_variant_checker_free (GgpVariantChecker *vc);

#endif /* GLIB_GCC_PLUGIN_GVARIANT_HH */
