Experiments with a GCC plugin for GLib
======================================

This tries to implement the variant format string checking. The
followup plans could be to implement variant type checking for variant
builders and so on.

Another further plan would be to implement `g_tuple` attribute that
will take a bunch of parameters that describe the format of varargs in
a function. Potential uses:

- `g_object_new`
- `g_log_structured`
- `g_variant_new` and other `GVariant` functions
- `soup_server_new`?
- there was some function in the `gupnp` stack…

Another thing - maybe signature string checks?
