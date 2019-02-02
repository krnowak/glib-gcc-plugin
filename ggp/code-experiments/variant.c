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
  guchar c = 13;
  gint32 i = 42;
  guint32 u = 35;
  const gchar *s = "foo";
  gdouble d = 1.5;
  gfloat f = 1.5f;
  GVariant *v = variant_new (33, "(iyyttssdddiiuuu)", /*0*/i, /*1*/c, /*2*/(guchar)(c), /*3*/(guint64)(c), /*4*/(guint64)42, /*5*/"foo", /*6*/s, /*7*/f, /*8*/d, /*9*/(gdouble)f, /*10*/c, /*11*/(gint32)c, /*12*/u, /*13*/c, /*14*/(guint32)c);
  //extern_variant_get (1, "(dms&st)", &d, NULL, &foo, (unsigned long long)5);
}
