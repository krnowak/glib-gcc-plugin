#include "ggp-vc.hh"
#include "ggp-util.hh"

#include "errors.h"

struct GgpVariantChecker_
{
  char *name;
};

static void
ggp_vc_finish_decl (void *gcc_data,
                    void *user_data)
{
  warning (0, "ggp_vc_finish_decl");
}

static void
ggp_vc_start_parse_function (void *gcc_data,
                             void *user_data)
{
  warning (0, "ggp_vc_start_parse_function");
}

static void
ggp_vc_finish_parse_function (void *gcc_data,
                              void *user_data)
{
  warning (0, "ggp_vc_finish_parse_function");
}

static void
ggp_vc_attributes (void *gcc_data,
                   void *user_data)
{
  warning (0, "ggp_vc_attributes");
}

GgpVariantChecker *
ggp_variant_checker_new (struct plugin_name_args *plugin_info)
{
  GgpVariantChecker *vc = new GgpVariantChecker;
  vc->name = ggp_util_subplugin_name (plugin_info, "vc");

  register_callback (vc->name,
                     PLUGIN_FINISH_DECL,
                     ggp_vc_finish_decl,
                     vc);

  register_callback (vc->name,
                     PLUGIN_START_PARSE_FUNCTION,
                     ggp_vc_start_parse_function,
                     vc);

  register_callback (vc->name,
                     PLUGIN_FINISH_PARSE_FUNCTION,
                     ggp_vc_finish_parse_function,
                     vc);

  register_callback (vc->name,
                     PLUGIN_ATTRIBUTES,
                     ggp_vc_attributes,
                     vc);
}

void
ggp_variant_checker_free (GgpVariantChecker *vc)
{
  GGP_UTIL_UNREGISTER_CALLBACK (vc->name, PLUGIN_ATTRIBUTES);
  GGP_UTIL_UNREGISTER_CALLBACK (vc->name, PLUGIN_FINISH_PARSE_FUNCTION);
  GGP_UTIL_UNREGISTER_CALLBACK (vc->name, PLUGIN_START_PARSE_FUNCTION);
  GGP_UTIL_UNREGISTER_CALLBACK (vc->name, PLUGIN_FINISH_DECL);
  delete[] vc->name;
  delete vc;
}
