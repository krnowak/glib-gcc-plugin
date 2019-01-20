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

/*< lib: type-print.hh >*/
/*< lib: type.hh >*/
/*< lib: variant-print.hh >*/
/*< stl: ostream >*/

namespace Ggp::Lib
{

namespace
{

auto
print_pointer (std::ostream& os, Pointer const& pointer) -> void;

auto
print_plain_type (std::ostream& os, PlainType const& plain_type) -> void;

auto
print_const (std::ostream& os, Const const& const_) -> void
{
  auto vh {VisitHelper {
    [&os](Pointer const& pointer) mutable { print_pointer (os, pointer); },
    [&os](PlainType const& plain_type) mutable { print_plain_type (os, plain_type); },
  }};
  os << "const< ";
  std::visit (vh, const_.v);
  os << " >";
}

auto
print_pointer_base (std::ostream& os, PointerBase const& pointer_base) -> void
{
  auto vh {VisitHelper {
    [&os](Pointer const& pointer) mutable { print_pointer (os, pointer); },
    [&os](Const const& const_) mutable { print_const (os, const_); },
    [&os](PlainType const& plain_type) mutable { print_plain_type (os, plain_type); },
  }};

  std::visit (vh, pointer_base.v);
}

auto
print_pointer (std::ostream& os, Pointer const& pointer) -> void
{
  os << "ptr< ";
  print_pointer_base (os, pointer);
  os << " >";
}

/*
auto
print_nullable_pointer (std::ostream& os, NullablePointer const& nullable_pointer) -> void
{
  os << "nullptr< ";
  print_pointer_base (os, nullable_pointer);
  os << " >";
}
*/

auto
print_integral (std::ostream& os, Integral const& integral) -> void
{
  os << "integral< " << '"' << integral.name << '"' << ", " << static_cast<int> (integral.size_in_bytes) << ", ";
  switch (integral.signedness)
  {
  case Signedness::Signed:
    os << "signed";
    break;

  case Signedness::Unsigned:
    os << "unsigned";
    break;

  case Signedness::Any:
    os << "any";
    break;

  default:
    os << "<BROKEN>";
    break;
  }

  os << " >";
}

auto
print_real (std::ostream& os, Real const& real) -> void
{
  os << "real< " << '"' << real.name << '"' << ", " << static_cast<int> (real.size_in_bytes) << " >";
}

auto
print_type_info (std::ostream& os, TypeInfo const& type_info) -> void
{
  auto vh {VisitHelper {
    [&os](VariantTypeUnspecified const&) mutable { os << "---"; },
    [&os](VariantType const& variant_type) mutable { os << variant_type; },
  }};

  os << "type< ";
  std::visit (vh, type_info.v);
  os << " >";
}

auto
print_variant_typed (std::ostream& os, VariantTyped const& variant_typed) -> void
{
  os << "variant-typed< " << '"' << variant_typed.name << '"' << ", ";
  print_type_info (os, variant_typed.info);
  os << " >";
}

auto
print_plain_type (std::ostream& os, PlainType const& plain_type) -> void
{
  auto vh {VisitHelper {
    [&os](Integral const& integral) mutable { print_integral (os, integral); },
    [&os](Real const& real) mutable { print_real (os, real); },
    [&os](VariantTyped const& variant_typed) mutable { print_variant_typed (os, variant_typed); },
  }};

  os << "plain< ";
  std::visit (vh, plain_type.v);
  os << " >";
}

auto
print_meh (std::ostream& os, Meh const&) -> void
{
  os << "meh";
}

auto
print_null_pointer (std::ostream& os, NullPointer const&) -> void
{
  os << "null";
}

auto
print_type (std::ostream& os, Type const& type) -> void
{
  auto vh {VisitHelper {
    [&os](Const const& const_) mutable { print_const (os, const_); },
    [&os](Pointer const& pointer) mutable { print_pointer (os, pointer); },
    //[&os](NullablePointer const& nullable_pointer) mutable { print_nullable_pointer (os, nullable_pointer); },
    [&os](PlainType const& plain_type) mutable { print_plain_type (os, plain_type); },
    [&os](NullPointer const& null_pointer) mutable { print_null_pointer (os, null_pointer); },
    [&os](Meh const& meh) mutable { print_meh (os, meh); },
  }};

  std::visit (vh, type.v);
}

auto
print_types (std::ostream& os, Types const& types) -> void
{
  os << "new: ";
  print_type (os, types.for_new);
  os << "; get: ";
  print_type (os, types.for_get);
}

} // anonymous namespace

auto
operator<< (std::ostream& os, Types const& types) -> std::ostream&
{
  print_types (os, types);

  return os;
}

} // namespace Ggp::Lib
