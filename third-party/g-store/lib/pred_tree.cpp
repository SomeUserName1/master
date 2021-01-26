#include "pred_tree.h"

#include "util_t.h"
#include "util.h"
#include "structs.h"
#include "read_query.h"

/* See dissertation Section 7.4 */
//todo: remove/deal with skip fields
//#include <g-store.h>

char* pred_parser::get_str_pos()
{
  return str0;
}

//reduced_pc : produces error if any item from pseudocolumn B is selected
//used_extended_ : true if any item from pseudocolumn C is selected, or LEVEL, or ISCYCLE
pred_tree_elem* pred_parser::parse(char* str0_, bool reduced_pc_, bool &used_extended_, bool set_extr) 
{
  str0 = str0_;
  str_eat_ws(str0);
  reduced_pc = reduced_pc_;
  used_extended = used_extended_;
  
  extrema_rownum_lvl extr_rownum_lvl;

  pred_tree_elem* tmp = parse_or(extr_rownum_lvl);
  
  if (extr_rownum_lvl.changed_rownum && extr_rownum_lvl.min_rownum > 1)
  {
    free_pred_tree(tmp);
    tmp = new pred_tree_elem_false();
    print_ln("Warning: A predicate on ROWNUM prevents the query from returning any results.");
  }

  if (set_extr)
  {
    QUERY::last_lvl_q = extr_rownum_lvl.max_lvl;
    QUERY::max_rownum = extr_rownum_lvl.max_rownum + 1;
  }

  used_extended_ = used_extended;
  return tmp;
}

void pred_parser::parse_seq(char* str0_) 
{
  str0 = str0_;
  reduced_pc = true;

  extrema_rownum_lvl extr_rownum_lvl;
  pred_tree_elem* tmp;
  
  for(;;) 
  {
    str_eat_ws(str0);
    if (str_tok_cmp_adv_qd(str0, "ANY", 3))
      tmp = new pred_tree_elem_true();
    else
      tmp = parse_or(extr_rownum_lvl);

    QUERY::pred_tree_through_seq.push_back(tmp);
    str_eat_ws(str0);
    if (*str0 == '.')
      str0++;
    else
      break;
  }

  QUERY::thr_seq_cnt = QUERY::pred_tree_through_seq.size();
}


void pred_parser::free_pred_tree(pred_tree_elem* e)
{
  if (e->left != NULL)
    free_pred_tree(e->left);
 
  if (e->right != NULL)
    free_pred_tree(e->right);

  delete e;
}

pred_tree_elem* pred_parser::parse_or(extrema_rownum_lvl &extr_result) 
{
  pred_tree_elem* left(parse_and(extr_result));

  if (!str_tok_cmp_adv_qd(str0, "OR", 1))
      return left;

  str_eat_ws(str0);
  
  extrema_rownum_lvl extr_right;
  pred_tree_elem *right = parse_or(extr_right);
  extr_result.apply_or(extr_right);
  
  return new pred_tree_elem_or(left, right);
}

pred_tree_elem* pred_parser::parse_and(extrema_rownum_lvl &extr_result) 
{
  pred_tree_elem* left(parse_not(extr_result));

  if (*str0 != ',' && !str_tok_cmp_adv_qd(str0, "AND", 1))
      return left;

  if (*str0 == ',')
    str0++;

  str_eat_ws(str0);

  extrema_rownum_lvl extr_right;
  pred_tree_elem *right = parse_and(extr_right);
  extr_result.apply_and(extr_right);

  return new pred_tree_elem_and(left, right);
}

pred_tree_elem* pred_parser::parse_not(extrema_rownum_lvl &extr_result) 
{
  bool not = false;

  while (str_tok_cmp_adv_qd(str0, "NOT", 1))
  {
    str_eat_ws(str0);
    not = !not;
  }

  pred_tree_elem* tmp = parse_parenth(extr_result);

  if (not)
  {
    extr_result.apply_not();
    return new pred_tree_elem_not(tmp);
  }
  else
    return tmp;
}

pred_tree_elem* pred_parser::parse_parenth(extrema_rownum_lvl &extr_result) 
{
  if (str_tok_cmp_adv(str0, "(", 2))
  {
    str_eat_ws(str0);
    
    pred_tree_elem* tmp(parse_or(extr_result));
    str_eat_ws_tok_qd(str0, ')');
    
    str_eat_ws(str0);
    return tmp;
  }
  else
    return parse_term(extr_result);
}

