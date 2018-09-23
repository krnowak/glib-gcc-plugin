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
