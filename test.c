static void
variant_set (int a, const char *format, ...)  __attribute__ ((glib_variant("set", 2, 3)));

static void
variant_set (int a, const char *format, ...)
{

}

void
extern_variant_get (int a, const char *format, ...) __attribute__((glib_variant("get", 2, 3)));

/*
static void
variant_get (char *format, ...)  __attribute__ ((glib_variant("get", 1, 2)));
*/

static void
bar (void)
{
  double d;
  variant_set (33, "i", 42);
  extern_variant_get (1, "d", &d);
}
