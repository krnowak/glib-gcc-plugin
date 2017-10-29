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

std::vector<Types>
foo ()
{
  return {};
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
    return {{{Pointer {Const {"gchar"s}}}, {Pointer {Pointer {"gchar"s}}}}};
  case VT::Basic::ObjectPath:
    return {{{Pointer {Const {"gchar"s}}}, {Pointer {Pointer {"gchar"s}}}}};
  case VT::Basic::Signature:
    return {{{Pointer {Const {"gchar"s}}}, {Pointer {Pointer {"gchar"s}}}}};
  case VT::Basic::Any:
    return {{{Pointer {"GVariant"s}}, {Pointer {Pointer {"GVariant"s}}}}};
  default:
    gcc_unreachable ();
    return {};
  }
}

std::vector<Types>
variant_type_sub_set_mod_to_types (VF::VTMod::VariantTypeSubSet const& vtss)
{
  auto v {Util::VisitHelper {
    [](VT::Basic const& basic) { return basic_type_to_types (basic); },
    [](VF::VTMod::Array const&) { return foo (); },
    [](VT::Variant const&) { return foo (); },
    [](VT::AnyTuple const&) { return foo (); },
    [](VT::AnyType const&) { return foo (); },
  }};
  return std::visit (v, vtss);
}

} // anonymous namespace

std::vector<Types>
expected_types_for_format (VariantFormat const& format)
{
  auto v {Util::VisitHelper {
    [](VF::VTMod::VariantTypeSubSet const& vtss) { return variant_type_sub_set_mod_to_types (vtss); },
    [](VF::AtVariantType const&) { return foo (); },
    [](VF::Pointer const&) { return foo (); },
    [](VF::Convenience const&) { return foo (); },
    [](VF::Maybe const&) { return foo (); },
    [](VF::Tuple const&) { return foo (); },
    [](VF::Entry const&) { return foo (); },
  }};
  return std::visit (v, format);
}

} // namespace Ggp