pred_tree_elem* pred_parser::parse_term(extrema_rownum_lvl &extr_result) 
{
  int field_id, cnt_str_len;
  ops op;
  char* tmp_str = malloc1<char>(GLOBALS::blk_writable, "todo");
  int8 tmp_int8;
  uint8 tmp_uint8;
  char* str1;
  
  pred_tree_elem_TERM* pred_ptr;
    
  if (str_tok_cmp_adv_qd(str0, "GID", 1))
    field_id = sel_gid_;
  else if (str_tok_cmp_adv_qd(str0, "COUNT_EDGES", 1))
    field_id = sel_count_edges_;     
  else if (str_tok_cmp_adv_qd(str0, "ISLEAF", 1))
    field_id = sel_isleaf_;
  else if (str_tok_cmp_adv_qd(str0, "ROWNUM", 1))
  {
    if (reduced_pc)
      error_exit("ROWNUM is not allowed as a predicate in END WITH or THROUGH.\nNot parsed: %s", str0);
    field_id = sel_rownum_;
  }
  else if (str_tok_cmp_adv_qd(str0, "LEVEL", 1))
  {
    used_extended = true;
    if (reduced_pc)
      error_exit("LEVEL is not allowed as a predicate in END WITH or THROUGH.\nNot parsed: %s", str0);
    field_id = sel_lvl_;
  }
  else if (str_tok_cmp_adv_qd(str0, "ISCYCLE", 1))
  {
    used_extended = true;
    if (reduced_pc)
      error_exit("ISCYCLE is not allowed as a predicate in END WITH or THROUGH.\nNot parsed: %s", str0);
    field_id = sel_iscycle_;
  }
  else
    field_id = parse_field_id_adv(str0);
  
  str_eat_ws(str0);

  if (str_tok_cmp_adv_qd(str0, "=", 2))
    op = eq_; 
  else if (str_tok_cmp_adv_qd(str0, "!=", 2))
    op = neq_;
  else if (str_tok_cmp_adv_qd(str0, "<=", 2))
    op = smeq_;
  else if (str_tok_cmp_adv_qd(str0, "<", 2))
    op = sm_;
  else if (str_tok_cmp_adv_qd(str0, ">=", 2))
    op = greq_;
  else if (str_tok_cmp_adv_qd(str0, ">", 2))
    op = gr_;
  else
    error_exit("Unknown operator in query definition, expecting = != <= < >= >\nNot parsed: %s", str0);

  str_eat_ws(str0);
  
  switch (field_id)
  {
  case sel_gid_:
    if (*str0 == '-')
    {
      pred_ptr = new pred_tree_elem_false();
      print_ln("Warning: A predicate in the query definition will never "
        "evaluate to true (negative GID)");
      break;
    }

    tmp_uint8 = _strtoui64(str0, &str1, 10);
    str0 = str1;

    if (tmp_uint8 > UINT4_MAX)
    {
      pred_ptr = new pred_tree_elem_false();
      print_ln("Warning: A predicate in the query definition will never "
        "evaluate to true (out of bounds GID)");
      break;
    }

    pred_ptr = new pred_tree_elem_gid((g_id) tmp_uint8);
    assign_op_funtion(op, ((pred_tree_elem_gid*)pred_ptr)->check_cond);
    break;

  case sel_count_edges_:
    tmp_int8 = _strtoi64(str0, &str1, 10);
    str0 = str1;
    if (tmp_int8 > UINT2_MAX)
    {
      pred_ptr = new pred_tree_elem_false();
      print_ln("Warning: A predicate in the query definition will never "
        "evaluate to true (out of bounds COUNT_EDGES)");
      break;
    }

    pred_ptr = new pred_tree_elem_countedges((b_sc) tmp_int8);
    assign_op_funtion(op, ((pred_tree_elem_countedges*)pred_ptr)->check_cond);
    break;

  case sel_isleaf_:
    if (op != eq_ && op != neq_)
      error_exit("Expecting operator = != after pseudocolumn ISLEAF.\nNot parsed: %s", str0);

    pred_ptr = new pred_tree_elem_countedges((b_sc) 0);

    if (*str0 == '1' || *str0 == 't' || *str0 == 'T')
    {
      if (op == eq_)
        assign_op_funtion(gr_, ((pred_tree_elem_countedges*)pred_ptr)->check_cond);  
      else 
        assign_op_funtion(eq_, ((pred_tree_elem_countedges*)pred_ptr)->check_cond);  
    }
    else if (*str0 == '0' || *str0 == 'f' || *str0 == 'F')
    {
      if (op == neq_)
        assign_op_funtion(gr_, ((pred_tree_elem_countedges*)pred_ptr)->check_cond);  
      else 
        assign_op_funtion(eq_, ((pred_tree_elem_countedges*)pred_ptr)->check_cond);  
    } 
    else
      error_exit("In query definition: Predicate on boolean field ISLEAF does "
      "not start with 1 t T 0 f F\nNot parsed: %s", field_id, str0);
    
    while (isalnum(*str0))
      str0++;
    break;
  
  case sel_rownum_:
    if (*str0 == '-')
    {
      pred_ptr = new pred_tree_elem_false();
      print_ln("Warning: A predicate in the query definition will never "
        "evaluate to true (negative ROWNUM)", field_id);
      break;
    }

    tmp_uint8 = _strtoui64(str0, &str1, 10);
    str0 = str1;

    if (tmp_uint8 > UINT4_MAX)
    {
      pred_ptr = new pred_tree_elem_false();
      print_ln("Warning: A predicate in the query definition will never "
        "evaluate to true (out of bounds ROWNUM)");
      break;
    }

    pred_ptr = new pred_tree_elem_rownum((uint4) tmp_uint8);
    assign_op_funtion(op, ((pred_tree_elem_rownum*)pred_ptr)->check_cond);
    extr_result.update_rownum((uint4) tmp_uint8, op);
    break;
  
  case sel_lvl_:
    tmp_int8 = _strtoi64(str0, &str1, 10);
    str0 = str1;

    if (tmp_int8 < 1 || tmp_int8 > INT4_MAX)
    {
      pred_ptr = new pred_tree_elem_false();
      print_ln("Warning: A predicate in the query definition will never "
        "evaluate to true (out of bounds LEVEL)", field_id);
      break;
    }

    pred_ptr = new pred_tree_elem_lvl((int4) tmp_int8);
    assign_op_funtion(op, ((pred_tree_elem_lvl*)pred_ptr)->check_cond);
    extr_result.update_lvl((int4) tmp_int8, op);
    break;

  
  case sel_iscycle_:
    if (op != eq_ && op != neq_)
      error_exit("Expecting operator = != after pseudocolumn ISCYCLE\nNot parsed: %s", str0);

    if (*str0 == '1' || *str0 == 't' || *str0 == 'T')
    {
      pred_ptr = new pred_tree_elem_iscycle(true);  
    }
    else if (*str0 == '0' || *str0 == 'f' || *str0 == 'F')
    {
      pred_ptr = new pred_tree_elem_iscycle(false);  
    } 
    else
      error_exit("In query definition: Predicate on boolean field ISLEAF does "
      "not start with 1 t T 0 f F\nNot parsed: %s", field_id, str0);

    while (isalnum(*str0))
      str0++;

    QUERY::sel_iscycle = true;
    assign_op_funtion(op, ((pred_tree_elem_iscycle*)pred_ptr)->check_cond);
    break;

  //////////////////////////////////////////////////////////////////////////
  default:
       
    switch (GLOBALS::rec_fields[field_id].field_datatype)
    {
    case bool_:
      if (op != eq_ && op != neq_)
        error_exit("Expecting operator = != after boolean field %d.\nNot parsed: %s", field_id, str0);

      if (*str0 == '1' || *str0 == 't' || *str0 == 'T')
        pred_ptr = new pred_tree_elem_num_bool<bool>(true);
      else if (*str0 == '0' || *str0 == 'f' || *str0 == 'F')
        pred_ptr = new pred_tree_elem_num_bool<bool>(false);
      else
        error_exit("In query definition: Predicate on boolean field %d does "
          "not start with 1 t T 0 f F\nNot parsed: %s", field_id, str0);
      
      assign_op_funtion(op, ((pred_tree_elem_num_bool<bool>*)pred_ptr)->check_cond);
      while (isalnum(*str0))
        str0++; 

      break;
    
    case uint_:
      if (*str0 == '-')
      {
        pred_ptr = new pred_tree_elem_false();
        print_ln("Warning: A predicate in the query definition will never "
          "evaluate to true (field %d; negative uint)", field_id);
        break;
      }
      
      tmp_uint8 = _strtoui64(str0, &str1, 10);
      str0 = str1;

      switch (GLOBALS::rec_fields[field_id].field_length)
      {
      case 1:
        if (tmp_uint8 > UINT1_MAX)
        {
          pred_ptr = new pred_tree_elem_false();
          print_ln("Warning: A predicate in the query definition will never "
            "evaluate to true (field %d; out of bounds uint(1))", field_id);
          break;
        }
        
        pred_ptr = new pred_tree_elem_num_bool<uint1>((uint1) tmp_uint8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<uint1>*)pred_ptr)->check_cond);        
        break;

      case 2:
        if (tmp_uint8 > UINT2_MAX)
        {
          pred_ptr = new pred_tree_elem_false();
          print_ln("Warning: A predicate in the query definition will never "
            "evaluate to true (field %d; out of bounds uint(2))", field_id);
          break;
        }

        pred_ptr = new pred_tree_elem_num_bool<uint2>((uint2) tmp_uint8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<uint2>*)pred_ptr)->check_cond);
        break;
      
      case 4:
        if (tmp_uint8 > UINT4_MAX)
        {
          pred_ptr = new pred_tree_elem_false();
          print_ln("Warning: A predicate in the query definition will never "
            "evaluate to true (field %d; out of bounds uint(4))", field_id);
          break;
        }
        
        pred_ptr = new pred_tree_elem_num_bool<uint4>((uint4) tmp_uint8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<uint4>*)pred_ptr)->check_cond);
        break;
      
      case 8:
        pred_ptr = new pred_tree_elem_num_bool<uint8>((uint8) tmp_uint8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<uint8>*)pred_ptr)->check_cond);
        break;
      }
      break;

    case int_:
      tmp_int8 = _strtoi64(str0, &str1, 10);
      str0 = str1;

      switch (GLOBALS::rec_fields[field_id].field_length)
      {
      case 1:
        if (tmp_int8 < INT1_MIN || tmp_int8 > INT1_MAX)
        {
          pred_ptr = new pred_tree_elem_false();
          print_ln("Warning: A predicate in the query definition will never "
            "evaluate to true (field %d; out of bounds int(1))", field_id);
          break;
        }
        
        pred_ptr = new pred_tree_elem_num_bool<int1>((int1) tmp_int8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<int1>*)pred_ptr)->check_cond);        
        break;

      case 2:
        if (tmp_int8 < INT2_MIN || tmp_int8 > INT2_MAX)
        {
          pred_ptr = new pred_tree_elem_false();
          print_ln("Warning: A predicate in the query definition will never "
            "evaluate to true (field %d; out of bounds int(2))", field_id);
          break;
        }
        
        pred_ptr = new pred_tree_elem_num_bool<int2>((int2) tmp_int8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<int2>*)pred_ptr)->check_cond);
        break;

      case 4:
        if (tmp_int8 < INT4_MIN || tmp_int8 > INT4_MAX)
        {
          pred_ptr = new pred_tree_elem_false();
          print_ln("Warning: A predicate in the query definition will never "
            "evaluate to true (field %d; out of bounds int(4))", field_id);
          break;
        }

        pred_ptr = new pred_tree_elem_num_bool<int4>((int4) tmp_int8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<int4>*)pred_ptr)->check_cond);
        break;

      case 8:
        pred_ptr = new pred_tree_elem_num_bool<int8>((int8) tmp_int8);
        assign_op_funtion(op, ((pred_tree_elem_num_bool<int8>*)pred_ptr)->check_cond);
        break;
      }
      break;

    case double_:
      pred_ptr = new pred_tree_elem_num_bool<double>(strtod(str0, &str1));
      assign_op_funtion(op, ((pred_tree_elem_num_bool<double>*)pred_ptr)->check_cond);
      str0 = str1;
      break;

    case varchar_:
      str_eat_ws_tok_qd(str0, '\"');
      str1 = tmp_str;
      cnt_str_len = str_read_after_quote_adv(str0, str1);
      *str1 = '\0';

      str1 = malloc1<char>(cnt_str_len + 1, "todo");
      strcpy(str1, tmp_str);

      pred_ptr = new pred_tree_elem_varchar(str1);
      assign_op_funtion(op, ((pred_tree_elem_varchar*)pred_ptr)->check_cond);
      break;

    case fixchar_:
      str_eat_ws_tok_qd(str0, '\"');
      str1 = tmp_str;
      cnt_str_len = str_read_after_quote_adv(str0, str1);

      if (cnt_str_len > GLOBALS::rec_fields[field_id].field_length)
      {
        pred_ptr = new pred_tree_elem_false();
        print_ln("Warning: A predicate in the query definition will never "
          "evaluate to true (field %d; string to long on fixed char field)", field_id);
        break;
      }
      
      *str1 = '\0';
      str1 = malloc1<char>(cnt_str_len + 1, "todo");
      strcpy(str1, tmp_str);

      pred_ptr = new pred_tree_elem_fixchar(str1, GLOBALS::rec_fields[field_id].field_length);
      
      assign_op_funtion(op, ((pred_tree_elem_fixchar*)pred_ptr)->check_cond);
      break;
    }
    pred_ptr->field_id = field_id;
    pred_ptr->h_fix_fld = GLOBALS::h_fix_off[field_id];
    pred_ptr->h_var_fld = GLOBALS::h_var_fld[field_id];
      
    break;
  }
  str_eat_ws(str0);

  free(tmp_str);

  return pred_ptr;
}