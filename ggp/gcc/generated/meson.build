# This file is part of glib-gcc-plugin.
#
# Copyright 2017 Krzesimir Nowak
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
