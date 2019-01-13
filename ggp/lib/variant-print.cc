/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2019 Krzesimir Nowak
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

/*< lib: variant-print.hh >*/
/*< lib: variant.hh >*/
/*< stl: ostream >*/

namespace Ggp::Lib
{

namespace
{

auto
print_basic (std::ostream& os, Leaf::Basic const& basic) -> void
{
  auto vh {VisitHelper {
    [&os](Leaf::Bool const&) mutable { os << 'b'; },
    [&os](Leaf::Byte const&) mutable { os << 'y'; },
    [&os](Leaf::I16 const&) mutable { os << 'n'; },
    [&os](Leaf::U16 const&) mutable { os << 'q'; },
    [&os](Leaf::I32 const&) mutable { os << 'i'; },
    [&os](Leaf::U32 const&) mutable { os << 'u'; },
    [&os](Leaf::I64 const&) mutable { os << 'x'; },
    [&os](Leaf::U64 const&) mutable { os << 't'; },
    [&os](Leaf::Handle const&) mutable { os << 'h'; },
    [&os](Leaf::Double const&) mutable { os << 'd'; },
  }};

  std::visit (vh, basic.v);
}

auto
print_string_type (std::ostream& os, Leaf::StringType const& string_type) -> void
{
  auto vh {VisitHelper {
    [&os](Leaf::String const&) mutable { os << 's'; },
    [&os](Leaf::ObjectPath const&) mutable { os << 'o'; },
    [&os](Leaf::Signature const&) mutable { os << 'g'; },
  }};

  std::visit (vh, string_type.v);
}

auto
print_variant_type (std::ostream& os, VariantType const& variant_type) -> void;

auto
print_maybe (std::ostream& os, VT::Maybe const& maybe) -> void
{
  os << 'm';
  print_variant_type (os, maybe.pointed_type);
}

auto
print_tuple (std::ostream& os, VT::Tuple const& tuple) -> void
{
  os << '(';
  for (auto const& type : tuple.types)
  {
    print_variant_type (os, type);
  }
  os << ')';
}

auto
print_array (std::ostream& os, VT::Array const& array) -> void
{
  os << 'a';
  print_variant_type (os, array.element_type);
}

auto
print_entry_key_type (std::ostream& os, VT::EntryKeyType const& entry_key_type) -> void
{
  auto vh {VisitHelper {
    [&os](Leaf::Basic const& basic) mutable { print_basic (os, basic); },
    [&os](Leaf::StringType const& string_type) mutable { print_string_type (os, string_type); },
    [&os](Leaf::AnyBasic const&) mutable { os << '?'; },
  }};

  std::visit (vh, entry_key_type.v);
}

auto
print_entry (std::ostream& os, VT::Entry const& entry) -> void
{
  os << '{';
  print_entry_key_type (os, entry.key);
  print_variant_type (os, entry.value);
  os << '}';
}

auto
print_variant_type (std::ostream& os, VariantType const& variant_type) -> void
{
  auto vh {VisitHelper {
    [&os](Leaf::Basic const& basic) mutable { print_basic (os, basic); },
    [&os](Leaf::AnyBasic const&) mutable { os << '?'; },
    [&os](Leaf::StringType const& string_type) mutable { print_string_type (os, string_type); },
    [&os](VT::Maybe const& maybe) mutable { print_maybe (os, maybe); },
    [&os](VT::Tuple const& tuple) mutable { print_tuple (os, tuple); },
    [&os](VT::Array const& array) mutable { print_array (os, array); },
    [&os](VT::Entry const& entry) mutable { print_entry (os, entry); },
    [&os](Leaf::Variant const&) mutable { os << 'v'; },
    [&os](Leaf::AnyTuple const&) mutable { os << 'r'; },
    [&os](Leaf::AnyType const&) mutable { os << '*'; },
  }};

  std::visit (vh, variant_type.v);
}

} // anonymous namespace

auto
operator<< (std::ostream& os, VariantType const& variant_type) -> std::ostream&
{
  print_variant_type (os, variant_type);

  return os;
}

namespace
{

auto
print_at_variant_type (std::ostream& os, VF::AtVariantType const& at) -> void
{
  os << '@';
  print_variant_type (os, at.type);
}

auto
print_pointer (std::ostream& os, VF::Pointer const& pointer) -> void
{
  os << '&';
  print_string_type (os, pointer.string_type);
}

auto
print_convenience (std::ostream& os, VF::Convenience const& convenience) -> void
{
  using ConType = VF::Convenience::Type;
  using ConKind = VF::Convenience::Kind;
  auto vh {VisitHelper {
    [&os](ConType::StringArray const&, ConKind::Constant const&) mutable { os << "a&s"; },
    [&os](ConType::StringArray const&, ConKind::Duplicated const&) mutable { os << "as"; },
    [&os](ConType::ObjectPathArray const&, ConKind::Constant const&) mutable { os << "a&o"; },
    [&os](ConType::ObjectPathArray const&, ConKind::Duplicated const&) mutable { os << "ao"; },
    [&os](ConType::ByteString const&, ConKind::Constant const&) mutable { os << "&ay"; },
    [&os](ConType::ByteString const&, ConKind::Duplicated const&) mutable { os << "ay"; },
    [&os](ConType::ByteStringArray const&, ConKind::Constant const&) mutable { os << "a&ay"; },
    [&os](ConType::ByteStringArray const&, ConKind::Duplicated const&) mutable { os << "aay"; },
  }};

  os << '^';
  std::visit (vh, convenience.type.v, convenience.kind.v);
}

auto
print_variant_format (std::ostream& os, VariantFormat const& variant_format) -> void;

auto
print_maybe_pointer (std::ostream& os, VF::MaybePointer const& maybe_pointer) -> void
{
  auto vh {VisitHelper {
    [&os](VT::Array const& array) mutable { print_array (os, array); },
    [&os](Leaf::StringType const& string_type) mutable { print_string_type (os, string_type); },
    [&os](Leaf::Variant const&) mutable { os << 'v'; },
    [&os](VF::AtVariantType const& at) mutable { print_at_variant_type (os, at); },
    [&os](VF::Pointer const& pointer) mutable { print_pointer (os, pointer); },
    [&os](VF::Convenience const& convenience) mutable { print_convenience (os, convenience); },
  }};

  std::visit (vh, maybe_pointer.v);
}

auto
print_entry (std::ostream& os, VF::Entry const& entry) -> void;
auto
print_maybe (std::ostream& os, VF::Maybe const& maybe) -> void;
auto
print_tuple (std::ostream& os, VF::Tuple const& tuple) -> void;

auto
print_maybe_bool (std::ostream& os, VF::MaybeBool const& maybe_bool) -> void
{
  auto vh {VisitHelper {
    [&os](Leaf::Basic const& basic) mutable { print_basic (os, basic); },
    [&os](VF::Entry const& entry) mutable { print_entry (os, entry); },
    [&os](VF::Tuple const& tuple) mutable { print_tuple (os, tuple); },
    [&os](VF::Maybe const& maybe) mutable { print_maybe (os, maybe); },
  }};

  std::visit (vh, maybe_bool.v);
}

auto
print_maybe (std::ostream& os, VF::Maybe const& maybe) -> void
{
  auto vh {VisitHelper {
    [&os](VF::MaybePointer const& maybe_pointer) mutable { print_maybe_pointer (os, maybe_pointer); },
    [&os](VF::MaybeBool const& maybe_bool) mutable { print_maybe_bool (os, maybe_bool); },
  }};

  os << 'm';
  std::visit (vh, maybe.v);
}

auto
print_tuple (std::ostream& os, VF::Tuple const& tuple) -> void
{
  os << '(';
  for (auto const& format : tuple.formats)
  {
    print_variant_format (os, format);
  }
  os << ')';
}

auto
print_at_entry_key_type (std::ostream& os, VF::AtEntryKeyType const& at) -> void
{
  os << '@';
  print_entry_key_type (os, at.entry_key_type);
}

auto
print_entry_key_format (std::ostream& os, VF::EntryKeyFormat const& entry_key_format) -> void
{
  auto vh {VisitHelper {
    [&os](Leaf::Basic const& basic) mutable { print_basic (os, basic); },
    [&os](Leaf::StringType const& string_type) mutable { print_string_type (os, string_type); },
    [&os](VF::AtEntryKeyType const& at) mutable { print_at_entry_key_type (os, at); },
    [&os](VF::Pointer const& pointer) mutable { print_pointer (os, pointer); },
  }};

  std::visit (vh, entry_key_format.v);
}

auto
print_entry (std::ostream& os, VF::Entry const& entry) -> void
{
  os << '{';
  print_entry_key_format (os, entry.key);
  print_variant_format (os, entry.value);
  os << '}';
}

auto
print_variant_format (std::ostream& os, VariantFormat const& variant_format) -> void
{
  auto vh {VisitHelper {
    [&os](Leaf::Basic const& basic) mutable { print_basic (os, basic); },
    [&os](Leaf::StringType const& string_type) mutable { print_string_type (os, string_type); },
    [&os](Leaf::Variant const&) mutable { os << 'v'; },
    [&os](VT::Array const& array) mutable { print_array (os, array); },
    [&os](VF::AtVariantType const& at) mutable { print_at_variant_type (os, at); },
    [&os](VF::Pointer const& pointer) mutable { print_pointer (os, pointer); },
    [&os](VF::Convenience const& convenience) mutable { print_convenience (os, convenience); },
    [&os](VF::Maybe const& maybe) mutable { print_maybe (os, maybe); },
    [&os](VF::Tuple const& tuple) mutable { print_tuple (os, tuple); },
    [&os](VF::Entry const& entry) mutable { print_entry (os, entry); },
  }};

  std::visit (vh, variant_format.v);
}

} // anonymous namespace

auto
operator<< (std::ostream& os, VariantFormat const& variant_format) -> std::ostream&
{
  print_variant_format (os, variant_format);

  return os;
}

} // namespace Ggp::Lib
