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

#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace Ggp
{

// variant type

enum struct Basic : std::uint8_t;
struct Variant;
struct AnyTuple;
struct Any;
struct Array;
struct Maybe;
struct Tuple;
struct Entry;

using VariantType = std::variant<Basic, Maybe, Tuple, Array, Entry, Variant, AnyTuple, Any>;

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
  Any // ?
};

struct Any
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

VariantType
parse_variant_type_string (std::string_view const& string);

// variant format

struct AtVariantType;
enum class ConvenienceFormat : std::uint8_t;
struct MaybeFormat;
struct TupleFormat;
struct EntryFormat;

using VariantFormat = std::variant<VariantType, AtVariantType, ConvenienceFormat, MaybeFormat, TupleFormat, EntryFormat>;

struct AtVariantType
{
  VariantType type;
};

enum class ConvenienceFormat : std::uint8_t
{
  StringPtr, // &s
  ObjectPathPtr, // &o,
  SignaturePtr, // &g
  StringArray, // ^as
  StringPtrArray, // ^a&s,
  ObjectPathArray, // ^ao
  ObjectPathPtrArray, // ^a&o
  ByteArray, // ^ay
  ByteArrayPtr, // ^&ay
  ByteArrayArray, // ^aay,
  ByteArrayPtrArray, // ^a&ay
};

struct MaybeFormat
{
  Util::Value<VariantFormat> format;
};

struct TupleFormat
{
  std::vector<VariantFormat> formats;
};

struct EntryFormat
{
  Util::Value<VariantFormat> key;
  Util::Value<VariantFormat> value;
};

VariantFormat
parse_variant_format_string (std::string_view const& string);

} // namespace Ggp

#endif /* GGP_VARIANT_HH */
