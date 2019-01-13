# This file is part of glib-gcc-plugin.
#
# Copyright 2018 Krzesimir Nowak
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
