/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2017, 2018, 2019 Krzesimir Nowak
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

/*< lib: type.hh >*/
/*< stl: algorithm >*/
/*< stl: optional >*/
/*< sizeof: int >*/

namespace Ggp::Lib
{

using namespace std::string_literals;

auto type_gboolean () -> Integral
{
  return {"gboolean"s/*, {}, {"gint"s, "int"s}*/, sizeof (int), Signedness::Signed};
}

auto type_gchar () -> Integral
{
  return {"gchar"s/*, {"char"s}, {}*/, 1u, Signedness::Any};
}

auto type_guchar () -> Integral
{
  return {"guchar"s/*, {}, {}*/, 1u, Signedness::Unsigned};
}

auto type_gint16 () -> Integral
{
  return {"gint16"s/*, {}, {}*/, 2u, Signedness::Signed};
}

auto type_guint16 () -> Integral
{
  return {"guint16"s/*, {}, {}*/, 2u, Signedness::Unsigned};
}

auto type_gint32 () -> Integral
{
  return {"gint32"s/*, {}, {}*/, 4u, Signedness::Signed};
}

auto type_guint32 () -> Integral
{
  return {"guint32"s/*, {}, {}*/, 4u, Signedness::Unsigned};
}

auto type_gint64 () -> Integral
{
  return {"gint64"s/*, {}, {}*/, 8u, Signedness::Signed};
}

auto type_guint64 () -> Integral
{
  return {"guint64"s/*, {}, {}*/, 8u, Signedness::Unsigned};
}

auto type_handle () -> Integral
{
  return {"gint32"s/*, {}, {}*/, 4u, Signedness::Signed};
}

auto type_gdouble () -> Real
{
  return {"gdouble"s/*, {"double"s}*/, 8u};
}

namespace
{

PlainType
gchar_plain_type ()
{
  return {{type_gchar ()}};
}

PlainType
gboolean_plain_type ()
{
  return {{type_gboolean ()}};
}

PlainType
guchar_plain_type ()
{
  return {{type_guchar ()}};
}

PlainType
gint16_plain_type ()
{
  return {{type_gint16 ()}};
}

PlainType
guint16_plain_type ()
{
  return {{type_guint16 ()}};
}

PlainType
gint32_plain_type ()
{
  return {{type_gint32 ()}};
}

PlainType
guint32_plain_type ()
{
  return {{type_guint32 ()}};
}

PlainType
gint64_plain_type ()
{
  return {{type_gint64 ()}};
}

PlainType
guint64_plain_type ()
{
  return {{type_guint64 ()}};
}

PlainType
handle_plain_type ()
{
  return {{type_handle ()}};
}

PlainType
gdouble_plain_type ()
{
  return {{type_gdouble ()}};
}

template <typename T>
std::vector<Types>
types (T const& new_type, T const& get_type)
{
  return {{{{new_type}}, {{NullablePointer {{get_type}}}}}};
}

template <typename T>
std::vector<Types>
types (T const& t)
{
  return types (t, t);
}

auto
type_info_unspecified () -> TypeInfo
{
  return {{variant_type_unspecified}};
}

auto
type_info_specified (VariantType const& vt) -> TypeInfo
{
  return {{vt}};
}

template <typename Ptr>
auto
gvariant_type (TypeInfo ti) -> Ptr
{
  return {{PlainType {{VariantTyped {"GVariant"s, std::move (ti)}}}}};
}

auto
gvariant_types_v (TypeInfo ti) -> std::vector<Types>
{
  return types (gvariant_type<Pointer> (std::move (ti)));
}

Pointer
const_str ()
{
  return {{Const {{gchar_plain_type ()}}}};
}

Pointer
str ()
{
  return {{gchar_plain_type ()}};
}

std::vector<Types>
string_types ()
{
  return {{{{const_str ()}}, {{NullablePointer {{str ()}}}}}};
}

std::vector<Types>
leaf_basic_to_types (Leaf::Basic const& basic)
{
  auto vh {VisitHelper {
    [](Leaf::Bool const&) { return types (gboolean_plain_type ()); },
    [](Leaf::Byte const&) { return types (guchar_plain_type ()); },
    [](Leaf::I16 const&) { return types (gint16_plain_type ()); },
    [](Leaf::U16 const&) { return types (guint16_plain_type ()); },
    [](Leaf::I32 const&) { return types (gint32_plain_type ()); },
    [](Leaf::U32 const&) { return types (guint32_plain_type ()); },
    [](Leaf::I64 const&) { return types (gint64_plain_type ()); },
    [](Leaf::U64 const&) { return types (guint64_plain_type ()); },
    [](Leaf::Handle const&) { return types (handle_plain_type ()); },
    [](Leaf::Double const&) { return types (gdouble_plain_type ()); },
  }};

  return std::visit (vh, basic.v);
}

std::vector<Types>
leaf_string_type_to_types (Leaf::StringType const& string_type)
{
  auto vh {VisitHelper {
    [](Leaf::String const&) { return string_types (); },
    [](Leaf::ObjectPath const&) { return string_types (); },
    [](Leaf::Signature const&) { return string_types (); },
  }};

  return std::visit (vh, string_type.v);
}

template <typename Ptr>
std::vector<Types>
array_to_types (VT::Array const& array)
{
  return types (Ptr {{PlainType {{VariantTyped {"GVariantBuilder"s, {{VariantType {{array}}}}}}}}}, Ptr {{Pointer {{PlainType {{VariantTyped {"GVariantIter"s, {{VariantType {{array}}}}}}}}}}});
}

std::vector<Types>
pointer_to_types ()
{
  return {{{{const_str ()}}, {{Pointer {{const_str ()}}}}}};
}

std::vector<Types>
char_array_array_to_types (VF::Convenience::Kind const& convenience_kind)
{
  auto for_new {Type {{Pointer {{Const {{const_str ()}}}}}}};
  auto vh {VisitHelper {
    [](VF::Convenience::Kind::Constant const&) { return Type {{Pointer {{Pointer {{const_str ()}}}}}}; },
    [](VF::Convenience::Kind::Duplicated const&) { return Type {{Pointer {{Pointer {{str ()}}}}}}; },
  }};

  return {{for_new, std::visit (vh, convenience_kind.v)}};
}

std::vector<Types>
byte_string_to_types (VF::Convenience::Kind const& convenience_kind)
{
  auto for_new {Type {{const_str ()}}};
  auto vh {VisitHelper {
    [](VF::Convenience::Kind::Constant const&) { return Type {{Pointer {{const_str ()}}}}; },
    [](VF::Convenience::Kind::Duplicated const&) { return Type {{Pointer {{str ()}}}}; },
  }};

  return {{for_new, std::visit (vh, convenience_kind.v)}};
}

std::vector<Types>
convenience_to_types (VF::Convenience const& convenience)
{
  auto vh {VisitHelper {
    [&convenience](VF::Convenience::Type::StringArray const&) { return char_array_array_to_types (convenience.kind); },
    [&convenience](VF::Convenience::Type::ObjectPathArray const&) { return char_array_array_to_types (convenience.kind); },
    [&convenience](VF::Convenience::Type::ByteStringArray const&) { return char_array_array_to_types (convenience.kind); },
    [&convenience](VF::Convenience::Type::ByteString const&) { return byte_string_to_types (convenience.kind); },
  }};
  return std::visit (vh, convenience.type.v);
}

std::vector<Types>
maybe_to_types (VF::Maybe const& maybe);

std::vector<Types>
tuple_to_types (VF::Tuple const& tuple);

std::vector<Types>
entry_to_types (VF::Entry const& entry);

std::vector<Types>
maybe_bool_to_types (VF::MaybeBool const& maybe_bool)
{
  auto all_types {leaf_basic_to_types (Leaf::Basic {Leaf::bool_})};
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return leaf_basic_to_types (basic); },
    [](VF::Entry const& entry) { return entry_to_types (entry); },
    [](VF::Tuple const& tuple) { return tuple_to_types (tuple); },
    [](VF::Maybe const& maybe) { return maybe_to_types (maybe); },
  }};
  auto more_types {std::visit (vh, maybe_bool.v)};

  std::move (more_types.begin (), more_types.end(), std::back_inserter (all_types));

  return all_types;
}

