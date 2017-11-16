subdir('lib')
subdir('gcc')
subdir('test')

ggp_gnu_compiler = find_program('gcc')

custom_target('dump-test',
              input: 'test.c',
              output: 'test.o',
              command: [ggp_gnu_compiler,
                        '-fplugin=@0@'.format(ggp_gcc_plugin.full_path()),
                        '-c',
                        '@INPUT@'],
              build_always: true,
              build_by_default: false,
              dependencies: [ggp_gcc_plugin])
