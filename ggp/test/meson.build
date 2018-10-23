subdir('generated')

test_sources = [
    'main.cc',
    'variant-test.cc',
    'type-test.cc',
]

test_lib = executable('variant-test',
                      sources: [test_sources, test_generated_sources],
                      include_directories: toplevel_inc,
                      cpp_args: ['-Wall', '-Wextra', '-Wpedantic', '-std=c++17'],
                      implicit_include_directories: false,
                      build_by_default: false)

test('test-lib', test_lib)
