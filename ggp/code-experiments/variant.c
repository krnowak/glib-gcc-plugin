typedef signed int gint32;
typedef unsigned int guint32;
typedef signed short gint16;
typedef unsigned short guint16;
typedef signed long gint64;
typedef unsigned long guint64;

typedef char   gchar;
typedef short  gshort;
typedef long   glong;
typedef int    gint;
typedef gint   gboolean;

typedef unsigned char   guchar;
typedef unsigned short  gushort;
typedef unsigned long   gulong;
typedef unsigned int    guint;

typedef float   gfloat;
typedef double  gdouble;

typedef struct
{} GVariant;

typedef struct
{} GVariantBuilder;

#define NULL ((void *) 0)

static GVariant *
variant_new (int a, const char *format, ...)  __attribute__ ((glib_variant("new", 2, 3)));

/*
void
extern_variant_get (int a, const char *format, ...) __attribute__((glib_variant("get", 2, 3)));
*/

static void
bar (void)
{
  //double d;
  guchar c = 13;
  gint32 i = 42;
  //const gchar *foo = NULL;
  GVariant *v = variant_new (33, "(iy)", i, c);
  //extern_variant_get (1, "(dms&st)", &d, NULL, &foo, (unsigned long long)5);
}
