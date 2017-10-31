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

Types
gvariant_types ()
{
  return {{Pointer {"GVariant"s}}, {Pointer {Pointer {"GVariant"s}}}};
}

std::vector<Types>
gvariant_types_v ()
{
  return {gvariant_types ()};
}

Type
const_str ()
{
  return {Pointer {Const {"gchar"s}}};
}

std::vector<Types>
basic_type_to_types (VT::Basic basic)
{
  switch (basic)
  {
  case VT::Basic::Bool:
    return {{{"gboolean"s}, {Pointer {"gboolean"s}}}};
  case VT::Basic::Byte:
    return {{{"guchar"s}, {Pointer {"guchar"s}}}};
  case VT::Basic::I16:
    return {{{"gint16"s}, {Pointer {"gint16"s}}}};
  case VT::Basic::U16:
    return {{{"guint16"s}, {Pointer {"guint16"s}}}};
  case VT::Basic::I32:
    return {{{"gint32"s}, {Pointer {"gint32"s}}}};
  case VT::Basic::U32:
    return {{{"guint32"s}, {Pointer {"guint32"s}}}};
  case VT::Basic::I64:
    return {{{"gint64"s}, {Pointer {"gint64"s}}}};
  case VT::Basic::U64:
    return {{{"guint64"s}, {Pointer {"guint64"s}}}};
  case VT::Basic::Handle:
    return {{{"gint32"s}, {Pointer {"gint32"s}}}};
  case VT::Basic::Double:
    return {{{"gdouble"s}, {Pointer {"gdouble"s}}}};
  case VT::Basic::String:
    return {{const_str (), {Pointer {Pointer {"gchar"s}}}}};
  case VT::Basic::ObjectPath:
    return {{const_str (), {Pointer {Pointer {"gchar"s}}}}};
  case VT::Basic::Signature:
    return {{const_str (), {Pointer {Pointer {"gchar"s}}}}};
  case VT::Basic::Any:
    return gvariant_types_v ();
  default:
    gcc_unreachable ();
    return {};
  }
}

std::vector<Types>
array_mod_to_types ()
{
  return {{{Pointer {"GVariantBuilder"s}}, {Pointer {Pointer {"GVariantIter"s}}}}};
}

std::vector<Types>
pointer_to_types ()
{
  return {{const_str (), {Pointer {Pointer {Const {"gchar"s}}}}}};
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
      auto for_new {Type {Pointer {Const {Pointer {Const {"gchar"s}}}}}};
      switch (convenience.kind)
      {
      case VF::Convenience::Kind::Constant:
        return {{for_new, {Pointer {Pointer {Pointer {Const {"gchar"s}}}}}}};
      case VF::Convenience::Kind::Duplicated:
        return {{for_new, {Pointer {Pointer {Pointer {"gchar"s}}}}}};
      default:
        gcc_unreachable ();
        return {};
      }
    }
  case VF::Convenience::Type::ByteString:
    {
      auto for_new {Type {Pointer {Const {"gchar"s}}}};
      switch (convenience.kind)
      {
      case VF::Convenience::Kind::Constant:
        return {{for_new, {Pointer {Pointer {Const {"gchar"s}}}}}};
      case VF::Convenience::Kind::Duplicated:
        return {{for_new, {Pointer {Pointer {"gchar"s}}}}};
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
  switch (bmb)
  {
  case VF::BasicMaybeBool::Bool:
    return {{{"gboolean"s}, {Pointer {"gboolean"s}}}};
  case VF::BasicMaybeBool::Byte:
    return {{{"guchar"s}, {Pointer {"guchar"s}}}};
  case VF::BasicMaybeBool::I16:
    return {{{"gint16"s}, {Pointer {"gint16"s}}}};
  case VF::BasicMaybeBool::U16:
    return {{{"guint16"s}, {Pointer {"guint16"s}}}};
  case VF::BasicMaybeBool::I32:
    return {{{"gint32"s}, {Pointer {"gint32"s}}}};
  case VF::BasicMaybeBool::U32:
    return {{{"guint32"s}, {Pointer {"guint32"s}}}};
  case VF::BasicMaybeBool::I64:
    return {{{"gint64"s}, {Pointer {"gint64"s}}}};
  case VF::BasicMaybeBool::U64:
    return {{{"guint64"s}, {Pointer {"guint64"s}}}};
  case VF::BasicMaybeBool::Handle:
    return {{{"gint32"s}, {Pointer {"gint32"s}}}};
  case VF::BasicMaybeBool::Double:
    return {{{"gdouble"s}, {Pointer {"gdouble"s}}}};
  default:
    gcc_unreachable ();
    return {};
  }
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
  auto types {basic_type_to_types (VT::Basic::Bool)};
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

std::vector<Types>
basic_maybe_pointer_to_types (VF::BasicMaybePointer const& bmp)
{
  switch (bmp)
  {
  case VF::BasicMaybePointer::String:
    return {{const_str (), {Pointer {Pointer {"gchar"s}}}}};
  case VF::BasicMaybePointer::ObjectPath:
    return {{const_str (), {Pointer {Pointer {"gchar"s}}}}};
  case VF::BasicMaybePointer::Signature:
    return {{const_str (), {Pointer {Pointer {"gchar"s}}}}};
  case VF::BasicMaybePointer::Any:
    return gvariant_types_v ();
  default:
    gcc_unreachable ();
    return {};
  }
}

std::vector<Types>
maybe_pointer_to_types (VF::MaybePointer const& mp)
{
  auto v {Util::VisitHelper {
    [](VF::VTMod::Array const&) { return array_mod_to_types (); },
    [](VF::AtVariantType const&) { return gvariant_types_v (); },
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
    [](VT::Basic const& basic) { return basic_type_to_types (basic); },
    [](VF::AtBasicType const&) { return gvariant_types_v (); },
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
format_to_types (VariantFormat const& format)
{
  auto v {Util::VisitHelper {
    [](VT::Basic const& basic) { return basic_type_to_types (basic); },
    [](VT::Array const&) { return array_mod_to_types (); },
    [](VF::AtVariantType const&) { return gvariant_types_v (); },
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

} // namespace Ggp
