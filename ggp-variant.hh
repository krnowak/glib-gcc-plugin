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

using Basic = std::variant
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

} // namespace Leaf

// variant type
namespace VT
{

// TODO: Drop it.
enum class Basic : std::uint8_t;
// TODO: Drop it.
struct Variant; // v
// TODO: Drop it.
struct AnyTuple; // r
// TODO: Drop it.
struct AnyType; // *
struct Array; // a
struct Maybe; // m
struct Tuple; // ()
struct Entry; // {}

} // namespace VT

using VariantType = std::variant
  <
  /*
  Leaf::Basic,

  Leaf::Variant,
  Leaf::AnyTuple,
  Leaf::AnyType,
   */
  // TODO: Drop it.
  VT::Basic,
  VT::Maybe,
  VT::Tuple,
  VT::Array,
  VT::Entry,
  // TODO: Drop it.
  VT::Variant,
  // TODO: Drop it.
  VT::AnyTuple,
  // TODO: Drop it.
  VT::AnyType
  >;

namespace VT
{

// TODO: Drop it.
enum class Basic : std::uint8_t
{
  Bool, // b
  Byte, // y
  I16,  // n
  U16,  // q
  I32,  // i
  U32,  // u
  I64,  // x
  U64,  // t
  Handle, // h
  Double, // d
  String, // s
  ObjectPath, // o
  Signature, // g
  Any, // ?
};

// TODO: Drop it.
struct AnyType
{
};

// TODO: Drop it.
struct Variant
{
};

// TODO: Drop it.
struct AnyTuple
{
};

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
  Basic key;
  Util::Value<VariantType> value;
};

} // namespace VT

std::optional<VariantType>
parse_variant_type_string (std::string_view const& string);

// variant format
namespace VF
{

/*
using BasicMaybePointer = std::variant
  <
  Leaf::String,
  Leaf::ObjectPath,
  Leaf::Signature,
  Leaf::AnyBasic,
  >;
*/

// TODO: Drop it.
enum class BasicMaybePointer : std::uint8_t
{
  String, // s
  ObjectPath, // o
  Signature, // g
  Any, // ?
};

/*
using BasicMaybeBool = std::variant
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
  Leaf::Double, // d
  >;
*/

// TODO: Drop it.
enum class BasicMaybeBool : std::uint8_t
{
  Bool, // b
  Byte, // y
  I16,  // n
  U16,  // q
  I32,  // i
  U32,  // u
  I64,  // x
  U64,  // t
  Handle, // h
  Double, // d
};

// TODO: I'm not sure, it is probably unnecessary. Drop it? I think
// that VariantTypeSubSet could be salvaged.
namespace VTMod
{

struct Array;
struct Maybe;
struct Tuple;
struct Entry;

using VariantTypeSubSet = std::variant
  <
  /*
  Leaf::Basic,

  Leaf::Variant,
  Leaf::AnyTuple,
  Leaf::AnyType,

  VT::Array,
   */
  // TODO: Drop it.
  VT::Basic,
  // TODO: Drop it.
  Array,
  // TODO: Drop it.
  VT::Variant,
  // TODO: Drop it.
  VT::AnyTuple,
  // TODO: Drop it.
  VT::AnyType
  >;

using VariantType = std::variant
  <
  VT::Basic,
  Maybe,
  Tuple,
  Array,
  Entry,
  VT::Variant,
  VT::AnyTuple,
  VT::AnyType
  >;

struct Array
{
  Util::Value<VariantType> element_type;
};

using MaybePointer = std::variant
  <
  Array,
  VF::BasicMaybePointer,
  VT::Variant,
  VT::AnyType,
  VT::AnyTuple
  >;

using MaybeBool = std::variant
  <
  VF::BasicMaybeBool,
  Entry,
  Tuple,
  Maybe
  >;

struct Maybe
{
  std::variant<MaybePointer, Util::Value<MaybeBool>> kind;
};

struct Tuple
{
  std::vector<VariantType> types;
};

struct Entry
{
  VT::Basic key;
  Util::Value<VariantType> value;
};

} // namespace VTMod

struct AtBasicType;
/*
using Pointer = std::variant
  <
  Leaf::String,
  Leaf::ObjectPath,
  Leaf::Signature,
  >;
*/
// TODO: Drop it.
enum class Pointer : std::uint8_t;

struct AtVariantType;
struct Convenience;

struct Tuple;
struct Entry;
struct Maybe;

using BasicFormat = std::variant
  <
  /*
  Leaf::Basic,
  */
  // TODO: Drop it.
  VT::Basic,
  AtBasicType,
  Pointer
  >;

using MaybePointer = std::variant
  <
  /*
  VT:Array
  Leaf::Variant,
  Leaf::AnyType,
  Leaf::AnyTuple,
  */
  // TODO: Drop it.
  VTMod::Array,
  AtVariantType,
  BasicMaybePointer,
  // TODO: Drop it.
  VT::Variant,
  // TODO: Drop it.
  VT::AnyType,
  // TODO: Drop it.
  VT::AnyTuple,
  Pointer,
  Convenience
  >;

using MaybeBool = std::variant
  <
  BasicMaybeBool,
  Entry,
  Tuple,
  Maybe
  >;

} // namespace VF

using VariantFormat = std::variant
  <
  VF::VTMod::VariantTypeSubSet,
  VF::AtVariantType,
  VF::Pointer,
  VF::Convenience,
  VF::Maybe,
  VF::Tuple,
  VF::Entry
  >;

namespace VF
{

struct AtBasicType
{
  /*
   Leaf::Basic
  */
  // TODO: Drop it.
  VT::Basic basic;
};

// TODO: Drop it.
enum class Pointer : std::uint8_t
{
  String,
  ObjectPath,
  Signature,
};

struct AtVariantType
{
  /*
  VT::VariantType type
  */
  // TODO: Drop it.
  VTMod::VariantType type;
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

struct Entry
{
  BasicFormat key;
  Util::Value<VariantFormat> value;
};

struct Maybe
{
  std::variant<MaybePointer, Util::Value<MaybeBool>> kind;
};

} // namespace VF

std::optional<VariantFormat>
parse_variant_format_string (std::string_view const& string);

VariantType
variant_format_to_type (VariantFormat const& format);

} // namespace Ggp

#endif /* GGP_VARIANT_HH */
