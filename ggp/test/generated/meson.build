test_generated_sources = []
foreach f : dependent_sources
  test_generated_sources += custom_target('ggp-test-generated-@0@'.format(f),
                                          input: ['../../lib/@0@'.format(f)],
                                          output: [f],
                                          command: [source_generator_script,
                                                    '--include-input=ggp/lib/@0@'.format(f),
                                                    '--input=@INPUT@',
                                                    '--out-components=ggp,test',
                                                    '--out-gen-components=ggp,test,generated',
                                                    '--output=@OUTPUT@',
                                                    '--style=stl'])
endforeach
