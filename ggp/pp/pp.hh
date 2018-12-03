#ifndef GGP_LIB_PP_PP_HH
#define GGP_LIB_PP_PP_HH

#include "pp-gen.hh"

#define DETAIL_PP_DEC_EX_1(num) GENERATED_DETAIL_PP_DEC_ ## num
#define PP_DEC(num) DETAIL_PP_DEC_EX_1(num)

// PP_DEC(3) // 2
// PP_DEC(2) // 1
// PP_DEC(1) // 0

#define DETAIL_PP_BOOL_EX_1(num) GENERATED_DETAIL_PP_BOOL_ ## num
#define PP_BOOL(num) DETAIL_PP_BOOL_EX_1(num)

// PP_BOOL(2) // 1
// PP_BOOL(1) // 1
// PP_BOOL(0) // 0

#define DETAIL_PP_PICK_EX_1_0(if_true, if_false) if_false
#define DETAIL_PP_PICK_EX_1_1(if_true, if_false) if_true
#define DETAIL_PP_PICK_EX_1(boolean, if_true, if_false) DETAIL_PP_PICK_EX_1_ ## boolean(if_true, if_false)
#define PP_PICK(boolean, if_true, if_false) DETAIL_PP_PICK_EX_1(boolean, if_true, if_false)

//
// tuple utils
//

// #define TUPLE ((int, i), (long, l), (double, d))

#define DETAIL_PP_TUPLE_EX_1(...) GENERATED_DETAIL_PP_TUPLE(__VA_ARGS__)
#define PP_TUPLE(...) DETAIL_PP_TUPLE_EX_1(__VA_ARGS__)

// PP_TUPLE(1, 2, 3) // ((1), (2), (3))

#define DETAIL_PP_TUPLE_SIZE_EX_1(tuple) GENERATED_DETAIL_PP_SEQ_ARG_N tuple
#define PP_TUPLE_SIZE(tuple) DETAIL_PP_TUPLE_SIZE_EX_1(tuple)

// PP_TUPLE_SIZE(((1), (2), (3))) // 3

#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_9(tuple, func) func tuple
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_8(tuple, func) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_9(tuple, func)
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_7(tuple, func_if_size_1, func_if_size_more, is_size_more) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_8(tuple, PP_PICK(is_size_more, func_if_size_more, func_if_size_1))
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_6(tuple, func_if_size_1, func_if_size_more, is_size_more) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_7(tuple, func_if_size_1, func_if_size_more, is_size_more)
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_5(tuple, func_if_size_1, func_if_size_more, dec_tuple_size) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_6(tuple, func_if_size_1, func_if_size_more, PP_BOOL(dec_tuple_size))
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_4(tuple, func_if_size_1, func_if_size_more, dec_tuple_size) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_5(tuple, func_if_size_1, func_if_size_more, dec_tuple_size)
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_3(tuple, func_if_size_1, func_if_size_more, tuple_size) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_4(tuple, func_if_size_1, func_if_size_more, PP_DEC(tuple_size))
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_2(tuple, func_if_size_1, func_if_size_more, tuple_size) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_3(tuple, func_if_size_1, func_if_size_more, tuple_size)
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_1(tuple, func_if_size_1, func_if_size_more) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_2(tuple, func_if_size_1, func_if_size_more, PP_TUPLE_SIZE(tuple))
#define DETAIL_PP_TUPLE_APPLY_ON_TUPLE(tuple, func_if_size_1, func_if_size_more) DETAIL_PP_TUPLE_APPLY_ON_TUPLE_EX_1(tuple, func_if_size_1, func_if_size_more)

#define DETAIL_PP_TUPLE_HEAD_SIZE_ONE(head) head
#define DETAIL_PP_TUPLE_HEAD_SIZE_MORE(head, ...) head
#define DETAIL_PP_TUPLE_HEAD_EX_1(tuple) DETAIL_PP_TUPLE_APPLY_ON_TUPLE(tuple, DETAIL_PP_TUPLE_HEAD_SIZE_ONE, DETAIL_PP_TUPLE_HEAD_SIZE_MORE)
// assumption: PP_TUPLE_SIZE(tuple) > 0
#define PP_TUPLE_HEAD(tuple) DETAIL_PP_TUPLE_HEAD_EX_1(tuple)

// PP_TUPLE_HEAD(((a), (b), (c))) // (a)