// TODO: we need to return NullablePointer here.
std::vector<Types>
maybe_pointer_to_types (VF::MaybePointer const& maybe_pointer)
{
  auto vh {VisitHelper {
    [](VT::Array const& array) { return array_to_types<NullablePointer> (array); },
    [](Leaf::StringType const& string_type) { return leaf_string_type_to_types (string_type); },
    [](Leaf::Variant const&) { return types (gvariant_type<NullablePointer> (type_info_unspecified ()), gvariant_type<Pointer> (type_info_unspecified ())); },
    [](VF::AtVariantType const& avt) { return types (gvariant_type<NullablePointer> (type_info_specified (avt.type)), gvariant_type<Pointer> (type_info_specified (avt.type))); },
    [](VF::Pointer const&) { return pointer_to_types (); },
    [](VF::Convenience const& convenience) { return convenience_to_types (convenience); },
  }};

  return std::visit (vh, maybe_pointer.v);
}

std::vector<Types>
maybe_to_types (VF::Maybe const& maybe)
{
  auto vh {VisitHelper {
    [](VF::MaybePointer const& maybe_pointer) { return maybe_pointer_to_types (maybe_pointer); },
    [](VF::MaybeBool const& maybe_bool) { return maybe_bool_to_types (maybe_bool); },
  }};

  return std::visit (vh, maybe.v);
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

auto
at_entry_key_type_to_types (VF::AtEntryKeyType const& at) -> std::vector<Types>
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return VariantType {{basic}}; },
    [](Leaf::StringType const& string_type) { return VariantType {{string_type}}; },
    [](Leaf::AnyBasic const& any_basic) { return VariantType {{any_basic}}; },
  }};

  return types (gvariant_type<Pointer> (type_info_specified (std::visit (vh, at.entry_key_type.v))));
}

