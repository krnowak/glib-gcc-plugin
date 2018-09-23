struct Foo
{};

static void
empty (struct Foo *f)
{}

static int
test (int i)
{
  return i % 2;
}

static struct Foo *
get_foo(void)
{
  return 0;
}

static void
loop_f (void)
{
  // var decl with no init
  // decl_expr also
  int i;
  // var decl with init being a call expr
  // decl_expr also
  struct Foo *f = get_foo ();
  // modify_expr for loop initializer (i = 0)
  // goto_expr for loop condition (i < 10)
  // label_expr for loop body
  // bind_expr for loop body (bind_expr is basically a block)
  // preincrement_expr for loop postiteration action (++i)
  // label_expr for loop condition (i < 10)
  // cond_expr for loop condition (i < 10)
  // label_expr for after loop_body
  //
  // cond_expr has three ops:
  //   0 - le_expr
  //       two ops, var_decl (i) and integer_cst (9)
  //       !! gcc changes < to <= and decrements the constant from 10 to 9
  //   1 - goto_expr to label for loop body
  //   2 - goto_expr to label after loop body
  for (i = 0; i < 10; ++i) // i = 0 - modify_expr, i < 10 - cond_expr
    {
      // var_decl with init being a var_decl
      // decl_expr also
      int j = i;
      // goto_expr for loop condition
      // label for loop body
      // call_expr for empty(f)
      // postincrement_expr for j++
      // postincrement_expr for f++
      // label_expr for loop condition
      // cond_expr for loop condition
      // label_expr for after loop body
      //
      // cond_expr has three ops:
      // 0 - ne_expr
      //     two ops, call_expr (test(j)) and integer_cst (0)
      // 1 - goto_expr for label for loop_body
      // 2 - goto_expr for label after loop_body
      while (test (j))
        {
          empty (f);
          j++;
          f++;
        }
    }
}
