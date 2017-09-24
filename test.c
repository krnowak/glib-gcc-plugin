static void
foo (char *first, ...) /* __attribute__ ((g_tuple("blah"))) */;

static void
foo (char *first, ...)
{
  /* nothing here */
}

static void
bar (void)
{
  foo ("baz");
}
