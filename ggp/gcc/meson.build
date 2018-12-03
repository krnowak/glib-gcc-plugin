subdir('generated')

ggp_gcc_sources = [
  'gcc.hh',
  'main.cc',
  'main.hh',
  'plugin.cc',
  'tc.cc',
  'tc.hh',
  'tree.cc',
  'tree.hh',
  'util.cc',
  'util.hh',
  'vc.cc',
  'vc.hh',
  'token.hh',
]

ggp_gcc_plugin_dir = run_command('g++', '-print-file-name=plugin').stdout().strip()
ggp_gcc_plugin_inc = include_directories(join_paths(ggp_gcc_plugin_dir, 'include'))

ggp_gcc_plugin = shared_module('glib-gcc-plugin',
                               sources: [ggp_gcc_sources, ggp_gcc_generated_sources, ggp_pp_generated_sources],
                               include_directories: [toplevel_inc, ggp_gcc_plugin_inc],
                               cpp_args: ['-Wall', '-Wextra', '-Wpedantic', '-std=c++17', '-fno-rtti'],
                               implicit_include_directories: false,
                               build_by_default: true,
                               name_prefix: '')
