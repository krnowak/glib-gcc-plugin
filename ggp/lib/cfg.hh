/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2018 Krzesimir Nowak
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

/*< lib: util.hh >*/
/*< stl: variant >*/
/*< stl: vector >*/

#ifndef GGP_LIB_CFG_HH
#define GGP_LIB_CFG_HH

#define GGP_LIB_CFG_HH_CHECK_VALUE GGP_LIB_CFG_HH_CHECK

namespace Ggp::Lib
{

template <typename Impl>
class BasicBlock
{
private:
  typename Impl::BasicBlock impl;
};

enum class EdgeType
{
  FALLTROUGH,
  TRUE,
  FALSE,
  ABNORMAL
};

template <typename Impl>
class Edge
{
public:
  EdgeType type () const;

private:
  typename Impl::Edge impl;
};

template <typename Impl>
class Loop
{

private:
  typename Impl::Loop impl;
};

template <typename Impl>
class Cfg
{
public:
  BasicBlock<typename Impl::BasicBlock> entry () const;

private:
  typename Impl::Cfg impl;
};

} // namespace Ggp::Lib

#else

#if GGP_LIB_CFG_HH_CHECK_VALUE != GGP_LIB_CFG_HH_CHECK
#error "This non standalone header file was included from two different wrappers."
#endif

#endif // GGP_LIB_CFG_HH
