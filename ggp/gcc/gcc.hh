/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2017 Krzesimir Nowak
 *
 * gcc-glib-plugin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * gcc-glib-plugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * gcc-glib-plugin. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GGP_GCC_HH
#define GGP_GCC_HH

#define INCLUDE_ALGORITHM
#define INCLUDE_LIST
#define INCLUDE_MAP
#define INCLUDE_SET
#define INCLUDE_STRING
#define INCLUDE_VECTOR

// Do not sort.

#include "gcc-plugin.h"
#include "diagnostic.h"
#include "errors.h"
#include "tree.h"
#include "dumpfile.h"
#include "tree-iterator.h"
#include "tree-pass.h"
#include "tree-cfg.h"
#include "context.h"
#include "stringpool.h"
#include "attribs.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"

// system.h header includes ctype.h, which defines the macros undeffed
// below. system.h actually indirectly undefs them and replaces them
// with some bogus stuff that would make the compilation to fail if
// any of the macros were used. But macros being macros, they blindly
// break stuff when declaring or using a function named like them but
// inside some namespace. For example including <iterator> triggers
// this.
#undef isalpha
#undef isalnum
#undef iscntrl
#undef isdigit
#undef isgraph
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit
#undef toupper
#undef tolower

#endif /* GGP_GCC_HH */