std::vector<Types>
entry_key_format_to_types (VF::EntryKeyFormat const& entry_key_format)
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return leaf_basic_to_types (basic); },
    [](Leaf::StringType const& string_type) { return leaf_string_type_to_types (string_type); },
    [](VF::AtEntryKeyType const& at) { return at_entry_key_type_to_types (at); },
    [](VF::Pointer const&) { return pointer_to_types (); },
  }};

  return std::visit (vh, entry_key_format.v);
}

std::vector<Types>
entry_to_types (VF::Entry const& entry)
{
  auto types {entry_key_format_to_types (entry.key)};
  auto value_types {format_to_types (entry.value)};

  std::move (value_types.begin (), value_types.end (), std::back_inserter (types));

  return types;
}

std::vector<Types>
format_array_to_types (VT::Array const& array)
{
  if (array.element_type->is_definite ())
  {
    return array_to_types<NullablePointer> (array);
  }
  else
  {
    return array_to_types<Pointer> (array);
  }
}

auto
format_to_types (VariantFormat const& format) -> std::vector<Types>
{
  auto vh {VisitHelper {
    [](Leaf::Basic const& basic) { return leaf_basic_to_types (basic); },
    [](Leaf::StringType const& string_type) { return leaf_string_type_to_types (string_type); },
    [](Leaf::Variant const&) { return gvariant_types_v (type_info_unspecified ()); },
    [](VT::Array const& array) { return format_array_to_types (array); },
    [](VF::AtVariantType const& avt) { return gvariant_types_v (type_info_specified (avt.type)); },
    [](VF::Pointer const&) { return pointer_to_types (); },
    [](VF::Convenience const& convenience) { return convenience_to_types (convenience); },
    [](VF::Maybe const& maybe) { return maybe_to_types (maybe); },
    [](VF::Tuple const& tuple) { return tuple_to_types (tuple); },
    [](VF::Entry const& entry) { return entry_to_types (entry); },
  }};
  return std::visit (vh, format.v);
}

} // anonymous namespace

std::vector<Types>
expected_types_for_format (VariantFormat const& format)
{
  return format_to_types (format);
}

