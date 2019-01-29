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

/*< check: GGP_LIB_VALUE_HH_CHECK >*/
/*< stl: cassert >*/
/*< stl: memory >*/
/*< stl: utility >*/

#ifndef GGP_LIB_VALUE_HH
#define GGP_LIB_VALUE_HH

#define GGP_LIB_VALUE_HH_CHECK_VALUE GGP_LIB_VALUE_HH_CHECK

namespace Ggp::Lib
{

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
  auto
  operator= (Value<U>&& u) noexcept -> Value&
  {
    Value tmp {std::move (u)};

    this->swap (tmp);
    return *this;
  }

  template <typename U>
  auto
  operator= (const Value<U>& u) -> Value&
  {
    Value tmp {u};
    this->swap (tmp);
    return *this;
  }

  auto
  swap (Value& other) noexcept -> void
  {
    using std::swap;

    swap (this->ptr, other.ptr);
  }

  operator T const& () const noexcept
  {
    auto p {this->ptr.get ()};
    assert (p != nullptr);
    return *p;
  }

  operator T& () noexcept
  {
    auto p {this->ptr.get ()};
    assert (p != nullptr);
    return *p;
  }

  auto
  operator-> () noexcept -> T*
  {
    auto p {this->ptr.get ()};
    assert (p != nullptr);
    return p;
  }

  auto
  operator-> () const noexcept -> T const*
  {
    auto p {this->ptr.get ()};
    assert (p != nullptr);
    return p;
  }

private:
  std::unique_ptr<T> ptr;
};

template <typename T, typename U>
inline auto
operator== (Value<T> const& lhs, Value<U> const& rhs) noexcept -> bool
{
  // This is to force the comparison after applying the implicit
  // conversion operators.
  return [](T const& vlhs, U const& vrhs) -> bool
  {
    return vlhs == vrhs;
  } (lhs, rhs);
}

template <typename T, typename U>
inline auto
operator!= (Value<T> const& lhs, Value<U> const& rhs) noexcept -> bool
{
  return !(lhs == rhs);
}

// TODO: drop it if unused
template <typename T, typename... Args>
inline auto
value (Args&&... args) -> Value<T>
{
  return {std::forward<Args> (args)...};
}

} // namespace Ggp::Lib

#else

#if GGP_LIB_VALUE_HH_CHECK_VALUE != GGP_LIB_VALUE_HH_CHECK
#error "This non standalone header file was included from two different wrappers."
#endif

#endif /* GGP_LIB_VALUE_HH */
