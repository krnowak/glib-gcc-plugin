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

#include "ggp-vc.hh"

#include "diagnostic.h"
#include "tree.h"
#include "dumpfile.h"

#include <optional>

namespace Ggp
{

namespace {

void
ggp_vc_finish_decl (void* /* gcc_data */,
                    void* /* user_data */)
{
  //tree tr = static_cast<tree> (gcc_data);
  //dump_tree (tr, "  ", 1);
}

void
ggp_vc_start_parse_function (void* /* gcc_data */,
                             void* /* user_data */)
{
  //tree tr = static_cast<tree> (gcc_data);
  //dump_tree (tr, "  ", 1);
}

void
ggp_vc_finish_parse_function (void* gcc_data,
                              void* /* user_data */)
{
  // TODO:
  // cast gcc_data to tree
  // traverse the tree to find call_exprs
  // from call_expr take fn
  // if fn is addr_expr, then there are two ways to get function type
  //   - type -> pointer type -> pointed type, which is our function type
  //   - op 0 -> function decl -> type, which is our function type
  // figure out if fn can be something else than addr_expr
  // use TYPE_ATTRIBUTES(function_type) to get attribute list
  // use `for (tree a = attrs; a; a = TREE_CHAIN (a))` to traverse attributes
  // use `is_attribute_p ("glib_variant", TREE_PURPOSE(a))` to check if it is our attribute
  // attribute args are in TREE_VALUE(a)
  dump_node (static_cast<const_tree> (gcc_data), 0, stderr);
}

std::optional<unsigned HOST_WIDE_INT>
get_int (tree expr)
{
  if (tree_fits_uhwi_p (expr))
  {
    return {TREE_INT_CST_LOW (expr)};
  }

  return {};
}

unsigned HOST_WIDE_INT
must_get_int (tree expr)
{
  auto maybe_int {get_int (expr)};

  gcc_assert (maybe_int.has_value ());

  return maybe_int.value ();
}

std::optional<std::string>
get_string (tree expr)
{
  if (expr != NULL_TREE &&
      TREE_CODE (expr) == STRING_CST)
  {
    return {TREE_STRING_POINTER (expr)};
  }

  return {};
}

std::string
must_get_string (tree expr)
{
  auto maybe_string {get_string (expr)};

  gcc_assert (maybe_string.has_value ());

  return maybe_string.value ();
}

enum class FormatType
{
  Get,
  Set
};

std::optional<FormatType>
get_format_type (std::string const& type_string)
{
  if (type_string == "get")
  {
    return {FormatType::Get};
  }

  if (type_string == "set")
  {
    return {FormatType::Set};
  }

  return {};
}

FormatType
must_get_format_type (std::string const& type_string)
{
  auto maybe_format_type {get_format_type (type_string)};

  gcc_assert (maybe_format_type.has_value ());

  return maybe_format_type.value ();
}

struct FormatInfo {
  FormatType type;
  unsigned HOST_WIDE_INT string_index;
  unsigned HOST_WIDE_INT args_index;
};

std::optional<FormatInfo>
get_format_info_from_args (tree attribute_args)
{
  auto maybe_format_type_string = get_string (TREE_VALUE (attribute_args));
  if (!maybe_format_type_string.has_value ())
  {
    error ("expected a string as a first parameter");
    return {};
  }

  auto maybe_format_type = get_format_type (maybe_format_type_string.value ());
  if (!maybe_format_type.has_value ())
  {
    error ("expected either \"get\" or \"set\" as a format type");
    return {};
  }

  auto maybe_string_index = get_int (TREE_VALUE (TREE_CHAIN (attribute_args)));
  if (!maybe_string_index.has_value ())
  {
    error("expected an integer as a second parameter");
    return {};
  }

  auto maybe_args_index = get_int (TREE_VALUE (TREE_CHAIN (TREE_CHAIN (attribute_args))));
  if (!maybe_args_index.has_value ())
  {
    error("expected an integer as a second parameter");
    return {};
  }

  if (maybe_string_index.value() >= maybe_args_index.value())
  {
    error("format string should come before formating arguments");
    return {};
  }

  return {FormatInfo {maybe_format_type.value(), maybe_string_index.value(), maybe_args_index.value()}};
}

bool
is_ptr_to_char (tree param)
{
  return ((param != NULL_TREE) &&
          (TREE_CODE (param) == POINTER_TYPE) &&
          (TYPE_MAIN_VARIANT (TREE_TYPE (param)) == char_type_node));
}

tree
vc_handler (tree* node,
            tree name,
            tree args,
            int flags,
            bool* no_add_attrs)
{
  auto maybe_format_info {get_format_info_from_args (args)};
  if (!maybe_format_info.has_value ())
  {
    *no_add_attrs = true;
    return NULL_TREE;
  }
  auto const& format_info = maybe_format_info.value ();

  auto param_format = NULL_TREE;
  auto param_count = 0u;
  auto param = NULL_TREE;
  function_args_iterator params_iter;

  FOREACH_FUNCTION_ARGS (*node, param, params_iter)
  {
    ++param_count;
    if (param_count == format_info.string_index)
    {
      param_format = param;
    }
  }
  if (param_format == NULL_TREE)
  {
    error("expected format string parameter at index "
          HOST_WIDE_INT_PRINT_UNSIGNED
          ", but function has only %u parameter(s)",
          format_info.string_index,
          param_count);
    *no_add_attrs = true;
    return NULL_TREE;
  }
  if (!is_ptr_to_char (param_format))
  {
    error("expected format string parameter to be a string");
    *no_add_attrs = true;
    return NULL_TREE;
  }
  if (param_count + 1 != format_info.args_index)
  {
    error("expected varargs to start at " HOST_WIDE_INT_PRINT_UNSIGNED
          ", not at %u",
          format_info.args_index, param_count + 1);
    *no_add_attrs = true;
    return NULL_TREE;
  }
  return NULL_TREE;
}

struct attribute_spec vc_attribute_spec =
{
  /* name:                   */ "glib_variant",
  /* min parameter count:    */ 3,
  /* max parameter count:    */ 3,
  /* decl required:          */ false,
  /* type required:          */ true,
  /* function type required: */ true,
  /* handler:                */ vc_handler,
  /* affects type identity:  */ false
};

void
ggp_vc_attributes (void* /* gcc_data, it is always NULL */,
                   void* /* user_data */)
{
  warning (0, "ggp_vc_attributes");
  register_attribute (&vc_attribute_spec);
}

} // namespace

VariantChecker::VariantChecker (struct plugin_name_args* plugin_info)
  : name {Util::subplugin_name (plugin_info, "vc")},
    finish_decl {name, PLUGIN_FINISH_DECL, ggp_vc_finish_decl, this},
    start_parse_function {name, PLUGIN_START_PARSE_FUNCTION, ggp_vc_start_parse_function, this},
    finish_parse_function {name, PLUGIN_FINISH_PARSE_FUNCTION, ggp_vc_finish_parse_function, this},
    attributes {name, PLUGIN_ATTRIBUTES, ggp_vc_attributes, this}
{}

} // namespace Ggp