namespace
{

GGP_LIB_VARIANT_STRUCT_ONLY(StrippedType,
                            Pointer,
                            PlainType,
                            NullPointer,
                            Meh);

GGP_LIB_VARIANT_STRUCT_ONLY(SimpleType,
                            Const,
                            Pointer,
                            PlainType,
                            NullPointer);

auto
strip_qualifiers (Type const& type) -> StrippedType
{
  auto vh {VisitHelper {
    [](Const const& const_) { return repackage<StrippedType> (const_); },
    [](Pointer const& other) { return StrippedType {{other}}; },
    //[](NullablePointer const& other) { return StrippedType {{repackage<Pointer> (other)}}; },
    [](PlainType const& other) { return StrippedType {{other}}; },
    [](NullPointer const& null_pointer) { return StrippedType {{null_pointer}}; },
    [](Meh const& other) { return StrippedType {{other}}; },
  }};
  return std::visit (vh, type.v);
}

auto
check_integral (Integral const& from,
                Integral const& to) -> bool
{
  /*
  if (from.name != to.name)
  {
    auto const& names = to.additional_names;
    if (std::find (names.cbegin(), names.cend(), from.name) == names.cend())
    {
      return false;
    }
    return false;
    // TODO: check accidental names
  }
  */
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

auto
check_real (Real const& from,
            Real const& to) -> bool
{
  /*
  if (from.name != to.name)
  {
    auto const& names = to.additional_names;
    if (std::find (names.cbegin(), names.cend(), from.name) == names.cend())
    {
      return false;
    }
    return false;
  }
  */
  if (from.size_in_bytes != to.size_in_bytes)
  {
    return false;
  }
  return true;
}

auto
check_variant_typed (VariantTyped const& from,
                     VariantTyped const& to) -> bool
{
  if (from.name != to.name)
  {
    return false;
  }
  // TODO: check type info.
  return true;
}

auto
check_plain_type (PlainType const& pt_from,
                  PlainType const& pt_to) -> bool
{
  auto vh {VisitHelper {
    [](Integral const& i_from,
       Integral const& i_to) { return check_integral (i_from, i_to); },
    [](Real const& r_from,
       Real const& r_to) { return check_real (r_from, r_to); },
    [](VariantTyped const& vt_from,
       VariantTyped const& vt_to) { return check_variant_typed (vt_from, vt_to); },
    [](auto const&,
       auto const&) { return false; },
  }};
  return std::visit (vh, pt_from.v, pt_to.v);
}

auto
check_simple_type (SimpleType const& from,
                   SimpleType const& to) -> bool
{
  auto vh {VisitHelper {
    [](Const const& const_from,
       Const const& const_to) { return check_simple_type (repackage<SimpleType> (const_from),
                                                          repackage<SimpleType> (const_to)); },
    [](Pointer const& ptr_from,
       Pointer const& ptr_to) { return check_simple_type (repackage<SimpleType> (ptr_from),
                                                          repackage<SimpleType> (ptr_to)); },
    [](NullPointer const& /*from*/,
       Pointer const& /*to*/) { return true; },
    [](PlainType const& pt_from,
       PlainType const& pt_to) { return check_plain_type (pt_from, pt_to); },
    [](auto const&,
       auto const&) { return false; },
  }};
  return std::visit (vh, from.v, to.v);
}

auto
first_pointer (Pointer const& ptr_from,
               Pointer const& ptr_to) -> bool
{
  auto vh {VisitHelper {
    [](Const const& const_from,
       Const const& const_to) { return check_simple_type (repackage<SimpleType> (const_from),
                                                          repackage<SimpleType> (const_to)); },
    [](Const const&,
       auto const&) { return false; },
    [](auto const& other_from,
       Const const& const_to) { return check_simple_type (SimpleType {{other_from}},
                                                          repackage<SimpleType> (const_to)); },
    [](auto const& other_from,
       auto const& other_to) { return check_simple_type (SimpleType {{other_from}},
                                                         SimpleType {{other_to}}); },
  }};
  return std::visit (vh, ptr_from.v, ptr_to.v);
}

} // anonymous namespace

auto
type_is_convertible_to_type (Type const& from,
                             Type const& to) -> bool
{
  auto stripped_from {strip_qualifiers (from)};
  auto stripped_to {strip_qualifiers (to)};
  auto vh {VisitHelper {
    [](Pointer const& ptr_from,
       Pointer const& ptr_to) { return first_pointer (ptr_from, ptr_to); },
    [](PlainType const& pt_from,
       PlainType const& pt_to) { return check_plain_type (pt_from, pt_to); },
    [](auto const&,
       auto const&) { return false; },
  }};
  return std::visit (vh, stripped_from.v, stripped_to.v);
}

namespace
{

namespace TB
{

namespace TopNS = ::Ggp::Lib;

GGP_LIB_TRIVIAL_TYPE(Const);
GGP_LIB_TRIVIAL_TYPE(Pointer);
inline constexpr Const const_ {};
inline constexpr Pointer pointer {};

GGP_LIB_VARIANT_STRUCT_ONLY(TypeDecorator,
                            Const,
                            Pointer);

struct BuiltType
{
  std::vector<TypeDecorator> decorators;
  std::optional<PlainType> plain_type;

