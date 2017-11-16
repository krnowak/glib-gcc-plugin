ggp_gcc_generated_sources = []
foreach f : dependent_sources
  ggp_gcc_generated_sources += custom_target('ggp-gcc-generated-@0@'.format(f),
                                             input: [join_paths('..', '..', 'lib', f)],
                                             output: [f],
                                             command: [source_generator_script,
                                                       '--in-components=ggp,lib',
                                                       '--input=@INPUT@',
                                                       '--out-components=ggp,gcc',
                                                       '--out-gen-components=ggp,gcc,generated',
                                                       '--output=@OUTPUT@',
                                                       '--style=gcc'])
endforeach
