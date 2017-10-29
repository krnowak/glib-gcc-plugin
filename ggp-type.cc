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

Types
gvariant_types ()
{
  using namespace std::string_literals;

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
  using namespace std::string_literals;

  return {Pointer {Const {"gchar"s}}};
}

std::vector<Types>
basic_type_to_types (VT::Basic basic)
{
  using namespace std::string_literals;

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
  using namespace std::string_literals;

  return {{{Pointer {"GVariantBuilder"s}}, {Pointer {Pointer {"GVariantIter"s}}}}};
}

std::vector<Types>
variant_type_sub_set_mod_to_types (VF::VTMod::VariantTypeSubSet const& vtss)
{
  auto v {Util::VisitHelper {
    [](VT::Basic const& basic) { return basic_type_to_types (basic); },
    [](VF::VTMod::Array const&) { return array_mod_to_types (); },
    [](VT::Variant const&) { return gvariant_types_v (); },
    [](VT::AnyTuple const&) { return gvariant_types_v (); },
    [](VT::AnyType const&) { return gvariant_types_v (); },
  }};
  return std::visit (v, vtss);
}

std::vector<Types>
pointer_to_types ()
{
  using namespace std::string_literals;

  return {{const_str (), {Pointer {Pointer {Const {"gchar"s}}}}}};
}

std::vector<Types>
convenience_to_types (VF::Convenience const& /* convenience */)
{
  return {};
}

std::vector<Types>
maybe_to_types (VF::Maybe const& /* maybe */)
{
  return {};
}

std::vector<Types>
tuple_to_types (VF::Tuple const& /* tuple */)
{
  return {};
}

std::vector<Types>
entry_to_types (VF::Entry const& /* entry */)
{
  return {};
}

} // anonymous namespace

std::vector<Types>
expected_types_for_format (VariantFormat const& format)
{
  auto v {Util::VisitHelper {
    [](VF::VTMod::VariantTypeSubSet const& vtss) { return variant_type_sub_set_mod_to_types (vtss); },
    [](VF::AtVariantType const&) { return gvariant_types_v (); },
    [](VF::Pointer const&) { return pointer_to_types (); },
    [](VF::Convenience const& convenience) { return convenience_to_types (convenience); },
    [](VF::Maybe const& maybe) { return maybe_to_types (maybe); },
    [](VF::Tuple const& tuple) { return tuple_to_types (tuple); },
    [](VF::Entry const& entry) { return entry_to_types (entry); },
  }};
  return std::visit (v, format);
}

} // namespace Ggp
