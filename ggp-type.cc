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

#include "ggp-type.hh"

namespace Ggp
{

namespace
{

using namespace std::string_literals;

PlainType
gchar_type ()
{
  return {{Integral {"gchar"s, {"char"s}, {}, 1u, Signedness::Any}}};
}

PlainType
gboolean_type ()
{
  return {{Integral {"gboolean"s, {}, {"gint"s, "int"s}, SIZEOF_INT, Signedness::Any}}};
}

PlainType
guchar_type ()
{
  return {{Integral {"guchar"s, {}, {}, 1u, Signedness::Unsigned}}};
}

PlainType
gint16_type ()
{
  return {{Integral {"gint16"s, {}, {}, 2u, Signedness::Signed}}};
}

PlainType
guint16_type ()
{
  return {{Integral {"guint16"s, {}, {}, 2u, Signedness::Unsigned}}};
}

PlainType
gint32_type ()
{
  return {{Integral {"gint32"s, {}, {}, 4u, Signedness::Signed}}};
}

PlainType
guint32_type ()
{
  return {{Integral {"guint32"s, {}, {}, 4u, Signedness::Unsigned}}};
}

PlainType
gint64_type ()
{
  return {{Integral {"gint64"s, {}, {}, 8u, Signedness::Signed}}};
}

PlainType
guint64_type ()
{
  return {{Integral {"guint64"s, {}, {}, 8u, Signedness::Unsigned}}};
}

PlainType
handle_type ()
{
  return {{Integral {"gint32"s, {}, {}, 4u, Signedness::Signed}}};
}

PlainType
gdouble_type ()
{
  return {{Real {"gdouble"s, {"double"s}, 8u}}};
}

template <typename T>
std::vector<Types>
types (T const& t)
{
  return {{{{t}}, {{Pointer {{t}}}}}};
}

Pointer
gvariant_type (VariantType const& vt)
{
  return {{PlainType {{VariantTyped {"GVariant"s, {{vt}}}}}}};
}

std::vector<Types>
gvariant_types_v (VariantType const& vt)
{
  return types (gvariant_type (vt));
}

Pointer
const_str ()
{
  return {{Const {{gchar_type ()}}}};
}

Pointer
str ()
{
  return {{gchar_type ()}};
}

std::vector<Types>
string_types ()
{
  return {{{{const_str ()}}, {{Pointer {{str ()}}}}}};
}

std::vector<Types>
leaf_basic_to_types (Leaf::Basic const& basic)
{
  auto v {Util::VisitHelper {
    [](Leaf::Bool const&) { return types (gboolean_type ()); },
    [](Leaf::Byte const&) { return types (guchar_type ()); },
    [](Leaf::I16 const&) { return types (gint16_type ()); },
    [](Leaf::U16 const&) { return types (guint16_type ()); },
    [](Leaf::I32 const&) { return types (gint32_type ()); },
    [](Leaf::U32 const&) { return types (guint32_type ()); },
    [](Leaf::I64 const&) { return types (gint64_type ()); },
    [](Leaf::U64 const&) { return types (guint64_type ()); },
    [](Leaf::Handle const&) { return types (handle_type ()); },
    [](Leaf::Double const&) { return types (gdouble_type ()); },
    [](Leaf::String const&) { return string_types (); },
    [](Leaf::ObjectPath const&) { return string_types (); },
    [](Leaf::Signature const&) { return string_types (); },
    [](Leaf::AnyBasic const& any) { return gvariant_types_v (VariantType {Leaf::Basic {any}}); },
  }};
  return std::visit (v, basic);
}

template <typename Ptr>
std::vector<Types>
array_to_types (VariantType const& vt)
{
  return {{{{Ptr {{PlainType {{VariantTyped {"GVariantBuilder"s, {{vt}}}}}}}}}, {{Ptr {{Pointer {{PlainType {{VariantTyped {"GVariantIter"s, {{vt}}}}}}}}}}}}};
}

std::vector<Types>
pointer_to_types ()
{
  return {{{{const_str ()}}, {{Pointer {{const_str ()}}}}}};
}

std::vector<Types>
convenience_to_types (VF::Convenience const& convenience)
{
  switch (convenience.type)
  {
  case VF::Convenience::Type::StringArray:
  case VF::Convenience::Type::ObjectPathArray:
  case VF::Convenience::Type::ByteStringArray:
    {
      auto for_new {Type {{Pointer {{Const {{const_str ()}}}}}}};
      switch (convenience.kind)
      {
      case VF::Convenience::Kind::Constant:
        return {{for_new, {{Pointer {{Pointer {{const_str ()}}}}}}}};
      case VF::Convenience::Kind::Duplicated:
        return {{for_new, {{Pointer {{Pointer {{str ()}}}}}}}};
      default:
        gcc_unreachable ();
        return {};
      }
    }
  case VF::Convenience::Type::ByteString:
    {
      auto for_new {Type {{const_str ()}}};
      switch (convenience.kind)
      {
      case VF::Convenience::Kind::Constant:
        return {{for_new, {{Pointer {{const_str ()}}}}}};
      case VF::Convenience::Kind::Duplicated:
        return {{for_new, {{Pointer {{str ()}}}}}};
      default:
        gcc_unreachable ();
        return {};
      }
    }
  default:
    gcc_unreachable ();
    return {};
  }
}

std::vector<Types>
basic_maybe_bool_to_types (VF::BasicMaybeBool const& bmb)
{
  return leaf_basic_to_types (Util::generalize<Leaf::Basic> (bmb));
}

std::vector<Types>
maybe_to_types (VF::Maybe const& maybe);

std::vector<Types>
tuple_to_types (VF::Tuple const& tuple);

std::vector<Types>
entry_to_types (VF::Entry const& entry);

std::vector<Types>
maybe_bool_to_types (VF::MaybeBool const& mb)
{
  auto types {leaf_basic_to_types (Leaf::Bool {})};
  auto v {Util::VisitHelper {
    [](VF::BasicMaybeBool const& bmb) { return basic_maybe_bool_to_types (bmb); },
    [](VF::Entry const& entry) { return entry_to_types (entry); },
    [](VF::Tuple const& tuple) { return tuple_to_types (tuple); },
    [](VF::Maybe const& maybe) { return maybe_to_types (maybe); },
  }};
  auto more_types {std::visit (v, mb)};
  std::move (more_types.begin (), more_types.end(), std::back_inserter (types));
  return types;
}

// TODO: const_str() returns Pointer, not NullablePointer
std::vector<Types>
basic_maybe_pointer_to_types (VF::BasicMaybePointer const& bmp)
{
  auto v {Util::VisitHelper {
    [](Leaf::String const&) { return std::vector {Types {{{const_str ()}}, {{Pointer {{str ()}}}}}}; },
    [](Leaf::ObjectPath const&) { return std::vector {Types {{{const_str ()}}, {{Pointer {{str ()}}}}}}; },
    [](Leaf::Signature const&) { return std::vector {Types {{{const_str ()}}, {{Pointer {{str ()}}}}}}; },
    [](Leaf::AnyBasic const& any) { return gvariant_types_v (VariantType {Leaf::Basic {any}}); },
  }};
  return std::visit (v, bmp);
}

// TODO: we need to return NullablePointer here.
std::vector<Types>
maybe_pointer_to_types (VF::MaybePointer const& mp)
{
  auto v {Util::VisitHelper {
    [](VT::Array const& array) { return array_to_types<NullablePointer> (array.element_type); },
    [](VF::AtVariantType const& avt) { return gvariant_types_v (avt.type); },
    [](VF::BasicMaybePointer const& bmb) { return basic_maybe_pointer_to_types (bmb); },
    [](VF::Pointer const&) { return pointer_to_types (); },
    [](VF::Convenience const& convenience) { return convenience_to_types (convenience); },
  }};
  return std::visit (v, mp);
}

std::vector<Types>
maybe_to_types (VF::Maybe const& maybe)
{
  auto v {Util::VisitHelper {
    [](VF::MaybePointer const& mp) { return maybe_pointer_to_types (mp); },
    [](VF::MaybeBool const& mb) { return maybe_bool_to_types (mb); },
  }};
  return std::visit (v, maybe.kind);
}

std::vector<Types>
format_to_types (VariantFormat const& format);

std::vector<Types>
tuple_to_types (VF::Tuple const& tuple)
{
  std::vector<Types> types {};
  for (auto const& format : tuple.formats)
  {
    auto element_types {format_to_types (format)};
    std::move (element_types.begin (), element_types.end (), std::back_inserter (types));
  }
  return types;
}

std::vector<Types>
basic_format_to_types (VF::BasicFormat const& bf)
{
  auto v {Util::VisitHelper {
    [](Leaf::Basic const& basic) { return leaf_basic_to_types (basic); },
    [](VF::AtBasicType const& abt) { return gvariant_types_v (VariantType {abt.basic}); },
    [](VF::Pointer const&) { return pointer_to_types (); },
  }};
  return std::visit (v, bf);
}

std::vector<Types>
entry_to_types (VF::Entry const& entry)
{
  auto types {basic_format_to_types (entry.key)};
  auto value_types {format_to_types (entry.value)};

  std::move (value_types.begin (), value_types.end (), std::back_inserter (types));
  return types;
}

std::vector<Types>
format_array_to_types (VariantType const& vt)
{
  if (variant_type_is_definite (vt))
  {
    return array_to_types<NullablePointer> (vt);
  }
  return array_to_types<Pointer> (vt);
}

std::vector<Types>
format_to_types (VariantFormat const& format)
{
  auto v {Util::VisitHelper {
    [](Leaf::Basic const& basic) { return leaf_basic_to_types (basic); },
    [](VT::Array const& array) { return format_array_to_types (array.element_type); },
    [](VF::AtVariantType const& avt) { return gvariant_types_v (avt.type); },
    [](VF::Pointer const&) { return pointer_to_types (); },
    [](VF::Convenience const& convenience) { return convenience_to_types (convenience); },
    [](VF::Maybe const& maybe) { return maybe_to_types (maybe); },
    [](VF::Tuple const& tuple) { return tuple_to_types (tuple); },
    [](VF::Entry const& entry) { return entry_to_types (entry); },
  }};
  return std::visit (v, format);
}

} // anonymous namespace

std::vector<Types>
expected_types_for_format (VariantFormat const& format)
{
  return format_to_types (format);
}

namespace
{

struct StrippedType
{
  using V = std::variant
    <
    Pointer,
    PlainType,
    Meh
    >;

