typedef int gint;
typedef gint gboolean;
typedef unsigned char guchar;
typedef char gchar;

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

static int
test (int i)
{
  if (i > 3)
    {
      int i2 = i + 3;
      return i2;
    }
  else
    {
      int i3 = i - 5;
      return i3;
    }
}

static int
test2 (int i)
{
  int i2 = (i > 3) ? (i + 3) : (i - 5);
  return i2;
}
