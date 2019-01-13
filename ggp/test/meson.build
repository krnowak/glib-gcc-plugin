# This file is part of glib-gcc-plugin.
#
# Copyright 2017, 2018 Krzesimir Nowak
#
# gcc-glib-plugin is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# gcc-glib-plugin is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# gcc-glib-plugin. If not, see <http://www.gnu.org/licenses/>.

subdir('generated')

test_sources = [
    'main.cc',
    'variant-test.cc',
    'type-test.cc',
    'test-print.hh',
    'test-print.cc',
]

test_lib = executable('variant-test',
                      sources: [test_sources, test_generated_sources, ggp_pp_generated_sources],
                      include_directories: toplevel_inc,
                      cpp_args: ['-Wall', '-Wextra', '-Wpedantic', '-std=c++17'],
                      implicit_include_directories: false,
                      build_by_default: false)

test('test-lib', test_lib)
