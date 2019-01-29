/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2017, 2018, 2019 Krzesimir Nowak
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
/*< lib: value.hh >*/
/*< lib: typeutil.hh >*/
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

#define GGP_LIB_TRIVIAL_NEQ_OP(Type)                                \
  inline auto                                                       \
  operator!= (Type const& lhs, Type const& rhs) noexcept -> bool    \
  {                                                                 \
    return !(lhs == rhs);                                           \
  }                                                                 \
  struct DONOTUSE

#define GGP_LIB_TRIVIAL_TYPE(Type)              \
  struct Type {}

#define GGP_LIB_TRIVIAL_EQ_OPS(Type)                     \
  inline auto                                            \
  operator== (Type const&, Type const&) noexcept -> bool \
  {                                                      \
    return true;                                         \
  }                                                      \
                                                         \
  GGP_LIB_TRIVIAL_NEQ_OP (Type)

#define GGP_LIB_TRIVIAL_TYPE_WITH_OPS(Type) \
  GGP_LIB_TRIVIAL_TYPE (Type);              \
  GGP_LIB_TRIVIAL_EQ_OPS (Type)

#define GGP_LIB_VARIANT_OPS(Type)                                \
  inline auto                                                    \
  operator== (Type const& lhs, Type const& rhs) noexcept -> bool \
  {                                                              \
    return lhs.v == rhs.v;                                       \
  }                                                              \
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

// Generates equality and inequality operators for a given struct
// type. Following parameters are types and names of the fields in the
// struct type. It makes it awkward to use, so use
// GGP_LIB_STRUCT_EQ_OPS2.
//
// Example: GGP_LIB_STRUCT(MyStruct, std::vector<Foo>, all_foos);
#define GGP_LIB_STRUCT_EQ_OPS(Type, ...)                                \
  inline auto                                                           \
  operator== (Type const& lhs, Type const &rhs) noexcept -> bool        \
  {                                                                     \
    return GGP_LIB_DETAIL_STRUCT_EQ_OP(__VA_ARGS__) true;               \
  }                                                                     \
                                                                        \
  GGP_LIB_TRIVIAL_NEQ_OP (Type)

// Generate a struct with given name and fields. Fields are pairs -
// type and name.
//
// Example: GGP_LIB_STRUCT(MyStruct, std::vector<Foo>, all_foos);
#define GGP_LIB_STRUCT(Type, ...)               \
  struct Type                                   \
  {                                             \
    GGP_LIB_DETAIL_STRUCT_FIELDS(__VA_ARGS__)   \
  };                                            \
                                                \
  GGP_LIB_STRUCT_EQ_OPS(Type, __VA_ARGS__)

namespace Ggp::Lib
{

namespace ThisNS = ::Ggp::Lib;

template<class... TypeP>
struct VisitHelper : TypeP...
{
  using TypeP::operator()...;
};
template<class... TypeP>
VisitHelper(TypeP...) -> VisitHelper<TypeP...>;

template <typename SuperVariant,
          typename SubVariant,
          typename = std::enable_if_t<DetailUtilHh::IsStdVariantV<SuperVariant> &&
                                      DetailUtilHh::IsStdVariantV<SubVariant>>>
auto
repackage_v (SubVariant&& v) -> SuperVariant
{
  auto vh {ThisNS::VisitHelper {
    [](auto&& value)
    {
      using ArgType = decltype (value);
      using ValueArgType = ThisNS::DropQualifiersT<ArgType>;
      using ArgTypeInVariant = ThisNS::DetailUtilHh::ArgTypeInVariant<ValueArgType, SuperVariant>;

      return SuperVariant {std::in_place_type<ArgTypeInVariant>, std::forward<ArgType> (value)};
    },
  }};
  return std::visit (vh, std::forward<SubVariant> (v));
}

template <typename SuperVariantStruct,
          typename SubVariantStruct>
auto
repackage (SubVariantStruct const& svs) -> SuperVariantStruct
{
  using SuperVariant = ThisNS::DropQualifiersT<decltype(std::declval<SuperVariantStruct> ().v)>;

  return {ThisNS::repackage_v<SuperVariant> (svs.v)};
}

template <typename OkType, typename FailureType>
struct Result
{
  static_assert (!std::is_same_v<OkType, FailureType>);

  template <typename OkHandler, typename FailureHandler>
  auto
  handle (OkHandler ok_handler, FailureHandler failure_handler) /* -> deduced */
  {
    auto vh {ThisNS::VisitHelper {ok_handler, failure_handler}};
    return std::visit (vh, this->v);
  }

  explicit operator bool () const noexcept
  {
    return std::holds_alternative<OkType>(this->v);
  }

  auto
  operator->() const -> OkType const*
  {
    return std::addressof (std::get<OkType> (this->v));
  }

  auto
  operator->() -> OkType*
  {
    return std::addressof (std::get<OkType> (this->v));
  }

  auto
  operator* () const -> OkType const&
  {
    return std::get<OkType> (this->v);
  }

  auto
  operator* () -> OkType&
  {
    return std::get<OkType> (this->v);
  }

  auto
  get_failure() const -> FailureType const&
  {
    return std::get<FailureType> (this->v);
  }

  auto
  get_failure() -> FailureType&
  {
    return std::get<FailureType> (this->v);
  }

  std::variant<OkType, FailureType> v;
};

template <typename Container>
struct ReverseAdapter
{
  Container& container;
};

template <typename Container>
auto
begin (ReverseAdapter<Container> adapter) /* -> deduced */
{
  return std::rbegin (adapter.container);
}

template <typename Container>
auto
end (ReverseAdapter<Container> adapter) /* -> deduced */
{
  return std::rend (adapter.container);
}

template <typename Container>
auto
reverse (Container& container) -> ReverseAdapter<Container>
{
  return { container };
}

} // namespace Ggp::Lib

#else

#if GGP_LIB_UTIL_HH_CHECK_VALUE != GGP_LIB_UTIL_HH_CHECK
#error "This non standalone header file was included from two different wrappers."
#endif

#endif /* GGP_LIB_UTIL_HH */
