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

#ifndef GGP_CSTDINT_HH
#define GGP_CSTDINT_HH

#include <cstdint>

// There is some bug in libstdc++ that causes macros below to be
// defined, which may wreak havoc in other C++ headers.

#undef isspace
#undef isprint
#undef iscntrl
#undef isupper
#undef islower
#undef isalpha
#undef isdigit
#undef ispunct
#undef isxdigit
#undef isalnum
#undef isgraph
#undef toupper
#undef tolower

#endif /* GGP_CSTDINT_HH */
