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

/*< check: GGP_LIB_VARIANT_HH_CHECK >*/
/*< lib: util.hh >*/
/*< stl: optional >*/
/*< stl: string_view >*/
/*< stl: tuple >*/
/*< stl: variant >*/
/*< stl: vector >*/

#ifndef GGP_LIB_VARIANT_HH
#define GGP_LIB_VARIANT_HH

#define GGP_LIB_VARIANT_HH_CHECK_VALUE GGP_LIB_VARIANT_HH_CHECK

namespace Ggp::Lib
{

namespace Leaf
{

GGP_LIB_TRIVIAL_TYPE_WITH_OPS (Variant); // v
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (AnyTuple); // r
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (AnyType); // *
inline constexpr Variant variant {};
inline constexpr AnyTuple any_tuple {};
inline constexpr AnyType any_type {};

GGP_LIB_TRIVIAL_TYPE_WITH_OPS (Bool); // b
inline constexpr Bool bool_ {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (Byte); // y
inline constexpr Byte byte_ {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (I16); // n
inline constexpr I16 i16 {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (U16); // q
inline constexpr U16 u16 {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (I32); // i
inline constexpr I32 i32 {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (U32); // u
inline constexpr U32 u32 {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (I64); // x
inline constexpr I64 i64 {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (U64); // t
inline constexpr U64 u64 {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (Handle); // h
inline constexpr Handle handle {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (Double); // d
inline constexpr Double double_ {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (String); // s
inline constexpr String string_ {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (ObjectPath); // o
inline constexpr ObjectPath object_path {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (Signature); // g
inline constexpr Signature signature {};
GGP_LIB_TRIVIAL_TYPE_WITH_OPS (AnyBasic); // ?
inline constexpr AnyBasic any_basic {};

GGP_LIB_VARIANT_STRUCT (Basic,
                        Bool,
                        Byte,
                        I16,
                        U16,
                        I32,
                        U32,
                        I64,
                        U64,
                        Handle,
                        Double,
                        String,
                        ObjectPath,
                        Signature,
                        AnyBasic);

} // namespace Leaf

struct VariantType;

namespace VT
{

struct Array
{
  Value<VariantType> element_type;
};

inline bool
operator== (Array const& lhs, Array const& rhs) noexcept
{
  return lhs.element_type == rhs.element_type;
}

GGP_LIB_TRIVIAL_NEQ_OP (Array);

struct Maybe
{
  Value<VariantType> pointed_type;
};

inline bool
operator== (Maybe const& lhs, Maybe const& rhs) noexcept
{
  return lhs.pointed_type == rhs.pointed_type;
}

GGP_LIB_TRIVIAL_NEQ_OP (Maybe);

struct Tuple
{
  std::vector<VariantType> types;
};

inline bool
operator== (Tuple const& lhs, Tuple const& rhs) noexcept
{
  return lhs.types == rhs.types;
}

GGP_LIB_TRIVIAL_NEQ_OP (Tuple);

struct Entry
{
  Leaf::Basic key;
  Value<VariantType> value;
};

inline bool
operator== (Entry const& lhs, Entry const& rhs) noexcept
{
  return std::tie (lhs.key, lhs.value) == std::tie (rhs.key, rhs.value);
}

GGP_LIB_TRIVIAL_NEQ_OP (Entry);

} // namespace VT

struct VariantParseError
{
  std::size_t offset;
  std::string reason;
};

template <typename ErrorT>
struct ErrorCascade
{
  std::vector<ErrorT> errors;
};

using VariantParseErrorCascade = ErrorCascade<VariantParseError>;

template <typename OkType>
using VariantResult = Result<OkType, VariantParseErrorCascade>;

struct VariantType
{
  using V = std::variant
  <
  Leaf::Basic,
  VT::Maybe,
  VT::Tuple,
  VT::Array,
  VT::Entry,
  Leaf::Variant,
  Leaf::AnyTuple,
  Leaf::AnyType
  >;

  static auto from_string (std::string_view const& string) -> VariantResult<VariantType>;

  auto is_definite() const -> bool;

  V v;
};

GGP_LIB_VARIANT_OPS (VariantType);

struct VariantFormat;

// variant format
namespace VF
{

GGP_LIB_VARIANT_STRUCT (BasicMaybePointer,
                        Leaf::String,
                        Leaf::ObjectPath,
                        Leaf::Signature,
                        Leaf::AnyBasic);

GGP_LIB_VARIANT_STRUCT (BasicMaybeBool,
                        Leaf::Bool,
                        Leaf::Byte,
                        Leaf::I16,
                        Leaf::U16,
                        Leaf::I32,
                        Leaf::U32,
                        Leaf::I64,
                        Leaf::U64,
                        Leaf::Handle,
                        Leaf::Double);

struct AtBasicType
{
  Leaf::Basic basic;
};

inline bool
operator== (AtBasicType const& lhs, AtBasicType const& rhs) noexcept
{
  return lhs.basic == rhs.basic;
}

GGP_LIB_TRIVIAL_NEQ_OP (AtBasicType);

GGP_LIB_VARIANT_STRUCT (Pointer,
                        Leaf::String,
                        Leaf::ObjectPath,
                        Leaf::Signature);

struct AtVariantType
{
  VariantType type;
};

inline bool
operator== (AtVariantType const& lhs, AtVariantType const& rhs) noexcept
{
  return lhs.type == rhs.type;
}

GGP_LIB_TRIVIAL_NEQ_OP (AtVariantType);

struct Convenience
{
  struct Type
  {
    GGP_LIB_TRIVIAL_TYPE (StringArray);
    static inline constexpr StringArray string_array {};
    GGP_LIB_TRIVIAL_TYPE (ObjectPathArray);
    static inline constexpr ObjectPathArray object_path_array {};
    GGP_LIB_TRIVIAL_TYPE (ByteString);
    static inline constexpr ByteString byte_string {};
    GGP_LIB_TRIVIAL_TYPE (ByteStringArray);
    static inline constexpr ByteStringArray byte_string_array {};

    using V = std::variant
      <
      StringArray,
      ObjectPathArray,
      ByteString,
      ByteStringArray
      >;

    V v;
  };

  struct Kind
  {
    GGP_LIB_TRIVIAL_TYPE (Constant);
    static inline constexpr Constant constant {};
    GGP_LIB_TRIVIAL_TYPE (Duplicated);
    static inline constexpr Duplicated duplicated {};

    using V = std::variant
      <
      Constant,
      Duplicated
      >;

    V v;
  };

  Type type;
  Kind kind;
};

GGP_LIB_TRIVIAL_EQ_OPS(Convenience::Type::StringArray);
GGP_LIB_TRIVIAL_EQ_OPS(Convenience::Type::ObjectPathArray);
GGP_LIB_TRIVIAL_EQ_OPS(Convenience::Type::ByteString);
GGP_LIB_TRIVIAL_EQ_OPS(Convenience::Type::ByteStringArray);
GGP_LIB_TRIVIAL_EQ_OPS(Convenience::Kind::Constant);
GGP_LIB_TRIVIAL_EQ_OPS(Convenience::Kind::Duplicated);
GGP_LIB_VARIANT_OPS(Convenience::Type);
GGP_LIB_VARIANT_OPS(Convenience::Kind);

inline bool
operator== (Convenience const& lhs, Convenience const& rhs) noexcept
{
  return std::tie (lhs.type, lhs.kind) == std::tie (rhs.type, rhs.kind);
}

GGP_LIB_TRIVIAL_NEQ_OP (Convenience);

struct Tuple
{
  std::vector<VariantFormat> formats;
};

inline bool
operator== (Tuple const& lhs, Tuple const& rhs) noexcept
{
  return lhs.formats == rhs.formats;
}

GGP_LIB_TRIVIAL_NEQ_OP (Tuple);

GGP_LIB_VARIANT_STRUCT (BasicFormat,
                        Leaf::Basic,
                        AtBasicType,
                        Pointer);

struct Entry
{
  BasicFormat key;
  Value<VariantFormat> value;
};

inline bool
operator== (Entry const& lhs, Entry const& rhs) noexcept
{
  return std::tie (lhs.key, lhs.value) == std::tie (rhs.key, rhs.value);
}

GGP_LIB_TRIVIAL_NEQ_OP (Entry);

GGP_LIB_VARIANT_STRUCT (MaybePointer,
                        VT::Array,
                        AtVariantType,
                        BasicMaybePointer,
                        Pointer,
                        Convenience);

struct MaybeBool;

GGP_LIB_VARIANT_STRUCT (Maybe,
                        MaybePointer,
                        Value<MaybeBool>);

GGP_LIB_VARIANT_STRUCT (MaybeBool,
                        BasicMaybeBool,
                        Entry,
                        Tuple,
                        Maybe);

} // namespace VF

struct VariantFormat
{
  using V = std::variant
  <
  Leaf::Basic,
  VT::Array,
  VF::AtVariantType,
  VF::Pointer,
  VF::Convenience,
  VF::Maybe,
  VF::Tuple,
  VF::Entry
  >;

  static auto from_string (std::string_view const& string) -> VariantResult<VariantFormat>;

  auto to_type () const -> VariantType;

  V v;
};

GGP_LIB_VARIANT_OPS (VariantFormat);

} // namespace Ggp::Lib

#else

#if GGP_LIB_VARIANT_HH_CHECK_VALUE != GGP_LIB_VARIANT_HH_CHECK
#error "This non standalone header file was included from two different wrappers."
#endif

#endif /* GGP_LIB_VARIANT_HH */
