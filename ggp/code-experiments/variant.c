typedef int gint;
typedef gint gboolean;
typedef unsigned char guchar;
typedef char gchar;
typedef struct
{} GVariant;

typedef struct
{} GVariantBuilder;

#define NULL ((void *) 0)

static GVariant *
variant_new (int a, const char *format, ...)  __attribute__ ((glib_variant("new", 2, 3)));

static GVariant *
variant_new (int a, const char *format, ...)
{
  return NULL;
}

void
extern_variant_get (int a, const char *format, ...) __attribute__((glib_variant("get", 2, 3)));

static void
bar (void)
{
  double d;
  guchar c = 13;
  const gchar *foo = NULL;
  GVariant *v = variant_new (33, "(iy)", 42, c);
  extern_variant_get (1, "(dms&st)", &d, NULL, &foo, (unsigned long long)5);
}
