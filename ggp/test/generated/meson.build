test_generated_sources = []
foreach f : dependent_sources
  test_generated_sources += custom_target('ggp-test-generated-@0@'.format(f),
                                          input: [join_paths('..', '..', 'lib', f)],
                                          output: [f],
                                          command: [source_generator_script,
                                                    '--in-components=ggp,lib',
                                                    '--input=@INPUT@',
                                                    '--out-components=ggp,test',
                                                    '--out-gen-components=ggp,test,generated',
                                                    '--output=@OUTPUT@',
                                                    '--style=std'])
endforeach