#define DETAIL_PP_TUPLE_REST_SIZE_ONE(head) ()
#define DETAIL_PP_TUPLE_REST_SIZE_MORE(head, ...) (__VA_ARGS__)
#define DETAIL_PP_TUPLE_REST_EX_1(tuple) DETAIL_PP_TUPLE_APPLY_ON_TUPLE(tuple, DETAIL_PP_TUPLE_REST_SIZE_ONE, DETAIL_PP_TUPLE_REST_SIZE_MORE)
// assumption: PP_TUPLE_SIZE(tuple) > 0
#define PP_TUPLE_REST(tuple) DETAIL_PP_TUPLE_REST_EX_1(tuple)

// PP_TUPLE_REST(((a), (b), (c))) // ((b), (c))

#define DETAIL_PP_TUPLE_OF_N_TUPLES_EX_3(n, size, ...) GENERATED_DETAIL_PP_TUPLE_OF_N_TUPLES_ ## n (size, __VA_ARGS__)
#define DETAIL_PP_TUPLE_OF_N_TUPLES_EX_2(n, size, ...) DETAIL_PP_TUPLE_OF_N_TUPLES_EX_3(n, size, __VA_ARGS__)
#define DETAIL_PP_TUPLE_OF_N_TUPLES_EX_1(n, ...) DETAIL_PP_TUPLE_OF_N_TUPLES_EX_2(n, PP_TUPLE_SIZE((__VA_ARGS__)), __VA_ARGS__)
#define PP_TUPLE_OF_N_TUPLES(n, ...) (DETAIL_PP_TUPLE_OF_N_TUPLES_EX_1(n, __VA_ARGS__))

// PP_TUPLE_OF_N_TUPLES(3, 1, 2, 3, a, b, c, X, Y, Z) // ((1, 2, 3), (a, b, c), (X, Y, Z))

//
// array utils
//

//#define ARRAY (3, ((int, i), (long, l), (double, d)))

#define DETAIL_PP_ARRAY_FROM_TUPLE_EX_1(tuple) (PP_TUPLE_SIZE(tuple), tuple)
#define PP_ARRAY_FROM_TUPLE(tuple) DETAIL_PP_ARRAY_FROM_TUPLE_EX_1(tuple)

// PP_ARRAY_FROM_TUPLE(((1), (2), (3))) // (3, ((1), (2), (3)))

#define DETAIL_PP_ARRAY_SIZE_EX_2(size, tuple) size
#define DETAIL_PP_ARRAY_SIZE_EX_1(array) DETAIL_PP_ARRAY_SIZE_EX_2 array
#define PP_ARRAY_SIZE(array) DETAIL_PP_ARRAY_SIZE_EX_1(array)

// PP_ARRAY_SIZE((2, ((a), (b)))) // 2

#define DETAIL_PP_ARRAY_FIRST_ITEM_EX_2(size, tuple) PP_TUPLE_HEAD(tuple)
#define DETAIL_PP_ARRAY_FIRST_ITEM_EX_1(array) DETAIL_PP_ARRAY_FIRST_ITEM_EX_2 array
#define PP_ARRAY_FIRST_ITEM(array) DETAIL_PP_ARRAY_FIRST_ITEM_EX_1(array)

// PP_ARRAY_FIRST_ITEM((2, ((a), (b)))) // (a)

#define DETAIL_PP_ARRAY_SHIFT_EX_2(size, tuple) (PP_DEC(size), PP_TUPLE_REST(tuple))
#define DETAIL_PP_ARRAY_SHIFT_EX_1(array) DETAIL_PP_ARRAY_SHIFT_EX_2 array
#define PP_ARRAY_SHIFT(array) DETAIL_PP_ARRAY_SHIFT_EX_1(array)

// PP_ARRAY_SHIFT((2, ((a), (b)))) // (1, ((b)))

#define DETAIL_PP_APPLY_EX_3(func, array, size) GENERATED_DETAIL_PP_APPLY_ ## size (func, array)
#define DETAIL_PP_APPLY_EX_2(func, array, size) DETAIL_PP_APPLY_EX_3(func, array, size)
#define DETAIL_PP_APPLY_EX_1(func, array) DETAIL_PP_APPLY_EX_2(func, array, PP_ARRAY_SIZE(array))
#define PP_APPLY(func, array) DETAIL_PP_APPLY_EX_1(func, array)

// #define ARRAY (3, ((int, i), (long, l), (double, d)))
// #define TEST_FUNC(a, b) a -> b
// PP_APPLY(TEST_FUNC, ARRAY)

#endif // GGP_LIB_PP_PP_HH
