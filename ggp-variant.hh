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
#include "ggp-cstdint.hh"

#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace Ggp
{

// variant type
namespace VT
{

enum class Basic : std::uint8_t;
struct Variant; // v
struct AnyTuple; // r
struct AnyType; // *
struct Array; // a
struct Maybe; // m
struct Tuple; // ()
struct Entry; // {}

} // namespace VT

using VariantType = std::variant
  <
  VT::Basic,
  VT::Maybe,
  VT::Tuple,
  VT::Array,
  VT::Entry,
  VT::Variant,
  VT::AnyTuple,
  VT::AnyType
  >;

namespace VT
{

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

struct AnyType
{
};

struct Variant
{
};

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

enum class BasicMaybePointer : std::uint8_t
{
  String, // s
  ObjectPath, // o
  Signature, // g
  Any, // ?
};

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

namespace VTMod
{

struct Array;
struct Maybe;
struct Tuple;
struct Entry;

using VariantTypeSubSet = std::variant
  <
  VT::Basic,
  Array,
  VT::Variant,
  VT::AnyTuple,
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
enum class Pointer : std::uint8_t;

struct AtVariantType;
struct Convenience;

struct Tuple;
struct Entry;
struct Maybe;

using BasicFormat = std::variant
  <
  VT::Basic,
  AtBasicType,
  Pointer
  >;

using MaybePointer = std::variant
  <
  VTMod::Array,
  AtVariantType,
  BasicMaybePointer,
  VT::Variant,
  VT::AnyType,
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
  VT::Basic basic;
};

enum class Pointer : std::uint8_t
{
  String,
  ObjectPath,
  Signature,
};

struct AtVariantType
{
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
