# This file is part of gcc-glib-plugin.
#
# Copyright 2017 Krzesimir Nowak
#
# gcc-glib-plugin is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# gcc-glib-plugin is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# gcc-glib-plugin. If not, see <http://www.gnu.org/licenses/>.

PLUGIN := glib-gcc-plugin.so

all: $(PLUGIN)

ifeq ($(CXX),g++)
COMPILER_CXXFLAGS := -std=c++17
endif
ifeq ($(CXX),clang++)
COMPILER_CXXFLAGS := -std=c++1z --stdlib=libc++
endif
ifeq ($(COMPILER_CXXFLAGS),)
$(error unknown compiler $(CXX), use clang++ or g++)
endif

CXXFLAGS := -I`gcc -print-file-name=plugin`/include -fno-rtti -O2 -fPIC -Wall -Wextra -Wpedantic $(COMPILER_CXXFLAGS) -fdiagnostics-color=auto

GGP_CC_HH_SOURCES := ggp-main.cc ggp-util.cc ggp-vc.cc ggp-tc.cc ggp-variant.cc ggp-type.cc

GGP_SOURCES := $(GGP_CC_HH_SOURCES) ggp-plugin.cc

%.o: Makefile

$(PLUGIN): $(GGP_SOURCES:.cc=.o)
	g++ -shared -fno-rtti -fPIC -o $@ $^

test: $(PLUGIN)
	gcc -fplugin=./$(PLUGIN) -c test.c

clean:
	rm -f $(GGP_SOURCES:.cc=.o) $(PLUGIN)

.PHONY: all test clean
