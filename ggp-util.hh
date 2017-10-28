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

#ifndef GGP_UTIL_HH
#define GGP_UTIL_HH

#include "ggp-gcc.hh"

#include <memory>

#define GGP_UTIL_STR_HELP_(x) #x
#define GGP_UTIL_STR(x) GGP_UTIL_STR_HELP_(x)

namespace Ggp
{

namespace Util
{

std::string
subplugin_name (struct plugin_name_args* plugin_info,
                const char* suffix);

struct CallbackRegistration
{
  CallbackRegistration (const std::string& plugin_name,
                        int event,
                        plugin_callback_func callback,
                        void* user_data);

  ~CallbackRegistration ();

private:
  std::string plugin_name;
  int event;
};

// after it is moved, it can only be destroyed
template <typename T>
class Value
{
public:
  Value () = delete;
  template <typename... Args>
  Value (Args&&... args)
    : ptr {std::make_unique<T> (std::forward<Args> (args)...)}
  {}

  Value (Value&& u) noexcept
    : ptr {std::move (u.ptr)}
  {}

  template <typename U>
  Value (Value<U>&& u) noexcept
    : ptr {std::move (u.ptr)}
  {}

  template <typename U>
  Value (Value<U> const& u)
    : ptr {std::make_unique<T> (*(u.ptr))}
  {}

  Value (Value const& u)
    : ptr {std::make_unique<T> (*(u.ptr))}
  {}

  ~Value () noexcept = default;

  template <typename U>
  Value&
  operator= (Value<U>&& u) noexcept
  {
    Value tmp {std::move (u)};

    swap (tmp);
    return *this;
  }

  template <typename U>
  Value&
  operator= (const Value<U>& u)
  {
    Value tmp {u};
    swap (tmp);
    return *this;
  }

  void
  swap (Value& other) noexcept
  {
    ptr.swap (other.ptr);
  }

  operator const T& () const noexcept
  {
    gcc_assert (ptr.get() != nullptr);
    return *ptr;
  }

  operator T& () noexcept
  {
    gcc_assert (ptr.get() != nullptr);
    return *ptr;
  }

private:
  std::unique_ptr<T> ptr;
};

template<class... TypeP> struct VisitHelper : TypeP... { using TypeP::operator()...; };
template<class... TypeP> VisitHelper(TypeP...) -> VisitHelper<TypeP...>;

} // namespace Util

} // namespace Ggp

#endif /* GGP_UTIL_HH */
