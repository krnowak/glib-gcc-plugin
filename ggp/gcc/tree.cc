/* This file is part of glib-gcc-plugin.
 *
 * Copyright 2018 Krzesimir Nowak
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

#include "ggp/gcc/tree.hh"

#include <queue>

namespace Ggp::Gcc
{

auto iterate_tree(tree root, std::function<void(tree)> callback) -> void
{
  std::queue<tree> trees;
  std::set<tree> visited_trees;

  trees.push (root);

  while (!trees.empty ())
  {
    auto queued_tree {trees.front ()};
    trees.pop ();

    if (queued_tree == nullptr)
    {
      continue;
    }

    if (auto [iter, inserted] {visited_trees.insert (queued_tree)}; !inserted)
    {
      continue;
    }

    callback(queued_tree);

    auto const code {TREE_CODE (queued_tree)};
    auto const code_class {TREE_CODE_CLASS (code)};

    if (IS_EXPR_CODE_CLASS (code_class))
    {
      trees.push (TREE_TYPE (queued_tree));

      switch (code_class)
      {
      case tcc_unary:
        trees.push (TREE_OPERAND (queued_tree, 0));
        break;

      case tcc_binary:
      case tcc_comparison:
        trees.push (TREE_OPERAND (queued_tree, 0));
        trees.push (TREE_OPERAND (queued_tree, 1));
        break;

      case tcc_expression:
      case tcc_reference:
      case tcc_statement:
      case tcc_vl_exp:
        /* These nodes are handled explicitly below.  */
        break;

      default:
        gcc_unreachable ();
      }
    }
    else if (DECL_P (queued_tree))
    {
      /* All declarations have names.  */
      if (DECL_NAME (queued_tree))
      {
        trees.push (DECL_NAME (queued_tree));
      }
      if (HAS_DECL_ASSEMBLER_NAME_P (queued_tree)
          && DECL_ASSEMBLER_NAME_SET_P (queued_tree)
          && DECL_ASSEMBLER_NAME (queued_tree) != DECL_NAME (queued_tree))
        trees.push (DECL_ASSEMBLER_NAME (queued_tree));
      if (DECL_ABSTRACT_ORIGIN (queued_tree))
        trees.push (DECL_ABSTRACT_ORIGIN (queued_tree));
      /* And types.  */
      trees.push (TREE_TYPE (queued_tree));
      trees.push (DECL_CONTEXT (queued_tree));
      if (DECL_CHAIN (queued_tree))
      {
        trees.push (DECL_CHAIN (queued_tree));
      }
    }
    else if (code_class == tcc_type)
    {
      /* All types have associated declarations.  */
      trees.push (TYPE_NAME (queued_tree));

      /* All types have a main variant.  */
      if (TYPE_MAIN_VARIANT (queued_tree) != queued_tree)
        trees.push (TYPE_MAIN_VARIANT (queued_tree));

      /* And sizes.  */
      trees.push (TYPE_SIZE (queued_tree));
    }
    else if (code_class == tcc_constant)
    {
      /* All constants can have types.  */
      trees.push (TREE_TYPE (queued_tree));
    }

    /* Now handle the various kinds of nodes.  */
    switch (code)
    {
    case TREE_LIST:
      trees.push (TREE_PURPOSE (queued_tree));
      trees.push (TREE_VALUE (queued_tree));
      trees.push (TREE_CHAIN (queued_tree));
      break;

    case STATEMENT_LIST:
      for (auto it = tsi_start (queued_tree);
           !tsi_end_p (it);
           tsi_next (&it))
      {
        trees.push (tsi_stmt (it));
      }
      break;

    case TREE_VEC:
      for (int i = 0; i < TREE_VEC_LENGTH (queued_tree); ++i)
      {
        trees.push (TREE_VEC_ELT (queued_tree, i));
      }
      break;

    case INTEGER_TYPE:
    case ENUMERAL_TYPE:
      trees.push (TYPE_MIN_VALUE (queued_tree));
      trees.push (TYPE_MAX_VALUE (queued_tree));

      if (code == ENUMERAL_TYPE)
      {
        trees.push (TYPE_VALUES (queued_tree));
      }
      break;

    case REAL_TYPE:
      break;

    case FIXED_POINT_TYPE:
      break;

    case POINTER_TYPE:
      trees.push (TREE_TYPE (queued_tree));
      break;

    case REFERENCE_TYPE:
      trees.push (TREE_TYPE (queued_tree));
      break;

    case METHOD_TYPE:
      trees.push (TYPE_METHOD_BASETYPE (queued_tree));
      /* Fall through.  */

    case FUNCTION_TYPE:
      trees.push (TREE_TYPE (queued_tree));
      trees.push (TYPE_ARG_TYPES (queued_tree));
      break;

    case ARRAY_TYPE:
      trees.push (TREE_TYPE (queued_tree));
      trees.push (TYPE_DOMAIN (queued_tree));
      break;

    case RECORD_TYPE:
    case UNION_TYPE:
      trees.push (TYPE_FIELDS (queued_tree));
      trees.push (TYPE_BINFO (queued_tree));
      break;

    case CONST_DECL:
      trees.push (DECL_INITIAL (queued_tree));
      break;

    case DEBUG_EXPR_DECL:
    case VAR_DECL:
    case PARM_DECL:
    case FIELD_DECL:
    case RESULT_DECL:
      if (code == PARM_DECL)
        trees.push (DECL_ARG_TYPE (queued_tree));
      else
        trees.push (DECL_INITIAL (queued_tree));
      trees.push (DECL_SIZE (queued_tree));

      if (code == FIELD_DECL)
      {
        if (DECL_FIELD_OFFSET (queued_tree))
          trees.push (bit_position (queued_tree));
      }
      break;

    case FUNCTION_DECL:
      trees.push (DECL_ARGUMENTS (queued_tree));
      trees.push (DECL_SAVED_TREE (queued_tree));
      break;

    case INTEGER_CST:
      break;

    case STRING_CST:
      break;

    case REAL_CST:
      break;

    case FIXED_CST:
      break;

    case TRUTH_NOT_EXPR:
    case ADDR_EXPR:
    case INDIRECT_REF:
    case CLEANUP_POINT_EXPR:
    case SAVE_EXPR:
    case REALPART_EXPR:
    case IMAGPART_EXPR:
      /* These nodes are unary, but do not have code class `1'.  */
      trees.push (TREE_OPERAND (queued_tree, 0));
      break;

    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
    case INIT_EXPR:
    case MODIFY_EXPR:
    case COMPOUND_EXPR:
    case PREDECREMENT_EXPR:
    case PREINCREMENT_EXPR:
    case POSTDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:
      /* These nodes are binary, but do not have code class `2'.  */
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      break;

    case COMPONENT_REF:
    case BIT_FIELD_REF:
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      trees.push (TREE_OPERAND (queued_tree, 2));
      break;

    case ARRAY_REF:
    case ARRAY_RANGE_REF:
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      trees.push (TREE_OPERAND (queued_tree, 2));
      trees.push (TREE_OPERAND (queued_tree, 3));
      break;

    case COND_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      trees.push (TREE_OPERAND (queued_tree, 2));
      break;

    case TRY_FINALLY_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      break;

    case CALL_EXPR:
      {
        tree arg;
        call_expr_arg_iterator iter;
        trees.push (CALL_EXPR_FN (queued_tree));
        FOR_EACH_CALL_EXPR_ARG (arg, iter, queued_tree)
        {
          trees.push (arg);
        }
      }
      break;

    case CONSTRUCTOR:
      {
        unsigned HOST_WIDE_INT cnt;
        tree index, value;
        FOR_EACH_CONSTRUCTOR_ELT (CONSTRUCTOR_ELTS (queued_tree), cnt, index, value)
        {
          trees.push (index);
          trees.push (value);
        }
      }
      break;

    case BIND_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      break;

    case LOOP_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      break;

    case EXIT_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      break;

    case RETURN_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      break;

    case TARGET_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      trees.push (TREE_OPERAND (queued_tree, 2));
      /* There really are two possible places the initializer can be.
         After RTL expansion, the second operand is moved to the
         position of the fourth operand, and the second operand
         becomes NULL.  */
      trees.push (TREE_OPERAND (queued_tree, 3));
      break;

    case CASE_LABEL_EXPR:
      trees.push (CASE_LABEL (queued_tree));
      if (CASE_LOW (queued_tree))
      {
        trees.push (CASE_LOW (queued_tree));
        if (CASE_HIGH (queued_tree))
          trees.push (CASE_HIGH (queued_tree));
      }
      break;

    case LABEL_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      break;

    case GOTO_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      break;

    case SWITCH_EXPR:
      trees.push (TREE_OPERAND (queued_tree, 0));
      trees.push (TREE_OPERAND (queued_tree, 1));
      if (TREE_OPERAND (queued_tree, 2))
      {
        trees.push (TREE_OPERAND (queued_tree, 2));
      }
      break;

    case OMP_CLAUSE:
      for (int i = 0; i < omp_clause_num_ops[OMP_CLAUSE_CODE (queued_tree)]; i++)
      {
        trees.push (OMP_CLAUSE_OPERAND (queued_tree, i));
      }
      break;

    default:
      break;
    }
  }
}

} // namespace Ggp::Gcc
