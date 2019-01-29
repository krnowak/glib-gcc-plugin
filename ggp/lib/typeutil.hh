/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2019 Krzesimir Nowak
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

/*< check: GGP_LIB_TYPEUTIL_HH_CHECK >*/
/*< stl: type_traits >*/

#ifndef GGP_LIB_TYPEUTIL_HH
#define GGP_LIB_TYPEUTIL_HH

#define GGP_LIB_TYPEUTIL_HH_CHECK_VALUE GGP_LIB_TYPEUTIL_HH_CHECK

namespace Ggp::Lib
{

namespace ThisNS = ::Ggp::Lib;

template< class T >
struct DropQualifiers {
  using Type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template< class T >
using DropQualifiersT = typename DropQualifiers<T>::Type;

} // namespace Ggp::Lib

#else

#if GGP_LIB_TYPEUTIL_HH_CHECK_VALUE != GGP_LIB_TYPEUTIL_HH_CHECK
#error "This non standalone header file was included from two different wrappers."
#endif

#endif /* GGP_LIB_TYPEUTIL_HH */