  auto
  is_pointer () const -> bool;
};

auto
BuiltType::is_pointer () const -> bool
{
  auto last_is_const = false;

  for (auto const& decorator : this->decorators)
  {
    if (std::holds_alternative<Pointer> (decorator.v))
    {
      return true;
    }
    if (last_is_const)
    {
      return false;
    }
    last_is_const = true;
  }

  return false;
}

GGP_LIB_VARIANT_STRUCT_ONLY(ResultType,
                            BuiltType,
                            Meh,
                            NullPointer);

GGP_LIB_VARIANT_STRUCT_ONLY(TempType,
                            TopNS::Const,
                            TopNS::Pointer,
                            PlainType,
                            Meh);

} // namespace TB

} // anonymous namespace

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"
struct TypeBuilder::TypeBuilderPrivate
{
  TB::ResultType result_type;
};
#pragma GCC diagnostic pop

TypeBuilder::TypeBuilder ()
  : priv {new TypeBuilderPrivate ()}
{}

TypeBuilder::~TypeBuilder ()
{}

auto
TypeBuilder::add_plain_type (PlainType plain_type) -> void
{
  auto failed = false;
  auto vh {VisitHelper {
    [plain_type = std::move (plain_type), &failed] (TB::BuiltType& built_type) mutable
    {
      failed = built_type.plain_type.has_value ();

      if (!failed)
      {
        built_type.plain_type = std::make_optional (std::move (plain_type));
      }
    },
    [](Meh&) {},
    [](NullPointer&) {},
  }};

  std::visit (vh, this->priv->result_type.v);

  if (failed)
  {
    this->priv->result_type.v = Meh {};
  }
}

auto
TypeBuilder::add_const () -> void
{
  auto vh {VisitHelper {
    [](TB::BuiltType& built_type) { built_type.decorators.push_back ({{TB::const_}}); },
    [](Meh&) {},
    [](NullPointer&) {},
  }};

  std::visit (vh, this->priv->result_type.v);
}

auto
TypeBuilder::add_pointer () -> void
{
  auto vh {VisitHelper {
    [](TB::BuiltType& built_type) { built_type.decorators.push_back ({{TB::pointer}}); },
    [](Meh&) {},
    [](NullPointer&) {},
  }};

  std::visit (vh, this->priv->result_type.v);
}

auto
TypeBuilder::add_meh () -> void
{
  this->priv->result_type.v = Meh {};
}

auto
TypeBuilder::add_null_pointer () -> void
{
  auto failed = false;
  auto vh {VisitHelper {
    [&failed] (TB::BuiltType const& built_type) mutable
    {
      failed = !built_type.is_pointer ();
    },
    [](Meh&) {},
    [](NullPointer&) {},
  }};

  std::visit (vh, this->priv->result_type.v);

  if (failed)
  {
    this->priv->result_type.v = Meh {};
  }
  else
  {
    this->priv->result_type.v = NullPointer {};
  }
}

auto
TypeBuilder::build_type () const -> Type
{
  auto vh {VisitHelper {
    [](TB::BuiltType const& built_type)
    {
      if (!built_type.plain_type)
      {
        return Type {{Meh {}}};
      }

      auto type {TB::TempType {{*built_type.plain_type}}};
      auto add_const =
        [](TB::TempType&& tmp_type) -> TB::TempType
        {
          auto inner_vh {VisitHelper {
            [](Const&&) { return TB::TempType {{Meh {}}}; },
            [](Pointer&& pointer)
            {
              return TB::TempType {{Const {{std::move (pointer)}}}};
            },
            [](PlainType&& plain_type)
            {
              return TB::TempType {{Const {{std::move (plain_type)}}}};
            },
            [](Meh&& meh) { return TB::TempType {{std::move (meh)}}; },
          }};

          return std::visit (inner_vh, std::move (tmp_type.v));
        };
      auto add_pointer =
        [](TB::TempType&& tmp_type) -> TB::TempType
        {
          auto inner_vh {VisitHelper {
            [](Const&& const_)
            {
              return TB::TempType {{Pointer {{std::move (const_)}}}};
            },
            [](Pointer&& pointer)
            {
              return TB::TempType {{Pointer {{std::move (pointer)}}}};
            },
            [](PlainType&& plain_type)
            {
              return TB::TempType {{Pointer {{std::move (plain_type)}}}};
            },
            [](Meh&& meh) { return TB::TempType {{std::move (meh)}}; },
          }};

          return std::visit (inner_vh, std::move (tmp_type.v));
        };

      for (auto const& decorator : reverse (built_type.decorators))
      {
        auto tmp {TB::TempType {std::move (type)}};
        auto decorator_vh {VisitHelper {
          [&tmp, &add_pointer](TB::Pointer const&)
          {
            return add_pointer (std::move (tmp));
          },
          [&tmp, &add_const](TB::Const const&)
          {
            return add_const (std::move (tmp));
          },
        }};

        type = std::visit (decorator_vh, decorator.v);
      }

      return repackage<Type> (type);
    },
    [](Meh const& meh) { return Type {{meh}}; },
    [](NullPointer const& null_pointer) { return Type {{null_pointer}}; },
  }};

  return std::visit (vh, this->priv->result_type.v);
}

} // namespace Ggp::Lib
