#include "pp-gen.hh"

// #define TUPLE ((int, i), (long, l), (double, d))

#define DETAIL_PP_TUPLE_EX_1(...) GENERATED_DETAIL_PP_TUPLE(__VA_ARGS__)
#define PP_TUPLE(...) DETAIL_PP_TUPLE_EX_1(__VA_ARGS__)

//PP_TUPLE(1, 2, 3) // ((1), (2), (3))

#define DETAIL_PP_TUPLE_SIZE_EX_1(tuple) GENERATED_DETAIL_PP_SEQ_ARG_N tuple
#define PP_TUPLE_SIZE(tuple) DETAIL_PP_TUPLE_SIZE_EX_1(tuple)

// PP_TUPLE_SIZE(((1), (2), (3))) // 3

#define DETAIL_PP_TUPLE_HEAD_EX_2(first, ...) first
#define DETAIL_PP_TUPLE_HEAD_EX_1(tuple) DETAIL_PP_TUPLE_HEAD_EX_2 tuple
#define PP_TUPLE_HEAD(tuple) DETAIL_PP_TUPLE_HEAD_EX_1(tuple)

//PP_TUPLE_HEAD(((a), (b), (c))) // (a)

#define DETAIL_PP_TUPLE_REST_EX_2(head, ...) (__VA_ARGS__)
#define DETAIL_PP_TUPLE_REST_EX_1(tuple) DETAIL_PP_TUPLE_REST_EX_2 tuple
#define PP_TUPLE_REST(tuple) DETAIL_PP_TUPLE_REST_EX_1(tuple)

//PP_TUPLE_REST(((a), (b), (c))) // ((b), (c))

//#define ARRAY (3, ((int, i), (long, l), (double, d)))

#define DETAIL_PP_DEC_EX_1(num) GENERATED_DETAIL_PP_DEC_ ## num
#define PP_DEC(num) DETAIL_PP_DEC_EX_1(num)

// PP_DEC(3) // 2
// PP_DEC(2) // 1
// PP_DEC(1) // 0

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

#define ARRAY (3, ((int, i), (long, l), (double, d)))
#define TEST_FUNC(a, b) a -> b
PP_APPLY(TEST_FUNC, ARRAY)
