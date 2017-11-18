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

#include "ggp/gcc/vc.hh"

#include "ggp/gcc/generated/type.hh"
#include "ggp/gcc/generated/variant.hh"

#include <optional>
#include <queue>

namespace Ggp::Gcc
{

namespace {

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

FormatInfo
must_get_format_info_from_args (tree attribute_args)
{
  auto format_type_string = must_get_string (TREE_VALUE (attribute_args));
  auto format_type = must_get_format_type (format_type_string);
  auto string_index = must_get_int (TREE_VALUE (TREE_CHAIN (attribute_args)));
  auto args_index = must_get_int (TREE_VALUE (TREE_CHAIN (TREE_CHAIN (attribute_args))));

  return FormatInfo {format_type, string_index, args_index};
}

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

struct CallSite
{
  tree call_expr;
  tree attribute;
};

void
ggp_vc_finish_parse_function (void* gcc_data,
                              void* /* user_data */)
{
  dump_node (static_cast<const_tree> (gcc_data), 0, stderr);
  auto function_decl = static_cast<tree> (gcc_data);
  gcc_assert (TREE_CODE (function_decl) == FUNCTION_DECL);

  auto function_body = DECL_SAVED_TREE (function_decl);
  gcc_assert (function_body);

  std::queue<tree> trees;
  std::vector<tree> call_exprs;
  std::vector<CallSite> call_sites;
  trees.push (static_cast<tree> (gcc_data));

  while (!trees.empty ())
  {
    auto queued_tree {trees.front ()};
    trees.pop ();

    if (queued_tree == nullptr)
    {
      continue;
    }

    switch (TREE_CODE (queued_tree))
    {
    case FUNCTION_DECL:
      trees.push (DECL_SAVED_TREE (queued_tree));
      break;

    case BIND_EXPR:
      // operand 1 is a body (0 are variable declarations)
      trees.push (TREE_OPERAND (queued_tree, 1));
      break;

    case STATEMENT_LIST:
      {
        for (auto tsi = tsi_start (queued_tree); !tsi_end_p (tsi); tsi_next (&tsi))
        {
          trees.push (tsi_stmt (tsi));
        }
        break;
      }

    case CALL_EXPR:
      call_exprs.push_back (queued_tree);
      break;

    default:
      break;
    }
  }

  for (auto call_expr : call_exprs)
  {
    auto called_function = CALL_EXPR_FN (call_expr);
    if (TREE_CODE (called_function) != ADDR_EXPR)
    {
      continue;
    }
    auto function_decl = TREE_OPERAND (called_function, 0);
    if (TREE_CODE (function_decl) != FUNCTION_DECL)
    {
      continue;
    }
    auto function_type = TREE_TYPE (function_decl);
    if (TREE_CODE (function_type) != FUNCTION_TYPE)
    {
      continue;
    }
    for (auto attr = TYPE_ATTRIBUTES(function_type); attr; attr = TREE_CHAIN (attr))
    {
      if (is_attribute_p ("glib_variant", TREE_PURPOSE (attr)))
      {
        call_sites.push_back ({call_expr, attr});
        break;
      }
    }
  }

  for (auto call_site : call_sites)
  {
    warning (0, "called a glib_variant function %s", IDENTIFIER_POINTER (DECL_NAME (TREE_OPERAND (CALL_EXPR_FN (call_site.call_expr), 0))));
    auto format_info = must_get_format_info_from_args (TREE_VALUE (call_site.attribute));

    auto arg = NULL_TREE;
    call_expr_arg_iterator ceai;
    unsigned int idx = 0;
    auto format_param = NULL_TREE;
    std::vector<tree> format_arg_params;

    FOR_EACH_CALL_EXPR_ARG (arg, ceai, call_site.call_expr)
    {
      ++idx;
      if (idx == format_info.string_index)
      {
        format_param = arg;
      }
      else if (idx >= format_info.args_index)
      {
        format_arg_params.push_back (arg);
      }
    }

    const char *format = nullptr;

    if (TREE_CODE (format_param) == NOP_EXPR)
    {
      auto nop_op_0 = TREE_OPERAND (format_param, 0);
      if (TREE_CODE (nop_op_0) == ADDR_EXPR)
      {
        auto addr_op_0 = TREE_OPERAND (nop_op_0, 0);
        if (TREE_CODE (addr_op_0) == STRING_CST)
        {
          format = TREE_STRING_POINTER (addr_op_0);
        }
      }
    }
    if (format)
    {
      warning (0, "format: %s", format);
    }
    else
    {
      warning (0, "format is not a string literal");
      continue;
    }
    auto mvf = Lib::VariantFormat::from_string (format);
    if (!mvf)
    {
      warning (0, "invalid variant format");
      continue;
    }
    auto types = Lib::expected_types_for_format (*mvf);
    if (types.size() != format_arg_params.size())
    {
      warning (0, "expected %lu parameters, got %lu", types.size(), format_arg_params.size());
    }
    // TODO: compare types to actual passed parameters.
  }
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
            tree /* name */,
            tree args,
            int /* flags */,
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

const pass_data vc_cfg_pass_data =
{
  GIMPLE_PASS, /* type */
  "vc_cfg", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_NONE, /* tv_id */
  PROP_cfg, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0, /* todo_flags_finish */
};

class vc_cfg_pass : public gimple_opt_pass
{
public:
  vc_cfg_pass(gcc::context *ctxt)
    : gimple_opt_pass(vc_cfg_pass_data, ctxt)
  {}

  /* opt_pass methods: */
  virtual unsigned int execute (function *);

};

unsigned int
vc_cfg_pass::execute (function *)
{
  warning (0, "Analyze cfg of function %s",
           IDENTIFIER_POINTER (DECL_NAME (current_function_decl)));
  gimple_debug_cfg (0);
  return 0;
}

std::unique_ptr<register_pass_info>
get_register_vc_cfg_pass_info ()
{
  // g - a global gcc::context
  register_pass_info pass_info { new vc_cfg_pass (g), "cfg", 1, PASS_POS_INSERT_AFTER };
  return std::make_unique<register_pass_info> (pass_info);
}

} // anonymous namespace

VariantChecker::VariantChecker (struct plugin_name_args* plugin_info)
  : name {subplugin_name (plugin_info, "vc")},
    finish_decl {name, PLUGIN_FINISH_DECL, ggp_vc_finish_decl, this},
    start_parse_function {name, PLUGIN_START_PARSE_FUNCTION, ggp_vc_start_parse_function, this},
    finish_parse_function {name, PLUGIN_FINISH_PARSE_FUNCTION, ggp_vc_finish_parse_function, this},
    attributes {name, PLUGIN_ATTRIBUTES, ggp_vc_attributes, this}
{
  auto reg_pass_info {get_register_vc_cfg_pass_info ()};
  // Nothing to unregister for the PLUGIN_PASS_MANAGER_SETUP event -
  // it takes no callback.
  ::register_callback (name.c_str (),
                       PLUGIN_PASS_MANAGER_SETUP,
                       NULL,
                       reg_pass_info.get ());
}

} // namespace Ggp::Gcc
