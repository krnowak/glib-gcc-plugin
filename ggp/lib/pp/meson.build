pp_gen = find_program('pp-gen.py')
ggp_lib_pp_generated_sources = []
ggp_lib_pp_generated_sources += custom_target('ggp-lib-pp-pp-gen.hh',
                                              input: ['pp-gen.hh.in'],
                                              output: ['pp-gen.hh'],
                                              command: [pp_gen,
                                                        '@INPUT@',
                                                        '@OUTPUT@'])