  V v;
};

struct SimpleType
{
  using V= std::variant
    <
    Const,
    Pointer,
    PlainType
    >;

  V v;
};

template <typename TargetType = SimpleType,
          typename SubSet = void /* whatever, it will be deduced from function call */>
TargetType
repackage (SubSet const& te)
{
  // TODO: use Util::generalize instead?
  auto v {Util::VisitHelper {
    [](auto const& inner) { return TargetType {{inner}}; },
  }};
  return std::visit (v, te.v);
}

StrippedType
strip_qualifiers (Type const& type)
{
  auto v {Util::VisitHelper {
    [](Const const& cnst) { return repackage<StrippedType> (cnst); },
    [](Pointer const& other) { return StrippedType {{other}}; },
    [](NullablePointer const& other) { return StrippedType {{repackage<Pointer> (other)}}; },
    [](PlainType const& other) { return StrippedType {{other}}; },
    [](Meh const& other) { return StrippedType {{other}}; },
  }};
  return std::visit (v, type.v);
}

bool
check_integral (Integral const& from,
                Integral const& to)
{
  if (from.name != to.name)
  {
    auto const& names = to.additional_names;
    if (std::find (names.cbegin(), names.cend(), from.name) == names.cend())
    {
      return false;
    }
    // TODO: check accidental names
  }
  if (from.signedness != Signedness::Any && from.signedness != to.signedness)
  {
    return false;
  }
  if (from.size_in_bytes != to.size_in_bytes)
  {
    return false;
  }
  return true;
}

bool
check_real (Real const& from,
            Real const& to)
{
  if (from.name != to.name)
  {
    auto const& names = to.additional_names;
    if (std::find (names.cbegin(), names.cend(), from.name) == names.cend())
    {
      return false;
    }
    // TODO: check accidental names
  }
  if (from.size_in_bytes != to.size_in_bytes)
  {
    return false;
  }
  return true;
}

bool
check_variant_typed (VariantTyped const& from,
                     VariantTyped const& to)
{
  if (from.name != to.name)
  {
    return false;
  }
  // TODO: check type info.
  return true;
}

bool
check_plain_type (PlainType const& pt_from,
                  PlainType const& pt_to)
{
  auto v {Util::VisitHelper {
    [](Integral const& i_from,
       Integral const& i_to) { return check_integral (i_from, i_to); },
    [](Real const& r_from,
       Real const& r_to) { return check_real (r_from, r_to); },
    [](VariantTyped const& vt_from,
       VariantTyped const& vt_to) { return check_variant_typed (vt_from, vt_to); },
    [](auto const&,
       auto const&) { return false; },
  }};
  return std::visit (v, pt_from.v, pt_to.v);
}

bool
check_simple_type (SimpleType const& from,
                   SimpleType const& to)
{
  auto v {Util::VisitHelper {
    [](Const const& cnst_from,
       Const const& cnst_to) { return check_simple_type (repackage (cnst_from),
                                                         repackage (cnst_to)); },
    [](Pointer const& ptr_from,
       Pointer const& ptr_to) { return check_simple_type (repackage (ptr_from),
                                                          repackage (ptr_to)); },
    [](PlainType const& pt_from,
       PlainType const& pt_to) { return check_plain_type (pt_from, pt_to); },
    [](auto const&,
       auto const&) { return false; },
  }};
  return std::visit (v, from.v, to.v);
}

bool
first_pointer (Pointer const& ptr_from, Pointer const& ptr_to)
{
  auto v {Util::VisitHelper {
    [](Const const& cnst_from,
       Const const& cnst_to) { return check_simple_type (repackage (cnst_from),
                                                         repackage (cnst_to)); },
    [](Const const&,
       auto const&) { return false; },
    [](auto const& other_from,
       Const const& cnst_to) { return check_simple_type (SimpleType {{other_from}},
                                                         repackage (cnst_to)); },
    [](auto const& other_from,
       auto const& other_to) { return check_simple_type (SimpleType {{other_from}},
                                                         SimpleType {{other_to}}); },
  }};
  return std::visit (v, ptr_from.v, ptr_to.v);
}

} // anonymous namespace

bool
type_is_convertible_to_type (Type const& from, Type const& to)
{
  auto stripped_from {strip_qualifiers (from)};
  auto stripped_to {strip_qualifiers (to)};
  auto v {Util::VisitHelper {
    [](Pointer const& ptr_from,
       Pointer const& ptr_to) { return first_pointer (ptr_from, ptr_to); },
    [](PlainType const& pt_from,
       PlainType const& pt_to) { return check_plain_type (pt_from, pt_to); },
    [](auto const&,
       auto const&) { return false; },
  }};
  return std::visit (v, stripped_from.v, stripped_to.v);
}

} // namespace Ggp
