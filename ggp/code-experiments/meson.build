ggp_code_experiments_gnu_c_compiler = find_program('gcc')
ggp_code_experiments_gnu_cxx_compiler = find_program('g++')

run_target('dump-test-c',
           command: ['./dump-test.sh',
                     '--compiler', ggp_code_experiments_gnu_c_compiler.path(),
                     '--plugin', ggp_gcc_plugin,
                     '--input-file', 'test.c',
                     '--output-file', 'dump-test-c.dir/test.o'],
           depends: [ggp_gcc_plugin])

run_target('dump-test-cxx',
           command: ['./dump-test.sh',
                     '--compiler', ggp_code_experiments_gnu_cxx_compiler.path(),
                     '--plugin', ggp_gcc_plugin,
                     '--input-file', 'test.cc',
                     '--output-file', 'dump-test-cxx.dir/test.o'],
           depends: [ggp_gcc_plugin])
