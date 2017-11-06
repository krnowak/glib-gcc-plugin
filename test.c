typedef int gint;
typedef gint gboolean;
typedef unsigned char guchar;

#define NULL ((void *) 0)

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
  guchar c = 13;
  const gchar *foo = NULL;
  variant_set (33, "(iy)", 42, c);
  extern_variant_get (1, "(dms&s)", &d, NULL, &foo);
}
