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

/*< check: GGP_LIB_UTIL_HH_CHECK >*/
/*< stl: cassert >*/
/*< stl: memory >*/
/*< stl: type_traits >*/
/*< stl: utility >*/
/*< stl: variant >*/

#ifndef GGP_LIB_UTIL_HH
#define GGP_LIB_UTIL_HH

#define GGP_LIB_UTIL_HH_CHECK_VALUE GGP_LIB_UTIL_HH_CHECK

#include "ggp/lib/detail/util.hh"

#define GGP_LIB_UTIL_STR(x) GGP_LIB_DETAIL_UTIL_STR_HELP_(x)

#define GGP_LIB_TRIVIAL_NEQ_OP(Type)                        \
  inline bool                                               \
  operator!= (Type const& lhs, Type const& rhs) noexcept    \
  {                                                         \
    return !(lhs == rhs);                                   \
  }                                                         \
  struct DONOTUSE

#define GGP_LIB_TRIVIAL_TYPE(Type)              \
  struct Type {}

#define GGP_LIB_TRIVIAL_EQ_OPS(Type)                     \
  inline bool                                            \
  operator== (Type const&, Type const&) noexcept         \
  {                                                      \
    return true;                                         \
  }                                                      \
                                                         \
  GGP_LIB_TRIVIAL_NEQ_OP (Type)

#define GGP_LIB_TRIVIAL_TYPE_WITH_OPS(Type) \
  GGP_LIB_TRIVIAL_TYPE (Type);              \
  GGP_LIB_TRIVIAL_EQ_OPS (Type)

#define GGP_LIB_VARIANT_OPS(Type)                           \
  inline bool                                               \
  operator== (Type const& lhs, Type const& rhs) noexcept    \
  {                                                         \
    return lhs.v == rhs.v;                                  \
  }                                                         \
                                                            \
  GGP_LIB_TRIVIAL_NEQ_OP (Type)


#define GGP_LIB_VARIANT_STRUCT_ONLY(Type, ...)              \
  struct Type                                               \
  {                                                         \
    using V = std::variant<__VA_ARGS__>;                    \
                                                            \
    V v;                                                    \
  }

#define GGP_LIB_VARIANT_STRUCT(Type, ...)                   \
  GGP_LIB_VARIANT_STRUCT_ONLY(Type, __VA_ARGS__);           \
                                                            \
  GGP_LIB_VARIANT_OPS (Type)

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
    using std::swap;

    swap (ptr, other.ptr);
  }

  operator T const& () const noexcept
  {
    auto p {ptr.get ()};
    assert (p != nullptr);
    return *p;
  }

  operator T& () noexcept
  {
    auto p {ptr.get ()};
    assert (p != nullptr);
    return *p;
  }

  T*
  operator-> () noexcept
  {
    auto p {ptr.get ()};
    assert (p != nullptr);
    return p;
  }

  const T*
  operator-> () const noexcept
  {
    auto p {ptr.get ()};
    assert (p != nullptr);
    return p;
  }

private:
  std::unique_ptr<T> ptr;
};

template <typename T, typename U>
inline bool
operator== (Value<T> const& lhs, Value<U> const& rhs) noexcept
{
  // This is to force the comparison after applying the implicit
  // conversion operators.
  return [](T const& vlhs, U const& vrhs) -> bool
  {
    return vlhs == vrhs;
  } (lhs, rhs);
}

template <typename T, typename U>
inline bool
operator!= (Value<T> const& lhs, Value<U> const& rhs) noexcept
{
  return !(lhs == rhs);
}

// TODO: drop it if unused
template <typename T, typename... Args>
Value<T>
value (Args&&... args)
{
  return {std::forward<Args> (args)...};
}

template<class... TypeP> struct VisitHelper : TypeP... { using TypeP::operator()...; };
template<class... TypeP> VisitHelper(TypeP...) -> VisitHelper<TypeP...>;

template <typename VariantTo,
          typename VariantFrom,
          typename = std::enable_if<Detail::is_std_variant_v<VariantTo> &&
                                    Detail::is_std_variant_v<VariantFrom>>>
VariantTo
generalize (VariantFrom&& v)
{
  return std::visit ([](auto&& value)
                     {
                       using ArgType = decltype (value);
                       using ArgTypeInVariant = std::decay_t<ArgType>;
                       Detail::std_variant_type_check<ArgTypeInVariant, VariantTo> ();
                       return VariantTo {std::in_place_type<ArgTypeInVariant>, std::forward<ArgType> (value)};
                     },
                     std::forward<VariantFrom> (v));
}

template <typename OkType, typename FailureType>
struct Result
{
  static_assert (!std::is_same_v<OkType, FailureType>);

  template <typename OkHandler, typename FailureHandler>
  auto handle(OkHandler ok_handler, FailureHandler failure_handler)
  {
    auto vh {VisitHelper {ok_handler, failure_handler}};
    return std::visit (vh, this->v);
  }

  explicit operator bool () const noexcept()
  {
    return this->v.holds_alternative<OkType> ();
  }

  auto operator->() const -> OkType const*
  {
    return std::addressof (std::get<OkType> (this->v));
  }

  auto operator->() -> OkType*
  {
    return std::addressof (std::get<OkType> (this->v));
  }

  auto operator* () const -> OkType const&
  {
    return std::get<OkType> (this->v);
  }

  auto operator* () -> OkType&
  {
    return std::get<OkType> (this->v);
  }

  auto get_failure() const -> FailureType const&
  {
    return std::get<FailureType> (this->v);
  }

  auto get_failure() -> FailureType&
  {
    return std::get<FailureType> (this->v);
  }

  std::variant<OkType, FailureType> v;
};

} // namespace Ggp::Lib

#else

#if GGP_LIB_UTIL_HH_CHECK_VALUE != GGP_LIB_UTIL_HH_CHECK
#error "This non standalone header file was included from two different wrappers."
#endif

#endif /* GGP_LIB_UTIL_HH */
