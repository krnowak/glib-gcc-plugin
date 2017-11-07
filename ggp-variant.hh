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

#ifndef GGP_VARIANT_HH
#define GGP_VARIANT_HH

#include "ggp-util.hh"

#include <optional>
#include <string_view>
#include <variant>

// TODO: Put variants in structs. Like in ggp-type.hh.

namespace Ggp
{

namespace Leaf
{

struct Variant {}; // v
struct AnyTuple {}; // r
struct AnyType {}; // *

struct Bool {}; // b
struct Byte {}; // y
struct I16 {}; // n
struct U16 {}; // q
struct I32 {}; // i
struct U32 {}; // u
struct I64 {}; // x
struct U64 {}; // t
struct Handle {}; // h
struct Double {}; // d
struct String {}; // s
struct ObjectPath {}; // o
struct Signature {}; // g
struct AnyBasic {}; // ?

struct Basic
{
  using V = std::variant
  <
  Leaf::Bool,
  Leaf::Byte,
  Leaf::I16,
  Leaf::U16,
  Leaf::I32,
  Leaf::U32,
  Leaf::I64,
  Leaf::U64,
  Leaf::Handle,
  Leaf::Double,
  Leaf::String,
  Leaf::ObjectPath,
  Leaf::Signature,
  Leaf::AnyBasic
  >;

  V v;
};

} // namespace Leaf

struct VariantType;

namespace VT
{

struct Array
{
  Util::Value<VariantType> element_type;
};

struct Maybe
{
  Util::Value<VariantType> pointed_type;
};

struct Tuple
{
  std::vector<VariantType> types;
};

struct Entry
{
  Leaf::Basic key;
  Util::Value<VariantType> value;
};

} // namespace VT

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

  V v;
};

std::optional<VariantType>
parse_variant_type_string (std::string_view const& string);

bool
variant_type_is_definite (VariantType const& vt);

struct VariantFormat;

// variant format
namespace VF
{

struct BasicMaybePointer
{
  using V = std::variant
  <
  Leaf::String,
  Leaf::ObjectPath,
  Leaf::Signature,
  Leaf::AnyBasic
  >;

  V v;
};

struct BasicMaybeBool
{
  using V = std::variant
  <
  Leaf::Bool, // b
  Leaf::Byte, // y
  Leaf::I16,  // n
  Leaf::U16,  // q
  Leaf::I32,  // i
  Leaf::U32,  // u
  Leaf::I64,  // x
  Leaf::U64,  // t
  Leaf::Handle, // h
  Leaf::Double // d
  >;

  V v;
};

struct AtBasicType
{
  Leaf::Basic basic;
};

struct Pointer
{
  using V = std::variant
  <
  Leaf::String,
  Leaf::ObjectPath,
  Leaf::Signature
  >;

  V v;
};

struct AtVariantType
{
  VariantType type;
};

struct Convenience
{
  enum class Type : std::uint8_t
  {
    StringArray,
    ObjectPathArray,
    ByteString,
    ByteStringArray,
  };

  enum class Kind : std::uint8_t
  {
    Constant,
    Duplicated,
  };

  Type type;
  Kind kind;
};

struct Tuple
{
  std::vector<VariantFormat> formats;
};

struct BasicFormat
{
  using V = std::variant
  <
  Leaf::Basic,
  AtBasicType,
  Pointer
  >;

  V v;
};

struct Entry
{
  BasicFormat key;
  Util::Value<VariantFormat> value;
};

struct MaybePointer
{
  using V = std::variant
  <
  VT::Array,
  AtVariantType,
  BasicMaybePointer,
  Pointer,
  Convenience
  >;

  V v;
};
struct MaybeBool;

struct Maybe
{
  using V = std::variant<MaybePointer, Util::Value<MaybeBool>>;

  V v;
};

struct MaybeBool
{
  using V = std::variant
  <
  BasicMaybeBool,
  Entry,
  Tuple,
  Maybe
  >;

  V v;
};

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

  V v;
};

std::optional<VariantFormat>
parse_variant_format_string (std::string_view const& string);

VariantType
variant_format_to_type (VariantFormat const& format);

} // namespace Ggp

#endif /* GGP_VARIANT_HH */
